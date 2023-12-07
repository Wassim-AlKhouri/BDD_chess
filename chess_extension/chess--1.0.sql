-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION chess" to load this file. \quit

/******************************************************************************
 * chessboard
 ******************************************************************************/

/* Input/Output */

CREATE OR REPLACE FUNCTION chessboard_in(cstring)
  RETURNS chessboard
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessboard_out(chessboard)
  RETURNS cstring
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessboard_recv(internal)
  RETURNS chessboard
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessboard_send(chessboard)
  RETURNS bytea
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE chessboard (
  internallength = VARIABLE,
  input          = chessboard_in,
  output         = chessboard_out,
  receive        = chessboard_recv,
  send           = chessboard_send,
  alignment      = double
);

/* CREATE OR REPLACE FUNCTION chessboard(text)
  RETURNS chessboard
  AS 'MODULE_PATHNAME', 'chessboard_cast_from_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION text(chessboard)
  RETURNS text
  AS 'MODULE_PATHNAME', 'chessboard_cast_to_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (text as chessboard) WITH FUNCTION chessboard(text) AS IMPLICIT;
CREATE CAST (chessboard as text) WITH FUNCTION text(chessboard); */

/* constructor */

CREATE FUNCTION chessboard(text)
  RETURNS chessboard
  AS 'MODULE_PATHNAME', 'chessboard_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
  * chessgame
  *****************************************************************************/

/* Input/Output */
CREATE OR REPLACE FUNCTION chessgame_in(cstring)
  RETURNS chessgame
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_out(chessgame)
  RETURNS cstring
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_recv(internal)
  RETURNS chessgame
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_send(chessgame)
  RETURNS bytea
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE chessgame (
  internallength = VARIABLE,
  input          = chessgame_in,
  output         = chessgame_out,
  receive        = chessgame_recv,
  send           = chessgame_send,
  alignment      = double
);

/* CREATE OR REPLACE FUNCTION chessgame(text)
  RETURNS chessgame
  AS 'MODULE_PATHNAME', 'chessgame_cast_from_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION text(chessgame)
  RETURNS text
  AS 'MODULE_PATHNAME', 'chessgame_cast_to_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (text as chessgame) WITH FUNCTION chessgame(text) AS IMPLICIT;
CREATE CAST (chessgame as text) WITH FUNCTION text(chessgame);
 */

CREATE FUNCTION chessgame(text)
  RETURNS chessgame
  AS 'MODULE_PATHNAME', 'chessgame_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
  * functions
  *****************************************************************************/
CREATE FUNCTION getBoard(chessgame, int)
  RETURNS chessboard
  AS 'MODULE_PATHNAME', 'getBoard'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
  
CREATE FUNCTION getFirstMoves(chessgame, int)
  RETURNS chessgame
  AS 'MODULE_PATHNAME', 'getFirstMoves'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION hasOpening(chessgame, chessgame)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'hasOpening'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION hasBoard(chessgame, chessboard, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'hasBoard'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

  
/*****************************************************************************
  * GIN
  *****************************************************************************/
   /* Operator */
  

  CREATE OR REPLACE FUNCTION gin_contains_chessboard(chessgame, chessboard)
    RETURNS boolean
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;
  
  CREATE OPERATOR @> (
    LEFTARG = chessgame, 
    RIGHTARG = chessboard,
    PROCEDURE = gin_contains_chessboard
  );

  /* Support functions */
  CREATE OR REPLACE FUNCTION gin_compare_chessgame(chessboard, chessboard)
    RETURNS integer
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;
  
  CREATE OR REPLACE FUNCTION gin_extract_value_chessgame(chessgame)
    RETURNS text[][]
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

  CREATE OR REPLACE FUNCTION gin_extract_query_chessgame(internal, internal, internal, internal, internal, internal, internal)
    RETURNS text[][]
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

  CREATE OR REPLACE FUNCTION gin_consistent_chessgame(internal, internal, internal, internal, internal, internal, internal, internal)
    RETURNS boolean
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

  /* Operator class */
  CREATE OPERATOR CLASS gin_chessgame_ops
  DEFAULT FOR TYPE chessgame USING gin AS
    --OPERATOR        1       @>(chessgame, chessboard) ,
    FUNCTION        1       gin_compare_chessgame(chessboard, chessboard),
    FUNCTION        2       gin_extract_value_chessgame(chessgame),
    FUNCTION        3       gin_extract_query_chessgame(internal, internal, internal, internal, internal, internal, internal),
    FUNCTION        4       gin_consistent_chessgame(internal, internal, internal, internal, internal, internal, internal, internal);

    
    
/******************************************************************************
 * B-Tree
 ******************************************************************************/

/* B-Tree comparison functions */

/* CREATE OR REPLACE FUNCTION chessgame_abs_eq(chessgame, chessgame)
  RETURNS boolean
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_abs_lt(chessgame, chessgame)
  RETURNS boolean
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_abs_le(chessgame, chessgame)
  RETURNS boolean
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_abs_gt(chessgame, chessgame)
  RETURNS boolean
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_abs_ge(chessgame, chessgame)
  RETURNS boolean
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 


/******************************************************************************/

/* B-Tree comparison operators */

CREATE OPERATOR = (
  LEFTARG = chessgame, RIGHTARG = chessgame,
  PROCEDURE = chessgame_abs_eq,
  COMMUTATOR = =, NEGATOR = <>
);
CREATE OPERATOR < (
  LEFTARG = chessgame, RIGHTARG = chessgame,
  PROCEDURE = chessgame_abs_lt,
  COMMUTATOR = >, NEGATOR = >=
);
CREATE OPERATOR <= (
  LEFTARG = chessgame, RIGHTARG = chessgame,
  PROCEDURE = chessgame_abs_le,
  COMMUTATOR = >=, NEGATOR = >
);
CREATE OPERATOR >= (
  LEFTARG = chessgame, RIGHTARG = chessgame,
  PROCEDURE = chessgame_abs_ge,
  COMMUTATOR = <=, NEGATOR = <
);
CREATE OPERATOR > (
  LEFTARG = chessgame, RIGHTARG = chessgame,
  PROCEDURE = chessgame_abs_gt,
  COMMUTATOR = <, NEGATOR = <=
);

/******************************************************************************/

/* B-Tree support function */

CREATE OR REPLACE FUNCTION chessgame_abs_cmp(chessgame, chessgame)
  RETURNS integer
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

/* B-Tree operator class */

CREATE OPERATOR CLASS chessgame_abs_ops
DEFAULT FOR TYPE chessgame USING btree
AS
        OPERATOR        1       <  ,
        OPERATOR        2       <= ,
        OPERATOR        3       =  ,
        OPERATOR        4       >= ,
        OPERATOR        5       >  ,
        FUNCTION        1       chessgame_abs_cmp(chessgame, chessgame);  */

/******************************************************************************/
