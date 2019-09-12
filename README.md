# HDL2Redstone

HDL2Redstone is a hardware compiler, it takes HDL(Verilog) files as input and outputs a Minecraft Schematic file. The Schematic file can be then used to import the design into a Minecraft world.

## Getting Started

### Prerequisites

This project has only be tested to work on Linux

```
gcc with C++17 support
cmake 3.10
makefile
python 3.7
clang-tools
zlib
flex
bison 3
```

### Installing

Make sure you have [Github ssh key](https://help.github.com/en/articles/connecting-to-github-with-ssh) setup already

#### Get the Source Code

```
git clone git@github.com:tech4me/HDL2Redstone.git
cd HDL2Redstone
git submodule update --init --recursive
```

#### Build the Project

```
mkdir -p build
cd build
cmake ..
make
```

### Run Example Design

```
HDL2Redstone/HDL2Redstone.py <verilog file>
```

## Developer Information

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
