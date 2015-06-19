TARG_ARCH=$(shell uname -m)

ifeq ($(TARG_ARCH),x86_64)
	LIBDIRS = -L/usr/lib/x86_64-linux-gnu
	TARG_LIBS = -lprofiler
	TARG_CF = -DPROFILER
endif
ifeq ($(TARG_ARCH),i686)
	LIBDIRS = 
	TARG_LIBS = -lprofiler
	TARG_CF = -DPROFILER
endif
ifeq ($(TARG_ARCH),armv6l)
	LIBDIRS = -L/usr/lib/arm-linux-gnueabihf
	TARG_LIBS =
	TARG_CF =
endif

BIN = bin
OD = obj
LIBS = -lboost_filesystem -lboost_system -lboost_serialization -ltag $(TARG_LIBS)
GD = Makefile
CF = -std=c++11 -Wall -g $(TARG_CF)

OBJS = $(OD)/audio_file_tags.o $(OD)/fs_utils.o $(OD)/scanner.o $(OD)/tmain.o \
	   $(OD)/audio_tags.o  $(OD)/tracks_db.o 

S_OBJS = $(OD)/audio_file_tags.o $(OD)/fs_utils.o $(OD)/scanner.o $(OD)/s_main.o \
	   $(OD)/audio_tags.o  $(OD)/songs_db.o $(OD)/sstring.o $(OD)/tracks_db.o

all: $(BIN)/rtags $(BIN)/s_rtags $(BIN)/exp $(BIN)/bsearch $(BIN)/sort_example $(BIN)/ios

.PHONY: clean

clean:
	rm -f $(OD)/*
	rm -f $(BIN)/*

$(BIN)/rtags: $(OBJS)
	g++ $(CF) -o $(BIN)/rtags $^ $(LIBDIRS) $(LIBS)

$(BIN)/s_rtags: $(S_OBJS)
	g++ $(CF) -o $(BIN)/s_rtags $^ $(LIBDIRS) $(LIBS)

$(OD)/%.o: %.cpp $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/tracks_db.o: tracks_db.cpp record_store.h audio_tags.h tracks_db.h
	g++ $(CF) -c -o $(@) $< 

$(OD)/sstring.o: sstring.cpp sstring.h $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/songs_db.o: songs_db.cpp record_store.h audio_tags.h audio_file_tags.h songs_db.h sstring.h
	g++ $(CF) -c -o $(@) $< 

$(OD)/exp.o: exp.cpp sstring.h $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/audio_file_tags.o: audio_file_tags.cpp audio_file_tags.h audio_tags.h $(GD) 
	g++ $(CF) -c -o $(@) $< -I /usr/include/taglib

$(OD)/fs_utils.o: fs_utils.cpp fs_utils.h $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/scanner.o: scanner.cpp audio_file_tags.h fs_utils.h scanner.h $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/tmain.o: tmain.cpp scanner.h $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/s_main.o: s_main.cpp sstring.h songs_db.h scanner.h audio_tags.h audio_file_tags.h
	g++ $(CF) -c -o $(@) $< 

bin/sort_example: $(OD)/sort_example.o
	g++ -o $(@) $<

bin/exp: $(OD)/exp.o $(OD)/sstring.o
	g++ -o $(@) $^  -lboost_filesystem -lboost_system -lboost_serialization

bin/bsearch: $(OD)/bsearch.o 
	g++ -o $(@) $^  -lboost_filesystem -lboost_system -lboost_serialization

$(BIN)/ios: $(OD)/ios.o
	g++ $(CF) -o $(BIN)/ios $^ $(LIBDIRS) $(LIBS)

