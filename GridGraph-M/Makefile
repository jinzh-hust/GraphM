
ROOT_DIR= $(shell pwd)
TARGETS= bin/concurrent_jobs

CXX?= g++
CXXFLAGS?= -O3 -Wall -std=c++11 -g -fopenmp -I$(ROOT_DIR)
HEADERS= $(shell find . -name '*.hpp')

bin/concurrent_jobs: examples/concurrent_jobs.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(SYSLIBS)

clean:
	rm -rf $(TARGETS)

