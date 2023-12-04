#include "chess.h"
#include "smallchesslib.h"
PG_MODULE_MAGIC;

//*************************************CHESSGAME*************************************

//************INTERNAL FUNCTIONS************
static bool isValidPawnMove(char* move){
	if (move[0] >= 'a' && move[0] <= 'h' && move[1] >= '1' && move[1] <= '8'){
			if (strlen(move) == 4 && move[2] == '=' && 
				(move[3] == 'Q' || move[3] == 'R' || move[3] == 'B' || move[3] == 'N')) {
				
				return true; // Valid promotion
				}
			else if (strlen(move) == 2) return true; // Valid move
	}
	return false; //Invalid move
}

static bool isValidPieceMove(char* move) {
    if ((move[0] == 'K' || move[0] == 'Q' || move[0] == 'R' || move[0] == 'B' || move[0] == 'N') &&
        move[1] >= 'a' && move[1] <= 'h' &&
        move[2] >= '1' && move[2] <= '8') {
        
        if (strlen(move) == 5 && move[3] == 'x' &&
            move[4] >= 'a' && move[4] <= 'h' &&
            move[5] >= '1' && move[5] <= '8') {
            return true;  // Valid capture
        }

        return strlen(move) == 3;  // Valid non-capture
    }

    return false;  // Invalid move
}

static bool isValidCastle(const char* move) {
    if (strlen(move) == 5 && strncmp(move, "O-O-O", 5) == 0) return true;  // Valid kingside castle
    else if (strlen(move) == 3 && strncmp(move, "O-O", 3) == 0) return true;  // Valid queenside castle
    return false;  // Invalid move
}

static bool isValidSANmove(char* move){
	if (move == NULL || strlen(move) < 2) return false;
	else if (isValidPawnMove(move)) return true;
	else if (isValidPieceMove(move)) return true;
	else if (isValidCastle(move)) return true;
	return false;
}

bool isPositiveInteger(const char *str) {
    char *endptr;
    long result = strtol(str, &endptr, 10);
    if (*endptr != '\0') {
        return false;  // Conversion failed, not a valid integer
    }
    return result > 0;
}

static bool isValidSan(char* game){
	//check if the string is not empty
	if (game == NULL || strlen(game) == 0) return false;

	char *copy = (char*) malloc(strlen(game)+1);
	strcpy(copy, game);
	char *token = strtok(copy, " ");

	//check if the string is in SAN format
	while (token != NULL){
		if(strlen(token) < 2){
			elog(ERROR, "Invalid move: %s", token);
			free(copy);
			return false;
		}
		//check if number of move (ie "number.")
		else if (token[strlen(token)-1] == '.'){ 
			token[strlen(token)-1] = '\0';
			if (!isPositiveInteger(token)) {
				elog(ERROR, "Invalid move number: %s", token);
				free(copy);
				return false; 
			}
		}else if (!isValidSANmove(token)) {
			elog(ERROR, "Invalid move: %s", token);
			free(copy);
			return false;
		}

		token = strtok(NULL, " ");
	}
	free(copy);
	return true;
} 

static	chessgame* chessgame_make(const char *SAN_moves)
{
	chessgame	*game = (chessgame *) palloc(VARHDRSZ + strlen(SAN_moves) + 1);
	if (game != NULL) {
		if (SAN_moves != NULL) {
			SET_VARSIZE(game, VARHDRSZ + strlen(SAN_moves) + 1);
			memcpy(game->moves, SAN_moves, strlen(SAN_moves) + 1);
		} else {
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("can't create a chessgame from NULL")));
		}
	} else {
		ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("out of memory")));
	}
	return(game);
}

static chessgame* chessgame_parse(const char *SAN_moves)
{
	//check if the string is in SAN format
	if (!isValidSan(SAN_moves))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("The string should be in SAN format")));
	return (chessgame_make(SAN_moves));
}

