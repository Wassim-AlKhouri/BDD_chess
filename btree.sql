-- In order for this test to work you must run test.sql first (to make sure the tables are created). Use the following command to run the file:
-- psql -d NAMEOFDATABASE -f chess_extension/test.sql
-- then run the insert.sql file to fill the tables with data. Use the following command to run the file:
-- psql -d NAMEOFDATABASE -f chess_extension/insert.sql

CREATE INDEX cg_btree_idx ON tcg USING btree (cg);

EXPLAIN ANALYZE SELECT * FROM tcg WHERE cg < '1.a4';

EXPLAIN ANALYZE SELECT * FROM tcg WHERE hasOpening(cg, '1.a4');

-- So the explain returns that the query is not using the btree index in both of the queries.
-- Even thought using the index would be faster.
-- As seen by the following explains:

SET enable_seqscan = off;

EXPLAIN ANALYZE SELECT * FROM tcg WHERE cg < '1.a4';

-- So the explain returns that the query is using the btree index.
-- And now we can try with the hasOpening function which uses the operators of the btree index:

EXPLAIN ANALYZE SELECT * FROM tcg WHERE hasOpening(cg, '1.a4');

-- It also uses the btree index.