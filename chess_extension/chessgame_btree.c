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


/*
* Function used to compare two chessgames
* @param a: the first chessgame
* @param b: the second chessgame
* @return: 0 if a = b, -1 if a < b, 1 if a > b (lexicographic order)
*/
static int
chessgame_cmp_internal(chessgame *a, chessgame *b)
{
    int result = strcmp(a->moves, b->moves);
    return result;
}


/*****************************************************************************/
/* Btree sorting functions for chessgame type */
/*
* 
*/
PG_FUNCTION_INFO_V1(chessgame_eq);
Datum
chessgame_eq(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_cmp_internal(game1, game2) == 0;
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_BOOL(result_cmpr);
}

PG_FUNCTION_INFO_V1(chessgame_ne);
Datum
chessgame_ne(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_cmp_internal(game1, game2) != 0;
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_BOOL(result_cmpr);
}

PG_FUNCTION_INFO_V1(chessgame_lt);
Datum
chessgame_lt(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_cmp_internal(game1, game2) < 0;
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_BOOL(result_cmpr);
}

PG_FUNCTION_INFO_V1(chessgame_le);
Datum
chessgame_le(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_cmp_internal(game1, game2) <= 0;
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_BOOL(result_cmpr);
}

PG_FUNCTION_INFO_V1(chessgame_gt);
Datum
chessgame_gt(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_cmp_internal(game1, game2) > 0;
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_BOOL(result_cmpr);
}

PG_FUNCTION_INFO_V1(chessgame_ge);
Datum
chessgame_ge(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_cmp_internal(game1, game2) >= 0;
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_BOOL(result_cmpr);
}

PG_FUNCTION_INFO_V1(chessgame_cmp);
Datum
chessgame_cmp(PG_FUNCTION_ARGS)
{
  chessgame *game1 = (chessgame *)PG_GETARG_CHESSGAME_P(0);
  chessgame *game2 = (chessgame *)PG_GETARG_CHESSGAME_P(1);
  int result_cmpr =  chessgame_cmp_internal(game1, game2);
  PG_FREE_IF_COPY(game1, 0);
  PG_FREE_IF_COPY(game2, 1);
  PG_RETURN_INT32(result_cmpr);
}