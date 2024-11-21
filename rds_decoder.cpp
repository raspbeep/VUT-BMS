#include <cstdio>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using BlockArray = std::array<ushort, 104>;
using GroupBitVector = std::vector<BlockArray>;

class Group0A {};

class ArgumentParser {
public:
  enum Error { NO_ERROR, ARGUMENT_COUNT, INVALID_FLAG, INVALID_VALUE };

private:
  Error error;
  std::string binary_string_value;
  GroupBitVector group_bit_vector;

public:
  ArgumentParser(int argc, char *argv[]) : error(NO_ERROR) {
    if (argc != 3) {
      error = ARGUMENT_COUNT;
      std::cout << "./rds_decoder -b BINARY_STRING" << std::endl;
      return;
    }

    if (std::string(argv[1]) != "-b") {
      std::cout << "Invalid flag: " << argv[1] << std::endl;
      error = INVALID_FLAG;
      return;
    }
    binary_string_value = argv[2];
    if (binary_string_value.size() % 104 != 0) {
      std::cout << "Invalid length of binary value "
                << binary_string_value.size() << std::endl;
      error = INVALID_VALUE;
      return;
    }

    for (size_t g = 0; g < binary_string_value.size() / 104; g++) {
      BlockArray block = {0};
      for (size_t i = 0; i < 104; i++) {
        char c = binary_string_value[g * 104 + i];
        if (c != '0' && c != '1') {
          std::cout << "Invalid character in binary value: " << c << std::endl;
          error = INVALID_VALUE;
          return;
        }
        block[i] = c == '0' ? 0 : 1;
      }
      group_bit_vector.push_back(block);
    }
  }

public:
  Error get_error() { return error; }

  GroupBitVector &get_group_bit_vector() { return group_bit_vector; }
};

int main(int argc, char *argv[]) {
  auto parser = ArgumentParser(argc, argv);
  if (parser.get_error() != ArgumentParser::NO_ERROR) {
    return 2;
  }
  std::cout << "here" << std::endl;

  return 0;
}