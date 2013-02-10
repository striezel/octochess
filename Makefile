CFLAGS = -O3 -g -pipe -march=corei7 -Wall -Wextra -flto -static -m64
#CFLAGS = -O0 -g -pipe -Wall -static
CXXFLAGS = $(CFLAGS) -std=gnu++0x

CC = gcc
CXX = g++

all: octochess bookgen

tables.cpp: tables_gen.cpp
	$(CXX) -m64 -static tables_gen.cpp -o tables_gen
	./tables_gen > tables.cpp
	rm tables_gen

UTIL_FILES = \
	util/logger.o \
	util/mutex_unix.o \
	util/platform.o \
	util/string.o \
	util/thread.o \
	util/time.o \

OBJECT_FILES = \
	book.o \
	calc.o \
	config.o \
	detect_check.o \
	endgame.o \
	epd.o \
	eval.o \
	eval_values.o \
	fen.o \
	hash.o \
	history.o \
	magic.o \
	moves.o \
	moves_captures.o \
	moves_noncaptures.o \
	pawn_structure_hash_table.o \
	pgn.o \
	phased_move_generator.o \
	position.o \
	pvlist.o \
	pv_move_picker.o \
	random.o \
	score.o \
	see.o \
	seen_positions.o \
	selftest.o \
	sqlite/sqlite3.o \
	sqlite/sqlite3_wrapper.o \
	statistics.o \
	tables.o \
	tweak.o \
	util.o \
	zobrist.o

CHESS_FILES = \
	chess.o \
	xboard.o \
	uci/info.o \
	uci/minimalistic_uci_protocol.o \
	uci/octochess_impl.o \
	uci/runner.o \
	uci/time_calculation.o


%.o: %.cpp *.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

sqlite/sqlite3.o: sqlite/sqlite3.c sqlite/sqlite3.h
	$(CC) $(CFLAGS) -DSQLITE_OMIT_LOAD_EXTENSION -pthread -c -o $@ $<


octochess: $(UTIL_FILES) $(OBJECT_FILES) $(CHESS_FILES)
	$(CXX) $(CXXFLAGS) -pthread -o $@ $^


bookgen: $(UTIL_FILES) $(OBJECT_FILES) bookgen.o
	$(CXX) $(CXXFLAGS) -pthread -o $@ $^

clean:
	rm -f octochess bookgen tables_gen
	rm -f *.gcda
	rm -f *.o
	rm -f uci/*.o
	rm -f uci/*.gcda
	rm -f util/*.o
	rm -f util/*.gcda
	rm -f tables.cpp
	rm -f sqlite/*.o
	rm -f sqlite/*.gcda

.PHONY: all clean
