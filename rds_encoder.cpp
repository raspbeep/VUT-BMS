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

const uint8_t group_type_code_0A = 0b00000;
const uint8_t group_type_code_2A = 0b00100;

const uint32_t offset_A = 252;
const uint32_t offset_B = 408;
const uint32_t offset_C = 360;
const uint32_t offset_D = 436;

constexpr std::bitset<26> crc_bitset = 0b10110111001;

const unsigned int PI_FLAG = 1;
const unsigned int PTY_FLAG = 2;
const unsigned int TP_FLAG = 4;
const unsigned int MS_FLAG = 8;
const unsigned int TA_FLAG = 16;
const unsigned int AF_FLAG = 32;
const unsigned int PS_FLAG = 64;
const unsigned int RT_FLAG = 128;
const unsigned int AB_FLAG = 256;

const unsigned int complete_group_0A_flags = 0b001111111;
const unsigned int complete_group_2A_flags = 0b110000111;

void print_lower_5_bits(uint8_t value) {
  for (int i = 4; i >= 0; --i) {
    std::cout << ((value >> i) & 1);
  }
}

void print_26_bits(uint32_t value) {
  for (int i = 25; i >= 0; --i) {
    std::cout << ((value >> i) & 1);
    // if (i == 10)
    //   std::cout << " ";
  }
  // std::cout << std::endl;
}

uint32_t crc(uint32_t value, uint32_t offset) {
  std::bitset<26> and_mask = 0b1111111111;
  std::bitset<26> offset_bitset = offset;
  std::bitset<26> and_check = 1 << 25;
  std::bitset<26> val = value;
  for (int i = 15; i >= 0; i--) {
    if ((val & and_check) == 0) {
      and_check >>= static_cast<uint8_t>(1);
      continue;
    }
    val ^= (crc_bitset << static_cast<uint8_t>(i));
    and_check >>= 1;
  }

  val &= and_mask;
  val ^= offset_bitset;
  return static_cast<uint32_t>(val.to_ulong());
}

class CommonGroup {
public:
  CommonGroup(const uint8_t gt_vc, const uint16_t program_identification,
              const bool traffic_program, const uint8_t program_type)
      : gt_vc(gt_vc), pi(program_identification), tp(traffic_program),
        pty(program_type) {}

  virtual void print_bits() = 0;

protected:
  /* Group Type Code + VC; 0A=0000, 2A=0010 */
  uint8_t gt_vc;
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
      : CommonGroup(group_type_code_2A, program_identification, traffic_program,
                    program_type),
        rt(radio_text_string), ab(radio_text_a_b), radio_text(radio_text_string) {}

