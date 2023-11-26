#include "chess.h"

/* static chessboard *
chessboard_make(char* board, char color, char* castling, char* enpassant, int halfmove, int fullmove){
	chessboard *cb = palloc0(sizeof(chessboard));
	cb->board = board;
	cb->color = color;
	cb->castling = castling;
	cb->enpassant = enpassant;
	cb->halfmove = halfmove;
	cb->fullmove = fullmove;
	return cb;
} */
static chessboard *
chessboard_make(char* board){
	chessboard *cb = palloc0(sizeof(int) + sizeof(char) * (strlen(board) + 1));
	cb->length = strlen(board);
	cb->board = pstrdup(board);
	return cb;
}

static chessboard *
chessboard_parse(char* board){
	return(chessboard_make(board));
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
  char* board = PG_GETARG_CSTRING(0);
  PG_RETURN_CHESSBOARD_P(chessboard_parse(board));
}

PG_FUNCTION_INFO_V1(chessboard_in);
Datum chessboard_in(PG_FUNCTION_ARGS) {
    char	*str = PG_GETARG_CSTRING(0);

	//check if format FEN
	/* chessboard *result = (chessboard *)palloc(sizeof(int) + sizeof(char) * (strlen(str) + 1));
	result->length = strlen(str);
	result->board = pstrdup(str); */
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
	StringInfo	buf = (StringInfo) PG_GETARG_CHESSBOARD_P(0);
	/* chessboard *result = (chessboard *) palloc(sizeof(int) + sizeof(char) * (strlen(str) + 1));
	result->length = pq_gesmsgint(buf, sizeof(int));
	result->board = pstrdup(pq_getmsgstring(buf)); */
	const char *board = pq_getmsgstring(buf);
	PG_RETURN_CHESSBOARD_P(chessboard_parse(board));
}

/* PG_FUNCTION_INFO_V1(chessboard_send);
Datum chessboard_send(PG_FUNCTION_ARGS){
	chessboard *cb = PG_GETARG_CHESSBOARD_P(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	pq_sendstring(&buf, cb->board);
	pq_sendint64(&buf, cb->color);
	pq_sendstring(&buf, cb->castling);
	pq_sendstring(&buf, cb->enpassant);
	pq_sendint64(&buf, cb->halfmove);
	pq_sendint64(&buf, cb->fullmove);
	PG_FREE_IF_COPY(cb, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
} */

PG_FUNCTION_INFO_V1(chessboard_send);
Datum chessboard_send(PG_FUNCTION_ARGS){
	chessboard *cb = PG_GETARG_CHESSBOARD_P(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	pq_sendint(&buf, cb->length, sizeof(int));
	pq_sendstring(&buf, cb->board);
	PG_FREE_IF_COPY(cb, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}


PG_FUNCTION_INFO_V1(chessboard_cast_from_text);
Datum chessboard_cast_from_text(PG_FUNCTION_ARGS)
{  	
	text *txt = PG_GETARG_TEXT_P(0);
	char *str = DatumGetCString(DirectFunctionCall1(textout,PointerGetDatum(txt)));

	
    PG_RETURN_POINTER(chessboard_parse(str)); 

}

/* PG_FUNCTION_INFO_V1(chessboard_cast_to_text);
Datum chessboard_cast_to_text(PG_FUNCTION_ARGS)
{
  chessboard *cb = PG_GETARG_CHESSBOARD_P(0);
  char *result = psprintf("%s %c %s %s %d %d", cb->board, cb->color, cb->castling, cb->enpassant, cb->halfmove, cb->fullmove);
  PG_RETURN_CSTRING(result);
} */

PG_FUNCTION_INFO_V1(chessboard_cast_to_text);
Datum chessboard_cast_to_text(PG_FUNCTION_ARGS)
{
  chessboard *cb = PG_GETARG_CHESSBOARD_P(0);
  PG_RETURN_CSTRING(cb->board);
}

///////////////

