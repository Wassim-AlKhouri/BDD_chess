
//#include "smallchesslib.h"
# include "postgres.h"
# include "fmgr.h"
# include "utils/builtins.h"
# include "libpq/pqformat.h"



typedef	struct
{
	int32	vl_len_;
	char	board[FLEXIBLE_ARRAY_MEMBER];
} chessboard;

typedef	struct
{
	int32 vl_len_;
	char moves[FLEXIBLE_ARRAY_MEMBER];
} chessgame;

#define DatumGetchessboardP(X)  ((chessboard *) DatumGetPointer(X))
#define chessboardPGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_CHESSBOARD_P(n) DatumGetchessboardP(PG_GETARG_DATUM(n))
#define PG_RETURN_CHESSBOARD_P(x) return chessboardPGetDatum(x)

#define DatumGetchessgameP(X)  ((chessgame *) DatumGetPointer(X))
#define ChessgamePGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_CHESSGAME_P(n) DatumGetchessgameP(PG_GETARG_DATUM(n))
#define PG_RETURN_CHESSGAME_P(x) return ChessgamePGetDatum(x)
