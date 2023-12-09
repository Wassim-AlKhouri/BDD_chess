#include "chess.h"
#include "smallchesslib.h"
PG_MODULE_MAGIC;

//*************************************CHESSGAME*************************************

//************INTERNAL FUNCTIONS************




char SAN_str [4096];

static	chessgame* chessgame_make(const char *SAN_moves)
{
	int	i = 0;
	chessgame	*game;
	/*while (SAN_moves == ' ')
		SAN_moves++;
	while (strlen(SAN_moves) > 0 && strlen(SAN_moves) - 1 == ' ')
		SAN_moves[strlen(SAN_moves) - 1] = 0;*/
	//nb_extraspaces = get_number_extraspaces(SAN_moves);
	game = (chessgame *) palloc0(VARHDRSZ + strlen(SAN_moves) + 1);
	if (game != NULL) {
		if (SAN_moves != NULL && strlen(SAN_moves) > 0) {
			SET_VARSIZE(game, VARHDRSZ + strlen(SAN_moves) + 1);
			memcpy(VARDATA(game), SAN_moves, VARSIZE_ANY_EXHDR(game));
		} else {
			memset(SAN_str, '\0', 4096);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("can't create a chessgame from NULL")));
		}
	} else {
		memset(SAN_str, '\0', 4096);
		ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("out of memory")));
	}
	memset(SAN_str, '\0', 4096);
	return(game);
}

static void putCharStr(char c)
{
  char *s = SAN_str;
  while (*s != 0){
	if (s - SAN_str >= 4095){
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("string too long")));
	}
    s++;
  }
  *s = c;
  *(s + 1) = '\0';
}

static chessgame* chessgame_parse(const char *SAN_moves)
{
	//check if the string is in SAN format
	SCL_Record r;
	SCL_recordInit(r);
	SCL_recordFromPGN(r, SAN_moves);
	memset(SAN_str, '\0', 4096);

	SCL_printPGN(r, putCharStr, 0);
	//elog(INFO, "SAN_str: %s", SAN_str);
	if (SAN_str[strlen(SAN_str) - 1] == '*' || SAN_str[strlen(SAN_str) - 1] == '#')
		SAN_str[strlen(SAN_str) - 1] = '\0';
	if (!isValidSan(SAN_str))
	 {
		memset(SAN_str, '\0', 4096);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The string should be in SAN format")));
	 }
	return (chessgame_make(SAN_str));
}

