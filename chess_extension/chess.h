

typedef struct
{
    int index;
    char *move;
} SAN;

typedef struct 
{
    char* event;
    char* site;
    char* date;
    char* round;
    char* white;
    char* black;
    char* result;
} PGN;

typedef struct 
{
    char* board;
    char* color;
    char* castling;
    char* enpassant;
    int halfMove;
    int fullMove;
} FEN;
    

#define DatumGetSAN(X)  ((SAN *) DatumGetPointer(X))
#define SANGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_SAN(n) DatumGetSAN(PG_GETARG_DATUM(n))
#define PG_RETURN_SAN(x) return SANGetDatum(x)

#define DatumGetPGN(X)  ((PGN *) DatumGetPointer(X))
#define PGNGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_PGN(n) DatumGetPGN(PG_GETARG_DATUM(n))
#define PG_RETURN_PGN(x) return PGNGetDatum(x)

#define DatumGetFEN(X)  ((FEN *) DatumGetPointer(X))
#define FENGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_FEN(n) DatumGetFEN(PG_GETARG_DATUM(n))
#define PG_RETURN_FEN(x) return FENGetDatum(x)


