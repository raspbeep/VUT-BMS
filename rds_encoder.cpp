#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

const std::regex frequency_regex(R"(^\d{2,3}\.\d$)");
// no decimal point for comparison with integer
const double MIN_FREQUENCY = 876;
const double MAX_FREQUENCY = 1079;

enum GroupType { GROUP_0A, GROUP_2A };

using argsMap = std::unordered_map<std::string, std::string>;
using GroupTypeMap = std::unordered_map<GroupType, uint8_t>;
using VersionCodeMap = std::unordered_map<GroupType, uint8_t>;
using OffsetMap = std::unordered_map<std::string, std::array<ushort, 10>>;
using BlockArray = std::array<ushort, 104>;
using GroupBitVector = std::vector<BlockArray>;
using RadioText = std::vector<std::array<ushort, 8>>;

GroupTypeMap group_type_code_map = {{GROUP_0A, 0}, {GROUP_2A, 2}};

VersionCodeMap version_code_map = {{GROUP_0A, 0}, {GROUP_2A, 1}};

OffsetMap offset_map = {
    {"A", {0, 0, 1, 1, 1, 1, 1, 1, 0, 0}},
    {"B", {0, 1, 1, 0, 0, 1, 1, 0, 0, 0}},
    {"C", {0, 1, 0, 1, 1, 0, 1, 0, 0, 0}},
    {"D", {0, 1, 1, 0, 1, 1, 0, 1, 0, 0}},
};

const unsigned int PI_FLAG = 1;
const unsigned int PTY_FLAG = 2;
const unsigned int TP_FLAG = 4;
const unsigned int MS_FLAG = 8;
const unsigned int TA_FLAG = 16;
const unsigned int AF_FLAG = 32;
const unsigned int PS_FLAG = 64;
const unsigned int RT_FLAG = 128;
const unsigned int AB_FLAG = 256;

const unsigned int complete_group_0A_flags = 0b1111111;
const unsigned int complete_group_2A_flags = 0b110000111;

void print_lower_5_bits(uint8_t value) {
  for (int i = 4; i >= 0; --i) {
    std::cout << ((value >> i) & 1);
  }
}

void print_16_bits(uint16_t value) {
  for (int i = 15; i >= 0; --i) {
    std::cout << ((value >> i) & 1);
  }
}

class CommonGroup {
public:
  CommonGroup(GroupType group_type_code, const uint16_t program_identification,
              const bool traffic_program, const uint8_t program_type)
      : group_type_code(group_type_code_map[group_type_code]),
        vc(version_code_map[group_type_code]), pi(program_identification),
        tp(traffic_program), pty(program_type) {}

  virtual GroupBitVector generate_group_bit_vector() = 0;

public:
  std::array<ushort, 1> fill_boolean_arr(bool val) { return {val}; }
  // 0001 0010 0011 0100
  template <std::size_t N>
  std::array<ushort, N> fill_n_bit_unsigned_arr(ushort val) {
    std::array<ushort, N> result = {0};
    for (size_t i = 0; i < N; i++) {
      result[i] = val % 2;
      val >>= 1;
    }
    std::reverse(std::begin(result), std::end(result));
    return result;
  }

protected:
  template <std::size_t N>
  void copy_n_bits_at_pos(BlockArray &dst, const std::array<ushort, N> &src,
                          size_t &pos) {
    for (size_t i = 0; i < N; i++) {
      dst[pos++] = src[i];
    }
  }

protected:
  /* Group Type 0 (0000), 1 (0001), 2 (0010), 3 (0011) */
  uint8_t group_type_code;
  /* Version Code A (0) or B (1)*/
  bool vc;
  /* Program Identification */
  uint16_t pi;
  /* Traffic Program */
  bool tp;
  /* Program Type */
  uint8_t pty;
};

