/**
 * @file       rds_encoder.hpp
 *
 * @author    Pavel Kratochvil \n
 *            Faculty of Information Technology \n
 *            Brno University of Technology \n
 *            xkrato61@fit.vutbr.cz
 *
 * @brief     RDS encoder header file for groups 0A and 2A
 *
 * @date      23 November  2024 \n
 */

#pragma once

#include <algorithm>
#include <bitset>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "common.hpp"

const char *helpMessage = R"(
Usage: rds_encoder -g [GROUP] [FLAGS...]

Description:
  This program encodes RDS radio data for groups 0A and 2A with customizable settings.

Group Selection:
  -g 0A        Encode Group 0A (Basic tuning and switching information).
  -g 2A        Encode Group 2A (Extended program information).

Common Flags:
  -pi VALUE    Program Identification (16-bit unsigned integerm range: 0-65535).
               Example: -pi 12345

  -pty VALUE   Program Type (5-bit unsigned integer, range: 0-31).
               Example: -pty 4

  -tp VALUE    Traffic Program flag (boolean: 0 or 1).
               0: No traffic program, 1: Traffic program.
               Example: -tp 1

Group 0A-Specific Flags:
  -ms VALUE    Music/Speech flag (boolean: 0 or 1).
               0: Speech, 1: Music.
               Example: -ms 1

  -ta VALUE    Traffic Announcement flag (boolean: 0 or 1).
               0: No announcement, 1: Traffic announcement in progress.
               Example: -ta 0

  -af F1,F2    Alternative Frequencies (two comma-separated float values with precision to 0.1).
               Example: -af 104.5,98.0

  -ps STRING   Program Service name (8-character string).
               If shorter than 8 characters, it will be padded with spaces.
               Example: -ps RadioXYZ

Group 2A-Specific Flags:
  -rt STRING   Radio Text (up to 64-character string).
               If shorter than 8 characters, it will be padded with spaces.
               Example: -rt "Now playing: Song Title"

  -ab VALUE    Radio Text A/B flag (boolean: 0 or 1).
               0: A version of the text, 1: B version of the text.
               Example: -ab 0

Examples:
  Encode Group 0A with music and alternative frequencies:
    ./rds_encoder -g 0A -pi 12345 -pty 4 -tp 1 -ms 1 -ta 0 -af 104.5,98.0 -ps "RadioXYZ"

  Encode Group 2A with basic settings:
    ./rds_encoder -g 2A -pi 54321 -pty 10 -tp 0 -rt "Now Playing Song Title by Artist" -ab 0
)";

const std::regex frequency_regex(R"(^\d{2,3}\.\d$)");
const std::regex string_regex("^[a-zA-Z0-9 ]*$");

// no decimal point for comparison with integer
const double MIN_FREQUENCY = 876;
const double MAX_FREQUENCY = 1079;

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

/**
 * Type alias for argument map.
 */
using argsMap = std::unordered_map<std::string, std::string>;

/**
 * Abstract base class representing a common group structure.
 */
class CommonGroup {
public:
  /**
   * Constructor for CommonGroup.
   * @param gt_vc Group Type + Version Code.
   * @param program_identification Program Identification (PI).
   * @param traffic_program Traffic Program (TP) flag.
   * @param program_type Program Type (PTY).
   */
  CommonGroup(const uint8_t gt_vc, const uint16_t program_identification,
              const bool traffic_program, const uint8_t program_type)
      : gt_vc(gt_vc), pi(program_identification), tp(traffic_program),
        pty(program_type) {}
  /**
   * Virtual function to print bits of the group.
   */
  virtual void print_bits() = 0;

protected:
  uint8_t gt_vc; /**< Group Type Code + Version Code. */
  uint16_t pi;   /**< Program Identification. */
  bool tp;       /**< Traffic Program flag. */
  uint8_t pty;   /**< Program Type. */
};

/**
 * Class representing Group 2A structure.
 */
class Group2A : public CommonGroup {
private:
  std::string rt;         /**< Radio text (up to 64 characters). */
  bool ab;                /**< Radio Text A/B flag. */
  std::string radio_text; /**< Radio Text string. */

public:
  /**
   * Constructor for Group2A.
   * @param program_identification Program Identification (PI).
   * @param program_type Program Type (PTY).
   * @param traffic_program Traffic Program (TP) flag.
   * @param radio_text_string Radio Text string.
   * @param radio_text_a_b Radio Text A/B flag.
   */
  Group2A(const u_int16_t program_identification, const uint8_t program_type,
          const bool traffic_program, const std::string &radio_text_string,
          const bool radio_text_a_b)
      : CommonGroup(group_type_code_2A, program_identification, traffic_program,
                    program_type),
        rt(radio_text_string), ab(radio_text_a_b),
        radio_text(radio_text_string) {}

