#!/usr/bin/env python3

import subprocess

yosys_command = """read -sv {}
hierarchy -check -auto-top

proc; fsm; opt; memory; opt

# TODO: Change this to map to our cell library
techmap; opt

# Remove unused cells and wires
clean

# Generate schematics using graphviz
show -format dot -prefix {}

# Write design to BLIF file
write_blif {}
"""

# Run Yosys to generate BLIF file
yosys_log = open("yosys.log", "w")
subprocess.call(["yosys", "-p", yosys_command.format("../examples/and_gate.v", "design", "design.blif")], stdout=yosys_log)
