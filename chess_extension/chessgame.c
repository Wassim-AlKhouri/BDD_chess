#include "chess.h"
#include "smallchesslib.h"
PG_MODULE_MAGIC;

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

static bool isValidSan(const char* game){
	//check if the string is not empty
	if (game == NULL || strlen(game) == 0) return false;

	char *copy = (char*) malloc(strlen(game) + 1);
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

static	int get_number_extraspaces(const char *SAN_moves)
{
	int	nb_extraspaces = 0;
	int	i = 0;
	while (SAN_moves && SAN_moves[i])
	{
		if (SAN_moves[i] = ' ')
		{
			i++;
			while (SAN_moves[i] = ' ')
			{
				nb_extraspaces++;
				i++;
			}
		}
		else
			i++;
	}
	return nb_extraspaces;
}

static	chessgame* chessgame_make(const char *SAN_moves)
{
	int	i = 0;
	chessgame	*game = (chessgame *) palloc(VARHDRSZ + strlen(SAN_moves) - nb_extraspaces + 1);
	if (game != NULL) {
		if (SAN_moves != NULL) {
			SET_VARSIZE(game, VARHDRSZ + strlen(SAN_moves - nb_extraspaces) + 1);
			while (SAN_moves[i])
			{
				game->moves[i] = SAN_moves[i];
				if (SAN_moves[i] == ' '){
					i++;
					while (SAN_moves[i] == ' ')
						SAN_moves++;
				}
				else
					i++;
			}
			game->moves[i] = 0;
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

static char* cutFirstMoves(char* moves, int halfMovesNbr){
	int	length = 0;
	int	movesCounter = 0;
	//char *moves  =chgame->moves;
	while (moves[length] && movesCounter < halfMovesNbr)
	{
		while (moves[length] == ' ')
			length++;
		if(!(moves[length] <'9' && moves[length] > '0'))
				movesCounter++;
		while (moves[length] && moves[length] != ' ')
			length++;

	}
	char *result = malloc(sizeof(char) * (length + 1));
	strncpy(result, moves, length);
	result[length] = '\0';
	return (result);
}

static int countMoves(char* moves){
	SCL_Record	r;
	SCL_Board	board;
	SCL_recordInit(r);
    SCL_recordFromPGN(r, moves);
	return (SCL_recordLength(r));
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