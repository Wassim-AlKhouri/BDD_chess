#include "chess.h"
#include "smallchesslib.h"
PG_MODULE_MAGIC;

//*************************************CHESSGAME*************************************

//************INTERNAL FUNCTIONS************

char SAN_str [4096];

static	chessgame* chessgame_make(char *SAN_moves)
{
	int	i = 0;
	chessgame	*game;

	game = (chessgame *) palloc0(VARHDRSZ + strlen(SAN_moves) + 1);
	if (game != NULL) {
		if (SAN_moves != NULL && strlen(SAN_moves) > 0) {
			SET_VARSIZE(game, VARHDRSZ + strlen(SAN_moves) + 1);
			memcpy(VARDATA(game), SAN_moves, VARSIZE_ANY_EXHDR(game));
			free(SAN_moves);
		} else {
			free(SAN_moves);
			pfree(game);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("can't create a chessgame from NULL")));
		}
	} else {
		ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("out of memory")));
	}
	return(game);
}

static void putCharStr(char c)
{
  char *s = SAN_str;
  while (*s != 0){
	if (s - SAN_str >= 4095){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),errmsg("string too long")));
	}
    s++;
  }
  *s = c;
  *(s + 1) = '\0';
}


static chessgame* chessgame_parse(const char *SAN_moves)
{
	//check if the string is in SAN format
	/* SCL_Record r;
	SCL_recordInit(r);
	SCL_recordFromPGN(r, SAN_moves);
	memset(SAN_str, '\0', 4096);
	SCL_printPGN(r, putCharStr, 0); */
	char	*formated_SAN;
	if (!SAN_moves || SAN_moves[0] == '\0')
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The string should not be null")));
	formated_SAN = formate_SAN(SAN_moves);
	if (!isValidSan(formated_SAN)){
		free(formated_SAN);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The string should be in SAN format")));
	 }
	return (chessgame_make(formated_SAN));
}

static char* cutFirstMoves(char* moves, int halfMovesNbr){
	int	length = 0;
	int	movesCounter = 0;
	char *result;
	while (moves[length] && movesCounter < halfMovesNbr)
	{
		while (moves[length] == ' ')
			length++;
		if(!(moves[length] <= '9' && moves[length] >= '0'))
				movesCounter++;
		while (moves[length] && moves[length] != ' ')
			length++;

	}
	result = malloc(sizeof(char) * (length + 1));
	strncpy(result, moves, length);
	result[length] = '\0';
	return (result);
}

static int countMoves(char* moves){
	SCL_Record	r;
	SCL_recordInit(r);
    SCL_recordFromPGN(r, moves);
	return (SCL_recordLength(r));
}

//************SQL FUNCTIONS************
PG_FUNCTION_INFO_V1(chessgame_in);
Datum 
chessgame_in(PG_FUNCTION_ARGS)
{
	const char *input = PG_GETARG_CSTRING(0);
	PG_RETURN_CHESSGAME_P(chessgame_parse(input));
}

PG_FUNCTION_INFO_V1(chessgame_out);
Datum 
chessgame_out(PG_FUNCTION_ARGS)
{
	chessgame	*game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
	char	*str;
	if (VARDATA_ANY(game) != NULL) {
		str = palloc0(sizeof(char) * VARSIZE_ANY_EXHDR(game));
		memcpy(str, VARDATA_ANY(game), VARSIZE_ANY_EXHDR(game));  
		PG_FREE_IF_COPY(game, 0);
		PG_RETURN_CSTRING(str);
    } else {
		PG_FREE_IF_COPY(game, 0);
        PG_RETURN_NULL();
    }
}

PG_FUNCTION_INFO_V1(chessgame_recv);
Datum 
chessgame_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	int	nbytes;
	const int length = pq_getmsgint(buf, sizeof(int));
	const char *recv_char = pq_getmsgtext(buf, length, &nbytes);
	PG_RETURN_CHESSGAME_P(chessgame_parse(recv_char));
}

