CXX=g++
CXXFLAGS=-std=c++14 -Wall -Wextra -Werror -pedantic -g -O0

.PHONY: all clean rds_encoder rds_decoder zip 

# Targets
all: rds_encoder rds_decoder

rds_encoder:
	$(CXX) $(CXXFLAGS) -o rds_encoder rds_encoder.cpp common.cpp

rds_decoder:
	$(CXX) $(CXXFLAGS) -o rds_decoder rds_decoder.cpp common.cpp

zip: clean
	zip xkrato61.zip rds_encoder.cpp rds_encoder.hpp \
	 rds_decoder.cpp rds_decoder.hpp common.cpp common.hpp \
	 Makefile xkrato61.pdf
	sh check_zip.sh xkrato61.zip

clean:
	rm -f *.o rds_encoder rds_decoder xkrato61.zip
