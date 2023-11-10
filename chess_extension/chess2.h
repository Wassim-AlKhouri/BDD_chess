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
	int		index;
	char	*moves;
	char	**boards;
} chessgame;

#define DatumGetchessboard(X)  ((chessboard *) DatumGetPointer(X))
#define chessboardGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_chessboard(n) DatumGetchessboard(PG_GETARG_DATUM(n))
#define PG_RETURN_chessboard(x) return chessboardGetDatum(x)

#define DatumGetchessgame(X)  ((chessgame *) DatumGetPointer(X))
#define chessgameGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_chessgame(n) DatumGetSAN(PG_GETARG_DATUM(n))
#define PG_RETURN_chessgame(x) return SANGetDatum(x)