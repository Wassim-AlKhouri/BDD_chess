CREATE TABLE tcg(cg chessgame);
INSERT INTO tcg VALUES ('1. f3 e5 2. g4 Qh4#');
INSERT INTO tcg VALUES ('1. Nf3 Nf6 2. c4 g6 3. Nc3 Bg7 4. d4 O-O 5. Bf4 d5 6. Qb3');
INSERT INTO tcg VALUES ('1. e4 d5 2. exd5 Qxd5 3. Nc3 Qd8 4. Bc4 Nf6 5. Nf3 Bg4 6. h3 Bxf3 7. Qxf3 e6 8. Qxb7 Nbd7 9. Nb5 Rc8 10. Nxa7 Nb6 11. Nxc8 Nxc8 12. d4 Nd6 13. Bb5+');
INSERT INTO tcg VALUES ('1.e4 e5 2.Nf3 Nc6 3.Bc4 Bc5 4.c3 Nf6 5.d4');

CREATE TABLE tcb(cb chessboard);
INSERT INTO tcb VALUES ('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1');
INSERT INTO tcb VALUES ('8/5k2/3p4/1p1Pp2p/pP2Pp1P/P4P1K/8/8 b - - 99 50');
INSERT INTO tcb VALUES ('r4r1k/ppp1qpb1/7p/4pRp1/1PB1P3/1QPR3P/P4P2/6K1 w - - 4 26');

SELECT * FROM tcg;
SELECT * from tcb;

SELECT getFirstMoves('1. f3 e5 2. g4 Qh4#', 2);
SELECT getFirstMoves('1. f3 e5 2. g4 Qh4#', 4);

SELECT getBoard('1. f3 e5 2. g4 Qh4#', 0);
SELECT getBoard('1. f3 e5 2. g4 Qh4#', 2);
SELECT getBoard('1. f3 e5 2. g4 Qh4#', 4);

SELECT * FROM tcg WHERE hasOpening(cg, '1. e4');
SELECT * FROM tcg WHERE hasOpening(cg, '1. f3 e5 2. g4 Qh4#');

SELECT * FROM tcg WHERE hasBoard(cg, 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1', 2);
SELECT * FROM tcg WHERE hasBoard(cg, 'rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1', 1);