static char* cutFirstMoves(char* moves, int halfMovesNbr){
	int	length = 0;
	int	movesCounter = 0;
	char *result;
	//char *moves  =chgame->moves;
	while (moves[length] && movesCounter < halfMovesNbr)
	{
		while (moves[length] == ' ')
			length++;
		if(!(moves[length] <'9' && moves[length] > '0'))
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
	//elog(INFO, "BEFORE IN");
	//elog(INFO, "%s", input);
	PG_RETURN_CHESSGAME_P(chessgame_parse(input));
}

PG_FUNCTION_INFO_V1(chessgame_out);
Datum 
chessgame_out(PG_FUNCTION_ARGS)
{
	chessgame	*game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
	char	*str;
	//elog(INFO, "BEFORE OUT");
	//elog(INFO, "%s", (char *) VARDATA_ANY(game));
	

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
	//elog(INFO, "BEFORE RECV");
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	
	int	nbytes;
	const int length = pq_getmsgint(buf, sizeof(int));
	const char *recv_char = pq_getmsgtext(buf, length, &nbytes);
	//elog(INFO, "AFTER RECV %s", recv_char);
	PG_RETURN_CHESSGAME_P(chessgame_parse(recv_char));
}

PG_FUNCTION_INFO_V1(chessgame_send);
Datum 
chessgame_send(PG_FUNCTION_ARGS)
{
	chessgame	*game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
	StringInfoData buf;
	size_t nchars;
	//elog(INFO, "BEFORE BEFORE SEND");
	//elog(INFO, "%s", (char *) VARDATA_ANY(game));
	initStringInfo(&buf);
	pq_begintypsend(&buf);
	//elog(INFO, "BEFORE SEND %s", (char *) VARDATA_ANY(game));
	nchars = VARSIZE_ANY_EXHDR(game);
	pq_sendint(&buf,nchars, sizeof(int));
	pq_sendtext(&buf,VARDATA_ANY(game), nchars);
	//elog(INFO, "AFTER SEND");
	//pq_sendtext(&buf, game->moves, nchars);
	PG_FREE_IF_COPY(game, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(chessgame_constructor);
Datum 
chessgame_constructor(PG_FUNCTION_ARGS)
{
	//elog(INFO, "BEFORE CONSTRUCTOR");
	text *input = PG_GETARG_TEXT_P(0);
	if (input != NULL) {
		const char *moves = text_to_cstring(input);
		PG_RETURN_CHESSGAME_P(chessgame_parse(moves));
	} else {
		PG_RETURN_NULL();
	}
}
//*************************************CHESSBOARD*************************************

//************INTERNAL FUNCTIONS************

static chessboard *
chessboard_make(const char* FEN_board){
	//if (FEN_board == NULL)
		//throw exception
	chessboard *cb = palloc0(sizeof(chessboard) + strlen(FEN_board) + 1);
	if (cb == NULL)
		ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("out of memory")));
	SET_VARSIZE(cb, VARHDRSZ + strlen(FEN_board) + 1);
	memcpy(cb->board, FEN_board, strlen(FEN_board));
	return cb;
}

static chessboard *
chessboard_parse(const char* FEN_board){
	SCL_Board	sclboard;

	if (SCL_boardFromFEN(sclboard, FEN_board) == 0)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The string should be in FENformat")));
	return(chessboard_make(FEN_board));
	
}
////////////////

/* PG_FUNCTION_INFO_V1(chessboard_in);
Datum chessboard_in(PG_FUNCTION_ARGS) {
    char	*str = PG_GETARG_CSTRING(0);

	//check if format FEN
	chessboard *result = (chessboard *)palloc(sizeof(chessboard));
	result->board = pstrdup(strtok(str, " "));
    result->color = *strtok(NULL, " ");
	result->castling = pstrdup(strtok(NULL, " "));
	result->enpassant = pstrdup(strtok(NULL, " "));
	result->halfmove = atoi(strtok(NULL, " "));
	result->fullmove = atoi(strtok(NULL, " "));
    PG_RETURN_CHESSBOARD_P(result);
} */

//************SQL FUNCTIONS************

PG_FUNCTION_INFO_V1(chessboard_constructor);
Datum
chessboard_constructor(PG_FUNCTION_ARGS)
{
	text *input = PG_GETARG_TEXT_P(0);
	if (input != NULL) {
		char *cb = text_to_cstring(input);
		PG_RETURN_CHESSBOARD_P(chessboard_parse(cb));
	}
	else
		PG_RETURN_NULL();
}

PG_FUNCTION_INFO_V1(chessboard_in);
Datum chessboard_in(PG_FUNCTION_ARGS) {
    char	*str = PG_GETARG_CSTRING(0);
    PG_RETURN_CHESSBOARD_P(chessboard_parse(str));
}

/* PG_FUNCTION_INFO_V1(chessboard_out);
Datum chessboard_out(PG_FUNCTION_ARGS){
	chessboard *c = (chessboard *)PG_GETARG_CHESSBOARD_P(0);
	char *result = psprintf("%s %c %s %s %d %d", c->board, c->color, c->castling, c->enpassant, c->halfmove, c->fullmove);
	PG_RETURN_CSTRING(result);
} */
PG_FUNCTION_INFO_V1(chessboard_out);
Datum chessboard_out(PG_FUNCTION_ARGS){
	chessgame	*board = (chessgame *) PG_GETARG_CHESSGAME_P(0);
	char		*str;
	//elog(INFO, "BEFORE CHESSBOARD_OUT");
	//elog(INFO, "%s", (char *) VARDATA_ANY(board));
	

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

/* PG_FUNCTION_INFO_V1(chessboard_recv);
Datum chessboard_recv(PG_FUNCTION_ARGS){
	StringInfo	buf = (StringInfo) PG_GETARG_CHESSBOARD_P(0);
	chessboard *result = (chessboard *)palloc(sizeof(chessboard));
	result->board = pstrdup(pq_getmsgstring(buf));
	result->color = *pq_getmsgstring(buf);
	result->castling = pstrdup(pq_getmsgstring(buf));
	result->enpassant = pstrdup(pq_getmsgstring(buf));
	result->halfmove = pq_getmsgint(buf, sizeof(int));
	result->fullmove = pq_getmsgint(buf, sizeof(int));
	PG_RETURN_POINTER(result);
} */

/* PG_FUNCTION_INFO_V1(chessboard_recv);
Datum chessboard_recv(PG_FUNCTION_ARGS){
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	int	nbytes;
	const int length = pq_getmsgint64(buf);
	char *recv_char = pq_getmsgtext(buf, length, &nbytes);
	PG_RETURN_CHESSBOARD_P(chessboard_parse(recv_char));
}

PG_FUNCTION_INFO_V1(chessboard_send);
Datum chessboard_send(PG_FUNCTION_ARGS){
	chessboard	*cb = (chessboard *) PG_GETARG_CHESSBOARD_P(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	int nchars = strlen(cb->board);
	pq_sendint64(&buf, nchars);
	pq_sendtext(&buf, cb->board, nchars);
	PG_FREE_IF_COPY(cb, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
} */

PG_FUNCTION_INFO_V1(chessboard_recv);
Datum 
chessboard_recv(PG_FUNCTION_ARGS)
{
	//elog(INFO, "BEFORE RECV");
	int	nbytes;
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	const int length = pq_getmsgint(buf, sizeof(int));
	const char *recv_char = pq_getmsgtext(buf, length, &nbytes);
	//elog(INFO, "AFTER RECV %s", recv_char);
	PG_RETURN_CHESSGAME_P(chessboard_parse(recv_char));
}

PG_FUNCTION_INFO_V1(chessboard_send);
Datum 
chessboard_send(PG_FUNCTION_ARGS)
{
	chessboard	*board = (chessboard *) PG_GETARG_CHESSBOARD_P(0);
	StringInfoData buf;
	size_t nchars = VARSIZE_ANY_EXHDR(board);
	//elog(INFO, "BEFORE BEFORE SEND");
	//elog(INFO, "%s", (char *) VARDATA_ANY(board));
	
	initStringInfo(&buf);
	pq_begintypsend(&buf);
	//elog(INFO, "BEFORE SEND %s", (char *) VARDATA_ANY(board));
	
	pq_sendint(&buf,nchars, sizeof(int));
	pq_sendtext(&buf,VARDATA_ANY(board), nchars);
	//elog(INFO, "AFTER SEND");
	//pq_sendtext(&buf, game->moves, nchars);
	PG_FREE_IF_COPY(board, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}
/* PG_FUNCTION_INFO_V1(chessboard_cast_from_text);
Datum chessboard_cast_from_text(PG_FUNCTION_ARGS)
{  	
	text *txt = PG_GETARG_TEXT_P(0);
	char *str = DatumGetCString(DirectFunctionCall1(textout,PointerGetDatum(txt)));

	
    PG_RETURN_POINTER(chessboard_parse(str)); 

} */

/* PG_FUNCTION_INFO_V1(chessboard_cast_to_text);
Datum chessboard_cast_to_text(PG_FUNCTION_ARGS)
{
  chessboard *cb = PG_GETARG_CHESSBOARD_P(0);
  char *result = psprintf("%s %c %s %s %d %d", cb->board, cb->color, cb->castling, cb->enpassant, cb->halfmove, cb->fullmove);
  PG_RETURN_CSTRING(result);
} */

/* PG_FUNCTION_INFO_V1(chessboard_cast_to_text);
Datum chessboard_cast_to_text(PG_FUNCTION_ARGS)
{
  chessboard *cb = PG_GETARG_CHESSBOARD_P(0);
  PG_RETURN_CSTRING(cb->board);
} */

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
    SCL_recordFromPGN(r, chgame->moves); 
	char fenstring[SCL_FEN_MAX_LENGTH];

	//int nb_move = countMoves(chgame->moves);

	if (halfMovesNbr < 0){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}
	if (halfMovesNbr > SCL_recordLength(r)){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}

	SCL_recordApply(r, board, halfMovesNbr);

	
	SCL_boardToFEN(board, fenstring);
	result = chessboard_make(fenstring);
	
	//result.board = board;

	/* result.board = strtok(fenString, " ");
	result.color = fenstring[sttrlen(result->board) + 1];
	result.castling = strtok(NULL, " ");
	result.enpassant = strtok(NULL, " ");
	result.halfMove = strtok(NULL, " ");
	result.fullMove = strtok(NULL, " "); */


	PG_FREE_IF_COPY(chgame, 0);
	PG_RETURN_CHESSBOARD_P(result);
}

PG_FUNCTION_INFO_V1(getFirstMoves);
Datum
getFirstMoves(PG_FUNCTION_ARGS){
	chessgame *chgame = (chessgame *)PG_GETARG_CHESSGAME_P(0);
	int halfMovesNbr = PG_GETARG_INT32(1);
	char *firstMoves;
   /* 	SCL_Record r;
    SCL_recordInit(r);
    SCL_recordFromPGN(r, chgame->moves); */ 
	int nb_move = countMoves(chgame->moves);
	
   	if (halfMovesNbr < 0){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}
	if (halfMovesNbr > nb_move){
		//ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}
		halfMovesNbr = nb_move;}
    
	firstMoves = cutFirstMoves(chgame->moves, halfMovesNbr);
    chessgame* result = chessgame_make(firstMoves);
	free(firstMoves);
	PG_FREE_IF_COPY(chgame, 0);
    PG_RETURN_CHESSGAME_P(result);
}

PG_FUNCTION_INFO_V1(hasOpening);
Datum
hasOpening(PG_FUNCTION_ARGS) {

	chessgame *game = (chessgame *)PG_GETARG_CHESSGAME_P(0);
	chessgame *game2 = (chessgame*)PG_GETARG_CHESSGAME_P(1);
	char* game1FirstMoves;
	int nb_move1 = countMoves(game->moves);
	int nb_move2 = countMoves(game2->moves);

	if (nb_move1 < nb_move2 ){
		PG_FREE_IF_COPY(game, 0);
		PG_FREE_IF_COPY(game2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The second game (opening) should be shorter than the first game (full game))")));
	}
 
    game1FirstMoves = cutFirstMoves(game->moves, nb_move2);
    //chessgame *comparator_cut = chessgame_make(firstMoves);
	
    if (strcmp(game1FirstMoves, game2->moves) == 0){
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

/* PG_FUNCTION_INFO_V1(hasBoard);
Datum
hasBoard(PG_FUNCTION_ARGS) {
	chessgame *game = (chessgame *)PG_GETARG_CHESSGAME_P(0);
	chessboard *cb = (chessboard *)PG_GETARG_CHESSBOARD_P(1);
	uint32_t halfMovesNbr =  PG_GETARG_INT32(2);

	SCL_Record	r;
	SCL_Board	SCLBoard;
	SCL_recordInit(r);
    SCL_recordFromPGN(r, game->moves); 
	
	char* copyBoard = strdup(cb->board);
	copyBoard = strtok(copyBoard, " ");

	if (SCL_recordLength(r) < halfMovesNbr)
		halfMovesNbr = SCL_recordLength(r);	
	char fenString[SCL_FEN_MAX_LENGTH];
	for(int i = 0; i < halfMovesNbr; i++)
	{
		SCL_recordApply(r, SCLBoard, i);
		SCL_boardToFEN(SCLBoard, fenString);
		//elog(INFO, "fenString: %s", fenString);
		//elog(INFO, "cb->board: %s", cb->board);
		if (strcmp(strtok(fenString," "), copyBoard) == 0){
			PG_FREE_IF_COPY(game, 0);
		    PG_FREE_IF_COPY(cb, 1);
			free(copyBoard);
			PG_RETURN_BOOL(true);
		}
	}
	PG_FREE_IF_COPY(game, 0);
	PG_FREE_IF_COPY(cb, 1);
	free(copyBoard);
    PG_RETURN_BOOL(false);
} */