import subprocess

ENCODER_PATH = './rds_encoder'
DECODER_PATH = './rds_decoder'

# [brief, command, expected_result_code, should_check_output, expected_stdout, expected_stderr]

test_encoder_0A = [
  # valid
  ["basic valid 0A", ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"], 0, True, "00010010001101000001101010000001001011000011111111101010101001101001000001101101010010011000011010101001000100100011010000011010100000010010110001100100011100000000000000000101101000011001000110100111110001100001001000110100000110101000000100101100100010001100000000000000000001011010000110111101011000010011101000010010001101000001101010000001001011001101001101010000000000000000010110100001011001010110100000100100"],
  ["valid pty", ["-g", "0A", "-pi", "4660", "-pty", "31", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"], 0, True, "00010010001101000001101010000001111111000010101011011010101001101001000001101101010010011000011010101001000100100011010000011010100000011111110001110001010000000000000000000101101000011001000110100111110001100001001000110100000110101000000111111100100111011111000000000000000001011010000110111101011000010011101000010010001101000001101010000001111111001100011001100000000000000000010110100001011001010110100000100100"],
  ["flag --help should print and return 0", ["--help"], 0, False, ""],
  # invalid
  ["invalid group", ["-g", "3A", "-pi", "4660", "-pty", "5", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid pi",    ["-g", "0A", "-pi", "-1", "-pty", "5", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid pi",    ["-g", "0A", "-pi", "65536", "-pty", "5", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid pty",   ["-g", "0A", "-pi", "4660", "-pty", "-1", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid pty",   ["-g", "0A", "-pi", "4660", "-pty", "32", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid tp",    ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "-1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid tp",    ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "2", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid ms",    ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "0", "-ms", "-1", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid ms",    ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "0", "-ms", "2", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid ta",    ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "0", "-ms", "0", "-ta", "-1", "-af", "104.5,98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid ta",    ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "0", "-ms", "1", "-ta", "2", "-af", "104.5,98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid freq space",  ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "0", "-ms", "0", "-ta", "1", "-af", "104.5, 98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid freq dec",  ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "0", "-ms", "0", "-ta", "1", "-af", "104.55,98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid freq dec",  ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "0", "-ms", "0", "-ta", "1", "-af", "104.5,98.04", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid freq range",  ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "0", "-ms", "0", "-ta", "1", "-af", "87.5,98.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid freq range",  ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "0", "-ms", "0", "-ta", "1", "-af", "104.5, 108.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid freq range",  ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "0", "-ms", "0", "-ta", "1", "-af", "87.5, 108.0", "-ps", "RadioXYZ"], 1, False, ""],
  ["invalid ps length", ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZZ"], 1, False, ""],
  ["missing arg", ["", "0A", "-pi", "4660", "-pty", "5", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"],   1, True, ""],
  ["missing arg", ["-g", "", "-pi", "4660", "-pty", "5", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"],   1, False, ""],
  ["missing arg", ["-g", "0A", "", "4660", "-pty", "5", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"],    1, False, ""],
  ["missing arg", ["-g", "0A", "-pi", "", "-pty", "5", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"],     1, False, ""],
  ["missing arg", ["-g", "0A", "-pi", "4660", "", "5", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"],     1, False, ""],
  ["missing arg", ["-g", "0A", "-pi", "4660", "-pty", "", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"],  1, False, ""],
  ["missing arg", ["-g", "0A", "-pi", "4660", "-pty", "5", "", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"],    1, False, ""],
  ["missing arg", ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"],  1, False, ""],
  ["missing arg", ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "1", "", "0", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"],    1, False, ""],
  ["missing arg", ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "1", "-ms", "", "-ta", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"],  1, False, ""],
  ["missing arg", ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "1", "-ms", "0", "", "1", "-af", "104.5,98.0", "-ps", "RadioXYZ"],    1, False, ""],
  ["missing arg", ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "1", "-ms", "0", "-ta", "", "-af", "104.5,98.0", "-ps", "RadioXYZ"],  1, False, ""],
  ["missing arg", ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "1", "-ms", "0", "-ta", "1", "", "104.5,98.0", "-ps", "RadioXYZ"],    1, False, ""],
  ["missing arg", ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "", "-ps", "RadioXYZ"],           1, False, ""],
  ["missing arg", ["-g", "0A", "-pi", "4660", "-pty", "5", "-tp", "1", "-ms", "0", "-ta", "1", "-af", "104.5,98.0", "", "RadioXYZ"],    1, False, ""],
]

test_encoder_2A = [
  ["basic valid 2A", ["-g", "2A", "-pi", "4660", "-pty", "5", "-tp", "1", "-rt", "Now Playing Song Title by Artist", "-ab", "0"], 0, True, "00010010001101000001101010001001001010000011111011100100111001101111100111101101110111001000001100100100000100100011010000011010100010010010100001100101011101010000011011001000101010011000010111100111110101010001001000110100000110101000100100101000100010011100011010010110111010011110010110011100100000000010111100010010001101000001101010001001001010001101001001010101001101101111011000010101101110011001111000001011000100100011010000011010100010010010100100001011001100100000010101001000010010011010010111010001011010110001001000110100000110101000100100101001010100001010011011000110010100000111010010000001100010111001100100010010001101000001101010001001001010011011110000010111100100100000100110100101000001011100100110110010000100100011010000011010100010010010100111100111100001110100011010010000010001011100110111010000100000010001001000110100000110101000100100101010000011101101001000000010000000000000000010000000100000001101110000010010001101000001101010001001001010100101010101000010000000100000000000000000100000001000000011011100000100100011010000011010100010010010101010111001111100100000001000000000000000001000000010000000110111000001001000110100000110101000100100101010111000100110001000000010000000000000000010000000100000001101110000010010001101000001101010001001001010110011101100000010000000100000000000000000100000001000000011011100000100100011010000011010100010010010101101100000100100100000001000000000000000001000000010000000110111000001001000110100000110101000100100101011100011000010001000000010000000000000000010000000100000001101110000010010001101000001101010001001001010111101011110110010000000100000000000000000100000001000000011011100", ""],
  ["basic valid 2A long rt", ["-g", "2A", "-pi", "4660", "-pty", "5", "-tp", "1", "-rt", "Now Playing Song Title by ArtistNow Playing Song Title by Artist", "-ab", "0"], 0, True, "00010010001101000001101010001001001010000011111011100100111001101111100111101101110111001000001100100100000100100011010000011010100010010010100001100101011101010000011011001000101010011000010111100111110101010001001000110100000110101000100100101000100010011100011010010110111010011110010110011100100000000010111100010010001101000001101010001001001010001101001001010101001101101111011000010101101110011001111000001011000100100011010000011010100010010010100100001011001100100000010101001000010010011010010111010001011010110001001000110100000110101000100100101001010100001010011011000110010100000111010010000001100010111001100100010010001101000001101010001001001010011011110000010111100100100000100110100101000001011100100110110010000100100011010000011010100010010010100111100111100001110100011010010000010001011100110111010000100000010001001000110100000110101000100100101010000011101101010011100110111110011110110111011100100000110010010000010010001101000001101010001001001010100101010101000101000001101100100010101001100001011110011111010101000100100011010000011010100010010010101010111001111101101001011011101001111001011001110010000000001011110001001000110100000110101000100100101010111000100110010100110110111101100001010110111001100111100000101100010010001101000001101010001001001010110011101100000010000001010100100001001001101001011101000101101011000100100011010000011010100010010010101101100000100101101100011001010000011101001000000110001011100110010001001000110100000110101000100100101011100011000010011110010010000010011010010100000101110010011011001000010010001101000001101010001001001010111101011110110111010001101001000001000101110011011101000010000001", ""],
  ["basic valid 2A empty rt", ["-g", "2A", "-pi", "4660", "-pty", "5", "-tp", "1", "-rt", "", "-ab", "0"], 0, True, "00010010001101000001101010001001001010000011111011100010000000100000000000000000100000001000000011011100000100100011010000011010100010010010100001100101011100100000001000000000000000001000000010000000110111000001001000110100000110101000100100101000100010011100001000000010000000000000000010000000100000001101110000010010001101000001101010001001001010001101001001010010000000100000000000000000100000001000000011011100000100100011010000011010100010010010100100001011001100100000001000000000000000001000000010000000110111000001001000110100000110101000100100101001010100001010001000000010000000000000000010000000100000001101110000010010001101000001101010001001001010011011110000010010000000100000000000000000100000001000000011011100000100100011010000011010100010010010100111100111100000100000001000000000000000001000000010000000110111000001001000110100000110101000100100101010000011101101001000000010000000000000000010000000100000001101110000010010001101000001101010001001001010100101010101000010000000100000000000000000100000001000000011011100000100100011010000011010100010010010101010111001111100100000001000000000000000001000000010000000110111000001001000110100000110101000100100101010111000100110001000000010000000000000000010000000100000001101110000010010001101000001101010001001001010110011101100000010000000100000000000000000100000001000000011011100000100100011010000011010100010010010101101100000100100100000001000000000000000001000000010000000110111000001001000110100000110101000100100101011100011000010001000000010000000000000000010000000100000001101110000010010001101000001101010001001001010111101011110110010000000100000000000000000100000001000000011011100", ""],
  
  ["invalid ab", ["-g", "2A", "-pi", "4660", "-pty", "5", "-tp", "1", "-rt", "Now Playing Song Title by Artist", "-ab", "-1"], 1, False, "", ""],
  ["invalid ab", ["-g", "2A", "-pi", "4660", "-pty", "5", "-tp", "1", "-rt", "Now Playing Song Title by Artist", "-ab", "2"], 1, False, "", ""],
]

test_decoder_0A = [
  ["basic valid 0A", ["-b", "00010010001101000001101010000001001011000011111111101010101001101001000001101101010010011000011010101001000100100011010000011010100000010010110001100100011100000000000000000101101000011001000110100111110001100001001000110100000110101000000100101100100010001100000000000000000001011010000110111101011000010011101000010010001101000001101010000001001011001101001101010000000000000000010110100001011001010110100000100100"], 0, True, "PI: 4660\nGT: 0A\nTP: 1\nPTY: 5\nTA: Active\nMS: Speech\nDI: 0\nAF: 104.5, 98.0\nPS: \"RadioXYZ\"\n"],
  ["basic valid 0A swap groups", ["-b", "00010010001101000001101010000001001011000110010001110000000000000000010110100001100100011010011111000110000100100011010000011010100000010010110000111111111010101010011010010000011011010100100110000110101010010001001000110100000110101000000100101100100010001100000000000000000001011010000110111101011000010011101000010010001101000001101010000001001011001101001101010000000000000000010110100001011001010110100000100100"], 0, True, "PI: 4660\nGT: 0A\nTP: 1\nPTY: 5\nTA: Active\nMS: Speech\nDI: 0\nAF: 104.5, 98.0\nPS: \"RadioXYZ\"\n"],
  ["basic valid 0A swap groups", ["-b", "00010010001101000001101010000001001011001101001101010000000000000000010110100001011001010110100000100100000100100011010000011010100000010010110010001000110000000000000000000101101000011011110101100001001110100001001000110100000110101000000100101100011001000111000000000000000001011010000110010001101001111100011000010010001101000001101010000001001011000011111111101010101001101001000001101101010010011000011010101001"], 0, True, "PI: 4660\nGT: 0A\nTP: 1\nPTY: 5\nTA: Active\nMS: Speech\nDI: 0\nAF: 104.5, 98.0\nPS: \"RadioXYZ\"\n"],
  ["basic valid 0A swap groups", ["-b", "01010010011000011010101001101010100110100100000110110000010010110000111111111000010010001101000001101010011001000110100111110001100000000000000000010110100000010010001101000001101010000001001011000110010001110001001000110100000110101000000100101100100010001100011011110101100001001110100000000000000000010110100000000100101100110100110101000100100011010000011010100101100101011010000010010000000000000000000101101000"], 0, True, "PI: 4660\nGT: 0A\nTP: 1\nPTY: 5\nTA: Active\nMS: Speech\nDI: 0\nAF: 104.5, 98.0\nPS: \"RadioXYZ\"\n"],
  # ["", ["-b", ""], 0, True, ""],
  # ["", ["-b", ""], 0, True, ""],
]

test_decoder_2A = [
  ["basic valid 2A", ["-b", "00010010001101000001101010001001001010000011111011100100111001101111100111101101110111001000001100100100000100100011010000011010100010010010100001100101011101010000011011001000101010011000010111100111110101010001001000110100000110101000100100101000100010011100011010010110111010011110010110011100100000000010111100010010001101000001101010001001001010001101001001010101001101101111011000010101101110011001111000001011000100100011010000011010100010010010100100001011001100100000010101001000010010011010010111010001011010110001001000110100000110101000100100101001010100001010011011000110010100000111010010000001100010111001100100010010001101000001101010001001001010011011110000010111100100100000100110100101000001011100100110110010000100100011010000011010100010010010100111100111100001110100011010010000010001011100110111010000100000010001001000110100000110101000100100101010000011101101001000000010000000000000000010000000100000001101110000010010001101000001101010001001001010100101010101000010000000100000000000000000100000001000000011011100000100100011010000011010100010010010101010111001111100100000001000000000000000001000000010000000110111000001001000110100000110101000100100101010111000100110001000000010000000000000000010000000100000001101110000010010001101000001101010001001001010110011101100000010000000100000000000000000100000001000000011011100000100100011010000011010100010010010101101100000100100100000001000000000000000001000000010000000110111000001001000110100000110101000100100101011100011000010001000000010000000000000000010000000100000001101110000010010001101000001101010001001001010111101011110110010000000100000000000000000100000001000000011011100"], 0, True, "PI: 4660\nGT: 2A\nTP: 1\nPTY: 5\nA/B: 0\nRT: \"Now Playing Song Title by Artist\"\n"],
  # ["", ["-b", ""], 0, True, ""],
  # ["", ["-b", ""], 0, True, ""],
]

def tester(path, test_cases):
  for idx, test_case in enumerate(test_cases):
    print('Decoder test #', idx, ' - ', test_case[0], end='')
    command = [path] + test_case[1]
    run_result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    expected_code = test_case[2]
    should_check_output = test_case[3]
    expected_stdout = test_case[4]
    actual_code = run_result.returncode
    actual_stdout = run_result.stdout.decode('utf-8')

    if expected_code != actual_code:
      print(' - FAIL')
      print(f'Unexpected result code. Should be {expected_code} but is {actual_code}')
      print('command: ', ' '.join(command))
      print('Expected:')
      print(expected_stdout)
      print('Actual:')
      print(actual_stdout)
      break
    if should_check_output and expected_stdout != actual_stdout:
      print(' - FAIL')
      print('Unexpected stdout.')
      print('command: ', ' '.join(command))
      print('Expected:')
      print(expected_stdout)
      print('Actual:')
      print(actual_stdout)
      break
    print(" - PASS")


def main():
  # tester(ENCODER_PATH, test_encoder_0A)
  # tester(ENCODER_PATH, test_encoder_2A)

  tester(DECODER_PATH, test_decoder_0A)
  tester(DECODER_PATH, test_decoder_2A)

if __name__ == '__main__':
  main()

