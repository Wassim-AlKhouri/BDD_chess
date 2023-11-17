typedef	struct
{
	char	*board;
	char	color;
	char	*castling;
	char	*enpassant;
	int		halfmove;
	int		fullmove;

} chessboard;

typedef	struct
{
	char	*moves;
	char	**boards;
} chessgame;

#define DatumGetchessboardP(X)  ((chessboard *) DatumGetPointer(X))
#define chessboardPGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_CHESSBOARD_P(n) DatumGetchessboardP(PG_GETARG_DATUM(n))
#define PG_RETURN_CHESSBOARD_P(x) return chessboardPGetDatum(x)

#define DatumGetchessgameP(X)  ((chessgame *) DatumGetPointer(X))
#define ChessgamePGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_CHESSGAME_P(n) DatumGetchessgameP(PG_GETARG_DATUM(n))
#define PG_RETURN_CHESSGAME_P(x) return ChessgamePGetDatum(x)


PG_MODULE_MAGIC;