class Group2A : public CommonGroup {
private:
  /* Radio text (up to 64 8bit chars) */
  std::string rt;
  /* Radio text A/B */
  bool ab;

private:
  std::string radio_text;

public:
  Group2A(const u_int16_t program_identification, const uint8_t program_type,
          const bool traffic_program, const std::string &radio_text_string,
          const bool radio_text_a_b)
      : CommonGroup(GROUP_2A, program_identification, traffic_program,
                    program_type),
        rt(radio_text_string), ab(radio_text_a_b),
        radio_text(radio_text_string) {}

  GroupBitVector generate_group_bit_vector() override {
    // GroupBitVector stream;
    // size_t n_groups = (radio_text.size() + 3) / 4;
    // if (n_groups < 16 && radio_text.size() % 4 == 0)
    //   n_groups++;
    // for (size_t g = 0; g < n_groups; g++) {
    //   BlockArray block = {0};
    //   size_t pos = 0;
    //   copy_n_bits_at_pos(block, pi, pos);
    //   pos += 10; // add CRC
    //   copy_n_bits_at_pos(block, group_type_code, pos);
    //   copy_n_bits_at_pos(block, vc, pos);
    //   copy_n_bits_at_pos(block, tp, pos);
    //   copy_n_bits_at_pos(block, pty, pos);
    //   copy_n_bits_at_pos(block, ab, pos);
    //   copy_n_bits_at_pos(block, this->fill_n_bit_unsigned_arr<4>((ushort)g),
    //                      pos);
    //   pos += 10; // add CRC
    //   // TODO: check string length
    //   rt_copy_n_bits_at_pos(block, rt, pos, g * 4);
    //   rt_copy_n_bits_at_pos(block, rt, pos, g * 4 + 1);
    //   pos += 10; // add CRC
    //   rt_copy_n_bits_at_pos(block, rt, pos, g * 4 + 2);
    //   rt_copy_n_bits_at_pos(block, rt, pos, g * 4 + 3);
    //   pos += 10; // add CRC

    //   stream.push_back(block);
    // }
    // // 4 groups
    // for (size_t g = 0; g < n_groups; g++) {
    //   // 4 blocks
    //   auto rts = this->fill_n_bit_unsigned_arr<4>((ushort)g);
    //   for (size_t b = 0; b < 4; b++) {
    //     for (size_t i = 0; i < 26; i++) {
    //       std::cout << stream[g][b * 26 + i];
    //       if (i == 15)
    //         std::cout << " ";
    //     }
    //     switch (b) {
    //     case 0:
    //       std::cout << " " << pi_number;
    //       break;
    //     case 1:
    //       std::cout << " 2A " << tp[0] << " " << pty[0] << pty[1] << pty[2]
    //                 << pty[3] << pty[4] << " " << ab[0] << " " << rts[0]
    //                 << rts[1] << rts[2] << rts[3];
    //       break;
    //     case 2:
    //       rt_print_char_at_pos(g * 4);
    //       rt_print_char_at_pos(g * 4 + 1);
    //       break;
    //     case 3:
    //       rt_print_char_at_pos(g * 4 + 2);
    //       rt_print_char_at_pos(g * 4 + 3);
    //       break;
    //     }
    //     std::cout << std::endl;
    //   }
    //   std::cout << std::endl;
    // }

    // return stream;
  }
};

class Group0A : public CommonGroup {
private:
  /* Speech (0) / Music (1) */
  bool ms;
  /* Traffic announcement */
  bool ta;
  /* Alternative Frequency 1 */
  uint8_t af1;
  /* Alternative Frequency 2 */
  uint8_t af2;
  /* Program Service (8bit char * 2 per message * 4 groups = 8 chars + 1 NULL
   * termination) */
  std::string ps;

public:
  Group0A(const uint16_t program_identification, const uint8_t program_type,
          const bool traffic_program, const bool music_speech,
          const bool traffic_announcement, const uint8_t af1, const uint8_t af2,
          const std::string &program_service)
      : CommonGroup(GROUP_0A, program_identification, traffic_program,
                    program_type),
        ms(music_speech), ta(traffic_announcement), af1(af1), af2(af2),
        ps(program_service) {}

  GroupBitVector generate_group_bit_vector() override {
    GroupBitVector stream;
    for (size_t b = 0; b < 4; b++) {
    }
  }
};

