# include "postgres.h"
# include "fmgr.h"
# include "utils/builtins.h"
# include "libpq/pqformat.h"

typedef	struct
{
	int32	vl_len_;		/* varlena header (do not touch directly!) */
	char moves[FLEXIBLE_ARRAY_MEMBER];
} chessgame;

#define DatumGetchessgameP(X)  ((chessgame *) DatumGetPointer(X))
#define ChessgamePGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_CHESSGAME_P(n) DatumGetchessgameP(PG_GETARG_DATUM(n))
#define PG_RETURN_CHESSGAME_P(x) return ChessgamePGetDatum(x)
