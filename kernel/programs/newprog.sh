#!/bin/bash

cp -r __template "$1"
echo -e "#include \"syslib.h\"\n\nint main(int argc, char** argv) {\n}\n" > "$1/$1.cpp"
