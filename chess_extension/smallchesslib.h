#ifndef SMALLCHESSLIB_H
# define SMALLCHESSLIB_H

/**
  @file smallchesslib.h

  Small and simple single header C99 public domain chess library and engine.

  author:F Miloslav Ciz (drummyfish)
  license: CC0 1.0 (public domain)
           found at https://creativecommons.org/publicdomain/zero/1.0/
           + additional waiver of all IP
  version: 0.8d

  Default notation format for this library is a coordinate one, i.e.
  
    squarefrom squareto [promotedpiece]

  e.g.: e2e4 or A2A1q

  This work's goal is to never be encumbered by any exclusive intellectual
  property rights. The work is therefore provided under CC0 1.0 + additional
  WAIVER OF ALL INTELLECTUAL PROPERTY RIGHTS that waives the rest of
  intellectual property rights not already waived by CC0 1.0. The WAIVER OF ALL
  INTELLECTUAL PROPERTY RGHTS is as follows:

  Each contributor to this work agrees that they waive any exclusive rights,
  including but not limited to copyright, patents, trademark, trade dress,
  industrial design, plant varieties and trade secrets, to any and all ideas,
  concepts, processes, discoveries, improvements and inventions conceived,
  discovered, made, designed, researched or developed by the contributor either
  solely or jointly with others, which relate to this work or result from this
  work. Should any waiver of such right be judged legally invalid or
  ineffective under applicable law, the contributor hereby grants to each
  affected person a royalty-free, non transferable, non sublicensable, non
  exclusive, irrevocable and unconditional license to this right.
*/

#include <stdint.h>

#ifndef SCL_DEBUG_AI
/** AI will print out a Newick-like tree of searched moves. */
  #define SCL_DEBUG_AI 0
#endif

/**
  Maximum number of moves a chess piece can have (a queen in the middle of the
  board).
*/
#define SCL_CHESS_PIECE_MAX_MOVES 25
#define SCL_BOARD_SQUARES 64

typedef uint8_t (*SCL_RandomFunction)(void);

#if SCL_COUNT_EVALUATED_POSITIONS
  uint32_t SCL_positionsEvaluated = 0; /**< If enabled by 
                                            SCL_COUNT_EVALUATED_POSITIONS, this
                                            will increment with every
                                            dynamically evaluated position (e.g.
                                            when AI computes its move). */
#endif

#ifndef SCL_CALL_WDT_RESET
  #define SCL_CALL_WDT_RESET 0 /**< Option that should be enabled on some
                                    Arduinos. If 1, call to watchdog timer
                                    reset will be performed during dynamic
                                    evaluation (without it if AI takes long the
                                    program will reset). */
#endif

/**
  Returns a pseudorandom byte. This function has a period 256 and returns each
  possible byte value exactly once in the period.
*/
uint8_t SCL_randomSimple(void);
void SCL_randomSimpleSeed(uint8_t seed);

/**
  Like SCL_randomSimple, but internally uses a 16 bit value, so the period is
  65536.
*/
uint8_t SCL_randomBetter(void);
void SCL_randomBetterSeed(uint16_t seed);

#ifndef SCL_EVALUATION_FUNCTION
  /**
    If defined, AI will always use the static evaluation function with this
    name. This helps avoid pointers to functions and can be faster but the
    function can't be changed at runtime.
  */
  #define SCL_EVALUATION_FUNCTION
  #undef SCL_EVALUATION_FUNCTION
#endif

#ifndef SCL_960_CASTLING
  /**
    If set, chess 960 (Fisher random) castling will be considered by the library
    rather than normal castling. 960 castling is slightly different (e.g.
    requires the inital rook positions to be stored in board state). The
    castling move is performed as "capturing own rook".
  */
  #define SCL_960_CASTLING 0
#endif

#ifndef SCL_ALPHA_BETA
  /**
    Turns alpha-beta pruning (AI optimization) on or off. This can gain
    performance and should normally be turned on. AI behavior should not
    change at all.
  */
  #define SCL_ALPHA_BETA 1
#endif

