import itertools
import subprocess

ENCODER_PATH = "./rds_encoder"

# Test cases for each argument (updated with flags)
pi_flag = "-pi"
pi = [
    # ("", False, "misses a value", pi_flag), 
    (0, True, "", pi_flag), 
    (1, True, "", pi_flag), 
    (45162, True, "", pi_flag), 
    (65535, True, "", pi_flag), 
    (65536, False, "is out of range", pi_flag), 
    (-1, False, "is out of range", pi_flag)
]

pty_flag = "-pty"
pty = [
    # ("", False, "misses a value", pty_flag), 
    (0, True, "", pty_flag), 
    (1, True, "", pty_flag), 
    (21, True, "", pty_flag), 
    (31, True, "", pty_flag), 
    (32, False, "is out of range", pty_flag), 
    (-1, False, "is out of range", pty_flag)
]

tp_flag = "-tp"
tp = [
    # ("", False, "misses a value", tp_flag), 
    (1, True, "", tp_flag), 
    (0, True, "", tp_flag), 
    (2, False, "should be 0/1", tp_flag), 
    (-1, False, "should be 0/1", tp_flag), 
    ("abc", False, "should be bool", tp_flag), 
    (-4, False, "should be bool", tp_flag)
]

ms_flag = "-ms"
ms = [
    # ("", False, "misses a value", ms_flag), 
    (1, True, "", ms_flag), 
    (0, True, "", ms_flag), 
    (2, False, "should be 0/1", ms_flag), 
    (-1, False, "should be 0/1", ms_flag), 
    ("abc", False, "should be bool", ms_flag), 
    (-4, False, "should be bool", ms_flag)
]

ta_flag = "-ta"
ta = [
    # ("", False, "misses a value", ta_flag), 
    (1, True, "", ta_flag), 
    (0, True, "", ta_flag), 
    (2, False, "should be 0/1", ta_flag), 
    (-1, False, "should be 0/1", ta_flag), 
    ("abc", False, "should be bool", ta_flag), 
    (-4, False, "should be bool", ta_flag)
]

af_flag = "-af"
af = [
    # ("", False, "misses a value", af_flag), 
    ("87.6,87.6", True, "", af_flag), 
    ("87.6,104.9", True, "", af_flag), 
    ("87.5,87.6", False, "is out of range", af_flag),
    ("87.6,108.0", False, "is out of range", af_flag), 
    ("87.65, 107.9", False, "has bad format", af_flag), 
    ("87.65", False, "has only one value", af_flag),
    ("87.6, 107.9", False, "has a bad format", af_flag), 
    ("87.6a,107.9", False, "has invalid value", af_flag)
]

ps_flag = "-ps"
ps = [
    # ("", False, "misses a value", ps_flag), 
    ("abc", True, "", ps_flag), 
    ("a" * 7, True, "", ps_flag), 
    ("a" * 8, True, "", ps_flag), 
    ("64", True, "", ps_flag), 
    ("a" * 9, False, "is too long", ps_flag),
    ("-1", False, "has invalid characters", ps_flag), 
    ("+*", False, "should be only [a-zA-Z0-9 ]*", ps_flag)
]

rt_flag = "-rt"
rt = [
    # ("", False, "misses a value", rt_flag), 
    ("abc", True, "", rt_flag), 
    ("a" * 7, True, "", rt_flag), 
    ("a" * 64, True, "", rt_flag), 
    ("64", True, "", rt_flag),
    ("a" * 65, False, "is too long", rt_flag),
    ("-1", False, "has invalid characters", rt_flag), 
    ("+*", False, "should be only [a-zA-Z0-9 ]*", rt_flag)
]

ab_flag = "-ab"
ab = [
    # ("", False, "misses a value", ab_flag), 
    (1, True, "", ab_flag), 
    (0, True, "", ab_flag), 
    (2, False, "should be 0/1", ab_flag), 
    (-1, False, "should be 0/1", ab_flag), 
    ("abc", False, "should be bool", ab_flag)
]

# Groups configuration
groups = {
  "0A": [(pi_flag, pi), (pty_flag, pty), (tp_flag, tp), (ms_flag, ms), (ta_flag, ta), (af_flag, af), (ps_flag, ps)],
  "2A": [(pi_flag, pi), (pty_flag, pty), (tp_flag, tp), (rt_flag, rt), (ab_flag, ab)]
}

def validate_encoder_arguments(group_name):
  group = groups[group_name]
  flags, test_data = zip(*group)
  count = 0
  # Generate all possible combinations for each flag
  for combination in itertools.product(*test_data):
    if count % 100 == 0:
      print(f"Processing {count}th combination")
    count += 1

    test_case = {flag: value for (flag, (value, _, _, _)) in zip(flags, combination)}

    results = [result for (_, result, _, _) in combination]

    overall_pass = all(results)
    args = " ".join([f"{flag} {value}" for flag, value in test_case.items()])
    fail_reasons = [f'{flag} {reason}' for (_, _, reason, flag) in combination if reason]
    # print(args, "=>", "PASS" if overall_pass else "FAIL", "because: ", [] if overall_pass else fail_reasons)
    args = [ENCODER_PATH, "-g", group_name] + [str(x) for x in [item for pair in zip(test_case.keys(), test_case.values()) for item in pair]]
    print(args)
    run_result = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    # print(args, "=>", "PASS" if overall_pass else "FAIL", "because: ", [] if overall_pass else fail_reasons)
    print("Expected failure", "because: ", [] if overall_pass else fail_reasons)
    print(run_result.stderr.decode('utf-8'))
    print(run_result.stdout.decode('utf-8'))

    if run_result.returncode != 0 and overall_pass:
      print("Unexpected failure")
      print(run_result.stdout)
      print(run_result.stderr)
      exit(1)
    elif run_result.returncode == 0 and not overall_pass:
      print("Unexpected success")
      exit(1)

# Run tests for both groups
validate_encoder_arguments("0A")
validate_encoder_arguments("2A")