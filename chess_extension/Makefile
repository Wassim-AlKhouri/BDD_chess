MODULE_big	= chess
OBJS = \
	$(WIN32RES) \
	chess.o \
	smallchesslib.o \
	chess_gin.o \

EXTENSION   = chess
DATA        = chess--1.0.sql
HEADERS_chess = chess.h

PG_CONFIG ?= pg_config
PGXS = $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)