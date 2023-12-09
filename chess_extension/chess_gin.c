#include "chess.h"
#include "smallchesslib.h"


// The explenation of the following functions can be found in https://www.postgresql.org/docs/current/gin-extensibility.html

static text **generateboards(chessgame *game, int32_t* nb_board)
{
	SCL_Record	r;
	SCL_Board	board;
    char fenString[SCL_FEN_MAX_LENGTH];
    text **allboards;
    char* truncatedBoard;

	SCL_recordInit(r);
    SCL_recordFromPGN(r, game->moves);
	*nb_board = (int32_t) SCL_recordLength(r) + 1;
	
	allboards = (text **) palloc(sizeof(text *) * (*nb_board));

	for(int i = 0; i < *nb_board; i++)
	{
		SCL_recordApply(r, board, i);
		SCL_boardToFEN(board, fenString);
        truncatedBoard = strtok(fenString, " ");
        allboards[i] = (text *) palloc(VARHDRSZ + strlen(truncatedBoard) + 1); 
        //allboards[i] = cstring_to_text(truncatedBoard);
        SET_VARSIZE(allboards[i], VARHDRSZ + strlen(truncatedBoard) + 1);
        memcpy(VARDATA(allboards[i]), truncatedBoard, strlen(truncatedBoard) + 1);
	}
	return (allboards);
} 

PG_FUNCTION_INFO_V1(gin_contains_chessboard);
Datum 
gin_contains_chessboard(PG_FUNCTION_ARGS)
{
    chessgame *a = (chessgame *) PG_GETARG_CHESSGAME_P(0);
    chessboard *b = (chessboard *) PG_GETARG_CHESSBOARD_P(1);
    char* truncatedBoard;
    // we need to compare the first part of the string (the board) and not the whole string
    // but strtok modifies the string so we need to work with a copy of it
    char* copyBoard = (char *) malloc(sizeof(char) * VARSIZE_ANY_EXHDR(b));
    text** boards;
    bool res;
    int nkeys;
    
    memcpy(copyBoard, VARDATA(b), VARSIZE_ANY_EXHDR(b));
    truncatedBoard = strtok(copyBoard, " ");
    boards = generateboards(a, &nkeys);
    res = false;

    for (int i = 0; i < nkeys; i++)
    {
        char* copyboard = text_to_cstring(boards[i]);
        char* truncatedBoard2 = strtok(copyboard, " ");
        if (strcmp(truncatedBoard2, truncatedBoard) == 0)
        {
            res = true;
            break;
        }
    }

    free(copyBoard);
    for (int i = 0; i < nkeys; i++) pfree(boards[i]);
    pfree(boards);

    PG_FREE_IF_COPY(a, 0);
    PG_FREE_IF_COPY(b, 1);
/*     ////elog(INFO, "CONTAINS");
    ////elog(INFO, "moves: %s", a->moves);
    ////elog(INFO, "board: %s", b->board);
    ////elog(INFO, "res: %d", res); */
    PG_RETURN_BOOL(res);
}

/* PG_FUNCTION_INFO_V1(hasBoard);
Datum
hasBoard(PG_FUNCTION_ARGS) {
	chessgame *game = (chessgame *)PG_GETARG_CHESSGAME_P(0);
	chessboard *cb = (chessboard *)PG_GETARG_CHESSBOARD_P(1);
	uint32_t halfMovesNbr =  PG_GETARG_INT32(2);

	SCL_Record	r;
	SCL_Board	SCLBoard;
	SCL_recordInit(r);
    SCL_recordFromPGN(r, game->moves); 
	
	char* copyBoard = strdup(cb->board);
	copyBoard = strtok(copyBoard, " ");

	if (SCL_recordLength(r) < halfMovesNbr)
		halfMovesNbr = SCL_recordLength(r);	
	char fenString[SCL_FEN_MAX_LENGTH];
	for(int i = 0; i < halfMovesNbr; i++)
	{
		SCL_recordApply(r, SCLBoard, i);
		SCL_boardToFEN(SCLBoard, fenString);
		////elog(INFO, "fenString: %s", fenString);
		////elog(INFO, "cb->board: %s", cb->board);
		if (strcmp(strtok(fenString," "), copyBoard) == 0){
			PG_FREE_IF_COPY(game, 0);
		    PG_FREE_IF_COPY(cb, 1);
			free(copyBoard);
			PG_RETURN_BOOL(true);
		}
	}
	PG_FREE_IF_COPY(game, 0);
	PG_FREE_IF_COPY(cb, 1);
	free(copyBoard);
    PG_RETURN_BOOL(false);
} */


