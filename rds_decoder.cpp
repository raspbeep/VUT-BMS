/**
 * @file       rds_decoder.cpp
 *
 * @author    Pavel Kratochvil \n
 *            Faculty of Information Technology \n
 *            Brno University of Technology \n
 *            xkrato61@fit.vutbr.cz
 *
 * @brief     RDS decoder for groups 0A and 2A
 *
 * @date      23 November  2024 \n
 */

#include "rds_decoder.hpp"

GroupType get_group(uint32_t block) {
  std::bitset<26> group_0A_bitset = group_type_code_0A << 21;
  std::bitset<26> group_2A_bitset = group_type_code_2A << 21;
  std::bitset<26> group_mask_bitset = group_type_mask << 21;
  std::bitset<26> block_bitset = block;

  // mask the first 21 bits
  std::bitset<26> block_bitset_masked = block_bitset & group_mask_bitset;

  if ((group_0A_bitset | block_bitset_masked) == group_0A_bitset) {
    return GROUP_0A;
  } else if ((group_2A_bitset | block_bitset_masked) == group_2A_bitset) {
    return GROUP_2A;
  } else {
    return UNKNOWN;
  }
}

ArgumentParser::ArgumentParser(int argc, char *argv[]) : error(NO_ERROR) {
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
    std::cout << "Invalid length of binary value " << binary_string_value.size()
              << std::endl;
    error = INVALID_VALUE;
    return;
  }

  for (size_t g = 0; g < binary_string_value.size() / 104; g++) {
    for (size_t i = 0; i < 4; i++) {
      uint32_t value = 0;
      for (size_t b = 0; b < 26; b++) {
        char c = binary_string_value[(g * 104) + (i * 26) + b];
        if (c != '0' && c != '1') {
          std::cout << "Invalid character in binary value: " << c << std::endl;
          error = INVALID_VALUE;
          return;
        }
        value |= (c == '1') ? 0b1 : 0b0;
        if (b < 25)
          value <<= 1;
      }
      blocks.push_back(value);
    }
  }
}

std::string format_frequency(uint32_t frequency) {
  std::string freq_str = std::to_string(frequency + 875);
  freq_str.insert(freq_str.size() - 1, ".");
  return freq_str;
}

std::string trim_space_end(std::string input) {
  std::string output;
  bool trim = true;
  for (int i = static_cast<int>(input.size()) - 1; i >= 0; i--) {
    if (input[static_cast<size_t>(i)] != ' ')
      trim = false;
    if (!trim)
      output.append(1, input[static_cast<size_t>(i)]);
  }
  std::reverse(output.begin(), output.end());
  return output;
}

void Group2A::print_info() {
  std::cout << "PI: " << pi << std::endl;
  std::cout << "GT: " << "2A" << std::endl;
  std::cout << "TP: " << tp << std::endl;
  std::cout << "PTY: " << (int)pty << std::endl;
  std::cout << "A/B: " << ab << std::endl;
  std::cout << "RT: \"" << trim_space_end(rt) << "\"" << std::endl;
}

