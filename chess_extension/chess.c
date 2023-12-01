#include "chess.h"
PG_MODULE_MAGIC;

static	chessgame* chessgame_make(const char *SAN_moves)
{
	chessgame	*game = (chessgame *) palloc(VARHDRSZ + strlen(SAN_moves) + 1);
	if (game != NULL) {
		if (SAN_moves != NULL) {
			SET_VARSIZE(game, VARHDRSZ + strlen(SAN_moves) + 1);
			memcpy(game->moves, SAN_moves, strlen(SAN_moves) + 1);
		} else {
			//game->moves = NULL;
		}
	} else {
		ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("out of memory")));
	}
	return(game);
}

static chessgame* chessgame_parse(const char *SAN_moves)
{
	//check if SAN
	return (chessgame_make(SAN_moves));
}

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
	//int len = VARSIZE_ANY_EXHDR(game);
	if (game->moves != NULL) {
		//PG_FREE_IF_COPY(game, 0);
		PG_RETURN_CSTRING(game->moves);
    } else {
        PG_RETURN_NULL();
    }
}

PG_FUNCTION_INFO_V1(chessgame_recv);
Datum 
chessgame_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	int	nbytes;
	const int length = pq_getmsgint64(buf);
	const char *recv_char = pq_getmsgtext(buf, length, &nbytes);
	PG_RETURN_CHESSGAME_P(chessgame_parse(recv_char));
}

PG_FUNCTION_INFO_V1(chessgame_send);
Datum 
chessgame_send(PG_FUNCTION_ARGS)
{
	chessgame	*game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	size_t nchars = strlen(game->moves);
	pq_sendint64(&buf, nchars);
	pq_sendtext(&buf, game->moves, nchars);
	PG_FREE_IF_COPY(game, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(chessgame_constructor);
Datum 
chessgame_constructor(PG_FUNCTION_ARGS)
{
	text *input = PG_GETARG_TEXT_P(0);
	if (input != NULL) {
		const char *moves = text_to_cstring(input);
		PG_RETURN_CHESSGAME_P(moves);
	} else {
		PG_RETURN_NULL();
	}
}

static chessboard *
chessboard_make(char* FEN_board){
	//if (FEN_board == NULL)
		//throw exception
	chessboard *cb = palloc0(sizeof(chessboard) + strlen(FEN_board));
	if (cb == NULL)
		ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("out of memory")));
	SET_VARSIZE(cb, VARHDRSZ + strlen(FEN_board));
	memcpy(cb->board, FEN_board, strlen(FEN_board));
	return cb;
}

static chessboard *
chessboard_parse(char* FEN_board){
	SCL_Board	sclboard;

	//if (SCL_boardFromFEN(sclboard, FEN_board) == 0)
		//ereport(ERROR, (errcode(, errmsg("not a fen representation"))));
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
	chessboard *c = (chessboard *)PG_GETARG_CHESSBOARD_P(0);
	char *result = psprintf("%s", c->board);
	PG_FREE_IF_COPY(c, 0);
	PG_RETURN_CSTRING(c->board);
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