// Function to handle Group 0A encoding
void encode_group_0A(const argsMap &args) {
  std::cout << "Encoding Group 0A...\n";
  std::cout << "Program Identification (PI): " << args.at("-pi") << "\n";
  std::cout << "Program Type (PTY): " << args.at("-pty") << "\n";
  std::cout << "Traffic Program (TP): " << args.at("-tp") << "\n";
  std::cout << "Music/Speech (MS): " << args.at("-ms") << "\n";
  std::cout << "Traffic Announcement (TA): " << args.at("-ta") << "\n";
}

// Function to handle Group 2A encoding
void encodeGroup2A(const argsMap &args) {
  std::cout << "Encoding Group 2A...\n";
  std::cout << "Program Identification (PI): " << args.at("-pi") << "\n";
  std::cout << "Program Type (PTY): " << args.at("-pty") << "\n";
  std::cout << "Traffic Program (TP): " << args.at("-tp") << "\n";
  std::cout << "Radio Text (RT): " << args.at("-rt") << "\n";
  std::cout << "Radio Text A/B flag (AB): " << args.at("-ab") << "\n";
}

class ArgumentParser {
public:
  enum Error {
    NO_ERROR,
    ARGUMENT_COUNT,
    INVALID_FLAG,
    MISSING_FLAG,
    MISSING_VALUE,
    UNSUPPORTED_GROUP,
    INVALID_FREQUENCIES,
    INVALID_PROGRAM_TYPE,
    INVALID_VALUE
  };

private:
  argsMap args;
  GroupType groupType;
  Error error;

  // common
  uint16_t pi;
  uint8_t pty;
  bool tp;

  // 0A helper
  std::string alternative_frequencies;
  // 0A
  bool ms;
  bool ta;

  uint8_t af1;
  uint8_t af2;
  std::string ps;

  // 2A
  std::string radio_text;
  bool radio_text_ab_flag;
  ////////////////////////////////////////////

private:
  void parse_frequencies() {
    std::stringstream ss(alternative_frequencies);
    std::string token;
    unsigned counter = 0;
    while (std::getline(ss, token, ',')) {
      if (counter >= 2) {
        error = INVALID_FREQUENCIES;
        break;
      }

      // Check if the token matches the frequency regex
      if (!std::regex_match(token, frequency_regex)) {
        std::cerr << "Error: Invalid frequency format " << token << "\n";
        error = INVALID_FREQUENCIES;
        break;
      }

      // Remove the decimal point and convert to integer
      std::string frequency_str = token;
      frequency_str.erase(
          std::remove(frequency_str.begin(), frequency_str.end(), '.'),
          frequency_str.end());
      int frequency_int = std::stoi(frequency_str);

      if (frequency_int < MIN_FREQUENCY || frequency_int > MAX_FREQUENCY) {
        std::cerr << "Error: Invalid frequency " << frequency_int
                  << " (must be in range <" << MIN_FREQUENCY << ","
                  << MAX_FREQUENCY << ">)\n";
        error = INVALID_FREQUENCIES;
        break;
      }

      // convert to 8 bit format as per RDS standard
      uint8_t f = frequency_int - 875;
      if (counter == 0) {
        af1 = f;
      } else {
        af2 = f;
      }

      counter++;
    }

    if (counter != 2) {
      error = INVALID_FREQUENCIES;
    }

    if (error == INVALID_FREQUENCIES) {
      std::cerr << "Error: Invalid alternative frequencies\n";
    }
  }

  int parse_boolean(const std::string &flag_value, bool &value) {
    if (flag_value != "0" && flag_value != "1") {
      std::cerr << "Error: Invalid value for boolean" << flag_value
                << " (must be 0 or 1)\n";
      error = INVALID_VALUE;
      return -1;
    } else {
      value = flag_value == "1";
      return 0;
    }
  }

