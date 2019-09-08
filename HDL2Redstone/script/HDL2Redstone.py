#!/usr/bin/env python3

import os
import sys
import argparse
import subprocess

file_path = os.path.dirname(os.path.realpath(__file__))

parser = argparse.ArgumentParser(description="HDL2Redstone main excutable.")
parser.add_argument("verilog_file", help="Input Verilog file")
args = parser.parse_args()

with open(os.path.join(file_path, "script/yosys.ys.txt"), "r") as yosys_command_f:
    yosys_command = yosys_command_f.read()

liberty_file = os.path.join(file_path, "cell_lib/HDL2Redstone.lib")
if not os.path.isfile(liberty_file):
    print("Liberty file not found!")
    sys.exit()

yosys_command = yosys_command.format(input_verilog_file=args.verilog_file, input_liberty_file=liberty_file, output_dot_file_name="design", output_blif_file="design.blif")

# Run Yosys to generate BLIF file
subprocess.call(["yosys", "-Q", "-q", "-l", "yosys.log", "-p", yosys_command])
