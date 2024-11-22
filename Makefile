CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -Werror -pedantic -O3

.PHONY: all clean rds_encoder rds_decoder 

all: rds_encoder rds_decoder

rds_encoder: rds_encoder.o
	$(CXX) $(CXXFLAGS) -o rds_encoder rds_encoder.cpp

rds_decoder: rds_decoder.o
	$(CXX) $(CXXFLAGS) -o rds_decoder rds_decoder.cpp

clean:
	rm -f *.o rds_encoder rds_decoder