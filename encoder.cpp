#include <iostream>
#include <cstdio>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <iomanip>
#include <unordered_map>

enum GroupType {
    GROUP_0A,
    GROUP_2A
};

using ushort = unsigned short;
using argsMap = std::map<std::string, std::string>;
using GroupTypeMap = std::unordered_map<GroupType, std::array<ushort, 4>>;
using VersionCodeMap = std::unordered_map<GroupType, std::array<ushort, 1>>;
using OffsetMap = std::unordered_map<std::string, std::array<ushort, 10>>;
using BlockArray = std::array<ushort, 104>;
using GroupBitVector = std::vector<BlockArray>;
using RadioText = std::vector<std::array<ushort, 8>>;

GroupTypeMap group_type_code_map = {
    {GROUP_0A, {0, 0, 0, 0}},
    {GROUP_2A, {0, 0, 1, 0}}
};

VersionCodeMap version_code_map = {
    {GROUP_0A, {0}},
    {GROUP_2A, {1}}
};

OffsetMap offset_map = {
    {"A", {0,0,1,1,1,1,1,1,0,0}},
    {"B", {0,1,1,0,0,1,1,0,0,0}},
    {"C", {0,1,0,1,1,0,1,0,0,0}},
    {"D", {0,1,1,0,1,1,0,1,0,0}},
};

class EncoderHelper {
public:
    const std::vector<std::string> all_frequencies;
    const std::unordered_map<std::string, std::array<ushort, 8>> frequency_codes_map;
    const std::unordered_map<char, std::array<ushort, 8>> character_encoding_map = {
        {'\r',{0, 0, 0, 0, 1, 1, 0, 1}}, // radio text ending 0D (hex) if length < 64
        {' ', {0, 0, 1, 0, 0, 0, 0, 0}},

        {'0', {0, 0, 1, 1, 0, 0, 0, 0}},
        {'1', {0, 0, 1, 1, 0, 0, 0, 1}},
        {'2', {0, 0, 1, 1, 0, 0, 1, 0}},
        {'3', {0, 0, 1, 1, 0, 0, 1, 1}},
        {'4', {0, 0, 1, 1, 0, 1, 0, 0}},
        {'5', {0, 0, 1, 1, 0, 1, 0, 1}},
        {'6', {0, 0, 1, 1, 0, 1, 1, 0}},
        {'7', {0, 0, 1, 1, 0, 1, 1, 1}},
        {'8', {0, 0, 1, 1, 1, 0, 0, 0}},
        {'9', {0, 0, 1, 1, 1, 0, 0, 1}},

        {'A', {0, 1, 0, 0, 0, 0, 0, 1}},
        {'B', {0, 1, 0, 0, 0, 0, 1, 0}},
        {'C', {0, 1, 0, 0, 0, 0, 1, 1}},
        {'D', {0, 1, 0, 0, 0, 1, 0, 0}},
        {'E', {0, 1, 0, 0, 0, 1, 0, 1}},
        {'F', {0, 1, 0, 0, 0, 1, 1, 0}},
        {'G', {0, 1, 0, 0, 0, 1, 1, 1}},
        {'H', {0, 1, 0, 0, 1, 0, 0, 0}},
        {'I', {0, 1, 0, 0, 1, 0, 0, 1}},
        {'J', {0, 1, 0, 0, 1, 0, 1, 0}},
        {'K', {0, 1, 0, 0, 1, 0, 1, 1}},
        {'L', {0, 1, 0, 0, 1, 1, 0, 0}},
        {'M', {0, 1, 0, 0, 1, 1, 0, 1}},
        {'N', {0, 1, 0, 0, 1, 1, 1, 0}},
        {'O', {0, 1, 0, 0, 1, 1, 1, 1}},

        {'P', {0, 1, 0, 1, 0, 0, 0, 0}},
        {'Q', {0, 1, 0, 1, 0, 0, 0, 1}},
        {'R', {0, 1, 0, 1, 0, 0, 1, 0}},
        {'S', {0, 1, 0, 1, 0, 0, 1, 1}},
        {'T', {0, 1, 0, 1, 0, 1, 0, 0}},
        {'U', {0, 1, 0, 1, 0, 1, 0, 1}},
        {'V', {0, 1, 0, 1, 0, 1, 1, 0}},
        {'W', {0, 1, 0, 1, 0, 1, 1, 1}},
        {'X', {0, 1, 0, 1, 1, 0, 0, 0}},
        {'Y', {0, 1, 0, 1, 1, 0, 0, 1}},
        {'Z', {0, 1, 0, 1, 1, 0, 1, 0}},

        {'a', {0, 1, 1, 0, 0, 0, 0, 1}},
        {'b', {0, 1, 1, 0, 0, 0, 1, 0}},
        {'c', {0, 1, 1, 0, 0, 0, 1, 1}},
        {'d', {0, 1, 1, 0, 0, 1, 0, 0}},
        {'e', {0, 1, 1, 0, 0, 1, 0, 1}},
        {'f', {0, 1, 1, 0, 0, 1, 1, 0}},
        {'g', {0, 1, 1, 0, 0, 1, 1, 1}},
        {'h', {0, 1, 1, 0, 1, 0, 0, 0}},
        {'i', {0, 1, 1, 0, 1, 0, 0, 1}},
        {'j', {0, 1, 1, 0, 1, 0, 1, 0}},
        {'k', {0, 1, 1, 0, 1, 0, 1, 1}},
        {'l', {0, 1, 1, 0, 1, 1, 0, 0}},
        {'m', {0, 1, 1, 0, 1, 1, 0, 1}},
        {'n', {0, 1, 1, 0, 1, 1, 1, 0}},
        {'o', {0, 1, 1, 0, 1, 1, 1, 1}},

        {'p', {0, 1, 1, 1, 0, 0, 0, 1}},
        {'q', {0, 1, 1, 1, 0, 0, 0, 1}},
        {'r', {0, 1, 1, 1, 0, 0, 1, 0}},
        {'s', {0, 1, 1, 1, 0, 0, 1, 1}},
        {'t', {0, 1, 1, 1, 0, 1, 0, 0}},
        {'u', {0, 1, 1, 1, 0, 1, 0, 1}},
        {'v', {0, 1, 1, 1, 0, 1, 1, 0}},
        {'w', {0, 1, 1, 1, 0, 1, 1, 1}},
        {'x', {0, 1, 1, 1, 1, 0, 0, 0}},
        {'y', {0, 1, 1, 1, 1, 0, 0, 1}},
        {'z', {0, 1, 1, 1, 1, 0, 1, 0}},
    };
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

