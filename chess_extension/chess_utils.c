#include "chess.h"

/*
* Function used to test if a move move is a valid pawn move
* @param move: the move to test
* @return: true if the move is a valid pawn move, false otherwise
*/
static bool isValidPawnMove(char* move){
	if (move[0] >= 'a' && move[0] <= 'h' && move[1] >= '1' && move[1] <= '8'){
			if (strlen(move) == 4 && move[2] == '=' && 
				(move[3] == 'Q' || move[3] == 'R' || move[3] == 'B' || move[3] == 'N')) {
				
				return true; // Valid promotion
				}
			else if (strlen(move) == 2) return true; // Valid move
	}if (strlen(move) == 4 &&
			move[0] >= 'a' && move[0] <= 'h' && 
			move[1] == 'x' &&
			move[2] >= 'a' && move[2] <= 'h' &&
			move[3] >= '1' && move[3] <= '8') {
			return true; // Valid capture
	} else if (strlen(move) == 6 &&
			move[0] >= 'a' && move[0] <= 'h' && 
			move[1] == 'x' &&
			move[2] >= 'a' && move[2] <= 'h' &&
			move[3] >= '1' && move[3] <= '8' &&
			move[4] == '=' &&
			(move[5] == 'Q' || move[5] == 'R' || move[5] == 'B' || move[5] == 'N')) {
			return true; // Valid capture and promotion
		}
	return false; //Invalid move
}

/*
* Function used to test if a move move is a valid piece move
* @param move: the move to test
* @return: true if the move is a valid piece move, false otherwise
*/
static bool isValidPieceMove(char* move) {
    if (strlen(move) == 3 &&
		(move[0] == 'K' || move[0] == 'Q' || move[0] == 'R' || move[0] == 'B' || move[0] == 'N') &&
        move[1] >= 'a' && move[1] <= 'h' &&
        move[2] >= '1' && move[2] <= '8') {

        return true;  // Valid non-capture
    }else if (strlen(move) == 4 && 
			( 
			((move[0] == 'K' || move[0] == 'Q' || move[0] == 'R' || move[0] == 'B' || move[0] == 'N') && move[1] == 'x') ||  
			( (move[0] == 'N' || move[0] == 'R' || move[0] == 'B' || move[0] == 'Q') && ((move[1] >= 'a' && move[1] <= 'h') ||(move[1] >= '1' && move[1] <= '8')) ) 
			) &&
            move[2] >= 'a' && move[2] <= 'h' &&
            move[3] >= '1' && move[3] <= '8') {
            return true;  // Valid capture or non-capture with a specified file or rank
    }else if (strlen(move) == 5 &&
			(move[0] == 'N' || move[0] == 'R' || move[0] == 'B' || move[0] == 'Q') &&
			( (move[1] >= 'a' && move[1] <= 'h') ||(move[1] >= '1' && move[1] <= '8') ) &&
			move[2] == 'x' &&
			move[3] >= 'a' && move[3] <= 'h' &&
			move[4] >= '1' && move[4] <= '8') {
			return true;  // Valid capture with a specified file or rank
	}else if (strlen(move) == 6 &&
			(move[0] == 'N' || move[0] == 'R' || move[0] == 'B' || move[0] == 'Q') &&
			move[1] >= 'a' && move[1] <= 'h' &&
			move[2] >= '1' && move[2] <= '8' &&
			move[3] == 'x' &&
			move[4] >= 'a' && move[4] <= 'h' &&
			move[5] >= '1' && move[5] <= '8') {
			return true;  // Valid capture with both file and rank specified
	}else if (strlen(move) == 5 &&
			(move[0] == 'N' || move[0] == 'R' || move[0] == 'B' || move[0] == 'Q') &&
			move[1] >= 'a' && move[1] <= 'h' &&
			move[2] >= '1' && move[2] <= '8' &&
			move[3] >= 'a' && move[3] <= 'h' &&
			move[4] >= '1' && move[4] <= '8') {
			return true;  // Valid non-capture with both file and rank specified
	}
    return false;  // Invalid move
}

/*
* Function used to test if a move move is a valid castle
* @param move: the move to test
* @return: true if the move is a valid castle, false otherwise
*/
static bool isValidCastle(const char* move) {
    if (strlen(move) == 5 && strncmp(move, "O-O-O", 5) == 0) return true;  // Valid kingside castle
    else if (strlen(move) == 3 && strncmp(move, "O-O", 3) == 0) return true;  // Valid queenside castle
    return false;  // Invalid move
}

