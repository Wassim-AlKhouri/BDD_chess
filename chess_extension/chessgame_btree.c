/*
 * chessgame_abs.C 
 *
 * Btree functions for the chessgame type
 *
 */

#include <postgres.h>
#include <float.h>
#include <math.h>

#include "fmgr.h"
#include "utils/builtins.h"
#include "libpq/pqformat.h"
#include "string.h"
#include "chess.h"
#include "smallchesslib.h"



static int
chessgame_abs_cmp_internal(chessgame *a, chessgame *b)
{
   //elog(ERROR, "chessgame_abs_cmp_internal: %s", "Not implemented yet");
    int result = strcmp(a->moves, b->moves);
    /* if (result == 0){
        if (nbMoves_game1 < nbMoves_game2){return -1;}
        else if (nbMoves_game1 > nbMoves_game2){return 1;}
        else{return 0;}
    }
    else{return result;} */
    //elog(ERROR, "chessgame_abs_cmp_internal: %s", "implemented yet");
    return result;
}


/*****************************************************************************/

//elog(ERROR, "chessgame_abs_eq: %s", "Not implemented yet");
PG_FUNCTION_INFO_V1(chessgame_abs_eq);
Datum
chessgame_abs_eq(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_abs_cmp_internal(game1, game2) == 0;
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_BOOL(result_cmpr);
}
//elog(ERROR, "chessgame_abs_eq: %s", "implemented yet");

//elog(ERROR, "chessgame_abs_ne: %s", "Not implemented yet");

PG_FUNCTION_INFO_V1(chessgame_abs_ne);
Datum
chessgame_abs_ne(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_abs_cmp_internal(game1, game2) != 0;
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_BOOL(result_cmpr);
}

//elog(ERROR, "chessgame_abs_ne: %s", "implemented yet");

PG_FUNCTION_INFO_V1(chessgame_abs_lt);
Datum
chessgame_abs_lt(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_abs_cmp_internal(game1, game2) < 0;
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_BOOL(result_cmpr);
}

PG_FUNCTION_INFO_V1(chessgame_abs_le);
Datum
chessgame_abs_le(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_abs_cmp_internal(game1, game2) <= 0;
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_BOOL(result_cmpr);
}

PG_FUNCTION_INFO_V1(chessgame_abs_gt);
Datum
chessgame_abs_gt(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_abs_cmp_internal(game1, game2) > 0;
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_BOOL(result_cmpr);
}

PG_FUNCTION_INFO_V1(chessgame_abs_ge);
Datum
chessgame_abs_ge(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_abs_cmp_internal(game1, game2) >= 0;
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_BOOL(result_cmpr);
}

PG_FUNCTION_INFO_V1(chessgame_abs_cmp);
Datum
chessgame_abs_cmp(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_abs_cmp_internal(game1, game2);
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_INT32(result_cmpr);
}

/*****************************************************************************/
