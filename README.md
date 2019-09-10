# HDL2Redstone

HDL2Redstone is a hardware compiler, it takes HDL(Verilog) files as input and outputs a Minecraft Schematic file. The Schematic file can be then used to import the design into a Minecraft world.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

What things you need to install the software and how to install them

```
gcc with C++17 support
cmake3.10
python3.7
zlib
flex
bison3
```

### Installing

A step by step series of examples that tell you how to get a development env running

Say what the step will be

```
Give the example
```

And repeat

```
until finished
```

End with an example of getting some data out of the system or using it for a little demo

## Developer Information

Useful information for people who work on this project.

### Static Code Analysis

Some static code analysis will be run by `clang-tidy` during each build, warnings are treated as error.

### Source Code Styling

We use [LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html) in this project, run `make format` in your build directory to format your source code before committing. You can also make sure your changes complies with the standard by running `make format-check`.

## Authors

* **Shizhang(Steven) Yin**
* **Zihan(Simon) Zhao**
* **Shiwen(Sarah) Zhu**

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

* Thanks to Prof. Vaughn Betz for supervising us.
