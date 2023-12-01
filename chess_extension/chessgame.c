/* /* #ifndef CHESS_H
#define CHESS_H */
/* #include "smallchesslib.h"
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "libpq/pqformat.h"
 */
//PG_MODULE_MAGIC;
/* static char **generateboards(char *moves)
{
	SCL_Record	r;
	SCL_Board	board;
	SCL_recordInit(r);
    SCL_recordFromPGN(r, moves);
	int nb_move = SCL_recordLength(r);
	printf("nb moves = %i", nb_move);
	char fenString[SCL_FEN_MAX_LENGTH];
	char **allboards = palloc(sizeof(char *) * (nb_move + 1));

	for(int i = 1; i <= nb_move; i++)
	{
		SCL_recordApply(r, board, i);
		SCL_boardToFEN(board, fenString);
		allboards[i] = pstrdup(strtok(fenString, " "));
		i++;
	}
	allboards[nb_move + 1] = NULL; //see if needed
	return (allboards);
} */
/* 
static	chessgame *chessgame_make(char	*SAN_moves)
{
	chessgame	*game = palloc(sizeof(chessgame));
	game->length = strlen(SAN_moves) + 1;
	game->moves = pstrdup(SAN_moves) + 1;
	return(game);
}

static chessgame	*chessgame_parse(char *str)
{
	//check if SAN
	return (chessgame_make(str));
}

PG_FUNCTION_INFO_V1(chessgame_constructor);
Datum chessgame_constructor(PG_FUNCTION_ARGS)
{
	char		*moves = PG_GETARG_CSTRING(0);
	PG_RETURN_CHESSGAME_P(chessgame_parse(moves));
}

PG_FUNCTION_INFO_V1(chessgame_in);
Datum chessgame_in(PG_FUNCTION_ARGS)
{
	char		*str = PG_GETARG_CSTRING(0);
	PG_RETURN_CHESSGAME_P(chessgame_parse(str));
}

PG_FUNCTION_INFO_V1(chessgame_out);
Datum chessgame_out(PG_FUNCTION_ARGS)
{
	chessgame	*game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
	char		*str = psprintf("%s", game->moves);
	PG_FREE_IF_COPY(game, 0);
	PG_RETURN_CSTRING(str);
}

PG_FUNCTION_INFO_V1(chessgame_recv);
Datum chessgame_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	const char *moves = pq_getmsgstring(buf);
	PG_RETURN_CHESSGAME_P(chessgame_parse(moves));
}

PG_FUNCTION_INFO_V1(chessgame_send);
Datum chessgame_send(PG_FUNCTION_ARGS)
{
	chessgame	*game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	pq_sendstring(&buf, game->moves);
	PG_FREE_IF_COPY(game, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}
 */
/* PG_FUNCTION_INFO_V1(chessgame_cast_from_text);
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
  PG_RETURN_CSTRING(cg->moves);
} */
/* #endif */
//#ifndef CHESS_H
//# define CHESS_H
# include "chess.h"
//#endif
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
	int nchars = strlen(game->moves);
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