import json

def write_line(f, indent_level, string):
    f.write("  " * indent_level + string + "\n")

def create_liberty_lib(json_file, output_file):
    with open(json_file) as json_file_f, open(output_file, "w") as output_file_f:
        json_lib = json.load(json_file_f)
        indent_level = 2
        write_line(output_file_f, 0, "library(HDL2Redstone) {")
        for cell in json_lib:
            if cell["liberty"]:
                write_line(output_file_f, 1, "cell({cell_name}) {{".format(cell_name=cell["name"]))
                for pin_name, pin_data in cell["pins"].items():
                    write_line(output_file_f, 2, "pin({pin_name}) {{".format(pin_name=pin_name))
                    write_line(output_file_f, 3, "direction: {direction};".format(direction=pin_data["direction"]))
                    if "function" in pin_data:
                        write_line(output_file_f, 3, "function: \"{function}\";".format(function=pin_data["function"]))
                    write_line(output_file_f, 2, "}")
                write_line(output_file_f, 1, "}")
        write_line(output_file_f, 0, "}")
