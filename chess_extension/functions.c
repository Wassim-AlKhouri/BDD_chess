#include "chess.h"
#include "smallchesslib.h"
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"

 chessboard getBoard(chessgame chgame, int halfMovesNbr) {
    SCL_Record r;
    SCL_Board board;
    SCL_recordInit(r);
    SCL_recordFromPGN(r, chgame.moves);

    if (halfMovesNbr < 0){
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}
    if (halfMovesNbr > SCL_recordLength(r)){
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-moves count")));}

    SCL_recordApply(r, board, halfMovesNbr);
    
    //char fenstring[SCL_FEN_MAX_LENGTH];
    //SCL_boardToFEN(board, fenstring);
    chessboard result ;
    memcpy(result.board, board, strlen(board));
    //result.board = board;

    /* result.board = strtok(fenString, " ");
    result.color = fenstring[sttrlen(result->board) + 1];
    result.castling = strtok(NULL, " ");
    result.enpassant = strtok(NULL, " ");
    result.halfMove = strtok(NULL, " ");
    result.fullMove = strtok(NULL, " "); */


    return result; 



}

chessgame getFirstMoves(chessgame chgame, int halfMovesNbr){
    /*
    char* buf = malloc(sizeof(char) * strlen(chgame.moves));
    strcpy(buf, chgame.moves);
    char    *ptr = chgame.moves;
    for (int i = 0; i < halfMovesNbr / 2 + 1; i++)
    {
        ptr = strchr(ptr, '.') - 1; // 1 . e
        
        //buf = strtok(chessgame.moves, ".");
    }
    
    len = ptr - chgame.moves;
    */

    SCL_Record r;
    SCL_Board board;
    SCL_recordInit(r);
    SCL_recordFromPGN(r, chgame.moves);
    
    chessgame result; 
    int i = 0;
    int j = 0;
    char fenString[SCL_FEN_MAX_LENGTH];
	//char **allboards = palloc(sizeof(char *) * (halfMovesNbr + 2));
    //char *token = strtok(chgame->moves, " ");
    // garder les n premiers, à faire
    /*while (token != NULL && i < halfMovesNbr + 1) {
        i+= 1 + i%2;
        result->moves = pstrdup(token);
        token = strtok(NULL, " ");
    }

    while (j<halfMovesNbr)
    {
        SCL_recordApply(r, board, j);
        SCL_boardToFEN(board, fenstring);
        allboards[j] = strtok(fenString, " ");
        j++;
    }
    result.boards = allboards;
    
    result->moves = SCL_printPGN(r,);*/

    return result;
}

bool hasOpening(chessgame comparator, chessgame game){
 
    SCL_Record r1;
    SCL_recordInit(r1);
    SCL_recordFromPGN(r1, comparator.moves);
 
    SCL_Record r2;
    SCL_recordInit(r2);
    SCL_recordFromPGN(r2, game.moves);
 
    int nbMoves_game = SCL_recordLength(r2);
    
    chessgame comparator_cut = getFirstMoves(comparator, nbMoves_game);
 
    if (strcmp(comparator_cut.moves, game.moves) == 0){
        return true;
    }
    return false;
 
    
    
}

/*bool hasBoard(chessgame game, chessboard board, int halfMovesNbr) {
    chessgame *firstMoves = getFirstMoves(game, halfMovesNbr);
    for (int i = 0; i < halfMovesNbr; i++) {
        if (strcmp(firstMoves->boards[i], board) == 0)
            return true;
    }
    return false;
}*/