  int parse_string(const std::string &value, std::string &result, size_t length,
                   bool pad) {
    if (value.size() > length) {
      std::cerr << "Error: Invalid value " << value << " (must be at most "
                << length << " characters, is " << value.size() << ")\n";
      error = INVALID_VALUE;
      return -1;
    }
    result = value;
    // add padding to full length if needed
    if (pad) {
      result.append(length - value.size(), ' ');
    }
    return 0;
  }

public:
  ArgumentParser(int argc, char *argv[]) : error(NO_ERROR) {
    if (argc < 3) {
      std::cerr << "Usage: " << argv[0] << " -g <GroupID> [other flags...]\n";
      error = ARGUMENT_COUNT;
    }
    std::string groupID;
    unsigned int flags = 0;
    // Iterate over command-line arguments and parse flags
    for (size_t i = 1; i < argc; ++i) {
      std::string flag = argv[i];
      if (flag == "-g") {
        groupID = argv[++i];
        if (groupID != "0A" && groupID != "2A") {
          std::cerr << "Error: Invalid group ID " << groupID << "\n";
          error = UNSUPPORTED_GROUP;
          break;
        }
        if (i + 1 >= argc) {
          std::cerr << "Error: Missing value for " << flag << "\n";
          error = MISSING_VALUE;
          break;
        }
        if (groupID == "0A") {
          groupType = GROUP_0A;
        } else if (groupID == "2A") {
          groupType = GROUP_2A;
        }
      } else if (i + 1 < argc) {
        args[flag] = argv[++i];
        if (flag == "-pi") {
          flags |= PI_FLAG;
          int pi_int_tmp{};
          try {
            pi_int_tmp = std::stoi(args.at("-pi"));
            if (pi_int_tmp < 0 ||
                pi_int_tmp > std::numeric_limits<uint16_t>::max()) {
              std::cerr << "Error: PI value out of range (max value: "
                        << ((1 << 16) - 1) << ")" << std::endl;
              error = INVALID_VALUE;
              break;
            } else {
              pi = static_cast<uint16_t>(pi_int_tmp);
            }
          } catch (const std::exception &e) {
            std::cerr << "Error: Invalid PI value " << args.at("-pi") << "\n";
            error = INVALID_VALUE;
            break;
          }
        } else if (flag == "-pty") {
          flags |= PTY_FLAG;
          int pty_int_tmp{};
          try {
            pty_int_tmp = std::stoi(args.at("-pty"));
            if (pty_int_tmp < 0 || pty_int_tmp > 31) {
              std::cerr << "Error: PTY value out of range (max value: "
                        << ((1 << 5) - 1) << ")" << std::endl;
              error = INVALID_VALUE;
            } else {
              pty = static_cast<uint8_t>(pty_int_tmp);
            }
          } catch (const std::exception &e) {
            std::cerr << "Error: Invalid PTY value " << args.at("-pty") << "\n";
            error = INVALID_VALUE;
            return;
          }
        } else if (flag == "-tp") {
          flags |= TP_FLAG;
          if (parse_boolean(args.at("-tp"), tp))
            break;
        } else if (flag == "-ms") {
          flags |= MS_FLAG;
          if (parse_boolean(args.at("-ms"), ms))
            break;
        } else if (flag == "-ta") {
          flags |= TA_FLAG;
          if (parse_boolean(args.at("-ta"), ta))
            break;
        } else if (flag == "-af") {
          flags |= AF_FLAG;
          alternative_frequencies = args.at("-af");
          parse_frequencies();
        } else if (flag == "-ps") {
          flags |= PS_FLAG;
          if (parse_string(args.at("-ps"), ps, 8, true))
            break;
        } else if (flag == "-rt") {
          flags |= RT_FLAG;
          if (parse_string(args.at("-rt"), radio_text, 64, false))
            break;
        } else if (flag == "-ab") {
          flags |= AB_FLAG;
          if (parse_boolean(args.at("-ab"), radio_text_ab_flag))
            break;
        } else {
          std::cerr << "Error: Unknown flag " << flag << "\n";
          break;
          error = INVALID_FLAG;
        }
      } else {
        std::cerr << "Error: Missing value for " << flag << "\n";
        error = MISSING_VALUE;
        break;
      }
    }

    if (error != NO_ERROR) {
      return;
    }

    if ((flags & PI_FLAG) == 0) {
      std::cerr << "Error: Missing PI flag\n";
    }
    if ((flags & PTY_FLAG) == 0) {
      std::cerr << "Error: Missing PTY flag\n";
    }
    if ((flags & TP_FLAG) == 0) {
      std::cerr << "Error: Missing TP flag\n";
    }

    if (groupType == GROUP_0A) {
      if (flags - complete_group_0A_flags != 0) {
        error = MISSING_FLAG;
        std::cerr << "Error: Argument Error for Group 0A\n";
        if ((flags & MS_FLAG) == 0) {
          std::cerr << "Error: Missing MS flag\n";
        }
        if ((flags & TA_FLAG) == 0) {
          std::cerr << "Error: Missing TA flag\n";
        }
        if ((flags & AF_FLAG) == 0) {
          std::cerr << "Error: Missing AF flag\n";
        }
        if ((flags & PS_FLAG) == 0) {
          std::cerr << "Error: Missing PS flag\n";
        }
        if ((flags & RT_FLAG) != 0) {
          std::cerr << "Error: RT flag should not be present for Group 0A\n";
        }
        if ((flags & AB_FLAG) != 0) {
          std::cerr << "Error: AB flag should not be present for Group 0A\n";
        }
      }
    } else if (groupType == GROUP_2A) {
      if (flags - complete_group_2A_flags != 0) {
        error = MISSING_FLAG;
        std::cerr << "Error: Argument Error for Group 2A\n";
        if ((flags & RT_FLAG) == 0) {
          std::cerr << "Error: Missing RT flag\n";
        }
        if ((flags & AB_FLAG) == 0) {
          std::cerr << "Error: Missing AB flag\n";
        }
        if ((flags & MS_FLAG) != 0) {
          std::cerr << "Error: MS flag should not be present for Group 2A\n";
        }
        if ((flags & TA_FLAG) != 0) {
          std::cerr << "Error: TA flag should not be present for Group 2A\n";
        }
        if ((flags & AF_FLAG) != 0) {
          std::cerr << "Error: AF flag should not be present for Group 2A\n";
        }
        if ((flags & PS_FLAG) != 0) {
          std::cerr << "Error: PS flag should not be present for Group 2A\n";
        }
      }
    }
  }