PG_FUNCTION_INFO_V1(chessgame_send);
Datum 
chessgame_send(PG_FUNCTION_ARGS)
{
	chessgame	*game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
	StringInfoData buf;
	size_t nchars;
	initStringInfo(&buf);
	pq_begintypsend(&buf);
	nchars = VARSIZE_ANY_EXHDR(game);
	pq_sendint(&buf,nchars, sizeof(int));
	pq_sendtext(&buf,VARDATA_ANY(game), nchars);
	PG_FREE_IF_COPY(game, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(chessgame_cast_from_text);
Datum chessgame_cast_from_text(PG_FUNCTION_ARGS)
{  	
	text *txt = PG_GETARG_TEXT_P(0);
	char *str = DatumGetCString(DirectFunctionCall1(textout,PointerGetDatum(txt)));
    PG_RETURN_POINTER(chessgame_parse(str)); 
}

PG_FUNCTION_INFO_V1(chessgame_cast_to_text);
Datum chessgame_cast_to_text(PG_FUNCTION_ARGS)
{
  chessgame *cg = PG_GETARG_CHESSGAME_P(0);
  text *result = (text *) palloc0(sizeof(char) * VARSIZE_ANY_EXHDR(cg) + VARHDRSZ);
  memcpy(VARDATA(result), VARDATA_ANY(cg), VARSIZE_ANY_EXHDR(cg));
  PG_RETURN_CSTRING(result);
} 

PG_FUNCTION_INFO_V1(chessgame_constructor);
Datum 
chessgame_constructor(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	PG_RETURN_CHESSGAME_P(chessgame_parse(input));
}
//*************************************CHESSBOARD*************************************

//************INTERNAL FUNCTIONS************
static chessboard *
chessboard_make(const char* FEN_board){
	chessboard *cb = palloc0(VARHDRSZ + strlen(FEN_board) + 1);
	if (cb == NULL)
		ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("out of memory")));
	SET_VARSIZE(cb, VARHDRSZ + strlen(FEN_board) + 1);
	memcpy(VARDATA(cb), FEN_board, strlen(FEN_board) + 1);
	return cb;
}

static chessboard *
chessboard_parse(const char* FEN_board){
	SCL_Board	sclboard;
	if (SCL_boardFromFEN(sclboard, FEN_board) == 0)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The string should be in FENformat")));
	return(chessboard_make(FEN_board));
}

//************SQL FUNCTIONS************
PG_FUNCTION_INFO_V1(chessboard_in);
Datum chessboard_in(PG_FUNCTION_ARGS) {
    char	*str = PG_GETARG_CSTRING(0);
    PG_RETURN_CHESSBOARD_P(chessboard_parse(str));
}

PG_FUNCTION_INFO_V1(chessboard_out);
Datum chessboard_out(PG_FUNCTION_ARGS){
	chessgame	*board = (chessgame *) PG_GETARG_CHESSGAME_P(0);
	char		*str;
	if (VARDATA_ANY(board) != NULL) {
		str = palloc0(sizeof(char) * VARSIZE_ANY_EXHDR(board));
		memcpy(str, VARDATA_ANY(board), VARSIZE_ANY_EXHDR(board));  
		PG_FREE_IF_COPY(board, 0);
		PG_RETURN_CSTRING(str);
    } else {
		PG_FREE_IF_COPY(board, 0);
        PG_RETURN_NULL();
    }
}

PG_FUNCTION_INFO_V1(chessboard_recv);
Datum 
chessboard_recv(PG_FUNCTION_ARGS)
{
	int	nbytes;
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	const int length = pq_getmsgint(buf, sizeof(int));
	const char *recv_char = pq_getmsgtext(buf, length, &nbytes);
	PG_RETURN_CHESSGAME_P(chessboard_parse(recv_char));
}

