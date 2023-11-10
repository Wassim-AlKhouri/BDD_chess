#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "chess2.h"
#include <string.h>

PG_FUNCTION_INFO_V1(chessboard_in);

Datum chessboard_in(PG_FUNCTION_ARGS) {
    char	*str = PG_GETARG_CSTRING(0);
	char	*token;

	//check if format FEN
	chessboard *result = (chessboard *)palloc(sizeof(chessboard));
	result->board = pstrdup(strtok(str, " "));
    result->color = *strtok(str, " ");
	result->castling = pstrdup(strtok(str, " "));
	result->enpassant = pstrdup(strtok(str, " "));
	result->halfmove = atoi(strtok(str, " "));
	result->fullmove = atoi(str);
    PG_RETURN_POINTER(result);
}