static char **generateboards(char *moves, int* nb_move)
{
	SCL_Record	r;
	SCL_Board	board;
	SCL_recordInit(r);
    SCL_recordFromPGN(r, moves);
	*nb_move = SCL_recordLength(r);
	char fenString[SCL_FEN_MAX_LENGTH];
	char **allboards = palloc(sizeof(char *) * (*nb_move));

	for(int i = 1; i < *nb_move; i++)
	{
		SCL_recordApply(r, board, i);
		SCL_boardToFEN(board, fenString);
		allboards[i] = pstrdup(strtok(fenString, " "));
		i++;
	}
	return (allboards);
}

//************SQL FUNCTIONS************

PG_FUNCTION_INFO_V1(chessgame_in);
Datum 
chessgame_in(PG_FUNCTION_ARGS)
{
	const char *input = PG_GETARG_CSTRING(0);
	PG_RETURN_CHESSGAME_P(chessgame_parse(input));
}

PG_FUNCTION_INFO_V1(chessgame_out);
Datum 
chessgame_out(PG_FUNCTION_ARGS)
{
	chessgame	*game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
	//int len = VARSIZE_ANY_EXHDR(game);
	if (game->moves != NULL) {
		//PG_FREE_IF_COPY(game, 0);
		PG_RETURN_CSTRING(game->moves);
    } else {
        PG_RETURN_NULL();
    }
}

PG_FUNCTION_INFO_V1(chessgame_recv);
Datum 
chessgame_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	int	nbytes;
	const int length = pq_getmsgint64(buf);
	const char *recv_char = pq_getmsgtext(buf, length, &nbytes);
	PG_RETURN_CHESSGAME_P(chessgame_parse(recv_char));
}

