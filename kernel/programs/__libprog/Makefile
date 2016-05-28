#########################################################
# Makefile
#  By Théophile Bastian and Nathanaël Courant
#
# Generates a program to be run by PiKern.
#########################################################

PROJROOT=../../

ARM_PREFIX = arm-none-eabi
CC = $(ARM_PREFIX)-g++
LD = $(ARM_PREFIX)-ld
AS = $(ARM_PREFIX)-as
AR = $(ARM_PREFIX)-ar
OD = $(ARM_PREFIX)-objdump
OC = $(ARM_PREFIX)-objcopy
APPLINK = ../app_link.sh
PYTHON = python3

BUILD=_build/
SRC=./
KERNEL_SRC=$(PROJROOT)src/
LIBKERN_DIR=$(PROJROOT)
TARGET=libprog.a

CFLAGS = -O2 -std=c++11 -mfpu=vfp -mfloat-abi=hard -march=armv6zk \
	-mtune=arm1176jzf-s -nostartfiles -Wall -Wextra -pedantic -fPIC \
	-mpic-data-is-text-relative \
	-I $(SRC) -I $(KERNEL_SRC)
CLIBS = -L$(LIBKERN_DIR) -lkern
BIN_CFLAGS= --specs=nosys.specs

#### PUT HERE ANY ADDITIONAL OBJECT THAT NEEDS COMPILATION! ###################
OBJS=\
	$(BUILD)io.o \
	$(BUILD)fs.o
###############################################################################

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) rs $@ $^

$(BUILD)%.o: $(SRC)%.cpp $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)
	rm -f $(TARGET)

