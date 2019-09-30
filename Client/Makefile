ALL_CPP_FILES := $(wildcard *.cc)
ALL_H_FILES := $(wildcard *.h)
ALL_O_FILES := $(ALL_CPP_FILES:.cc=.o) 

FLAGS_SERVER := -g -Wall -std=c++11 -pthread -ldl

all: server

$(ALL_O_FILES): %.o: %.cc $(ALL_H_FILES)
	g++ $(FLAGS_SERVER) -c $<

server: $(ALL_O_FILES)
	g++  $(FLAGS_SERVER) $(ALL_O_FILES) -o server

.PHONY: clean
clean: 
	rm -f $(ALL_O_FILES) server