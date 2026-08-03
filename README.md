## How to build + run

### Prerequisites
Needs Python 3 + jinja2 package for GLAD code generator:
pip install jinja2

If this repo wqas freshly cloned, run:
git submodule update --init --recursive

### Windows
mkdir -p build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
cd ..
./build/bin/project.exe

### Linux/Mac
mkdir -p build
cd build
cmake .. -G "Unix Makefiles"
cmake --build .
cd ..
./build/bin/project


