#!/usr/bin/env python3

import os
import sys
import argparse
import subprocess

file_path = os.path.dirname(os.path.realpath(__file__))

parser = argparse.ArgumentParser(description="HDL2Redstone main excutable.")
parser.add_argument("verilog_file", help="Input Verilog file")
parser.add_argument("--output", help="Output directory")
args = parser.parse_args()

with open(os.path.join(file_path, "script/yosys.ys.txt"), "r") as yosys_command_f:
    yosys_command = yosys_command_f.read()

if not os.path.isfile(args.verilog_file):
    sys.exit("Error: Verilog file provided does not exist.")
verilog_file = os.path.abspath(args.verilog_file)

liberty_file = os.path.abspath(os.path.join(file_path, "cell_lib/HDL2Redstone.lib"))
if not os.path.isfile(liberty_file):
    sys.exit("Error: Liberty file does not exist.")

# Set the current working directory to output directory if it is specified, otherwise use the verilog file directory
if args.output:
    if os.path.isdir(args.output):
        output_dir = args.output
    else:
        sys.exit("Error: Output path provided is not valid.")
else:
    if os.path.isdir(os.path.dirname(args.verilog_file)):
        output_dir = os.path.dirname(args.verilog_file)
    else:
        sys.exit("Error: Verilog file's parent directory does not exist.")
os.chdir(output_dir)

yosys_command = yosys_command.format(input_verilog_file=verilog_file, input_liberty_file=liberty_file, output_dot_file_name="design", output_blif_file="design.blif")

# Run Yosys to generate BLIF file
subprocess.call(["yosys", "-Q", "-q", "-l", "yosys.log", "-p", yosys_command])

# Run HDL2Redstone main executable to generate schematics
subprocess.call([os.path.join(file_path, "HDL2Redstone")])