PG_FUNCTION_INFO_V1(chessboard_send);
Datum 
chessboard_send(PG_FUNCTION_ARGS)
{
	chessboard	*board = (chessboard *) PG_GETARG_CHESSBOARD_P(0);
	StringInfoData buf;
	size_t nchars = VARSIZE_ANY_EXHDR(board);
	
	initStringInfo(&buf);
	pq_begintypsend(&buf);
	
	pq_sendint(&buf,nchars, sizeof(int));
	pq_sendtext(&buf,VARDATA_ANY(board), nchars);
	PG_FREE_IF_COPY(board, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(chessboard_cast_from_text);
Datum chessboard_cast_from_text(PG_FUNCTION_ARGS)
{  	
	text *txt = PG_GETARG_TEXT_P(0);
	char *str = DatumGetCString(DirectFunctionCall1(textout,PointerGetDatum(txt)));
    PG_RETURN_POINTER(chessboard_parse(str)); 
}

PG_FUNCTION_INFO_V1(chessboard_cast_to_text);
Datum chessboard_cast_to_text(PG_FUNCTION_ARGS)
{
  chessboard *cb = PG_GETARG_CHESSBOARD_P(0);
  text *result = (text *) palloc0(sizeof(char) * VARSIZE_ANY_EXHDR(cb) + VARHDRSZ);
  memcpy(VARDATA(result), VARDATA_ANY(cb), VARSIZE_ANY_EXHDR(cb));
  PG_RETURN_CSTRING(result);
} 

PG_FUNCTION_INFO_V1(chessboard_constructor);
Datum
chessboard_constructor(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	PG_RETURN_CHESSBOARD_P(chessboard_parse(input));
}
//*************************************FUNCTIONS*************************************
PG_FUNCTION_INFO_V1(getBoard);
Datum
getBoard(PG_FUNCTION_ARGS) {
	//chessgame chgame, int halfMovesNbr
	chessgame *chgame = (chessgame *)PG_GETARG_CHESSGAME_P(0);
	int halfMovesNbr = PG_GETARG_INT32(1);
	chessboard* result;

	SCL_Record	r;
	SCL_Board	board;
	SCL_recordInit(r);
    SCL_recordFromPGN(r, VARDATA_ANY(chgame)); 
	char fenstring[SCL_FEN_MAX_LENGTH];

	if (halfMovesNbr < 0){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}
	else if (halfMovesNbr > SCL_recordLength(r)){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}

	SCL_recordApply(r, board, halfMovesNbr);

	SCL_boardToFEN(board, fenstring);
	result = chessboard_make(fenstring);
	PG_FREE_IF_COPY(chgame, 0);
	PG_RETURN_CHESSBOARD_P(result);
}

PG_FUNCTION_INFO_V1(getFirstMoves);
Datum
getFirstMoves(PG_FUNCTION_ARGS){
	chessgame *chgame = (chessgame *)PG_GETARG_CHESSGAME_P(0);
	int halfMovesNbr = PG_GETARG_INT32(1);
	char *firstMoves;

	int nb_move = countMoves(VARDATA_ANY(chgame));
	
   	if (halfMovesNbr < 0){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}
	else if (halfMovesNbr > nb_move){
		halfMovesNbr = nb_move;
	}
    
	firstMoves = cutFirstMoves(VARDATA_ANY(chgame), halfMovesNbr);
    chessgame* result = chessgame_make(firstMoves);
	//free(firstMoves);
	PG_FREE_IF_COPY(chgame, 0);
    PG_RETURN_CHESSGAME_P(result);
}

PG_FUNCTION_INFO_V1(hasOpening);
Datum
hasOpening(PG_FUNCTION_ARGS) {
	chessgame *game = (chessgame *)PG_GETARG_CHESSGAME_P(0);
	chessgame *game2 = (chessgame*)PG_GETARG_CHESSGAME_P(1);
	char* game1FirstMoves;
	int nb_move1 = countMoves(VARDATA_ANY(game));
	int nb_move2 = countMoves(VARDATA_ANY(game2));

	if (nb_move1 < nb_move2 ){
		PG_FREE_IF_COPY(game, 0);
		PG_FREE_IF_COPY(game2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The second game (opening) should be shorter than the first game (full game))")));
	}
 
    game1FirstMoves = cutFirstMoves(VARDATA_ANY(game), nb_move2);
	
    if (strcmp(game1FirstMoves, VARDATA_ANY(game2)) == 0){
		PG_FREE_IF_COPY(game, 0);
		PG_FREE_IF_COPY(game2, 1);
		free(game1FirstMoves);
        PG_RETURN_BOOL(true);

    }
	PG_FREE_IF_COPY(game, 0);
	PG_FREE_IF_COPY(game2, 1);
	free(game1FirstMoves);
    PG_RETURN_BOOL(false);
}