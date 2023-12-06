#include "chess.h"
#include "smallchesslib.h"

static chessboard *
chessboard_make(char* FEN_board){
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
chessboard_parse(char* FEN_board){
	SCL_Board	sclboard;

	if (SCL_boardFromFEN(sclboard, FEN_board) == 0)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The string should be in FENformat")));
	return(chessboard_make(FEN_board));
	
}

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

PG_FUNCTION_INFO_V1(chessboard_out);
Datum chessboard_out(PG_FUNCTION_ARGS){
	chessboard *c = (chessboard *)PG_GETARG_CHESSBOARD_P(0);
	char *result = psprintf("%s", c->board);
	PG_FREE_IF_COPY(c, 0);
	PG_RETURN_CSTRING(c->board);
}

PG_FUNCTION_INFO_V1(chessboard_recv);
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
}

//*************************************FUNCTIONS*************************************
PG_FUNCTION_INFO_V1(getBoard);
Datum
getBoard(PG_FUNCTION_ARGS) {
	chessgame *chgame = (chessgame *)PG_GETARG_CHESSGAME_P(0);
	int halfMovesNbr = PG_GETARG_INT32(1);

	SCL_Record	r;
	SCL_Board	board;
	SCL_recordInit(r);
    SCL_recordFromPGN(r, chgame->moves); 

	int nb_move = countMoves(chgame->moves);

	if (halfMovesNbr < 0){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}
	if (halfMovesNbr > SCL_recordLength(r)){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}

	SCL_recordApply(r, board, halfMovesNbr);
	char fenstring[SCL_FEN_MAX_LENGTH];
	SCL_boardToFEN(board, fenstring);
	chessboard* result = chessboard_make(fenstring);
	PG_FREE_IF_COPY(chgame, 0);
	PG_RETURN_CHESSBOARD_P(result);
}

PG_FUNCTION_INFO_V1(getFirstMoves);
Datum
getFirstMoves(PG_FUNCTION_ARGS){
	chessgame *chgame = (chessgame *)PG_GETARG_CHESSGAME_P(0);
	int halfMovesNbr = PG_GETARG_INT32(1);
	int nb_move = countMoves(chgame->moves);
	
   	if (halfMovesNbr < 0){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}
	if (halfMovesNbr > nb_move){
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}
    
	char *firstMoves = cutFirstMoves(chgame->moves, halfMovesNbr);
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
	int nb_move1 = countMoves(game->moves);
	int nb_move2 = countMoves(game2->moves);

	if (nb_move1 < nb_move2 ){
		PG_FREE_IF_COPY(game, 0);
		PG_FREE_IF_COPY(game2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The second game (opening) should be shorter than the first game (full game))")));
	}
 
    char* game1FirstMoves = cutFirstMoves(game->moves, nb_move2);
	
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

PG_FUNCTION_INFO_V1(hasBoard);
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
		elog(INFO, "fenString: %s", fenString);
		elog(INFO, "cb->board: %s", cb->board);
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
}