int Group2A::parse() {
  if (mData.size() != 64) {
    std::cout << "Invalid number of blocks for group 2A" << std::endl;
    return 1;
  }

  uint16_t tmp_pi{};
  uint8_t tmp_gt_vc{};
  bool tmp_tp{};
  uint8_t tmp_pty{};
  bool tmp_ab{};
  uint8_t tmp_segment{};

  uint8_t tmp_c1{};
  uint8_t tmp_c2{};
  uint8_t tmp_c3{};
  uint8_t tmp_c4{};

  uint32_t block{};
  uint32_t check_crc{};

  for (int group = 0; group < 16; group++) {
    block = mData[static_cast<size_t>(group) * 4];
    check_crc = crc((block & inv_crc_mask), offset_A);
    if (check_crc != (block & crc_mask)) {
      std::cout << "CRC check failed for group: " << group << " block 0"
                << std::endl;
      return 2;
    }
    tmp_pi = (block & pi_mask) >> 10;

    block = mData[static_cast<size_t>(group) * 4 + 1];
    check_crc = crc((block & inv_crc_mask), offset_B);
    if (check_crc != (block & crc_mask)) {
      std::cout << "CRC check failed for group: " << group << " block 1"
                << std::endl;
      return 2;
    }
    tmp_gt_vc = (block & gt_vc_mask) >> 21;
    tmp_tp = (block & tp_mask) >> 20;
    tmp_pty = (block & pty_mask) >> 15;
    tmp_ab = (block & ta_mask) >> 14;
    tmp_segment = (block & segment_mask) >> 10;

    block = mData[static_cast<size_t>(group) * 4 + 2];
    check_crc = crc((block & inv_crc_mask), offset_C);
    if (check_crc != (block & crc_mask)) {
      std::cout << "CRC check failed for group: " << group << " block 2"
                << std::endl;
      return 2;
    }
    tmp_c1 = (block & c1_mask) >> 18;
    tmp_c2 = (block & c2_mask) >> 10;

    block = mData[static_cast<size_t>(group) * 4 + 3];
    check_crc = crc((block & inv_crc_mask), offset_D);
    if (check_crc != (block & crc_mask)) {
      std::cout << "CRC check failed for group: " << group << " block 3"
                << std::endl;
      return 2;
    }
    tmp_c3 = (block & c1_mask) >> 18;
    tmp_c4 = (block & c2_mask) >> 10;

    // append the characters to the string
    rt.append(1, static_cast<char>(tmp_c1));
    rt.append(1, static_cast<char>(tmp_c2));
    rt.append(1, static_cast<char>(tmp_c3));
    rt.append(1, static_cast<char>(tmp_c4));

    // first group, save values
    if (group == 0) {
      pi = tmp_pi;
      gt_vc = tmp_gt_vc;
      tp = tmp_tp;
      pty = tmp_pty;
      ab = tmp_ab;
      segment = tmp_segment;
    } else {
      // compare the values with the saved ones
      if (tmp_pi != pi) {
        std::cout << "Inconsistent PI value across blocks" << std::endl;
        return 1;
      }
      if (tmp_gt_vc != gt_vc) {
        std::cout
            << "Inconsistent Group Type or Version Code value across blocks"
            << std::endl;
        return 1;
      }
      if (tmp_tp != tp) {
        std::cout << "Inconsistent TP value across blocks" << std::endl;
        return 1;
      }
      if (tmp_pty != pty) {
        std::cout << "Inconsistent PTY value across blocks" << std::endl;
        return 1;
      }
    }
  }
  return 0;
}

void Group0A::print_info() {
  std::cout << "PI: " << pi << std::endl;
  std::cout << "GT: " << "0A" << std::endl;
  std::cout << "TP: " << tp << std::endl;
  std::cout << "PTY: " << (int)pty << std::endl;
  std::cout << "TA: " << (ta ? "Active" : "Inactive") << std::endl;
  std::cout << "MS: " << (ms ? "Music" : "Speech") << std::endl;
  std::cout << "DI: " << (int)di << std::endl;
  std::cout << "AF: " << format_frequency(af1) << ", " << format_frequency(af2)
            << std::endl;
  std::cout << "PS: \"" << trim_space_end(ps) << "\"" << std::endl;
}

