#include "chess2.h"
#include "smallchesslib.h"
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "libpq/pqformat.h"

PG_FUNCTION_INFO_V1(gin_extract_board_states);
Datum 
gin_extract_board_states(PG_FUNCTION_ARGS)
{
	chessgame	*game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
    char		**boards = game->boards;
	PG_FREE_IF_COPY(game, 0);
    
    int num_boards = 0;
    while (boards[num_boards] != NULL){
        num_boards++;
    }
    // Constructing a text array
    ArrayType *result = construct_array(
        (Datum *) boards,          
        num_boards,                
        TEXTOID,            // element type       
        sizeof(char *),                
        true,               // contains nulls
        'c'                 // alignment ('i' = int)       
    );
    PG_RETURN_ARRAYTYPE_P(result);
}