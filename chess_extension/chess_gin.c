#include "chess.h"


// The explenation of the following functions can be found in https://www.postgresql.org/docs/current/gin-extensibility.html


PG_FUNCTION_INFO_V1(gin_compare_chessgame);
Datum 
gin_compare_chessgame(PG_FUNCTION_ARGS)
{
    text *a = (text *) PG_GETARG_TEXT_P(0);
    text *b = (text *) PG_GETARG_TEXT_P(1);
    uint32_t res = strcmp(VARDATA(a), VARDATA(b));
    PG_FREE_IF_COPY(a, 0);
    PG_FREE_IF_COPY(b, 1);
    PG_RETURN_INT32(res);
}

PG_FUNCTION_INFO_V1(gin_extract_value_chessgame);
Datum 
gin_extract_value_chessgame(PG_FUNCTION_ARGS)
{
    chessgame *game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
    uint32_t *nkeys = (uint32_t *) PG_GETARG_POINTER(1);
    text** boards = generateboards(game, nkeys);
    PG_FREE_IF_COPY(game, 0);
    PG_RETURN_POINTER(boards);
}

PG_FUNCTION_INFO_V1(gin_extract_query_chessgame);
Datum 
gin_extract_query_chessgame(PG_FUNCTION_ARGS)
{
    text *query = (text *) PG_GETARG_TEXT_P(0); // chessboard
    uint32_t *nkeys = (uint32_t *) PG_GETARG_UINT32(1);
    uint16_t strategyNumber = PG_GETARG_UINT16(2);
    bool **pmatch = (bool **) PG_GETARG_POINTER(3); // not used (no partial match)
    Pointer **extra_data = (Pointer **) PG_GETARG_POINTER(4); // not used (no extra data)
    bool** nullFlags = (bool **) PG_GETARG_POINTER(5); // not used (no nulls)
    uint32_t* searchMode = (uint32_t *) PG_GETARG_UINT32(6);
    
    // there is only one strategy (@>)
    *searchMode = GIN_SEARCH_MODE_DEFAULT;
    *nkeys = 1;
    text** boards = (text **) palloc(sizeof(text *));
    boards[0] = query;
    PG_RETURN_POINTER(boards);
}

PG_FUNCTION_INFO_V1(gin_consistent_chessgame);
Datum
gin_consistent_chessgame(PG_FUNCTION_ARGS)
{
    bool *check = (bool *) PG_GETARG_POINTER(0);
    uint16_t strategyNumber = PG_GETARG_UINT16(1);
    text *query = (text *) PG_GETARG_TEXT_P(2);
    uint32_t nkeys = PG_GETARG_UINT32(3);
    Pointer *extra_data = (Pointer *) PG_GETARG_POINTER(4); // not used (no extra data)
    bool *recheck = (bool *) PG_GETARG_POINTER(5); // not used (no recheck)
    Datum *queryKeys = (Datum *) PG_GETARG_POINTER(6); // not used
    bool *nullFlags = (bool *) PG_GETARG_POINTER(7); // not used (no nulls)
    for (int i = 0; i < nkeys; i++) {if (check[i]) PG_RETURN_BOOL(true);}
    PG_RETURN_BOOL(false);
}