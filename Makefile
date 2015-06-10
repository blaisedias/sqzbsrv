#TARG_ARCH = arm-linux-gnueabihf
TARG_ARCH = x86_64-linux-gnu
BIN = bin
OD = obj
LIBS = -lboost_filesystem -lboost_system -lboost_serialization -ltag -lprofiler
LIBDIRS = -L/usr/lib/$(TARG_ARCH)
GD = Makefile
#GD = 
CF = -std=c++11 -Wall -g -DCPP11
CF = -Wall -g

OBJS = $(OD)/audio_file_tags.o $(OD)/fs_utils.o $(OD)/scanner.o $(OD)/main.o \
	   $(OD)/audio_tags.o  $(OD)/tracks_db.o 

S_OBJS = $(OD)/audio_file_tags.o $(OD)/fs_utils.o $(OD)/scanner.o $(OD)/s_main.o \
	   $(OD)/audio_tags.o  $(OD)/songs_db.o $(OD)/sstring.o

all: $(BIN)/rtags $(BIN)/s_rtags $(BIN)/exp $(BIN)/bsearch $(BIN)/sort_example $(BIN)/ios

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

$(OD)/main.o: main.cpp scanner.h $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/s_main.o: s_main.cpp sstring.h songs_db.h scanner.h audio_tags.h audio_file_tags.h
	g++ $(CF) -c -o $(@) $< 

.PHONY: clean

clean:
	rm -f $(OD)/*
	rm -f $(BIN)/*

bin/sort_example: $(OD)/sort_example.o
	g++ -o $(@) $<

bin/exp: $(OD)/exp.o $(OD)/sstring.o
	g++ -o $(@) $^  -lboost_filesystem -lboost_system -lboost_serialization

bin/bsearch: $(OD)/bsearch.o 
	g++ -o $(@) $^  -lboost_filesystem -lboost_system -lboost_serialization

$(BIN)/ios: $(OD)/ios.o
	g++ $(CF) -o $(BIN)/ios $^ $(LIBDIRS) $(LIBS)

