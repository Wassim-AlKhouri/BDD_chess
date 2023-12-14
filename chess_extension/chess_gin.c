#include "chess.h"
#include "smallchesslib.h"


// The explenation of the following functions can be found in https://www.postgresql.org/docs/current/gin-extensibility.html

/*
* Function used to generate the boards from a chessgame
* @param game: the chessgame
* @param nb_board: a pointer to store the number of boards
* @return: an array of palloced text* containing the boards
*/
static text **generateboards(chessgame *game, int32_t* nb_board)
{
	SCL_Record	r;
	SCL_Board	board;
    char fenString[SCL_FEN_MAX_LENGTH];
    text **allboards;
    char* truncatedBoard;

	SCL_recordInit(r);
    SCL_recordFromPGN(r, VARDATA_ANY(game));
	*nb_board = (int32_t) SCL_recordLength(r) + 1;
	
	allboards = (text **) palloc(sizeof(text *) * (*nb_board));

	for(int i = 0; i < *nb_board; i++)
	{
		SCL_recordApply(r, board, i);
		SCL_boardToFEN(board, fenString);
        truncatedBoard = strtok(fenString, " ");
        allboards[i] = (text *) palloc(VARHDRSZ + strlen(truncatedBoard) + 1); 
        SET_VARSIZE(allboards[i], VARHDRSZ + strlen(truncatedBoard) + 1);
        memcpy(VARDATA(allboards[i]), truncatedBoard, strlen(truncatedBoard) + 1);
	}
	return (allboards);
} 

/*
* Function used to test if a chessgame contains a chessboard (@> operator)
* @param a: the chessgame
* @param b: the chessboard
* @return: true if a contains b, false otherwise 
*/
PG_FUNCTION_INFO_V1(gin_contains_chessboard);
Datum 
gin_contains_chessboard(PG_FUNCTION_ARGS)
{
    chessgame *a = (chessgame *) PG_GETARG_CHESSGAME_P(0);
    chessboard *b = (chessboard *) PG_GETARG_CHESSBOARD_P(1);

    text** boards;
    bool res;
    int nkeys;

    boards = generateboards(a, &nkeys);
    res = false;

    for (int i = 0; i < nkeys; i++)
    {
        int j = 0;
        while(j < VARSIZE_ANY_EXHDR(boards[i]) && j < VARSIZE_ANY_EXHDR(b)){
            if ( (VARDATA_ANY(boards[i])[j] == '\0' || VARDATA_ANY(boards[i])[j] == ' ')  && (VARDATA_ANY(b)[j] == '\0' || VARDATA_ANY(b)[j]  == ' ')){
                res = true;
                break;
            }else if (VARDATA_ANY(boards[i])[j] == '\0'){
                break;
            }else if (VARDATA_ANY(b)[j] == ' '){
                break;
            }else if (VARDATA_ANY(boards[i])[j] != VARDATA_ANY(b)[j]){
                break;
            }
            j++;
        }
        if (res == true) {break;}
    }

    for (int i = 0; i < nkeys; i++) pfree(boards[i]);
    pfree(boards);

    PG_FREE_IF_COPY(a, 0);
    PG_FREE_IF_COPY(b, 1);
    PG_RETURN_BOOL(res);
}

/*
* Function used to compare two chessboards (keys of the gin index) 
* @param a: the first chessboard
* @param b: the second chessboard
* @return: 0 if a = b, -1 if a < b, 1 if a > b (lexicographic order)
*/
PG_FUNCTION_INFO_V1(gin_compare_chessgame);
Datum 
gin_compare_chessgame(PG_FUNCTION_ARGS)
{
    chessboard *a = (chessboard *) PG_GETARG_CHESSBOARD_P(0);
    chessboard *b = (chessboard *) PG_GETARG_CHESSBOARD_P(1);
    int i = 0;
    int res = 0;
    while(i < VARSIZE_ANY_EXHDR(a) && i < VARSIZE_ANY_EXHDR(b)){
        if ((VARDATA_ANY(a)[i] == '\0' || VARDATA_ANY(a)[i] == ' ') && (VARDATA_ANY(b)[i] == '\0' || VARDATA_ANY(b)[i] == ' ')){
            res = 0;
            break;
        }else if (VARDATA_ANY(a)[i] == ' '){
            res = -1;
            break;
        }else if (VARDATA_ANY(b)[i] == ' '){
            res = 1;
            break;
        }else if (VARDATA_ANY(a)[i] != VARDATA_ANY(b)[i]){
            res = VARDATA_ANY(a)[i] - VARDATA_ANY(b)[i];
            break;
        }
        i++;
    }
    PG_FREE_IF_COPY(a, 0);
    PG_FREE_IF_COPY(b, 1);
    PG_RETURN_INT32(res);
}

