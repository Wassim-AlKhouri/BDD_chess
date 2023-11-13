#include "chess2.h"
#include "smallchesslib.h"
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"


char **generateboards(char *moves)
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