    std::unordered_map<std::string, std::array<ushort, 8>> generate_frequency_codes_map() {
        std::unordered_map<std::string, std::array<ushort, 8>> binary_values = {};
        for (size_t i = 0; i < all_frequencies.size(); ++i) {
            binary_values[all_frequencies[i]] = fill_n_bit_unsigned_arr_rev(i + 1);
        }
        return binary_values;
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
public:
    EncoderHelper() : all_frequencies(generate_frequencies()), frequency_codes_map(generate_frequency_codes_map()) {}
public:
    template <std::size_t N>
    std::array<ushort, N> arr_to_number(const std::array<ushort, N>& arr) {
        unsigned int result = 0;
        for (size_t i = 0; i < N; ++i) {
            result |= (arr[i] << (N - 1 - i));
        }
        return result;
    }
};

EncoderHelper FE = EncoderHelper();

class CommonGroup {
public:
    CommonGroup(GroupType group_type_code, ushort program_identification, bool traffic_program, ushort program_type)
                : group_type_code(group_type_code_map[group_type_code]),
                  vc(version_code_map[group_type_code]),
                  pi(fill_n_bit_unsigned_arr<16>(program_identification)),
                  tp(fill_boolean_arr(traffic_program)),
                  pty(fill_n_bit_unsigned_arr<5>(program_type)),
                  pi_number(program_identification) {}