  /**
   * Override function to print bits of Group 2A.
   */
  void print_bits() override;
};

/**
 * Class representing Group 0A structure.
 */
class Group0A : public CommonGroup {
private:
  bool ms;        /**< Music/Speech flag. */
  bool ta;        /**< Traffic Announcement flag. */
  uint8_t af1;    /**< Alternative Frequency 1. */
  uint8_t af2;    /**< Alternative Frequency 2. */
  std::string ps; /**< Program Service string (up to 8 characters). */

public:
  /**
   * Constructor for Group0A.
   * @param program_identification Program Identification (PI).
   * @param program_type Program Type (PTY).
   * @param traffic_program Traffic Program (TP) flag.
   * @param music_speech Music/Speech flag.
   * @param traffic_announcement Traffic Announcement flag.
   * @param af1 Alternative Frequency 1.
   * @param af2 Alternative Frequency 2.
   * @param program_service Program Service string.
   */
  Group0A(const uint16_t program_identification, const uint8_t program_type,
          const bool traffic_program, const bool music_speech,
          const bool traffic_announcement, const uint8_t af1, const uint8_t af2,
          const std::string &program_service)
      : CommonGroup(group_type_code_0A, program_identification, traffic_program,
                    program_type),
        ms(music_speech), ta(traffic_announcement), af1(af1), af2(af2),
        ps(program_service) {}

  /**
   * Function to print bits of Group 0A.
   */
  void print_bits() override;
};

/**
 * Class for parsing command-line arguments and validating them based on group
 * type requirements.
 */
class ArgumentParser {
public:
  /**
   * Enum representing various error types during argument parsing.
   */
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

  argsMap args;        /**< Map storing parsed arguments. */
  GroupType groupType; /**< Parsed group type. */
  Error error;         /**< Error status during parsing. */

  /* Common fields */
  uint16_t pi; /**< Program Identification (PI). */
  uint8_t pty; /**< Program Type (PTY). */
  bool tp;     /**< Traffic Program flag. */

  /* Fields specific to Group 0A */
  std::string alternative_frequencies;
  bool ms;
  bool ta;
  uint8_t af1;
  uint8_t af2;
  std::string ps;

  /* Fields specific to Group 2A */
  std::string rt;
  bool ab;

public:
  /**
   * Constructor for ArgumentParser that processes command-line arguments.
   * @param argc Argument count from main().
   * @param argv Argument values from main().
   */
  ArgumentParser(int argc, char *argv[]);
  
  /**
   * @brief Parses the alternative frequencies string and validates each frequency.
   * 
   * This function processes a comma-separated string of alternative frequencies,
   * validates their format and range, and converts them to an 8-bit format as per
   * the RDS standard. If any frequency is invalid or if there are not exactly two
   * frequencies, an error is set.
   * 
   * @note The function expects the frequencies to be in a specific format and range.
   * 
   * Error conditions:
   * - More than two frequencies are provided.
   * - Frequency format does not match the expected regex.
   * - Frequency is out of the valid range.
   * - Less than two valid frequencies are provided.
   */
  void parse_frequencies();

  /**
   * Parses a boolean value from a string flag.
   *
   * @param flag_value The string representation of the boolean value ("0" or "1").
   * @param value Reference to a boolean variable where the parsed value will be stored.
   * @return 0 if parsing is successful, -1 if the input is invalid.
   */
  int parse_boolean(const std::string &flag_value, bool &value);
  
  /**
   * @brief Parses a string value, validates its length and content, and stores the result.
   *
   * This function checks if the input string `value` does not exceed the specified `length`
   * and contains only alphanumeric characters. If the string is valid, it is stored in `result`
   * and padded with spaces to match the specified length. If the string is invalid, an error
   * message is printed to `std::cerr` and an error code is set.
   *
   * @param value The input string to be parsed and validated.
   * @param result The output string where the validated and padded result is stored.
   * @param length The maximum allowed length for the input string.
   * @return 0 if the string is valid, -1 if the string is invalid.
   */
  int parse_string(const std::string &value, std::string &result,
                   size_t length);
};
