/**
 * @file       common.hpp
 *
 * @author    Pavel Kratochvil \n
 *            Faculty of Information Technology \n
 *            Brno University of Technology \n
 *            xkrato61@fit.vutbr.cz
 *
 * @brief     Common header file for RDS encoder and decoder
 *            with shared constants and functions
 *
 * @date      23 November  2024 \n
 */

#pragma once
#include <cstdint>
#include <bitset>
#include <iostream>

/* Constants for offsets and masks used in CRC and data extraction */
const uint32_t offset_A = 252;
const uint32_t offset_B = 408;
const uint32_t offset_C = 360;
const uint32_t offset_D = 436;

const uint8_t group_type_code_0A = 0b00000; /* Code for Group 0A */
const uint8_t group_type_code_2A = 0b00100; /* Code for Group 2A */

/* Enum for RDS group types */
enum GroupType {
  GROUP_0A, /**< Group 0A */
  GROUP_2A, /**< Group 2A */
  UNKNOWN   /**< Unknown group type */
};

constexpr std::bitset<26> crc_bitset = 0b10110111001; /**< CRC polynomial */

/**
 * Compute CRC for a given value and offset.
 * @param value The input value.
 * @param offset The offset to apply in the computation.
 * @return The computed CRC as a 32-bit integer.
 */
uint32_t crc(uint32_t value, uint32_t offset);

/**
 * Print 26 bits of a value.
 * @param value The 32-bit value to process.
 */
void print_26_bits(uint32_t value);