PG_FUNCTION_INFO_V1(chessgame_send);
Datum 
chessgame_send(PG_FUNCTION_ARGS)
{
	chessgame	*game = (chessgame *) PG_GETARG_CHESSGAME_P(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	size_t nchars = strlen(game->moves);
	pq_sendint64(&buf, nchars);
	pq_sendtext(&buf, game->moves, nchars);
	PG_FREE_IF_COPY(game, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(chessgame_constructor);
Datum 
chessgame_constructor(PG_FUNCTION_ARGS)
{
	text *input = PG_GETARG_TEXT_P(0);
	if (input != NULL) {
		const char *moves = text_to_cstring(input);
		PG_RETURN_CHESSGAME_P(moves);
	} else {
		PG_RETURN_NULL();
	}
}
//*************************************CHESSBOARD*************************************

//************INTERNAL FUNCTIONS************

static chessboard *
chessboard_make(char* FEN_board){
	//if (FEN_board == NULL)
		//throw exception
	chessboard *cb = palloc0(sizeof(chessboard) + strlen(FEN_board));
	if (cb == NULL)
		ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("out of memory")));
	SET_VARSIZE(cb, VARHDRSZ + strlen(FEN_board));
	memcpy(cb->board, FEN_board, strlen(FEN_board));
	return cb;
}

static chessboard *
chessboard_parse(char* FEN_board){
	SCL_Board	sclboard;

	//if (SCL_boardFromFEN(sclboard, FEN_board) == 0)
		//ereport(ERROR, (errcode(, errmsg("not a fen representation"))));
	return(chessboard_make(FEN_board));
	
}
////////////////

/* PG_FUNCTION_INFO_V1(chessboard_in);
Datum chessboard_in(PG_FUNCTION_ARGS) {
    char	*str = PG_GETARG_CSTRING(0);

	//check if format FEN
	chessboard *result = (chessboard *)palloc(sizeof(chessboard));
	result->board = pstrdup(strtok(str, " "));
    result->color = *strtok(NULL, " ");
	result->castling = pstrdup(strtok(NULL, " "));
	result->enpassant = pstrdup(strtok(NULL, " "));
	result->halfmove = atoi(strtok(NULL, " "));
	result->fullmove = atoi(strtok(NULL, " "));
    PG_RETURN_CHESSBOARD_P(result);
} */

//************SQL FUNCTIONS************

PG_FUNCTION_INFO_V1(chessboard_constructor);
Datum
chessboard_constructor(PG_FUNCTION_ARGS)
{
	text *input = PG_GETARG_TEXT_P(0);
	if (input != NULL) {
		char *cb = text_to_cstring(input);
		PG_RETURN_CHESSBOARD_P(chessboard_parse(cb));
	}
	else
		PG_RETURN_NULL();
}

PG_FUNCTION_INFO_V1(chessboard_in);
Datum chessboard_in(PG_FUNCTION_ARGS) {
    char	*str = PG_GETARG_CSTRING(0);
    PG_RETURN_CHESSBOARD_P(chessboard_parse(str));
}

/* PG_FUNCTION_INFO_V1(chessboard_out);
Datum chessboard_out(PG_FUNCTION_ARGS){
	chessboard *c = (chessboard *)PG_GETARG_CHESSBOARD_P(0);
	char *result = psprintf("%s %c %s %s %d %d", c->board, c->color, c->castling, c->enpassant, c->halfmove, c->fullmove);
	PG_RETURN_CSTRING(result);
} */
PG_FUNCTION_INFO_V1(chessboard_out);
Datum chessboard_out(PG_FUNCTION_ARGS){
	chessboard *c = (chessboard *)PG_GETARG_CHESSBOARD_P(0);
	char *result = psprintf("%s", c->board);
	PG_FREE_IF_COPY(c, 0);
	PG_RETURN_CSTRING(c->board);
}

/* PG_FUNCTION_INFO_V1(chessboard_recv);
Datum chessboard_recv(PG_FUNCTION_ARGS){
	StringInfo	buf = (StringInfo) PG_GETARG_CHESSBOARD_P(0);
	chessboard *result = (chessboard *)palloc(sizeof(chessboard));
	result->board = pstrdup(pq_getmsgstring(buf));
	result->color = *pq_getmsgstring(buf);
	result->castling = pstrdup(pq_getmsgstring(buf));
	result->enpassant = pstrdup(pq_getmsgstring(buf));
	result->halfmove = pq_getmsgint(buf, sizeof(int));
	result->fullmove = pq_getmsgint(buf, sizeof(int));
	PG_RETURN_POINTER(result);
} */

PG_FUNCTION_INFO_V1(chessboard_recv);
Datum chessboard_recv(PG_FUNCTION_ARGS){
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	int	nbytes;
	const int length = pq_getmsgint64(buf);
	char *recv_char = pq_getmsgtext(buf, length, &nbytes);
	PG_RETURN_CHESSBOARD_P(chessboard_parse(recv_char));
}

PG_FUNCTION_INFO_V1(chessboard_send);
Datum chessboard_send(PG_FUNCTION_ARGS){
	chessboard	*cb = (chessboard *) PG_GETARG_CHESSBOARD_P(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	int nchars = strlen(cb->board);
	pq_sendint64(&buf, nchars);
	pq_sendtext(&buf, cb->board, nchars);
	PG_FREE_IF_COPY(cb, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}
/* PG_FUNCTION_INFO_V1(chessboard_cast_from_text);
Datum chessboard_cast_from_text(PG_FUNCTION_ARGS)
{  	
	text *txt = PG_GETARG_TEXT_P(0);
	char *str = DatumGetCString(DirectFunctionCall1(textout,PointerGetDatum(txt)));

	
    PG_RETURN_POINTER(chessboard_parse(str)); 

} */

/* PG_FUNCTION_INFO_V1(chessboard_cast_to_text);
Datum chessboard_cast_to_text(PG_FUNCTION_ARGS)
{
  chessboard *cb = PG_GETARG_CHESSBOARD_P(0);
  char *result = psprintf("%s %c %s %s %d %d", cb->board, cb->color, cb->castling, cb->enpassant, cb->halfmove, cb->fullmove);
  PG_RETURN_CSTRING(result);
} */

/* PG_FUNCTION_INFO_V1(chessboard_cast_to_text);
Datum chessboard_cast_to_text(PG_FUNCTION_ARGS)
{
  chessboard *cb = PG_GETARG_CHESSBOARD_P(0);
  PG_RETURN_CSTRING(cb->board);
} */

