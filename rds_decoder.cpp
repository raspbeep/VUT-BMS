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

bool is_group_empty(std::vector<uint32_t>& data, int idx) {
  // unpack each block and check if they are zero
  uint32_t b0 = data[idx * 4];
  uint32_t b1 = data[idx * 4 + 1];
  uint32_t b2 = data[idx * 4 + 2];
  uint32_t b3 = data[idx * 4 + 3];

  if (!b0 && !b1 && !b2 && !b3) return true;
  return false;
}

int get_block_addr(uint32_t block) {
  uint32_t check_crc{};
  check_crc = crc((block & inv_crc_mask), offset_A);
  if (check_crc == (block & crc_mask)) return 0;

  check_crc = crc((block & inv_crc_mask), offset_B);
  if (check_crc == (block & crc_mask)) return 1;

  check_crc = crc((block & inv_crc_mask), offset_C);
  if (check_crc == (block & crc_mask)) return 2;

  check_crc = crc((block & inv_crc_mask), offset_D);
  if (check_crc == (block & crc_mask)) return 3;

  return -1;
}

int sort_blocks(std::vector<uint32_t>& data) {
  std::vector<uint32_t> output_data(data.size());
  
  int block_addr{};

  // iterate over groups
  for (int i = 0; i < static_cast<int>(data.size()) / 4; i ++) {
    // skip empty groups
    if (is_group_empty(data, i)) continue;
    uint32_t block;
    for (int j = 0; j < 4; j++) {
      block = data[i * 4 + j];
      block_addr = get_block_addr(block);
      // if no CRC passes return error
      if (block_addr == -1) return 2;
      output_data[i * 4 + block_addr] = block;
    }
  }
  data = output_data;
  return 0;
}

void Group2A::sort_2A_data() {
  // 16 groups of 4 blocks, initiated to 0
  std::vector<uint32_t> output_data(16 * 4);
  
  // iterate over 4 groups
  for (int i = 0; i <  static_cast<int>(mData.size()) / 4; i++) {
    // look into the 2nd block of each group
    uint32_t second_block = mData[i * 4 + 1];
    uint8_t segment_address = (second_block & segment_mask_2A) >> 10;
    // assign to the correct address in the output
    output_data[segment_address * 4] = mData[i * 4];
    output_data[segment_address * 4 + 1] = mData[i * 4 + 1];
    output_data[segment_address * 4 + 2] = mData[i * 4 + 2];
    output_data[segment_address * 4 + 3] = mData[i * 4 + 3];
  }

  mData = output_data;
}

void Group0A::sort_0A_data() {
  // 4 groups of 4 blocks, initiated to 0
  std::vector<uint32_t> output_data(4 * 4);
  
  // iterate over groups (each group has 4 blocks)
  for (int i = 0; i < static_cast<int>(mData.size()) / 4; i++) {
    // look into the 2nd block of each group
    uint32_t second_block = mData[i * 4 + 1];
    uint8_t segment_address = (second_block & segment_mask_0A) >> 10;
    // assign to the correct address in the output
    output_data[segment_address * 4] = mData[i * 4];
    output_data[segment_address * 4 + 1] = mData[i * 4 + 1];
    output_data[segment_address * 4 + 2] = mData[i * 4 + 2];
    output_data[segment_address * 4 + 3] = mData[i * 4 + 3];
  }

  mData = output_data;
}

