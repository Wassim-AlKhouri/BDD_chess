#include "chess.h"
#include "smallchesslib.h"
PG_MODULE_MAGIC;

//*************************************CHESSGAME*************************************

//************INTERNAL FUNCTIONS************

/*
* Function used to create a chessgame from a SAN string
* @param SAN_moves the SAN string
* @return a chessgame
*/
static	chessgame* chessgame_make(char *SAN_moves)
{
	chessgame	*game = (chessgame *) palloc0(VARHDRSZ + strlen(SAN_moves) + 1);
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

/*
* Function used to make sure the string is in San format. If true chessgame_make is called
* @param str the string to check
* @return a chessgame if the string is in SAN format, launches an error otherwise
*/
static chessgame* chessgame_parse(const char *SAN_moves)
{
	char	*formated_SAN;
	if (!SAN_moves || SAN_moves[0] == '\0')
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The string should not be null"))); 
	formated_SAN = formate_SAN(SAN_moves);
	if (!formated_SAN || formated_SAN[0] == '\0')
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The string should not be empty")));
	if (!isValidSan(formated_SAN)){
		free(formated_SAN);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The string should be in SAN format")));
	 }
	return (chessgame_make(formated_SAN));
}

/*
* Function used to get the first "halfMovesNbr" moves of a string
* @param str the string to cut
* @param halfMovesNbr the number of moves to get
* @return the first "halfMovesNbr" moves of the string
*/
static char* cutFirstMoves(char* moves, int halfMovesNbr){
	int	length = 0;
	int	movesCounter = 0;
	char *result;
	while (moves[length] && movesCounter < halfMovesNbr)
	{
		while (moves[length] == ' ')
			length++; //skip spaces
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

/*
* Function used to count the number of moves in a string
* @param str the string to count
* @return the number of moves
*/
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
	PG_FREE_IF_COPY(txt,0);
    PG_RETURN_POINTER(chessgame_parse(str)); 
}

PG_FUNCTION_INFO_V1(chessgame_cast_to_text);
Datum chessgame_cast_to_text(PG_FUNCTION_ARGS)
{
  chessgame *cg = PG_GETARG_CHESSGAME_P(0);
  text *result = (text *) palloc0(sizeof(char) * VARSIZE_ANY_EXHDR(cg) + VARHDRSZ);
  memcpy(VARDATA(result), VARDATA_ANY(cg), VARSIZE_ANY_EXHDR(cg));
  PG_FREE_IF_COPY(cg, 0);
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
/*
* Function used to create a chessboard from a FEN string
* @param FEN_board the FEN string that has been allocated on stack (no malloc)
* @return a chessboard
*/
static chessboard *
chessboard_make(const char* FEN_board){
	chessboard *cb = palloc0(VARHDRSZ + strlen(FEN_board) + 1);
	if (cb == NULL)
		ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("out of memory")));
	SET_VARSIZE(cb, VARHDRSZ + strlen(FEN_board) + 1);
	memcpy(VARDATA(cb), FEN_board, strlen(FEN_board) + 1);
	return cb;
}

/*
* Function used to make sure the string is in FEN format. If true chessboard_make is called
* @param str the string to check
* @return a chessboard if the string is in FEN format, launches an error otherwise
*/
static chessboard *
chessboard_parse(const char* FEN_board){
	SCL_Board	sclboard;
	if (!FEN_board || FEN_board[0] == 0 || SCL_boardFromFEN(sclboard, FEN_board) == 0)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The string should be in FEN format")));
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
	PG_FREE_IF_COPY(txt,0);
    PG_RETURN_POINTER(chessboard_parse(str)); 
}

PG_FUNCTION_INFO_V1(chessboard_cast_to_text);
Datum chessboard_cast_to_text(PG_FUNCTION_ARGS)
{
  chessboard *cb = PG_GETARG_CHESSBOARD_P(0);
  text *result = (text *) palloc0(sizeof(char) * VARSIZE_ANY_EXHDR(cb) + VARHDRSZ);
  memcpy(VARDATA(result), VARDATA_ANY(cb), VARSIZE_ANY_EXHDR(cb));
  PG_FREE_IF_COPY(cb, 0);
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
/*
* Function used to get the board (in FEN format) from a chessgame at a given half-moves count
* @param chgame the chessgame
* @param halfMovesNbr the half-moves count
* @return the board at the given half-moves count
*/
PG_FUNCTION_INFO_V1(getBoard);
Datum
getBoard(PG_FUNCTION_ARGS) {
	//chessgame chgame, int halfMovesNbr
	chessgame *chgame = (chessgame *)PG_GETARG_CHESSGAME_P(0);
	int halfMovesNbr = PG_GETARG_INT32(1);
	chessboard* result;
	char fenstring[SCL_FEN_MAX_LENGTH];

	SCL_Record	r;
	SCL_Board	board;
	SCL_recordInit(r);
    SCL_recordFromPGN(r, VARDATA_ANY(chgame)); 

	if (halfMovesNbr < 0){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}
	else if (halfMovesNbr > SCL_recordLength(r)){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Not enough moves in the game")));}

	SCL_recordApply(r, board, halfMovesNbr);

	SCL_boardToFEN(board, fenstring);
	result = chessboard_make(fenstring);
	PG_FREE_IF_COPY(chgame, 0);
	PG_RETURN_CHESSBOARD_P(result);
}

/*
* Function used to get the first "halfMovesNbr" moves of a chessgame
* @param chgame the chessgame
* @param halfMovesNbr the number of moves to get
* @return the first "halfMovesNbr" moves of the chessgame
*/
PG_FUNCTION_INFO_V1(getFirstMoves);
Datum
getFirstMoves(PG_FUNCTION_ARGS){
	chessgame *chgame = (chessgame *)PG_GETARG_CHESSGAME_P(0);
	int halfMovesNbr = PG_GETARG_INT32(1);
	char *firstMoves;
	chessgame* result;
	int nb_move = countMoves(VARDATA_ANY(chgame));
	
   	if (halfMovesNbr < 0){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}
	else if (halfMovesNbr == 0){
		PG_FREE_IF_COPY(chgame, 0);
    	PG_RETURN_CHESSGAME_P(chessgame_make(strdup("1.")));
	}
	else if (halfMovesNbr > nb_move){
		halfMovesNbr = nb_move;
	}
    
	firstMoves = cutFirstMoves(VARDATA_ANY(chgame), halfMovesNbr);
    result = chessgame_make(firstMoves);
	PG_FREE_IF_COPY(chgame, 0);
    PG_RETURN_CHESSGAME_P(result);
}

/*
* Function used to add the highest lexicographic order move to a chessgame (useful for hasOpening see the sql file)
* @param chgame the chessgame
* @return the chessgame with the highest lexicographic order move added (hxh8=R+
*/
PG_FUNCTION_INFO_V1(AddLastMove);
Datum
AddLastMove(PG_FUNCTION_ARGS){
	chessgame *chgame = (chessgame *)PG_GETARG_CHESSGAME_P(0);
	char *modifiedGame = (char *) palloc0(VARSIZE_ANY_EXHDR(chgame) + 8);
	chessgame *result;
	strcpy(modifiedGame, VARDATA_ANY(chgame));
	modifiedGame = strcat(modifiedGame, " hxh8=R+");
	result = chessgame_parse(modifiedGame);
	pfree(modifiedGame);
	PG_FREE_IF_COPY(chgame, 0);
	PG_RETURN_CHESSGAME_P(result);
}