/**
 * @file       rds_encoder.cpp
 *
 * @author    Pavel Kratochvil \n
 *            Faculty of Information Technology \n
 *            Brno University of Technology \n
 *            xkrato61@fit.vutbr.cz
 *
 * @brief     RDS encoder for groups 0A and 2A
 *
 * @date      23 November  2024 \n
 */

#include "rds_encoder.hpp"

void Group2A::print_bits() {
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

void Group0A::print_bits() {
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
    line |= crc(line, offset_C);
    print_26_bits(line);

    line = 0;
    uint8_t c1 = static_cast<uint8_t>(ps[(b * 2)]);
    uint8_t c2 = static_cast<uint8_t>(ps[(b * 2) + 1]);
    line |= static_cast<uint32_t>(c1) << 18;
    line |= static_cast<uint32_t>(c2) << 10;
    line |= crc(line, offset_D);
    print_26_bits(line);
  }
}

void ArgumentParser::parse_frequencies() {
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

int ArgumentParser::parse_boolean(const std::string &flag_value, bool &value) {
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

int ArgumentParser::parse_string(const std::string &value, std::string &result,
                                 size_t length) {
  if (value.size() > length) {
    std::cerr << "Error: Invalid value " << value << " (must be at most "
              << length << " characters, is " << value.size() << ")\n";
    error = INVALID_VALUE;
    return -1;
  }
  // check if the string is only [a-zA-Z0-9]*
  if (!std::regex_match(value, string_regex)) {
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

ArgumentParser::ArgumentParser(int argc, char *argv[]) : error(NO_ERROR) {
  if (argc != 13 && argc != 17) {
    error = ARGUMENT_COUNT;
  }
  std::string groupID;
  unsigned int flags = 0;
  // Iterate over command-line arguments and parse flags
  for (int i = 1; i < argc; ++i) {
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
        groupType = GroupType::GROUP_0A;
      } else if (groupID == "2A") {
        groupType = GroupType::GROUP_2A;
      } else {
        std::cerr << "Error: Invalid group ID " << groupID << "\n";
        error = UNSUPPORTED_GROUP;
        break;
      }
    } else if (i + 1 < argc) {
      args[flag] = argv[++i];
      if (flag == "-pi") {
        flags |= PI_FLAG;
        int pi_int_tmp{};
        try {
          pi_int_tmp = std::stoi(args.at("-pi"));
          if (pi_int_tmp < 0 || pi_int_tmp > 65535) {
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
        if (parse_string(args.at("-rt"), rt, 64))
          break;
      } else if (flag == "-ab") {
        flags |= AB_FLAG;
        if (parse_boolean(args.at("-ab"), ab))
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
  if (parser.error != ArgumentParser::NO_ERROR) {
    std::cout << helpMessage;
    return 1;
  }

  if (parser.groupType == GroupType::GROUP_0A) {
    Group0A group(parser.pi, parser.pty, parser.tp, parser.ms, parser.ta,
                  parser.af1, parser.af2, parser.ps);
    group.print_bits();

  } else if (parser.groupType == GroupType::GROUP_2A) {
    Group2A group(parser.pi, parser.pty, parser.tp, parser.rt, parser.ab);
    group.print_bits();
  }

  return 0;
}