/**
  A set of game squares as a bit array, each bit representing one game square.
  Useful for representing e.g. possible moves. To easily iterate over the set
  use provided macros (SCL_SQUARE_SET_ITERATE, ...).
*/
typedef uint8_t SCL_SquareSet[8];

#define SCL_SQUARE_SET_EMPTY {0, 0, 0, 0, 0, 0, 0, 0}

void SCL_squareSetClear(SCL_SquareSet squareSet);
void SCL_squareSetAdd(SCL_SquareSet squareSet, uint8_t square);
uint8_t SCL_squareSetContains(const SCL_SquareSet squareSet, uint8_t square);
uint8_t SCL_squareSetSize(const SCL_SquareSet squareSet);
uint8_t SCL_squareSetEmpty(const SCL_SquareSet squareSet);

/**
  Returns a random square from a square set.
*/
uint8_t SCL_squareSetGetRandom(const SCL_SquareSet squareSet,
  SCL_RandomFunction randFunc);

#define SCL_SQUARE_SET_ITERATE_BEGIN(squareSet) \
  { uint8_t iteratedSquare = 0;\
    uint8_t iterationEnd = 0;\
    for (int8_t _i = 0; _i < 8 && !iterationEnd; ++_i) {\
      uint8_t _row = squareSet[_i];\
      if (_row == 0) { iteratedSquare += 8; continue; }\
      \
      for (uint8_t _j = 0; _j < 8 && !iterationEnd; ++_j) {\
        if (_row & 0x01) {
/*
  Between SCL_SQUARE_SET_ITERATE_BEGIN and _END iteratedSquare variable
  represents the next square contained in the set. To break out of the
  iteration set iterationEnd to 1.
*/

#define SCL_SQUARE_SET_ITERATE_END }\
  _row >>= 1;\
  iteratedSquare++;}\
  } /*for*/ }

#define SCL_SQUARE_SET_ITERATE(squareSet,command)\
  SCL_SQUARE_SET_ITERATE_BEGIN(squareSet)\
  { command }\
  SCL_SQUARE_SET_ITERATE_END
 
#define SCL_BOARD_STATE_SIZE 69

/**
  Represents chess board state as a string in this format:
  - First 64 characters represent the chess board (A1, B1, ... H8), each field
    can be either a piece (PRNBKQprnbkq) or empty ('.'). I.e. the state looks
    like this:

      0 (A1) RNBQKBNR 
             PPPPPPPP
             ........
             ........
             ........
             ........
             pppppppp
             rnbqkbnr 63 (H8)

  - After this more bytes follow to represent global state, these are:
    - 64: bits holding en-passant and castling related information:
      - bits 0-3 (lsb): Column of the pawn that can, in current turn, be
        taken by en-passant (0xF means no pawn can be taken this way).
      - bit 4: Whether white is not prevented from short castling by previous
        king or rook movement.
      - bit 5: Same as 4, but for long castling.
      - bit 6: Same as 4, but for black.
      - bit 7: Same as 4, but for black and long castling.
    - 65: Number saying the number of ply (half-moves) that have already been
      played, also determining whose turn it currently is.
    - 66: Move counter used in the 50 move rule, says the number of ply since
      the last pawn move or capture.
    - 67: Extra byte, left for storing additional info in variants. For normal
      chess this byte should always be 0.
    - 68: The last byte is always 0 to properly terminate the string in case
      someone tries to print it.
  - The state is designed so as to be simple and also print-friendly, i.e. you
    can simply print it with line break after 8 characters to get a human
    readable representation of the board.

  NOTE: there is a much more compact representation which however trades some
  access speed which would affect the AI performance and isn't print friendly,
  so we don't use it. In it each square takes 4 bits, using 15 out of 16
  possible values (empty square and W and B pieces including 2 types of pawns,
  one "en-passant takeable"). Then only one extra byte needed is for castling
  info (4 bits) and ply count (4 bits).
*/
typedef char SCL_Board[SCL_BOARD_STATE_SIZE];

#define SCL_BOARD_ENPASSANT_CASTLE_BYTE 64
#define SCL_BOARD_PLY_BYTE 65
#define SCL_BOARD_MOVE_COUNT_BYTE 66
#define SCL_BOARD_EXTRA_BYTE 67

#if SCL_960_CASTLING
  #define _SCL_EXTRA_BYTE_VALUE (0 | (7 << 3)) // rooks on classic positions
#else
  #define _SCL_EXTRA_BYTE_VALUE 0
#endif

#define SCL_BOARD_START_STATE \
  {82, 78, 66, 81, 75, 66, 78, 82,\
   80, 80, 80, 80, 80, 80, 80, 80,\
   46, 46, 46, 46, 46, 46, 46, 46,\
   46, 46, 46, 46, 46, 46, 46, 46,\
   46, 46, 46, 46, 46, 46, 46, 46,\
   46, 46, 46, 46, 46, 46, 46, 46,\
   112,112,112,112,112,112,112,112,\
   114,110,98, 113,107,98, 110,114,\
   (char) 0xff,0,0,_SCL_EXTRA_BYTE_VALUE,0}

#define SCL_FEN_START \
  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define SCL_FEN_HORDE \
  "ppp2ppp/pppppppp/pppppppp/pppppppp/3pp3/8/PPPPPPPP/RNBQKBNR w KQ - 0 1"

#define SCL_FEN_UPSIDE_DOWN \
  "RNBKQBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbkqbnr w - - 0 1"

#define SCL_FEN_PEASANT_REVOLT \
  "1nn1k1n1/4p3/8/8/8/8/PPPPPPPP/4K3 w - - 0 1"

#define SCL_FEN_ENDGAME \
  "4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1"

#define SCL_FEN_KNIGHTS \
  "N6n/1N4n1/2N2n2/3Nn3/k2nN2K/2n2N2/1n4N1/n6N w - - 0 1"

/**
  Holds an info required to undo a single move.
*/
typedef struct
{
  uint8_t squareFrom;      ///< start square
  uint8_t squareTo;        ///< target square
  char enPassantCastle;    ///< previous en passant/castle byte
  char moveCount;          ///< previous values of the move counter byte
  uint8_t other;           /**< lowest 7 bits: previous value of target square,
                                highest bit: if 1 then the move was promotion or
                                en passant */
} SCL_MoveUndo;

#define SCL_GAME_STATE_PLAYING         0x00
#define SCL_GAME_STATE_WHITE_WIN       0x01
#define SCL_GAME_STATE_BLACK_WIN       0x02
#define SCL_GAME_STATE_DRAW            0x10 ///< further unspecified draw
#define SCL_GAME_STATE_DRAW_STALEMATE  0x11 ///< draw by stalemate
#define SCL_GAME_STATE_DRAW_REPETITION 0x12 ///< draw by repetition
#define SCL_GAME_STATE_DRAW_50         0x13 ///< draw by 50 move rule
#define SCL_GAME_STATE_DRAW_DEAD       0x14 ///< draw by dead position
#define SCL_GAME_STATE_END             0xff ///< end without known result

/**
  Converts square in common notation (e.g. 'c' 8) to square number. Only accepts
  lowercase column.
*/
#define SCL_SQUARE(colChar,rowInt) (((rowInt) - 1) * 8 + ((colChar) - 'a'))
#define SCL_S(c,r) SCL_SQUARE(c,r)

void SCL_boardInit(SCL_Board board);
void SCL_boardCopy(const SCL_Board boardFrom, SCL_Board boardTo);

/**
  Initializes given chess 960 (Fisher random) position. If SCL_960_CASTLING
  is not set, castling will be disabled by this function.
*/
void SCL_boardInit960(SCL_Board board, uint16_t positionNumber);

void SCL_boardDisableCastling(SCL_Board board);

uint32_t SCL_boardHash32(const SCL_Board board);

#define SCL_PHASE_OPENING 0
#define SCL_PHASE_MIDGAME 1
#define SCL_PHASE_ENDGAME 2

/**
  Estimates the game phase: opening, midgame or endgame.
*/
uint8_t SCL_boardEstimatePhase(SCL_Board board);

/**
  Sets the board position. The input string should be 64 characters long zero
  terminated C string representing the board as squares A1, A2, ..., H8 with
  each char being either a piece (RKBKQPrkbkqp) or an empty square ('.').
*/
void SCL_boardSetPosition(SCL_Board board, const char *pieces,
  uint8_t castlingEnPassant, uint8_t moveCount, uint8_t ply);

uint8_t SCL_boardsDiffer(SCL_Board b1, SCL_Board b2);

/**
  Gets a random move on given board for the player whose move it is.
*/
void SCL_boardRandomMove(SCL_Board board, SCL_RandomFunction randFunc,
  uint8_t *squareFrom, uint8_t *squareTo, char *resultProm);

#define SCL_FEN_MAX_LENGTH 90

/**
  Converts a position to FEN (Forsyth–Edwards Notation) string. The string has
  to have at least SCL_FEN_MAX_LENGTH bytes allocated to guarantee the
  function won't write to unallocated memory. The string will be terminated by
  0 (this is included in SCL_FEN_MAX_LENGTH). The number of bytes written
  (including the terminating 0) is returned.
*/
uint8_t SCL_boardToFEN(SCL_Board board, char *string);

/**
  Loads a board from FEN (Forsyth–Edwards Notation) string. Returns 1 on
  success, 0 otherwise. XFEN isn't supported fully but a start position in
  chess960 can be loaded with this function. 
*/
uint8_t SCL_boardFromFEN(SCL_Board board, const char *string);

/**
  Returns an approximate/heuristic board rating as a number, 0 meaning equal
  chances for both players, positive favoring white, negative favoring black.
*/
typedef int16_t (*SCL_StaticEvaluationFunction)(SCL_Board);

/* 
  NOTE: int8_t as a return value was tried for evaluation function, which would
  be simpler, but it fails to capture important non-material position
  differences counted in fractions of pawn values, hence we have to use int16_t.
*/

/**
  Basic static evaluation function. WARNING: this function supposes a standard
  chess game, for non-standard positions it may either not work well or even
  crash the program. You should use a different function for non-standard games.
*/
int16_t SCL_boardEvaluateStatic(SCL_Board board);

/**
  Dynamic evaluation function (search), i.e. unlike SCL_boardEvaluateStatic,
  this one performs a recursive search for deeper positions to get a more
  accurate score. Of course, this is much slower and hugely dependent on
  baseDepth (you mostly want to keep this under 5).
*/
int16_t SCL_boardEvaluateDynamic(SCL_Board board, uint8_t baseDepth,
  uint8_t extensionExtraDepth, SCL_StaticEvaluationFunction evalFunction);

#define SCL_EVALUATION_MAX_SCORE 32600 // don't increase this, we need a margin

/**
  Checks if the board position is dead, i.e. mate is impossible (e.g. due to
  insufficient material), which by the rules results in a draw. WARNING: This
  function may fail to detect some dead positions as this is a non-trivial task.
*/
uint8_t SCL_boardDead(SCL_Board board);

/**
  Tests whether given player is in check.
*/
uint8_t SCL_boardCheck(SCL_Board board, uint8_t white);

/**
  Checks whether given move resets the move counter (used in the 50 move rule).
*/
uint8_t SCL_boardMoveResetsCount(SCL_Board board,
  uint8_t squareFrom, uint8_t squareTo);

uint8_t SCL_boardMate(SCL_Board board);

/**
  Performs a move on a board WITHOUT checking if the move is legal. Returns an
  info with which the move can be undone.
*/
SCL_MoveUndo SCL_boardMakeMove(SCL_Board board, uint8_t squareFrom, uint8_t squareTo,
  char promotePiece);

void SCL_boardUndoMove(SCL_Board board, SCL_MoveUndo moveUndo);

/**
  Checks if the game is over, i.e. the current player to move has no legal
  moves, the game is in dead position etc.
*/
uint8_t SCL_boardGameOver(SCL_Board board);

/**
  Checks if given move is legal.
*/
uint8_t SCL_boardMoveIsLegal(SCL_Board board, uint8_t squareFrom,
  uint8_t squareTo);

/**
  Checks if the player to move has at least one legal move.
*/
uint8_t SCL_boardMovePossible(SCL_Board board);

#define SCL_POSITION_NORMAL    0x00
#define SCL_POSITION_CHECK     0x01
#define SCL_POSITION_MATE      0x02
#define SCL_POSITION_STALEMATE 0x03
#define SCL_POSITION_DEAD      0x04

uint8_t SCL_boardGetPosition(SCL_Board board);

/**
  Returns 1 if the square is attacked by player of given color. This is used to
  examine checks, so for performance reasons the functions only checks whether
  or not the square is attacked (not the number of attackers).
*/
uint8_t SCL_boardSquareAttacked(SCL_Board board, uint8_t square,
  uint8_t byWhite);

/**
  Gets pseudo moves of a piece: all possible moves WITHOUT eliminating moves
  that lead to own check. To get only legal moves use SCL_boardGetMoves.
*/
void SCL_boardGetPseudoMoves(
  SCL_Board board,
  uint8_t pieceSquare,
  uint8_t checkCastling,
  SCL_SquareSet result);

/**
  Gets all legal moves of given piece.
*/
void SCL_boardGetMoves(
  SCL_Board board,
  uint8_t pieceSquare,
  SCL_SquareSet result);

static inline uint8_t SCL_boardWhitesTurn(SCL_Board board);

static inline uint8_t SCL_pieceIsWhite(char piece); 
static inline uint8_t SCL_squareIsWhite(uint8_t square);
char SCL_pieceToColor(uint8_t piece, uint8_t toWhite);

/**
  Converts square coordinates to square number. Each coordinate must be a number
  <1,8>. Validity of the coordinates is NOT checked.
*/
static inline uint8_t SCL_coordsToSquare(uint8_t row, uint8_t column);

#ifndef SCL_VALUE_PAWN
  #define SCL_VALUE_PAWN 256
#endif

#ifndef SCL_VALUE_KNIGHT
  #define SCL_VALUE_KNIGHT 768
#endif

#ifndef SCL_VALUE_BISHOP
  #define SCL_VALUE_BISHOP 800
#endif

#ifndef SCL_VALUE_ROOK
  #define SCL_VALUE_ROOK 1280
#endif

#ifndef SCL_VALUE_QUEEN
  #define SCL_VALUE_QUEEN 2304
#endif

#ifndef SCL_VALUE_KING
  #define SCL_VALUE_KING 0
#endif

#define SCL_ENDGAME_MATERIAL_LIMIT \
  (2 * (SCL_VALUE_PAWN * 4 + SCL_VALUE_QUEEN + \
  SCL_VALUE_KING + SCL_VALUE_ROOK + SCL_VALUE_KNIGHT))

#define SCL_START_MATERIAL \
  (16 * SCL_VALUE_PAWN + 4 * SCL_VALUE_ROOK + 4 * SCL_VALUE_KNIGHT + \
    4 * SCL_VALUE_BISHOP + 2 * SCL_VALUE_QUEEN + 2 * SCL_VALUE_KING)

#ifndef SCL_RECORD_MAX_LENGTH
  #define SCL_RECORD_MAX_LENGTH 256
#endif

#define SCL_RECORD_MAX_SIZE (SCL_RECORD_MAX_LENGTH * 2)

/**
  Records a single chess game. The format is following:

  Each record item consists of 2 bytes which record a single move (ply):

  abxxxxxx cdyyyyyy

    xxxxxx  Start square of the move, counted as A0, A1, ...
    yyyyyy  End square of the move in the same format as the start square.
    ab      00 means this move isn't the last move of the game, other possible
            values are 01: white wins, 10: black wins, 11: draw or end for
            other reasons.
    cd      In case of pawn promotion move this encodes the promoted piece as
            00: queen, 01: rook, 10: bishop, 11: knight (pawn isn't allowed by
            chess rules).

  Every record should be ended by an ending move (ab != 00), empty record should
  have one move where xxxxxx == yyyyyy == 0 and ab == 11.
*/
typedef uint8_t SCL_Record[SCL_RECORD_MAX_SIZE];

#define SCL_RECORD_CONT 0x00
#define SCL_RECORD_W_WIN 0x40
#define SCL_RECORD_B_WIN 0x80
#define SCL_RECORD_END 0xc0

#define SCL_RECORD_PROM_Q 0x00
#define SCL_RECORD_PROM_R 0x40
#define SCL_RECORD_PROM_B 0x80
#define SCL_RECORD_PROM_N 0xc0

#define SCL_RECORD_ITEM(s0,s1,p,e) ((e) | (s0)),((p) | (s1))

void SCL_recordInit(SCL_Record r);

void SCL_recordCopy(SCL_Record recordFrom, SCL_Record recordTo);

/**
  Represents a complete game of chess (or a variant with different staring
  position). This struct along with associated functions allows to easily
  implement a chess game that allows undoing moves, detecting draws, recording
  the moves etc. On platforms with extremely little RAM one can reduce
  SCL_RECORD_MAX_LENGTH to reduce the size of this struct (which will however
  possibly limit how many moves can be undone).
*/
typedef struct
{
  SCL_Board board;
  SCL_Record record;          /**< Holds the game record. This record is here 
                              firstly because games are usually recorded and
                              secondly this allows undoing moves up to the 
                              beginning of the game. This infinite undoing will
                              only work as long as the record is able to hold
                              the whole game; if the record is full, undoing is
                              no longet possible. */
  uint16_t state;
  uint16_t ply;               ///< ply count (board ply counter is only 8 bit)

  uint32_t prevMoves[14];     ///< stores last moves, for repetition detection

  const char *startState;     /**< Optional pointer to the starting board state.
                              If this is null, standard chess start position is
                              assumed. This is needed for undoing moves with
                              game record. */
} SCL_Game;

/**
  Initializes a new chess game. The startState parameter is optional and allows
  for setting up chess variants that differ by starting positions, setting this
  to 0 will assume traditional starting position. WARNING: if startState is
  provided, the pointed to board mustn't be deallocated afterwards, the string
  is not internally copied (for memory saving reasons).
*/
void SCL_gameInit(SCL_Game *game, const SCL_Board startState);

void SCL_gameMakeMove(SCL_Game *game, uint8_t squareFrom, uint8_t squareTo,
  char promoteTo);

uint8_t SCL_gameUndoMove(SCL_Game *game);

/**
  Gets a move which if played now would cause a draw by repetition. Returns 1
  if such move exists, 0 otherwise. The results parameters can be set to 0 in
  which case they will be ignored and only the existence of a draw move will be
  tested.
*/
uint8_t SCL_gameGetRepetiotionMove(SCL_Game *game,
  uint8_t *squareFrom, uint8_t *squareTo);

/**
  Leads a game record from PGN string. The function will probably not strictly
  adhere to the PGN input format, but should accept most sanely written PGN
  strings.
*/
void SCL_recordFromPGN(SCL_Record r, const char *pgn);

uint16_t SCL_recordLength(const SCL_Record r);

/**
  Gets the move out of a game record, returns the end state of the move
  (SCL_RECORD_CONT, SCL_RECORD_END etc.)
*/
uint8_t SCL_recordGetMove(const SCL_Record r,  uint16_t index,
  uint8_t *squareFrom, uint8_t *squareTo, char *promotedPiece);

/**
  Adds another move to the game record. Terminating the record is handled so
  that the last move is always marked with end flag, endState is here to only
  indicate possible game result (otherwise pass SCL_RECORD_CONT). Returns 1 if
  the item was added, otherwise 0 (replay was already of maximum size).
*/
uint8_t SCL_recordAdd(SCL_Record r, uint8_t squareFrom,
  uint8_t squareTo, char promotePiece, uint8_t endState);

/**
  Removes the last move from the record, returns 1 if the replay is non-empty
  after the removal, otherwise 0.
*/
uint8_t SCL_recordRemoveLast(SCL_Record r);

/**
  Applies given number of half-moves (ply) to a given board (the board is
  automatically initialized at the beginning).
*/
void SCL_recordApply(const SCL_Record r, SCL_Board b, uint16_t moves);

int16_t SCL_pieceValue(char piece);
int16_t SCL_pieceValuePositive(char piece);

#define SCL_PRINT_FORMAT_NONE 0
#define SCL_PRINT_FORMAT_NORMAL 1
#define SCL_PRINT_FORMAT_COMPACT 2
#define SCL_PRINT_FORMAT_UTF8 3
#define SCL_PRINT_FORMAT_COMPACT_UTF8 4

/**
  Gets the best move for the currently moving player as computed by AI. The
  return value is the value of the move (with the same semantics as the value
  of an evaluation function). baseDepth is depth in plys to which all moves will
  be checked. If baseDepth 0 is passed, the function makes a random move and
  returns the evaluation of the board. extensionExtraDepth is extra depth for
  checking specific situations like exchanges and checks. endgameExtraDepth is
  extra depth which is added to baseDepth in the endgame. If the randomness
  function is 0, AI will always make the first best move it finds, if it is
  not 0 and randomness is 0, AI will randomly pick between the equally best
  moves, if it is not 0 and randomness is positive, AI will randomly choose
  between best moves with some bias (may not pick the best rated move).
*/
int16_t SCL_getAIMove(
  SCL_Board board,
  uint8_t baseDepth,
  uint8_t extensionExtraDepth,
  uint8_t endgameExtraDepth,
  SCL_StaticEvaluationFunction evalFunc,
  SCL_RandomFunction randFunc,
  uint8_t randomness,
  uint8_t repetitionMoveFrom,
  uint8_t repetitionMoveTo,
  uint8_t *resultFrom,
  uint8_t *resultTo,
  char *resultProm);

/**
  Function that prints out a single character. This is passed to printing
  functions.
*/
typedef void (*SCL_PutCharFunction)(char);

/**
  Prints given chessboard using given format and an abstract printing function.
*/
void SCL_printBoard(
  SCL_Board board,
  SCL_PutCharFunction putCharFunc,
  SCL_SquareSet highlightSquares,
  uint8_t selectSquare,
  uint8_t format,
  uint8_t offset,
  uint8_t labels,
  uint8_t blackDown);

void SCL_printBoardSimple(
  SCL_Board board,
  SCL_PutCharFunction putCharFunc,
  uint8_t selectSquare,
  uint8_t format);

void SCL_printSquareUTF8(uint8_t square, SCL_PutCharFunction putCharFunc);
void SCL_printPGN(SCL_Record r, SCL_PutCharFunction putCharFunc,
  SCL_Board initialState);

/**
  Reads a move from string (the notation format is described at the top of this
  file). The function is safe as long as the string is 0 terminated. Returns 1
  on success or 0 on fail (invalid move string).
*/
uint8_t SCL_stringToMove(const char *moveString, uint8_t *resultFrom, 
  uint8_t *resultTo, char *resultPromotion);

char *SCL_moveToString(SCL_Board board, uint8_t s0, uint8_t s1,
  char promotion, char *string);

/**
  Function used in drawing, it is called to draw the next pixel. The first
  parameter is the pixel color, the second one if the sequential number of the
  pixel.
*/
typedef void (*SCL_PutPixelFunction)(uint8_t, uint16_t);

#define SCL_BOARD_PICTURE_WIDTH 64

/**
  Draws a simple 1bit 64x64 pixels board using a provided abstract function for
  drawing pixels. The function renders from top left to bottom right, i.e. no
  frame buffer is required.
*/
void SCL_drawBoard(
  SCL_Board board,
  SCL_PutPixelFunction putPixel,
  uint8_t selectedSquare,
  SCL_SquareSet highlightSquares,
  uint8_t blackDown);

/**
  Converts square number to string representation (e.g. "d2"). This function
  will modify exactly the first two bytes of the provided string.
*/
static inline char *SCL_squareToString(uint8_t square, char *string);

/**
  Converts a string, such as "A1" or "b4", to square number. The string must
  start with a letter (lower or upper case) and be followed by a number <1,8>.
  Validity of the string is NOT checked.
*/
uint8_t SCL_stringToSquare(const char *square);

//=============================================================================
// privates:



#endif // guard
