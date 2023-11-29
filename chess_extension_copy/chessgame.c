#include "chess.h"
PG_MODULE_MAGIC;

static	chessgame* chessgame_make(const char	*SAN_moves)
{
	elog(LOG, "!!!!!!!!!!!!!!MAKE!!!!: START");
	chessgame	*game = (chessgame *) palloc(VARHDRSZ + strlen(SAN_moves));
	if (game != NULL) {
		if (SAN_moves != NULL) {
			//SET_VARSIZE(game->vl_len, strlen(SAN_moves) + 1);
			SET_VARSIZE(game, VARHDRSZ + strlen(SAN_moves));
			memcpy(game->moves, SAN_moves, strlen(SAN_moves));
			elog(LOG, "!!!!!!!!!!!!!!MAKE--IN!!!!: %s|end", SAN_moves);
			//game->moves = (char *) palloc(sizeof(char) * strlen(SAN_moves) + 1);
			//game -> moves = pstrdup(SAN_moves);
			//strcpy(game->moves, SAN_moves);
			//game->moves = (char *) palloc(sizeof(char) * strlen(SAN_moves) + 1);
			elog (LOG, "!!!!!!!!!!!!!!MAKE--STORED!!!!: %s|end", game->moves);
			//elog (LOG, "!!!!!!!!!!!!!!MAKE--STORED!!!!: %s |end", VARDATA_ANY(game));
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
	elog(LOG, "!!!!!!!!!!!!!IN!!!!: START");
	const char *input = PG_GETARG_CSTRING(0);
	elog(LOG, "!!!!!!!!!!!!!IN!!!!: %s|end", input);
	PG_RETURN_CHESSGAME_P(chessgame_parse(input));
}

PG_FUNCTION_INFO_V1(chessgame_out);
Datum 
chessgame_out(PG_FUNCTION_ARGS)
{
	elog(LOG, "!!!!!!!!!!!!!OUT!!!!: START");
	chessgame	*game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
	//int len = VARSIZE_ANY_EXHDR(game);
	if (game->moves != NULL) {
		elog(LOG, "!!!!!!!!!!!!!OUT!!!!: %s|end", game->moves);
		//PG_FREE_IF_COPY(game, 0);
		PG_RETURN_CSTRING(game->moves);
    } else {
		elog(LOG, "!!!!!!!!!!!!!OUT!!!!: NULL");
        PG_RETURN_NULL();
    }
}

PG_FUNCTION_INFO_V1(chessgame_recv);
Datum 
chessgame_recv(PG_FUNCTION_ARGS)
{
	elog(LOG, "!!!!!!!!!!!!!RECV!!!!: START");
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	int	nbytes;
	const int length = pq_getmsgint64(buf);
	const char *recv_char = pq_getmsgtext(buf, length, &nbytes);
	elog(LOG, "!!!!!!!!!!!!!RECV!!LENG!!: %d |end", length);
	elog(LOG, "!!!!!!!!!!!!!RECV!!!!: %s |end", recv_char);
	PG_RETURN_CHESSGAME_P(chessgame_parse(recv_char));
}

PG_FUNCTION_INFO_V1(chessgame_send);
Datum 
chessgame_send(PG_FUNCTION_ARGS)
{
	elog(LOG, "!!!!!!!!!!!!!SEND!!!!: START");
	chessgame	*game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	int nchars = strlen(game->moves);
	pq_sendint64(&buf, nchars);
	pq_sendtext(&buf, game->moves, nchars);
	PG_FREE_IF_COPY(game, 0);
	elog(LOG, "!!!!!!!!!!!!!SEND!!!!: %s |end", game->moves);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(chessgame_constructor);
Datum 
chessgame_constructor(PG_FUNCTION_ARGS)
{
	elog(LOG, "!!!!!!!!!!!!!CONSTRUCTOR!!!!: START");
	text *input = PG_GETARG_TEXT_P(0);
	if (input != NULL) {
		const char *moves = text_to_cstring(input);
		elog(LOG, "!!!!!!!!!!!!!CONSTRUCTOR!!!!: %s |end", moves);
		PG_RETURN_CHESSGAME_P(chessgame_parse(text_to_cstring(input)));
	} else {
		elog(LOG, "!!!!!!!!!!!!!CONSTRUCTOR!!!!: NULL");
		PG_RETURN_NULL();
	}
}