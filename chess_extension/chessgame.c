#include "chess2.h"
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"


// void generateboards(char** boards, char *moves)

PG_FUNCTION_INFO_V1(chessgame_in);
Datum chessgame_in(PG_FUNCTION_ARGS)
{
	char		*str = PG_GETARG_CSTRING(0);
	chessgame	*result = (chessgame *)palloc(sizeof(chessboard));
	
	result->moves = pstrdup(str);
    // parser
	//result->boards = palloc(sizeof(char *) * nb_moves);
	//generateboards(result->boards, result->moves)
    // SCL_Board board = SCL_BOARD_START_STATE;
    // for chaque move get boardstate et le stocker dans une liste
	/*int i = 0;
	uint8_t	size;
	char	*cb;

	char * fenString = malloc(sizeof(char) * SCL_FEN_MAX_LENGTH);
	while(i < nb_move)
	{
		SCL_boardMakeMove(board,SCL_SQUARE('e','4'),SCL_SQUARE('e','5'),'q');
		size = SCL_boardToFEN(SCL_Board board, char *str);
		cb = pstrdup(str);
		
        
		boards[i] = cb;
		i++;
	}
	free(str);
}