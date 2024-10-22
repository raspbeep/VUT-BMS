#include <iostream>
#include <cstdio>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <iomanip>
#include <unordered_map>

using BlockArray = std::array<ushort, 104>;
using GroupBitVector = std::vector<BlockArray>;

class EncoderHelper {
public:
  const std::vector<std::string> all_frequencies;
  const std::unordered_map<ushort, std::string> frequency_codes_map;
  const std::unordered_map<ushort, char> character_encoding_map;
  const std::vector<std::pair<std::array<ushort, 8>, char>> character_encoding_vector = {
        {{0, 0, 0, 0, 1, 1, 0, 1}, '\r'}, // radio text ending 0D (hex) if length < 64
        {{0, 0, 1, 0, 0, 0, 0, 0}, ' '},

        {{0, 0, 1, 1, 0, 0, 0, 0}, '0'},
        {{0, 0, 1, 1, 0, 0, 0, 1}, '1'},
        {{0, 0, 1, 1, 0, 0, 1, 0}, '2'},
        {{0, 0, 1, 1, 0, 0, 1, 1}, '3'},
        {{0, 0, 1, 1, 0, 1, 0, 0}, '4'},
        {{0, 0, 1, 1, 0, 1, 0, 1}, '5'},
        {{0, 0, 1, 1, 0, 1, 1, 0}, '6'},
        {{0, 0, 1, 1, 0, 1, 1, 1}, '7'},
        {{0, 0, 1, 1, 1, 0, 0, 0}, '8'},
        {{0, 0, 1, 1, 1, 0, 0, 1}, '9'},

        {{0, 1, 0, 0, 0, 0, 0, 1}, 'A'},
        {{0, 1, 0, 0, 0, 0, 1, 0}, 'B'},
        {{0, 1, 0, 0, 0, 0, 1, 1}, 'C'},
        {{0, 1, 0, 0, 0, 1, 0, 0}, 'D'},
        {{0, 1, 0, 0, 0, 1, 0, 1}, 'E'},
        {{0, 1, 0, 0, 0, 1, 1, 0}, 'F'},
        {{0, 1, 0, 0, 0, 1, 1, 1}, 'G'},
        {{0, 1, 0, 0, 1, 0, 0, 0}, 'H'},
        {{0, 1, 0, 0, 1, 0, 0, 1}, 'I'},
        {{0, 1, 0, 0, 1, 0, 1, 0}, 'J'},
        {{0, 1, 0, 0, 1, 0, 1, 1}, 'K'},
        {{0, 1, 0, 0, 1, 1, 0, 0}, 'L'},
        {{0, 1, 0, 0, 1, 1, 0, 1}, 'M'},
        {{0, 1, 0, 0, 1, 1, 1, 0}, 'N'},
        {{0, 1, 0, 0, 1, 1, 1, 1}, 'O'},

        {{0, 1, 0, 1, 0, 0, 0, 0}, 'P'},
        {{0, 1, 0, 1, 0, 0, 0, 1}, 'Q'},
        {{0, 1, 0, 1, 0, 0, 1, 0}, 'R'},
        {{0, 1, 0, 1, 0, 0, 1, 1}, 'S'},
        {{0, 1, 0, 1, 0, 1, 0, 0}, 'T'},
        {{0, 1, 0, 1, 0, 1, 0, 1}, 'U'},
        {{0, 1, 0, 1, 0, 1, 1, 0}, 'V'},
        {{0, 1, 0, 1, 0, 1, 1, 1}, 'W'},
        {{0, 1, 0, 1, 1, 0, 0, 0}, 'X'},
        {{0, 1, 0, 1, 1, 0, 0, 1}, 'Y'},
        {{0, 1, 0, 1, 1, 0, 1, 0}, 'Z'},

        {{0, 1, 1, 0, 0, 0, 0, 1}, 'a'},
        {{0, 1, 1, 0, 0, 0, 1, 0}, 'b'},
        {{0, 1, 1, 0, 0, 0, 1, 1}, 'c'},
        {{0, 1, 1, 0, 0, 1, 0, 0}, 'd'},
        {{0, 1, 1, 0, 0, 1, 0, 1}, 'e'},
        {{0, 1, 1, 0, 0, 1, 1, 0}, 'f'},
        {{0, 1, 1, 0, 0, 1, 1, 1}, 'g'},
        {{0, 1, 1, 0, 1, 0, 0, 0}, 'h'},
        {{0, 1, 1, 0, 1, 0, 0, 1}, 'i'},
        {{0, 1, 1, 0, 1, 0, 1, 0}, 'j'},
        {{0, 1, 1, 0, 1, 0, 1, 1}, 'k'},
        {{0, 1, 1, 0, 1, 1, 0, 0}, 'l'},
        {{0, 1, 1, 0, 1, 1, 0, 1}, 'm'},
        {{0, 1, 1, 0, 1, 1, 1, 0}, 'n'},
        {{0, 1, 1, 0, 1, 1, 1, 1}, 'o'},

        {{0, 1, 1, 1, 0, 0, 0, 1}, 'p'},
        {{0, 1, 1, 1, 0, 0, 0, 1}, 'q'},
        {{0, 1, 1, 1, 0, 0, 1, 0}, 'r'},
        {{0, 1, 1, 1, 0, 0, 1, 1}, 's'},
        {{0, 1, 1, 1, 0, 1, 0, 0}, 't'},
        {{0, 1, 1, 1, 0, 1, 0, 1}, 'u'},
        {{0, 1, 1, 1, 0, 1, 1, 0}, 'v'},
        {{0, 1, 1, 1, 0, 1, 1, 1}, 'w'},
        {{0, 1, 1, 1, 1, 0, 0, 0}, 'x'},
        {{0, 1, 1, 1, 1, 0, 0, 1}, 'y'},
        {{0, 1, 1, 1, 1, 0, 1, 0}, 'z'},
    };
public:
    EncoderHelper() : all_frequencies(generate_frequencies()), character_encoding_map(generate_character_encoding_map()) {}
private:
    std::vector<std::string> generate_frequencies() {
        std::vector<std::string> frequencies = {};
        for (size_t first = 87; first < 108; ++first) {
            for (size_t second = 0; second < 10; ++second) {
                if ((first == 87 && second < 6)) {
                    continue;
                }
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(1) << first << "." << second;
                frequencies.push_back(oss.str());
            }
        }
        return frequencies;
    }