/*
* Function used to test if a "move" is a actually the result of the game (ie 1-0, 0-1, 1/2-1/2)
* @param move: the move to test
* @return: true if the "move" is a valid end of game, false otherwise
*/
static bool isEndOfGame(const char* move) {
	if (strlen(move) == 3 && strncmp(move, "1-0", 3) == 0) return true;  // Valid white win
	else if (strlen(move) == 3 && strncmp(move, "0-1", 3) == 0) return true;  // Valid black win
	else if (strlen(move) == 7 && strncmp(move, "1/2-1/2", 7) == 0) return true;  // Valid draw
	return false;  // Invalid move
}

/*
* Function used to test if a move move is a valid SAN move
* @param move: the move to test
* @return: true if the move is a valid SAN move, false otherwise
*/
static bool isValidSANmove(char* move){
	if (strlen(move) == 1 && move[0] == '*') return true; // Valid checkmate
	else if (move == NULL || strlen(move) < 2) return false;
	else if (move[strlen(move) -1] == '+' || move[strlen(move) -1] == '#'){
		move[strlen(move) -1] = '\0';
		return isValidSANmove(move);
	}
	else if (isValidPawnMove(move)) return true;
	else if (isValidPieceMove(move)) return true;
	else if (isValidCastle(move)) return true;
	else if (isEndOfGame(move)) return true;
	return false;
}

/*
* Function used to test if a string is a positive integer
* @param str: the string to test
* @return: true if the string is a positive integer, false otherwise
*/
bool isPositiveInteger(const char *str) {
    char *endptr;
    long result = strtol(str, &endptr, 10);
    if (*endptr != '\0') {
        return false;  // Conversion failed, not a valid integer
    }
    return result > 0;
}

/*
* Function used to test if a string is a valid SAN string
* @param game: the string to test
* @return: true if the string is a valid SAN string, false otherwise
*/
bool isValidSan(const char* game){
	//check if the string is not empty
	char *copy;
	char *token;
	int counter;
	if (game == NULL || strlen(game) == 0) return false;

	copy = (char*) malloc(strlen(game) + 1);
	strcpy(copy, game);
	token = strtok(copy, " ");
	counter = 1;
	//check if the string is in SAN format
	while (token != NULL){
		if(strlen(token) < 2 && token[0] != '*'){
			elog(ERROR, "Invalid move: %s", token);
			free(copy);
			return false;
		}
		//check if number of move (ie "number.")
		else if (token[strlen(token) - 1] == '.'){ 
			token[strlen(token)-1] = '\0';
			if (!isPositiveInteger(token) || atoi(token) != counter) {
				elog(ERROR, "Invalid move number: %s", token);
				free(copy);
				return false; 
			}else counter++;	
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

/*
* Function used to formate a string. It removes all extra spaces and adds a space after a dot.
* @param str: the string to formate
* @return: the formated string
*/
char	*formate_SAN(const char *str)
{
	char	*SAN;
	int		nb_extraspace = 0;
	int		nb_left_spaces = 0;
	int		nb_right_spaces = 0;
	int		nb_space_to_add = 0;
	int		i = 0;
	int		j = 0;
	int		SAN_len;
	
	while(str[strlen(str) - 1 - nb_right_spaces] == ' ')
		nb_right_spaces ++; //remove all right spaces
	while(str[nb_left_spaces] == ' ')
		nb_left_spaces++; //remove all left spaces
	while(str[nb_left_spaces + i] && nb_left_spaces + i < strlen(str) - nb_right_spaces){
		if (str[nb_left_spaces + i] == ' ')
		{
			i++;
			while (str[nb_left_spaces + i] == ' '){
				i++;
				nb_extraspace++;
			}
		}
		else if(str[nb_left_spaces + i] == '.'){
			i++;
			if (str[nb_left_spaces + i] != ' ')
				nb_space_to_add++;
		}
		else
			i++;
	}
	SAN_len = strlen(str) - nb_extraspace - nb_left_spaces - nb_right_spaces + nb_space_to_add;
	SAN = malloc(sizeof(char) * (SAN_len + 1));
	i = 0;
	j = nb_left_spaces;
	while(i < SAN_len){
		SAN[i] = str[j];
		if (str[j] == ' '){
			i++;
			j++;
			while (str[j] == ' ')
				j++; //remove extra spaces
		}
		else if (str[j] == '.' && str[j + 1] != ' '){
			SAN[i + 1] = ' ';
			i += 2;
			j++;
		}
		else{
			i++;
			j++;
		}
	}
	SAN[SAN_len] = '\0';
	return (SAN);
}