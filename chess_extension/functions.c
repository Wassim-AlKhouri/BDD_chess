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

bool hasOpening(chessgame comparator, chessgame game){
 
    SCL_Record r1;
    SCL_recordInit(r1);
    SCL_recordFromPGN(r1, comparator->moves);
 
    SCL_Record r2;
    SCL_recordInit(r2);
    SCL_recordFromPGN(r2, game->moves);
 
    if
    int nbMoves1 = SCL_recordLength(r1);
    int nbMoves2 = SCL_recordLength(r2);
 
    chessgame comparator_cut = getFirstMoves(comparator, nbMoves1);
 
    if (strcmp(comparator_cut->moves, game->move) == 0){
        return true;
    }
    else {return false}
 
    pfree(comparator_cut->moves);
    pfree(comparator_cut->boards);
    pfree(comparator_cut);
    
}

bool hasBoard(chessgame game, chessboard board, int halfMovesNbr) {
    chessboard *result = getBoard(game, halfMovesNbr);
    chessgame *firstMoves = getFirstMoves(game, halfMovesNbr);
    for (int i = 0; i < halfMovesNbr; i++) {
        if (strcmp(firstMoves->boards[i], result->board) == 0)
            return true;
    }
    return false;
}