/*
* Function used to extract the keys (boards) from a chessgame
* @param game: the chessgame
* @param nkeys: a pointer to store the number of boards
* @return: an array of palloced text* containing the boards
*/
PG_FUNCTION_INFO_V1(gin_extract_value_chessgame);
Datum 
gin_extract_value_chessgame(PG_FUNCTION_ARGS)
{
    chessgame *game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
    int32_t *nkeys = (int32_t *) PG_GETARG_POINTER(1);
    bool** nullFlags = (bool **) PG_GETARG_POINTER(2); // not used (no nulls)
    text** boards = generateboards(game, nkeys);
    PG_FREE_IF_COPY(game, 0);
    PG_RETURN_POINTER(boards);
}

/*
* Function used to extract the keys from the right side of the query. Where the left side if the indexed column.
* @param query: the type depends on the strategy used
* @param nkeys: a pointer to store the number of boards
* @param strategyNumber: the strategy used
* @param pmatch: not used (no partial match)
* @param extra_data: not used (no need to pass info to the consistent function)
* @param nullFlags: not used (no nulls)
* @param searchMode: the search mode used by the gin
* @return: an array of palloced text* containing the boards
*/
PG_FUNCTION_INFO_V1(gin_extract_query_chessgame);
Datum 
gin_extract_query_chessgame(PG_FUNCTION_ARGS)
{
    // there is only one strategy (chessgame @> chessboard)
    chessboard *query = (chessboard *) PG_GETARG_CHESSBOARD_P(0); // chessboard

    int32_t *nkeys = (int32_t *) PG_GETARG_POINTER(1);
    int16_t strategyNumber = PG_GETARG_INT16(2);
    bool **pmatch = (bool **) PG_GETARG_POINTER(3); // not used (no partial match)
    int **extra_data = (int **) PG_GETARG_POINTER(4); 
    bool** nullFlags = (bool **) PG_GETARG_POINTER(5); // not used (no nulls)
    int32_t* searchMode = (int32_t *) PG_GETARG_POINTER(6);
    text** boards;
    
    *searchMode = GIN_SEARCH_MODE_DEFAULT;
    *nkeys = 1;

    boards = (text **) palloc0(sizeof(text *));
    boards[0] = (text *) palloc0(VARHDRSZ + VARSIZE_ANY_EXHDR(query) + 1);
    SET_VARSIZE(boards[0], VARHDRSZ + VARSIZE_ANY_EXHDR(query) + 1);
    memcpy(VARDATA(boards[0]), VARDATA_ANY(query), VARSIZE_ANY_EXHDR(query) + 1);
    PG_RETURN_POINTER(boards);
}

/*
* Function used to test if a chessgame contains a chessboard (@> operator)
* @param check: an array of bools where postgres stored if a board matches the query
* @param strategyNumber: the strategy used
* @param query: the type depends on the strategy used
* @param nkeys: the number of boards
* @param extra_data: not used (no extra data)
* @param recheck: not used (no recheck)
* @param queryKeys: not used
* @param nullFlags: not used (no nulls)
* @return: true if a contains b, false otherwise 
*/
PG_FUNCTION_INFO_V1(gin_consistent_chessgame);
Datum
gin_consistent_chessgame(PG_FUNCTION_ARGS)
{
    bool *check = (bool *) PG_GETARG_POINTER(0);
    int16_t strategyNumber = PG_GETARG_INT16(1);
    text *query = (text *) PG_GETARG_TEXT_P(2);
    int32_t nkeys = PG_GETARG_INT32(3);
    int *extra_data = (int *) PG_GETARG_POINTER(4); // extra_data is the number of halfmoves that the board should be in
    bool *recheck = (bool *) PG_GETARG_POINTER(5); // not used (no recheck)
    Datum *queryKeys = (Datum *) PG_GETARG_POINTER(6); // not used
    bool *nullFlags = (bool *) PG_GETARG_POINTER(7); // not used (no nulls)

    for (int i = 0; i < nkeys; i++){
        if (check[i]) PG_RETURN_BOOL(true);
    }
    PG_RETURN_BOOL(false);
}