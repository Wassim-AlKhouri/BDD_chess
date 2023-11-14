#include "chess2.h"
#include "smallchesslib.h"
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"

 chessboard *getBoard(chessgame *chgame, int halfMovesNbr) {
    SCL_Record r;
    SCL_Board board;
    SCL_recordInit(r);
    SCL_recordFromPGN(r, chgame->moves);

    SCL_recordApply(r, board, halfMovesNbr);
    
    char fenstring[SCL_FEN_MAX_LENGTH];
    SCL_boardToFEN(board, fenstring);
    chessboard *result = (chessboard *)palloc(sizeof(chessboard));
    result->board = pstrdup(strtok(fenString, " "));
    result->color = fenstring[sttrlen(result->board) + 1];
    result->castling = pstrdup(strtok(NULL, " "));
    result->enpassant = pstrdup(strtok(NULL, " "));
    result->halfMove = atoi(strtok(NULL, " "));
    result->fullMove = atoi(strtok(NULL, " "));

    return result;
}

chessgame *getFirstMoves(chessgame *chgame, int halfMovesNbr){
    SCL_Record r;
    SCL_Board board;
    SCL_recordInit(r);
    SCL_recordFromPGN(r, chgame->moves);

    chessgame *result = (chessgame *)palloc(sizeof(chessgame));
    int i = 0;
    int j = 0;
    char fenString[SCL_FEN_MAX_LENGTH];
	char **allboards = palloc(sizeof(char *) * (halfMovesNbr + 2));
    char *token = strtok(chgame->moves, " ");

    while (token != NULL && i < halfMovesNbr) {
        result->moves = pstrdup(token);
        i++;
        token = strtok(NULL, " ");
    }

    while (j<halfMovesNbr)
    {
        SCL_recordApply(r, board, j);
        SCL_boardToFEN(board, fenstring);
        allboards[j] = pstrdup(strtok(fenString, " "));
        j++;
    }
    result->boards = allboards;

    return result 
}

