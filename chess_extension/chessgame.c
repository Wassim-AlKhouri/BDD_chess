#include "chess2.h"
#include "smallchesslib.h"
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"


static char **generateboards(char *moves)
{
	SCL_Record	r;
	SCL_Board	board;
	SCL_recordInit(r);
    SCL_recordFromPGN(r, moves);

	int		i = 0;
	int		nb_move = SCL_recordLength(r);
	char 	fenString[SCL_FEN_MAX_LENGTH];
	char	**allboards = palloc(sizeof(char *) * (nb_moves + 1));

	while(i < nb_move)
	{
		SCL_recordApply(r, board, i);
		SCL_boardToFEN(board, fenstring);
		allboards[i] = pstrdup(strtok(fenString, " "));
		i++;
	}
	allboards[i] = NULL; //see if needed
	return (allboards);
}

PG_FUNCTION_INFO_V1(chessgame_in);
Datum chessgame_in(PG_FUNCTION_ARGS)
{
	char		*str = PG_GETARG_CSTRING(0);
	chessgame	*result = (chessgame *) palloc(sizeof(chessboard));
	
	result->moves = pstrdup(str);
	result->boards = generateboards(result->moves);
	return (result);
}

PG_FUNCTION_INFO_V1(chessgame_out);
Datum chessgame_out(PG_FUNCTION_ARGS)
{
	chessgame	*game = (chessgame *) PG_GETARG_POINTER(0);
	PG_FREE_IF_COPY(game, 0);
	PG_RETURN_CSTRING(game->moves);
}

PG_FUNCTION_INFO_V1(chessgame_recv);
Datum chessgame_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	chessgame	*game = (chessgame *) palloc(sizeof(chessgame));

	game->moves = pq_getmsgstring(buf);

	SCL_Record r;
	SCL_RecordInit(r);
	SCL_recordFromPGN(r, game->moves);
	int nbr_moves = SCL_recordLength(r);

	game->boards = palloc(sizeof(char *) * (nbr_moves + 1));
	for (int i = 0; i < nbr_moves; i++) 
	{
		game->boards[i] = pstrdup(pq_getmsgstring(buf));
	}
	//or we can just generate the boards from the moves
	//game->boards = generateboards(game->moves);
	PG_RETURN_CSTRING(game);
}

PG_FUNCTION_INFO_V1(chessgame_send);
Datum chessgame_send(PG_FUNCTION_ARGS)
{
	chessgame	*game = (chessgame *) PG_GETARG_POINTER(0);
	StringInfoData	buf;
	pq_begintypsend(&buf);
	pq_sendstring(&buf, game->moves);
	int nbr_moves = SCL_recordLength(r);
	for (int i = 0; i < nbr_moves; i++) 
	{
		pq_sendstring(&buf, game->boards[i]);
	}
	PG_FREE_IF_COPY(game, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}