PG_FUNCTION_INFO_V1(gin_compare_chessgame);
Datum 
gin_compare_chessgame(PG_FUNCTION_ARGS)
{
    chessboard *a = (chessboard *) PG_GETARG_CHESSBOARD_P(0);
    chessboard *b = (chessboard *) PG_GETARG_CHESSBOARD_P(1);
    
    ////elog(INFO, "COMPARE");
    ////elog(INFO, "a->board: %s", a->board);
    ////elog(INFO, "b->board: %s", b->board);

    char* copya = (char *) malloc(sizeof(char) * VARSIZE_ANY_EXHDR(a));
    memcpy(copya, VARDATA(a), VARSIZE_ANY_EXHDR(a));
    char* truncatedBoarda = strtok(copya, " ");

    char* copyb = (char *) malloc(sizeof(char) * VARSIZE_ANY_EXHDR(b));
    memcpy(copyb, VARDATA(b), VARSIZE_ANY_EXHDR(b));
    char* truncatedBoardb = strtok(copyb, " ");

    int32_t res = strcmp(truncatedBoarda, truncatedBoardb);

    free(copya);
    free(copyb);
    PG_FREE_IF_COPY(a, 0);
    PG_FREE_IF_COPY(b, 1);
    PG_RETURN_INT32(res);
}

PG_FUNCTION_INFO_V1(gin_extract_value_chessgame);
Datum 
gin_extract_value_chessgame(PG_FUNCTION_ARGS)
{
    chessgame *game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
    /* ////elog(INFO, "EXTRACT VALUE");
    ////elog(INFO, "game->moves: %s", game->moves); */
    int32_t *nkeys = (int32_t *) PG_GETARG_POINTER(1);
    text** boards = generateboards(game, nkeys); // à définir
    PG_FREE_IF_COPY(game, 0);
    PG_RETURN_POINTER(boards);
}

PG_FUNCTION_INFO_V1(gin_extract_query_chessgame);
Datum 
gin_extract_query_chessgame(PG_FUNCTION_ARGS)
{
    // there is only one strategy (chessgame @> chessboard)
    chessboard *query = (chessboard *) PG_GETARG_CHESSBOARD_P(0); // chessboard
    /* ////elog(INFO, "EXTRACT QUERY");
    //elog(INFO, "query->board: %s", query->board); */

    /* char* copy = (char *) malloc(sizeof(char) * VARSIZE_ANY_EXHDR(query));
    memcpy(copy, VARDATA(query), VARSIZE_ANY_EXHDR(query));
    int i = strlen(copy);
    while (copy[i - 1] == ' ')
        i--;
    while (copy[i - 1] <= '9' && copy[i - 1]>= '1')
        i--;
    int nb_halfmove = atoi(copy[i]);
    ////elog (INFO, "!!!!!!!!!!nb_halfmove: %d", nb_halfmove); */


    int32_t *nkeys = (int32_t *) PG_GETARG_POINTER(1);
    int16_t strategyNumber = PG_GETARG_INT16(2);
    bool **pmatch = (bool **) PG_GETARG_POINTER(3); // not used (no partial match)
    Pointer **extra_data = (Pointer **) PG_GETARG_POINTER(4); // not used (no extra data)
    bool** nullFlags = (bool **) PG_GETARG_POINTER(5); // not used (no nulls)
    int32_t* searchMode = (int32_t *) PG_GETARG_POINTER(6);
    text** boards;
    
    *searchMode = GIN_SEARCH_MODE_DEFAULT;
    *nkeys = 1;
    boards = (text **) palloc(sizeof(text *));
    boards[0] = (text *) palloc(VARHDRSZ + VARSIZE_ANY_EXHDR(query) + 1);
    SET_VARSIZE(boards[0], VARHDRSZ + VARSIZE_ANY_EXHDR(query) + 1);
    memcpy(VARDATA(boards[0]), VARDATA_ANY(query), VARSIZE_ANY_EXHDR(query) + 1);
    PG_RETURN_POINTER(boards);
}

PG_FUNCTION_INFO_V1(gin_consistent_chessgame);
Datum
gin_consistent_chessgame(PG_FUNCTION_ARGS)
{
    bool *check = (bool *) PG_GETARG_POINTER(0);
    int16_t strategyNumber = PG_GETARG_INT16(1);
    text *query = (text *) PG_GETARG_TEXT_P(2);
    int32_t nkeys = PG_GETARG_INT32(3);
    Pointer *extra_data = (Pointer *) PG_GETARG_POINTER(4); // not used (no extra data)
    bool *recheck = (bool *) PG_GETARG_POINTER(5); // not used (no recheck)
    Datum *queryKeys = (Datum *) PG_GETARG_POINTER(6); // not used
    bool *nullFlags = (bool *) PG_GETARG_POINTER(7); // not used (no nulls)
    for (int i = 0; i < nkeys; i++) {
        if (check[i]) PG_RETURN_BOOL(true);
    }
    PG_RETURN_BOOL(false);
}

