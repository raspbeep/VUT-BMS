/**
 * @file       rds_decoder.hpp
 *
 * @author    Pavel Kratochvil \n
 *            Faculty of Information Technology \n
 *            Brno University of Technology \n
 *            xkrato61@fit.vutbr.cz
 *
 * @brief     RDS decoder header file for groups 0A and 2A
 *
 * @date      23 November  2024 \n
 */

#pragma once

#include <algorithm>
#include <bitset>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "common.hpp"

const char *helpMessage = R"(
Usage: ./rds_encoder -b BINARY_STRING

Description:
  This program decodes RDS data from a binary string and display the information for Group 0A or 2A.
)";

// Constants for group type codes and masks
const uint8_t group_type_mask = 0b11111; // Mask for group type

const uint32_t pi_mask = 67107840;
const uint32_t gt_vc_mask = 65011712;
const uint32_t tp_mask = 1048576;
const uint32_t pty_mask = 1015808;
const uint32_t ta_mask = 16384;
const uint32_t ms_mask = 8192;
const uint32_t di_mask = 4096;
const uint32_t segment_mask_0A = 3072;
const uint32_t segment_mask_2A = 15360;
const uint32_t af1_mask = 66846720;
const uint32_t af2_mask = 261120;
const uint32_t c1_mask = 66846720;
const uint32_t c2_mask = 261120;

/**
 * Determines the RDS group type from the first block of data.
 * @param block The 32-bit block value
 */
GroupType get_group(uint32_t block);

/**  */
/**
 * Formats a frequency value into a human-readable string.
 * @param frequency The 32-bit frequency value.
 */
std::string format_frequency(uint32_t frequency);

/** Trims trailing spaces from a string. */
std::string trim_space_end(std::string input);

/**
 * Parses command-line arguments and validates input.
 */
class ArgumentParser {
public:
  enum Error {
    NO_ERROR,       /**< No error occurred */
    ARGUMENT_COUNT, /**< Incorrect number of arguments */
    INVALID_FLAG,   /**< Invalid flag provided */
    INVALID_VALUE   /**< Invalid binary string value */
  };
  std::vector<uint32_t> blocks;    /**< Parsed blocks of data */

private:
  Error error;                     /**< Stores parsing errors */
  std::string binary_string_value; /**< Binary string input from arguments */

public:
  /** Constructor that parses command-line arguments. */
  ArgumentParser(int argc, char *argv[]);

  int sort_blocks();

  /** Returns the error status of the parser. */
  Error get_error() { return error; }

  /** Returns the parsed blocks of data. */
  std::vector<uint32_t> get_blocks() { return blocks; }
};

/**
 * Represents common fields shared across different RDS groups.
 */
class CommonGroup {
public:
  /** Constructor that initializes the group with data blocks. */
  CommonGroup(std::vector<uint32_t> data) : mData(data) {}

  /** Virtual function to print group-specific information. */
  virtual void print_info() = 0;

  std::vector<uint32_t> mData; // Data blocks for the group
protected:
  uint8_t gt_vc; /**< Group Type Code + Version Code */
  uint16_t pi;   /**< Program Identification code */
  bool tp;       /**< Traffic Program flag */
  uint8_t pty;   /**< Program Type code */
};

/**
 * Handles parsing and displaying information for Group 2A (Radio Text).
 */
class Group2A : public CommonGroup {
private:
  std::string rt;  /**< Radio text (up to 64 characters) */
  bool ab;         /**< Radio text A/B flag */
  uint8_t segment; /**< Segment address code */

public:
  /** Constructor initializing with data blocks. */
  Group2A(std::vector<uint32_t> data) : CommonGroup(data), rt(64, ' ') {}

  /**
   * Sorts groups into a new vector with empty places for absent ones
   * @param data Vector containing all received data
   * @returns sorted data of 16 groups (4 blocks each)
   */
  void sort_2A_data(); 

  /** Prints information specific to Group 2A. */
  void print_info() override;

  /** Parses the data blocks for Group 2A. */
  int parse();
};

/**
 * Handles parsing and displaying information for Group 0A.
 */
class Group0A : public CommonGroup {
private:
  bool ta;         /**< Traffic Announcement flag */
  bool ms;         /**< Music/Speech indicator */
  uint8_t di;      /**< Decoder Information control code */
  uint8_t segment; /**< Segment address code */
  uint32_t af1;    /**< Alternative Frequency #1 */
  uint32_t af2;    /**< Alternative Frequency #2 */
  std::string ps;  /**< Program Service name (up to 8 characters) */

public:
  /** Constructor initializing with data blocks. */
  Group0A(std::vector<uint32_t> data) : CommonGroup(data), ps(8, ' ') {}

  /**
   * Sorts groups into a new vector with empty places for absent ones
   * @param data Vector containing all received data
   * @returns sorted data of 4 groups (4 blocks each)
   */
  void sort_0A_data();

  /** Prints information specific to Group 0A. */
  void print_info() override;

  /** Parses the data blocks for Group 0A. */
  int parse();
};
