#!/bin/bash

cp -r __template "$1"
echo -e "#include \"syslib.h\"
#include \"proglib.h\"

int main(int argc, char** argv) {
}
" > "$1/$1.cpp"
