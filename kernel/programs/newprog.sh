#!/bin/bash

cp -r __template "$1"
echo -e "#inlcude \"syslib.h\"\n\nint main(int argc, char** argv) {\n}\n" > "$1/$1.cpp"