ArgumentParser::ArgumentParser(int argc, char *argv[]) : error(NO_ERROR) {
  if (argc != 3) {
    error = ARGUMENT_COUNT;
    std::cout << helpMessage;
    return;
  }

  if (std::string(argv[1]) != "-b") {
    std::cout << "Invalid flag: " << argv[1] << std::endl;
    error = INVALID_FLAG;
    return;
  }
  binary_string_value = argv[2];
  if (binary_string_value.size() % 104 != 0 || !binary_string_value.size()) {
    std::cout << "Invalid length of binary value (length: " << binary_string_value.size() << ")"
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

int Group2A::parse() {
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
  bool first_valid_group = true;

  for (int group = 0; group < 16; group++) {
    // if the group is empty, do not process it
    if (is_group_empty(mData, group)) continue;

    // block 0
    block = mData[static_cast<size_t>(group) * 4];
    tmp_pi = (block & pi_mask) >> 10;

    // block 1
    block = mData[static_cast<size_t>(group) * 4 + 1];
    tmp_gt_vc = (block & gt_vc_mask) >> 21;
    tmp_tp = (block & tp_mask) >> 20;
    tmp_pty = (block & pty_mask) >> 15;
    tmp_ab = (block & ta_mask) >> 14;
    tmp_segment = (block & segment_mask_2A) >> 10;

    // block 2
    block = mData[static_cast<size_t>(group) * 4 + 2];
    tmp_c1 = (block & c1_mask) >> 18;
    tmp_c2 = (block & c2_mask) >> 10;
    // fill the characters into the string
    rt[group * 4] = static_cast<char>(tmp_c1);
    rt[group * 4 + 1] = static_cast<char>(tmp_c2);

    // block 3
    block = mData[static_cast<size_t>(group) * 4 + 3];
    tmp_c3 = (block & c1_mask) >> 18;
    tmp_c4 = (block & c2_mask) >> 10;
    // fill the characters into the string
    rt[group * 4 + 2] = static_cast<char>(tmp_c3);
    rt[group * 4 + 3] = static_cast<char>(tmp_c4);

    // first group, save values
    if (first_valid_group) {
      first_valid_group = false;
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

void Group2A::print_info() {
  std::cout << "PI: " << pi << std::endl;
  std::cout << "GT: " << "2A" << std::endl;
  std::cout << "TP: " << tp << std::endl;
  std::cout << "PTY: " << (int)pty << std::endl;
  std::cout << "A/B: " << ab << std::endl;
  std::cout << "RT: \"" << trim_space_end(rt) << "\"" << std::endl;
}

int Group0A::parse() {
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
  uint32_t block{};

  bool first_valid_group = true;

  // iterate over 4 groups, some might be empty
  for (int group = 0; group < 4; group++) {
    // if the group is empty, do not process it
    if (is_group_empty(mData, group)) continue;

    // block 0
    block = mData[static_cast<size_t>(group) * 4];
    tmp_pi = (block & pi_mask) >> 10;

    // block 1
    block = mData[static_cast<size_t>(group) * 4 + 1];
    tmp_gt_vc = (block & gt_vc_mask) >> 21;
    tmp_tp = (block & tp_mask) >> 20;
    tmp_pty = (block & pty_mask) >> 15;
    tmp_ta = (block & ta_mask) >> 14;
    tmp_ms = (block & ms_mask) >> 13;
    tmp_di = (block & di_mask) >> 12;
    tmp_segment = (block & segment_mask_0A) >> 10;

    // block 2
    block = mData[static_cast<size_t>(group) * 4 + 2];
    tmp_af1 = (block & af1_mask) >> 18;
    tmp_af2 = (block & af2_mask) >> 10;

    // block 3
    block = mData[static_cast<size_t>(group) * 4 + 3];
    tmp_c1 = (block & c1_mask) >> 18;
    tmp_c2 = (block & c2_mask) >> 10;
    // fill the characters into the string
    ps[group * 2] = static_cast<char>(tmp_c1);
    ps[group * 2 + 1] = static_cast<char>(tmp_c2);

    // first group, save values
    if (first_valid_group) {
      first_valid_group = false;
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
        std::cout << "Inconsistent PI value across blocks" << std::endl;
        return 1;
      }
      if (tmp_gt_vc != gt_vc) {
        std::cout << "Inconsistent Group Type or Version Code value across blocks"
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
      if (tmp_ta != ta) {
        std::cout << "Inconsistent TA value across blocks" << std::endl;
        return 1;
      }
      if (tmp_ms != ms) {
        std::cout << "Inconsistent MS value across blocks" << std::endl;
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

  int sort_res = sort_blocks(parser.blocks);
  if (sort_res != 0) return sort_res;

  // check the group type from the first block
  GroupType groupType = get_group(parser.blocks[1]);
  if (groupType == GROUP_0A) {
    Group0A group0A(parser.blocks);
    group0A.sort_0A_data();
    int ret = group0A.parse();
    if (ret != 0) return ret;
    
    group0A.print_info();

  } else if (groupType == GROUP_2A) {
    Group2A group2A(parser.get_blocks());
    group2A.sort_2A_data();
    int ret = group2A.parse();
    if (ret != 0) return ret;
    group2A.print_info();
  } else {
    std::cout << "Unsupported group type" << std::endl;
    return 1;
  }
  return 0;
}