int Group0A::parse() {
  if (mData.size() != 16) {
    std::cout << "Invalid number of blocks for group 0A" << std::endl;
    return 1;
  }

  uint16_t tmp_pi{};
  uint8_t tmp_gt_vc{};
  bool tmp_tp{};
  uint8_t tmp_pty{};
  bool tmp_ta{};
  bool tmp_ms{};
  uint8_t tmp_di{};
  uint8_t tmp_segment{};
  uint32_t tmp_af1{};
  uint32_t tmp_af2{};

  uint8_t tmp_c1{};
  uint8_t tmp_c2{};

  for (int group = 0; group < 4; group++) {
    uint32_t block0 = mData[static_cast<size_t>(group) * 4];
    uint32_t check_crc0 = crc((block0 & inv_crc_mask), offset_A);
    if (check_crc0 != (block0 & crc_mask)) {
      std::cout << "CRC check failed for group: " << group << " block 0"
                << std::endl;
      return 2;
    }
    tmp_pi = (block0 & pi_mask) >> 10;

    uint32_t block1 = mData[static_cast<size_t>(group) * 4 + 1];
    uint32_t check_crc1 = crc((block1 & inv_crc_mask), offset_B);
    if (check_crc1 != (block1 & crc_mask)) {
      std::cout << "CRC check failed for group: " << group << " block 1"
                << std::endl;
      return 2;
    }
    tmp_gt_vc = (block1 & gt_vc_mask) >> 21;
    tmp_tp = (block1 & tp_mask) >> 20;
    tmp_pty = (block1 & pty_mask) >> 15;
    tmp_ta = (block1 & ta_mask) >> 14;
    tmp_ms = (block1 & ms_mask) >> 13;
    tmp_di = (block1 & di_mask) >> 12;
    tmp_segment = (block1 & segment_mask) >> 10;

    uint32_t block2 = mData[static_cast<size_t>(group) * 4 + 2];
    uint32_t check_crc2 = crc((block2 & inv_crc_mask), offset_C);
    if (check_crc2 != (block2 & crc_mask)) {
      std::cout << "CRC check failed for group: " << group << " block 2"
                << std::endl;
      return 2;
    }
    tmp_af1 = (block2 & af1_mask) >> 18;
    tmp_af2 = (block2 & af2_mask) >> 10;

    uint32_t block3 = mData[static_cast<size_t>(group) * 4 + 3];
    uint32_t check_crc3 = crc((block3 & inv_crc_mask), offset_D);
    if (check_crc3 != (block3 & crc_mask)) {
      std::cout << "CRC check failed for group: " << group << " block 3"
                << std::endl;
      return 2;
    }
    tmp_c1 = (block3 & c1_mask) >> 18;
    tmp_c2 = (block3 & c2_mask) >> 10;

    // append the characters to the string
    ps.append(1, static_cast<char>(tmp_c1));
    ps.append(1, static_cast<char>(tmp_c2));

    // first group, save values
    if (group == 0) {
      pi = tmp_pi;
      gt_vc = tmp_gt_vc;
      tp = tmp_tp;
      pty = tmp_pty;
      ta = tmp_ta;
      ms = tmp_ms;
      di = tmp_di;
      segment = tmp_segment;
      af1 = tmp_af1;
      af2 = tmp_af2;
    } else {
      // compare the values with the saved ones
      if (tmp_pi != pi) {
        std::cout << "Inconsistent PI value" << std::endl;
        return 1;
      }
      if (tmp_gt_vc != gt_vc) {
        std::cout << "Inconsistent Group Type or Version Code value"
                  << std::endl;
        return 1;
      }
      if (tmp_tp != tp) {
        std::cout << "Inconsistent TP value" << std::endl;
        return 1;
      }
      if (tmp_pty != pty) {
        std::cout << "Inconsistent PTY value" << std::endl;
        return 1;
      }
      if (tmp_ta != ta) {
        std::cout << "Inconsistent TA value" << std::endl;
        return 1;
      }
      if (tmp_ms != ms) {
        std::cout << "Inconsistent MS value" << std::endl;
        return 1;
      }
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    std::cout << helpMessage;
    return 1;
  }
  std::string first_arg = argv[1];
  if (first_arg == "--help") {
    std::cout << helpMessage;
    return 0;
  }

  auto parser = ArgumentParser(argc, argv);
  if (parser.get_error() != ArgumentParser::NO_ERROR) {
    return 1;
  }

  // check the group type from the first block
  GroupType groupType = get_group(parser.get_blocks()[1]);

  if (groupType == GROUP_0A) {
    Group0A group0A(parser.get_blocks());
    int ret = group0A.parse();
    if (ret != 0)
      return ret;
    group0A.print_info();

  } else if (groupType == GROUP_2A) {
    Group2A group2A(parser.get_blocks());
    int ret = group2A.parse();
    if (ret != 0)
      return ret;
    group2A.print_info();
  } else {
    std::cout << "Unsupported group type" << std::endl;
    return 1;
  }
  return 0;
}
