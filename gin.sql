-- This test will take a while to run. Because creating the gin index takes a while.
-- In order for this test to work you must run test.sql first (to make sure the tables are created). Use the following command to run the file:
-- psql -d NAMEOFDATABASE -f chess_extension/test.sql
-- then run the insert.sql file to fill the tables with data. Use the following command to run the file:
-- psql -d NAMEOFDATABASE -f chess_extension/insert.sql

CREATE INDEX cg_gin_idx ON tcg USING gin (cg);

EXPLAIN ANALYZE SELECT * FROM tcg WHERE cg @> 'r1bq1rk1/1pp2p1n/2np3p/p1b1p1p1/P1B1P3/2PP1NB1/1P3PPP/RN1Q1RK1 w - - 2 11'; -- to test

-- So the explain returns that the query is using the gin index.
-- But now if we try with hasBoard function:

EXPLAIN ANALYZE SELECT * FROM tcg WHERE hasBoard(cg, 'r1bq1rk1/1pp2p1n/2np3p/p1b1p1p1/P1B1P3/2PP1NB1/1P3PPP/RN1Q1RK1 w - - 2 11', 20);

-- The explain returns that the query is not using the gin index. Even though the hasBoard function is using the @> operator.
-- And even if we set the enable_seqscan to off it still doesn't use the gin index.

SET enable_seqscan = off;
EXPLAIN ANALYZE SELECT * FROM tcg WHERE hasBoard(cg, 'r1bq1rk1/1pp2p1n/2np3p/p1b1p1p1/P1B1P3/2PP1NB1/1P3PPP/RN1Q1RK1 w - - 2 11', 20);