  Error get_error() { return error; }

  GroupType get_group_type() { return groupType; }

  uint8_t get_af1() { return af1; }

  uint8_t get_af2() { return af2; }

  uint16_t get_pi() { return pi; }

  uint8_t get_pty() { return pty; }

  bool get_tp() { return tp; }

  bool get_ta() { return ta; }

  bool get_ms() { return ms; }

  std::string get_ps() { return ps; }

  std::string get_rt() { return radio_text; }

  bool get_ab() { return radio_text_ab_flag; }
};

int main(int argc, char *argv[]) {
  auto parser = ArgumentParser(argc, argv);
  if (parser.get_error() != ArgumentParser::NO_ERROR) {
    std::cerr << "Usage: " << argv[0] << " -g <GroupID> [other flags...]\n";
    return 1;
  }

  if (parser.get_group_type() == GroupType::GROUP_0A) {
    Group0A group(parser.get_pi(), parser.get_pty(), parser.get_tp(),
                  parser.get_ms(), parser.get_ta(), parser.get_af1(),
                  parser.get_af2(), parser.get_ps());

    std::cout << "Group 0A" << std::endl;

    //   std::cout << "Group 0A" << std::endl;
    //   group.generate_group_bit_vector();
    // } else if (parser.get_group_type() == GroupType::GROUP_2A) {
    //   Group2A group(parser.get_pi(), parser.get_pty(), parser.get_tp(),
    //                 parser.get_rt(), parser.get_ab());
    //   std::cout << "Group 2A" << std::endl;
    //   group.generate_group_bit_vector();
  }

  return 0;
}