    virtual GroupBitVector generate_group_bit_vector() = 0;

public:
    std::array<ushort, 1> fill_boolean_arr(bool val) {
        return {val};
    }
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
    std::array<std::array<ushort, 8>, 8> fill_character_string(const std::string& str) {
        std::array<std::array<ushort, 8>, 8> result;
        for (size_t i = 0; i < str.size(); i++) {
            std::array<ushort, 8> encoding = FE.character_encoding_map.at(str[i]);
            for (size_t j = 0; j < encoding.size(); j++) {
                result[i][j] = encoding[j];
            }
        }
        return result;
    }

    template <std::size_t N>
    void copy_n_bits_at_pos(BlockArray& dst, const std::array<ushort, N>& src, size_t& pos) {
        for (size_t i = 0; i < N; i++) {
            dst[pos++] = src[i];
        }
    }
protected:
    /* Group Type 0 (0000), 1 (0001), 2 (0010), 3 (0011) */
    std::array<ushort, 4> group_type_code ;
    /* Version Code A (0) or B (1)*/
    std::array<ushort, 1> vc;
    /* Program Identification */
    std::array<ushort, 16> pi;
    /* Traffic Program */
    std::array<ushort, 1> tp;  
    /* Program Type */
    std::array<ushort, 5> pty;
protected:
    /* Program Identification in decimal for debugging */
    const ushort pi_number;
};

class Group2A : public CommonGroup {
private:
    /* Radio text (up to 64 8bit chars) */
    RadioText rt;
    /* Radio text A/B */
    std::array<ushort, 1> ab;
private:
    std::string radio_text;
public:
    Group2A(const ushort program_identification,
            const ushort program_type,
            const bool traffic_program,
            const std::string& radio_text_string,
            const bool radio_text_a_b
            ) 
            : CommonGroup(GROUP_2A, program_identification, traffic_program, program_type),
            rt(fill_n_bit_unsigned_vec(radio_text_string)),
            ab(fill_boolean_arr(radio_text_a_b)),
            radio_text(radio_text_string) {}

