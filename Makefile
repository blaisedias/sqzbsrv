#
# On debian derived platforms
# apt-get install g++ libtag1-dev libboost-system-dev libboost-filesystem-dev libboost-serialization-dev 
# For builds using -DPROFILER install libgoogle-perftools-dev
#
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

#DEFS = -DSSTRING_HAS_DEFAULT_CONTEXT
DEFS =
SRC = src
BIN = bin
OD = obj
LIBS = -lboost_filesystem -lboost_system -lboost_serialization -ltag -lpthread $(TARG_LIBS)
GD = Makefile
CF = -std=c++11 -Wall -g $(TARG_CF) $(DEFS)

OBJS = 	$(OD)/audio_file_tags.o $(OD)/fs_utils.o $(OD)/scanner.o \
		$(OD)/audio_tags.o  $(OD)/tracks_db.o \
		$(OD)/bdrwlock.o \
	   	$(OD)/tmain.o

S_OBJS = $(OD)/audio_file_tags.o $(OD)/fs_utils.o $(OD)/scanner.o \
  		 $(OD)/audio_tags.o  $(OD)/tracks_db.o \
  		 $(OD)/songs_db.o $(OD)/sstring.o \
		 $(OD)/bdrwlock.o \
		 $(OD)/s_main.o

all: $(BIN)/rtags $(BIN)/s_rtags $(BIN)/exp $(BIN)/exp2

.PHONY: clean

clean:
	rm -f $(OD)/*
	rm -f $(BIN)/*

$(BIN)/rtags: $(OBJS)
	g++ $(CF) -o $(BIN)/rtags $^ $(LIBDIRS) $(LIBS)

$(BIN)/s_rtags: $(S_OBJS)
	g++ $(CF) -o $(BIN)/s_rtags $^ $(LIBDIRS) $(LIBS)

$(OD)/%.o: $(SRC)/%.cpp $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/tracks_db.o: $(SRC)/tracks_db.cpp $(SRC)/record_store.h $(SRC)/audio_tags.h $(SRC)/tracks_db.h
	g++ $(CF) -c -o $(@) $< 

$(OD)/sstring.o: $(SRC)/sstring.cpp $(SRC)/sstring.h $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/songs_db.o: $(SRC)/songs_db.cpp $(SRC)/record_store.h $(SRC)/audio_tags.h $(SRC)/audio_file_tags.h $(SRC)/songs_db.h $(SRC)/sstring.h
	g++ $(CF) -c -o $(@) $< 

$(OD)/audio_file_tags.o: $(SRC)/audio_file_tags.cpp $(SRC)/audio_file_tags.h $(SRC)/audio_tags.h $(GD) 
	g++ $(CF) -c -o $(@) $< -I /usr/include/taglib

$(OD)/audio_tags.o: $(SRC)/audio_tags.cpp $(SRC)/audio_tags.h $(GD)
	g++ $(CF) -c -o $(@) $<

$(OD)/fs_utils.o: $(SRC)/fs_utils.cpp $(SRC)/fs_utils.h $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/scanner.o: $(SRC)/scanner.cpp $(SRC)/audio_file_tags.h $(SRC)/fs_utils.h $(SRC)/scanner.h $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/tmain.o: $(SRC)/tmain.cpp $(SRC)/scanner.h $(SRC)/tracks_db.h $(SRC)/audio_tags.h $(SRC)/audio_file_tags.h $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/s_main.o: $(SRC)/s_main.cpp $(SRC)/sstring.h $(SRC)/songs_db.h $(SRC)/scanner.h $(SRC)/audio_tags.h $(SRC)/audio_file_tags.h
	g++ $(CF) -c -o $(@) $< 

$(OD)/bdrwlock.o: $(SRC)/bdrwlock.cpp $(SRC)/bdrwlock.h $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/exp.o: $(SRC)/exp.cpp $(SRC)/sstring.h $(GD)
	g++ $(CF) -c -o $(@) $< 

$(OD)/exp2.o: $(SRC)/exp2.cpp $(SRC)/sstring.h $(GD)
	g++ $(CF) -c -o $(@) $< 

bin/exp: $(OD)/exp.o $(OD)/sstring.o
	g++ -o $(@) $^ $(LIBDIRS) $(LIBS)

bin/exp2: $(OD)/exp2.o $(OD)/sstring.o
	g++ -o $(@) $^ $(LIBDIRS) $(LIBS)

