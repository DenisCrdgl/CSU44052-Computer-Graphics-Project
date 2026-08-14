## How to build + run

### Prerequisites
Needs Python 3 + jinja2 package for GLAD code generator:
pip install jinja2

If this repo was freshly cloned, run:
git submodule update --init --recursive

### Windows
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
cd ..

Run executable in build/bin

### Linux/Mac
mkdir -p build
cd build
cmake .. -G "Unix Makefiles"
cmake --build .
cd ..

Run executable in build/bin