    GroupBitVector generate_group_bit_vector() override {
        GroupBitVector stream;
        size_t n_groups = (radio_text.size() + 3) / 4;
        if (n_groups < 16 && radio_text.size() % 4 == 0) n_groups++;
        for (size_t g = 0; g < n_groups; g++) {
            BlockArray block = { 0 };
            size_t pos = 0;
            copy_n_bits_at_pos(block, pi, pos);
            pos += 10; // add CRC
            copy_n_bits_at_pos(block, group_type_code, pos);
            copy_n_bits_at_pos(block, vc, pos);
            copy_n_bits_at_pos(block, tp, pos);
            copy_n_bits_at_pos(block, pty, pos);
            copy_n_bits_at_pos(block, ab, pos);
            copy_n_bits_at_pos(block, this->fill_n_bit_unsigned_arr<4>((ushort)g), pos);
            pos += 10; // add CRC
            // TODO: check string length
            rt_copy_n_bits_at_pos(block, rt, pos, g * 4);
            rt_copy_n_bits_at_pos(block, rt, pos, g * 4 + 1);
            pos += 10; // add CRC
            rt_copy_n_bits_at_pos(block, rt, pos, g * 4 + 2);
            rt_copy_n_bits_at_pos(block, rt, pos, g * 4 + 3);
            pos += 10; // add CRC

            stream.push_back(block);
        }
        // 4 groups
        for (size_t g = 0; g < n_groups; g++) {
            // 4 blocks
            auto rts = this->fill_n_bit_unsigned_arr<4>((ushort)g);
            for (size_t b = 0; b < 4; b++) {
                for (size_t i = 0; i < 26; i++) {
                    std::cout << stream[g][b * 26 + i];
                    if (i == 15) std::cout << " ";
                }
                switch (b) {
                    case 0:
                        std::cout << " " << pi_number;
                        break;
                    case 1:
                        std::cout << " 2A " << tp[0] << " " << pty[0] << pty[1] << pty[2] << pty[3] << pty[4] << " " << ab[0] << " " << rts[0] << rts[1] << rts[2] << rts[3];
                        break;
                    case 2:
                        rt_print_char_at_pos(g * 4);
                        rt_print_char_at_pos(g * 4 + 1);
                        break;
                    case 3:
                        rt_print_char_at_pos(g * 4 + 2);
                        rt_print_char_at_pos(g * 4 + 3);
                        break;
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }

        return stream;
    }

private:
    RadioText fill_n_bit_unsigned_vec(const std::string& str) {
        RadioText result;
        for (size_t i = 0; i < str.size(); i++) {
            result.push_back(FE.character_encoding_map.at(str[i]));
        }
        return result;
    }

    void rt_copy_n_bits_at_pos(BlockArray& block, const RadioText& rt, size_t& block_pos, size_t rt_pos) {
        size_t rt_size = rt.size();
        std::array<ushort, 8> char_enc_to_copy = FE.character_encoding_map.at(' ');
        if (rt_pos == rt_size) {
            // end radio text with carriage return
            char_enc_to_copy = FE.character_encoding_map.at('\r');
        } else if (rt_pos < rt_size) {
            // fill with spaces
            char_enc_to_copy = rt[rt_pos];
        }
        copy_n_bits_at_pos(block, char_enc_to_copy, block_pos);
    }

    void rt_print_char_at_pos(const size_t pos) {
        if (pos > radio_text.size()) {
            // empty char
           std::cout << " -";
        } else if (pos == radio_text.size()) {
            // end radio text with carriage return
           std::cout << " #";
        } else if (pos < radio_text.size()) {
            // fill with spaces
            std::cout << " " << radio_text[pos];
        }
    }
};

class Group0A : public CommonGroup {
private:
    /* Speech (0) / Music (1) */
    std::array<ushort, 1>  ms;
    /* Traffic announcement */
    std::array<ushort, 1>  ta;
    /* Alternative Frequency 1 */
    std::array<ushort, 8>  af1;
    /* Alternative Frequency 2 */
    std::array<ushort, 8>  af2;
    /* Program Service (8bit char * 2 per message * 4 groups = 8 chars) */
    std::array<std::array<ushort, 8>, 8>  ps;
private:
    const std::array<std::string, 2> af_strings;
    const std::string ps_string;

public:
    Group0A(const ushort program_identification,
            const ushort program_type,
            const bool traffic_program,
            const bool music_speech,
            const bool traffic_announcement,
            const std::array<std::string, 2> alternative_frequencies,
            const std::string& program_service)
            : CommonGroup(GROUP_0A, program_identification, traffic_program, program_type),
              ms(fill_boolean_arr(music_speech)),
              ta(fill_boolean_arr(traffic_announcement)),
              af1(FE.frequency_codes_map.at(alternative_frequencies[0])),
              af2(FE.frequency_codes_map.at(alternative_frequencies[1])),
              ps(fill_character_string(program_service)),
              af_strings(alternative_frequencies),
              ps_string(program_service) {}

    GroupBitVector generate_group_bit_vector() override {
        GroupBitVector stream;
        for (size_t b = 0; b < 4; b++) {
            BlockArray block = { 0 };
            size_t pos = 0;
            copy_n_bits_at_pos(block, pi, pos);
            pos += 10; // add CRC
            copy_n_bits_at_pos(block, group_type_code, pos);
            copy_n_bits_at_pos(block, vc, pos);
            copy_n_bits_at_pos(block, tp, pos);
            copy_n_bits_at_pos(block, pty, pos);
            copy_n_bits_at_pos(block, ta, pos);
            copy_n_bits_at_pos(block, ms, pos);
            pos += 3; // add DI segment
            pos += 10; // add CRC
            copy_n_bits_at_pos(block, af1, pos);
            copy_n_bits_at_pos(block, af2, pos);
            pos += 10; // add CRC
            copy_n_bits_at_pos(block, ps[b * 2], pos);
            copy_n_bits_at_pos(block, ps[b * 2 + 1], pos);
            pos += 10; // add CRC
            stream.push_back(block);
        }
        // 4 groups
        for (size_t i = 0; i < 4; i++) {
            // 4 blocks
            for (size_t j = 0; j < 4; j++) {
                for (size_t k = 0; k < 26; k++) {
                    std::cout << stream[i][j * 26 + k];
                    if (k == 15) std::cout << " ";
                }
                switch(j) {
                    case 0:
                        std::cout << " " << pi_number;
                        break;
                    case 1:
                        std::cout << " 0A "<< pty[0] << pty[1] << pty[2] << pty[3] << pty[4];
                        break;
                    case 2:
                        std::cout << " " << af_strings[0] << " " << af_strings[1];
                        break;
                    case 3:
                        std::cout << " " << ps_string[i * 2] << " " << ps_string[i * 2 + 1];
                        break;
                }
                std::cout << std::endl;
            }
            
        }
        return stream;
    }
private:
    void copy_bool_at_pos(BlockArray& dst, const bool& src, size_t& pos) {
        dst[pos++] = src ? 1 : 0;
    }
};

// Function to handle Group 0A encoding
void encode_group_0A(const argsMap& args) {
    std::cout << "Encoding Group 0A...\n";
    std::cout << "Program Identification (PI): " << args.at("-pi") << "\n";
    std::cout << "Program Type (PTY): " << args.at("-pty") << "\n";
    std::cout << "Traffic Program (TP): " << args.at("-tp") << "\n";
    std::cout << "Music/Speech (MS): " << args.at("-ms") << "\n";
    std::cout << "Traffic Announcement (TA): " << args.at("-ta") << "\n";
}

// Function to handle Group 2A encoding
void encodeGroup2A(const argsMap& args) {
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
    ushort pi;
    ushort pty;
    bool tp; 
    
    // 0A helper
    std::string alternative_frequencies;
    // 0A
    bool ms;
    bool ta;
    std::array<std::string, 2> frequencies_strings;
    std::string ps;

    // 2A
    std::string radio_text;
    bool radio_text_ab_flag;

private:
    void parse_frequencies() {
        std::stringstream ss(alternative_frequencies);
        std::string token;
        unsigned counter = 0;
        while (std::getline(ss, token, ',')) {
            if (counter >= 2) error = INVALID_FREQUENCIES;
            if (std::find(FE.all_frequencies.begin(), FE.all_frequencies.end(), token) == FE.all_frequencies.end()) {
                std::cerr << "Error: Invalid frequency " << token << "\n";
                error = INVALID_FREQUENCIES;
            }
            frequencies_strings[counter] = token;
            counter++;
        }
        if (counter != 2) error = INVALID_FREQUENCIES;
        
        if (error == INVALID_FREQUENCIES) {
            std::cerr << "Error: Invalid alternative frequencies\n";
        }
    }

    void parse_boolean(const std::string& flag_value, bool& value) {
        if (flag_value != "0" && flag_value != "1") {
            std::cerr << "Error: Invalid value for boolean" << flag_value << " (must be 0 or 1)\n";
            error = INVALID_VALUE;
        } else {
            value = flag_value == "1";
        }
    }

    template <typename N>
    typename std::enable_if<std::is_integral<N>::value>::type
    parse_n_bit_unsigned_int(const std::string& value, N& result, unsigned n) {
        size_t pos;
        try {
            int res = std::stoi(value, &pos);
            if (pos < value.size() || res < 0 || res >= (1 << n)) {
                std::cerr << "Error: Invalid value " << value << " (must be in range <0," << (1 << n) - 1 << ">)\n";
                error = MISSING_VALUE;
                return;
            }
            result = static_cast<N>(res);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: Invalid argument " << value << "\n";
            error = MISSING_VALUE;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: Out of range " << value << "\n";
            error = MISSING_VALUE;
        }
    }

    void parse_string(const std::string& value, std::string& result, size_t length, bool pad) {
        if (value.size() > length) {
            std::cerr << "Error: Invalid value " << value << " (must be at most " << length << " characters, is " << value.size() << ")\n";
            error = INVALID_VALUE;
        }
        result = value;
        // add padding to full length if needed
        if (pad) result.append(length - value.size(), ' ');
    }

public:
    ArgumentParser(int argc, char* argv[]): error(NO_ERROR) {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " -g <GroupID> [other flags...]\n";
            error = ARGUMENT_COUNT;
        }
        std::string groupID;
        
        // Iterate over command-line arguments and parse flags
        for (size_t i = 1; i < argc; ++i) {
            std::string flag = argv[i];
            if (flag == "-g") {
                groupID = argv[++i];
            } else if (i + 1 < argc) {
                args[flag] = argv[++i];
            } else {
                std::cerr << "Error: Missing value for " << flag << "\n";
                error = MISSING_VALUE;
            }
        }
        
        // Common flag validation
        if (args.find("-pi") == args.end() || args.find("-pty") == args.end() || args.find("-tp") == args.end()) {
            std::cerr << "Error: Missing common flags -pi, -pty, or -tp\n";
            error = MISSING_FLAG;
        }

        // Group-specific processing
        if (groupID == "0A") {
            // Group 0A-specific flags validation
            if (args.find("-ms") == args.end() || args.find("-ta") == args.end() || args.find("-af") == args.end() || args.find("-ps") == args.end()) {
                std::cerr << "Error: Missing Group 0A-specific flags\n";
                error = MISSING_FLAG;
            }
            groupType = GROUP_0A;
            alternative_frequencies = args.at("-af");
        } else if (groupID == "2A") {
            // Group 2A-specific flags validation
            if (args.find("-rt") == args.end() || args.find("-ab") == args.end()) {
                std::cerr << "Error: Missing Group 2A-specific flags\n";
                error = MISSING_FLAG;
            }
            groupType = GROUP_2A;
        } else {
            std::cerr << "Error: Unknown group ID " << groupID << "\n";
            error = UNSUPPORTED_GROUP;
        }

        // parse common flags' values
        if (error == NO_ERROR) {
            parse_n_bit_unsigned_int(args.at("-pi"), pi, 16);
            parse_n_bit_unsigned_int(args.at("-pty"), pty, 5);
            parse_boolean(args.at("-tp"), tp);
        }
        switch (groupType)
        {
        case GROUP_0A:
            parse_boolean(args.at("-ms"), ms);
            parse_boolean(args.at("-ta"), ta);
            parse_frequencies();
            parse_string(args.at("-ps"), ps, 8, true);
            break;
        case GROUP_2A:
            parse_string(args.at("-rt"), radio_text, 64, false);
            parse_boolean(args.at("-ab"), radio_text_ab_flag);
            break;
        /* Modularity for future groups */
         default:
            break;
        }
    }

    Error getError() {
        return error;
    }

    GroupType getGroupType() {
        return groupType;
    }

    std::array<std::string, 2> get_frequencies() {
        return frequencies_strings;
    }

    ushort get_pi() {
        return pi;
    }

    ushort get_pty() {
        return pty;
    }
    
    bool get_tp() {
        return tp;
    }

    bool get_ta() {
        return ta;
    }
    
    bool get_ms() {
        return ms;
    }

    std::string get_ps() {
        return ps;
    }

    std::string get_rt() {
        return radio_text;
    }

    bool get_ab() {
        return radio_text_ab_flag;
    }
};

int main(int argc, char* argv[]) {
    auto parser = ArgumentParser(argc, argv);
    if (parser.getError() != ArgumentParser::NO_ERROR) {
        return 1;
    }

    if (parser.getGroupType() == GroupType::GROUP_0A) {
        Group0A group(parser.get_pi(),
                      parser.get_pty(),
                      parser.get_tp(),
                      parser.get_ms(),
                      parser.get_ta(),
                      parser.get_frequencies(),
                      parser.get_ps());
            std::cout << "Group 0A" << std::endl;
            group.generate_group_bit_vector();
    } else if (parser.getGroupType() == GroupType::GROUP_2A) {
        Group2A group(parser.get_pi(),
                      parser.get_pty(),
                      parser.get_tp(),
                      parser.get_rt(),
                      parser.get_ab());
        std::cout << "Group 2A" << std::endl;
        group.generate_group_bit_vector();
    }

    return 0;
}
