-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION chess" to load this file. \quit


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

CREATE FUNCTION chessgame(text)
  RETURNS chessgame
  AS 'MODULE_PATHNAME', 'chessgame_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
