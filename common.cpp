/**
 * @file       common.cpp
 *
 * @author    Pavel Kratochvil \n
 *            Faculty of Information Technology \n
 *            Brno University of Technology \n
 *            xkrato61@fit.vutbr.cz
 *
 * @brief     Common source file for RDS encoder and decoder.
 *
 * @date      23 November  2024 \n
 */

#include "common.hpp"

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

void print_26_bits(uint32_t value) {
  for (int i = 25; i >= 0; --i) {
    std::cout << ((value >> i) & 1);
  }
}