    template <std::size_t N>
    ushort arr_to_ushort(const std::array<ushort, N>& arr) {
        ushort result = 0;
        for (size_t i = 0; i < N; ++i) {
            result = (result << 1) | arr[i];
        }
        return result;
    }

    std::unordered_map<ushort, char> generate_character_encoding_map() {
        std::unordered_map<ushort, char> m = {};
        for (size_t i = 0; i < character_encoding_vector.size(); i++) {
            m[arr_to_ushort(character_encoding_vector[i].first)] = character_encoding_vector[i].second;
        }
        return m;
    }

    std::unordered_map<ushort, std::string> generate_frequency_codes_map() {
        std::unordered_map<ushort, std::string> m = {};
        for (size_t i = 0; i < all_frequencies.size(); ++i) {
            m[arr_to_ushort(fill_n_bit_unsigned_arr_rev(i + 1))] = all_frequencies[i];
        }
        return m;
    }
  
    std::array<ushort, 8> fill_n_bit_unsigned_arr_rev(size_t val) {
        std::array<ushort, 8> result = {0};
        for (size_t i = 0; i < 8; i++) {
            result[i] = val % 2;
            val >>= 1;
        }
        std::reverse(std::begin(result), std::end(result));
        return result;
    }
};
EncoderHelper FE = EncoderHelper();


class Group0A {


};

class ArgumentParser {
public:
  enum Error {
        NO_ERROR,
        ARGUMENT_COUNT,
        INVALID_FLAG,
        INVALID_VALUE
    };
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
      std::cout << "Invalid length of binary value " << binary_string_value.size() << std::endl;
      error = INVALID_VALUE;
      return;
    }

    for (size_t g = 0; g < binary_string_value.size() / 104; g++) {
      BlockArray block = { 0 };
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
  Error get_error() {
    return error;
  }

  GroupBitVector& get_group_bit_vector() {
    return group_bit_vector;
  }
};

int main(int argc, char *argv[]) {
  auto parser = ArgumentParser(argc, argv);
  if (parser.get_error() != ArgumentParser::NO_ERROR) {
    return 2;
  }

  return 0;
}