  void print_bits() override {
    uint32_t line{};
    for (size_t b = 0; b < 16; b++) {
      line = 0;
      line |= static_cast<uint32_t>(pi) << 10;
      line |= crc(line, offset_A);
      print_26_bits(line);

      line = 0;
      line |= static_cast<uint32_t>(gt_vc) << 21;
      line |= static_cast<uint32_t>(tp) << 20;
      line |= static_cast<uint32_t>(pty) << 15;
      line |= static_cast<uint32_t>(ab) << 14;
      line |= static_cast<uint32_t>(b) << 10;
      line |= crc(line, offset_B);
      print_26_bits(line);

      line = 0;
      // radio text segment
      uint8_t c0 = static_cast<uint8_t>(rt[(b * 4)]);
      uint8_t c1 = static_cast<uint8_t>(rt[(b * 4) + 1]);
      line |= static_cast<uint32_t>(c0) << 18;
      line |= static_cast<uint32_t>(c1) << 10;
      line |= crc(line, offset_C);
      print_26_bits(line);

      line = 0;
      // radio text segment
      uint8_t c2 = static_cast<uint8_t>(rt[(b * 4) + 2]);
      uint8_t c3 = static_cast<uint8_t>(rt[(b * 4) + 3]);
      line |= static_cast<uint32_t>(c2) << 18;
      line |= static_cast<uint32_t>(c3) << 10;
      line |= crc(line, offset_D);
      print_26_bits(line);
    }
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
      : CommonGroup(group_type_code_0A, program_identification, traffic_program,
                    program_type),
        ms(music_speech), ta(traffic_announcement), af1(af1), af2(af2),
        ps(program_service) {}

  void print_bits() {
    uint32_t line{};
    for (size_t b = 0; b < 4; b++) {
      line = 0;
      line |= static_cast<uint32_t>(pi) << 10;
      line |= crc(line, offset_A);
      print_26_bits(line);

      line = 0;
      line |= static_cast<uint32_t>(gt_vc) << 21;
      line |= static_cast<uint32_t>(tp) << 20;
      line |= static_cast<uint32_t>(pty) << 15;
      line |= static_cast<uint32_t>(ta) << 14;
      line |= static_cast<uint32_t>(ms) << 13;
      line |= static_cast<uint32_t>(b) << 10;
      line |= crc(line, offset_B);
      print_26_bits(line);

      line = 0;
      if (b == 0) {
        line |= static_cast<uint32_t>(af1) << 18;
        line |= static_cast<uint32_t>(af2) << 10;
      }
      // line |= static_cast<uint32_t>(af1) << 18;
      // line |= static_cast<uint32_t>(af2) << 10;
      line |= crc(line, offset_C);
      print_26_bits(line);

      line = 0;
      uint8_t c1 = static_cast<uint8_t>(ps[(b * 2)]);
      uint8_t c2 = static_cast<uint8_t>(ps[(b * 2) + 1]);
      line |= static_cast<uint32_t>(c1) << 18;
      line |= static_cast<uint32_t>(c2) << 10;
      line |= crc(line, offset_D);
      print_26_bits(line);
      // std::cout << std::endl;
    }
  }
};

// Function to handle Group 0A encoding
void encode_group_0A(const argsMap &args) {
  std::cout << "Encoding Group 0A..." << std::endl;
  std::cout << "Program Identification (PI): " << args.at("-pi") << std::endl;
  std::cout << "Program Type (PTY): " << args.at("-pty") << std::endl;
  std::cout << "Traffic Program (TP): " << args.at("-tp") << std::endl;
  std::cout << "Music/Speech (MS): " << args.at("-ms") << std::endl;
  std::cout << "Traffic Announcement (TA): " << args.at("-ta") << std::endl;
  std::cout << "Alternative Frequencies (AF): " << args.at("-af") << std::endl;
  std::cout << "Program Service (PS): " << args.at("-ps") << std::endl;
  std::cout << std::endl;
}

// Function to handle Group 2A encoding
void encodeGroup2A(const argsMap &args) {
  std::cout << "Encoding Group 2A..." << std::endl;
  std::cout << "Program Identification (PI): " << args.at("-pi") << std::endl;
  std::cout << "Program Type (PTY): " << args.at("-pty") << std::endl;
  std::cout << "Traffic Program (TP): " << args.at("-tp") << std::endl;
  std::cout << "Radio Text (RT): " << args.at("-rt") << std::endl;
  std::cout << "Radio Text A/B flag (AB): " << args.at("-ab") << std::endl;
  std::cout << std::endl;
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
      uint8_t f = static_cast<uint8_t>(frequency_int - 875);
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
      std::cerr << "Error: Invalid value\"" << flag_value
                << "\" (must be 0 or 1)\n";
      error = INVALID_VALUE;
      return -1;
    } else {
      value = flag_value == "1";
      return 0;
    }
  }

  int parse_string(const std::string &value, std::string &result, size_t length) {
    if (value.size() > length) {
      std::cerr << "Error: Invalid value " << value << " (must be at most "
                << length << " characters, is " << value.size() << ")\n";
      error = INVALID_VALUE;
      return -1;
    }
    // check if the string is only [a-zA-Z0-9]*
    if (!std::regex_match(value, std::regex("^[a-zA-Z0-9 ]*$"))) {
      std::cerr << "Error: Invalid value " << value
                << " (must be alphanumeric)\n";
      error = INVALID_VALUE;
      return -1;
    }
    result = value;
    // add padding to full length if needed
    result.append(length - value.size(), ' ');
    return 0;
  }

public:
  ArgumentParser(int argc, char *argv[]) : error(NO_ERROR) {
    if (argc != 13 && argc != 17) {
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
                        << ((1 << 16) - 1) << std::endl;
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
                        << ((1 << 5) - 1) << std::endl;
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
          if (parse_string(args.at("-ps"), ps, 8))
            break;
        } else if (flag == "-rt") {
          flags |= RT_FLAG;
          if (parse_string(args.at("-rt"), radio_text, 64))
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

  argsMap get_args() { return args; }
};

int main(int argc, char *argv[]) {
  std::string first_arg = argv[1];
  if (first_arg == "--help") {
    // TODO: more useful help message
    std::cout << "Usage: " << argv[0] << " -g <GroupID> [other flags...]\n";
    return 0;
  }
  auto parser = ArgumentParser(argc, argv);
  if (parser.get_error() != ArgumentParser::NO_ERROR) {
    // TODO: more useful help message
    std::cerr << "Usage: " << argv[0] << " -g <GroupID> [other flags...]\n";
    return 1;
  }

  if (parser.get_group_type() == GroupType::GROUP_0A) {
    Group0A group(parser.get_pi(), parser.get_pty(), parser.get_tp(),
                  parser.get_ms(), parser.get_ta(), parser.get_af1(),
                  parser.get_af2(), parser.get_ps());

    // encode_group_0A(parser.get_args());
    group.print_bits();

  } else if (parser.get_group_type() == GroupType::GROUP_2A) {
    Group2A group(parser.get_pi(), parser.get_pty(), parser.get_tp(),
                  parser.get_rt(), parser.get_ab());

    // encodeGroup2A(parser.get_args());
    group.print_bits();
  }

  return 0;
}
