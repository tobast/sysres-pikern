#########################################################
# Makefile
#  By Théophile Bastian and Nathanaël Courant
#
# Generates kernel.img
#########################################################

ARM_PREFIX = arm-none-eabi
CC = $(ARM_PREFIX)-g++
LD = $(ARM_PREFIX)-ld
AS = $(ARM_PREFIX)-as
OD = $(ARM_PREFIX)-objdump
OC = $(ARM_PREFIX)-objcopy

SDCARD_DEVICE = /dev/mmcblk0p1

BUILD = _build/
SOURCE = src/
TARGET = kernel.img
LIST = $(BUILD)kernel.list
MAP = $(BUILD)kernel.map
LINKER = kernel.ld

BIN = $(BUILD)master.bin

CFLAGS = -O2 -std=c++11 -mfpu=vfp -mfloat-abi=hard -march=armv6zk \
	-mtune=arm1176jzf-s -nostartfiles -Wall -Wextra -pedantic \
	-I $(SOURCE) -I $(USPI_INCLUDE_DIR)
CLIBS = -L $(LIBUSPI_DIR) -luspi
HARDWARE= -DHW_PI_1B
BIN_CFLAGS= --entry=main --specs=nosys.specs


OBJS = \
	   $(BUILD)malloc.o \
	   $(BUILD)interrupts.o \
	   $(BUILD)process.o \
	   $(BUILD)assert.o \
	   $(BUILD)gpio.o \
	   $(BUILD)sleep.o \
	   $(BUILD)svc.o \
	   $(BUILD)mailbox.o \
	   $(BUILD)uspi_interface.o \
	   $(BUILD)kernel.o

LIBUSPI=uspi/lib/libuspi.a
LIBUSPI_CFG=uspi/Config.mk
LIBUSPI_DIR=uspi/lib/
USPI_INCLUDE_DIR=uspi/include

all: $(TARGET) $(LIST)

$(LIST): $(BUILD)output.elf
	$(OD) -d $< > $@

$(TARGET): $(BUILD)output.elf
	$(OC) $< -O binary $@

$(BUILD)output.elf: $(BIN) $(LINKER)
	$(LD) --no-undefined $(BIN) -Map $(MAP) -o $@ -T $(LINKER)

$(BIN): $(LIBUSPI) $(OBJS)
	$(CC) $(CFLAGS) $(CLIBS) $(HARDWARE) $(BIN_CFLAGS) $^ -o $@

$(BUILD)%.o: $(SOURCE)%.cpp $(BUILD)
	$(CC) $(CFLAGS) $(CLIBS) $(HARDWARE) -c $< -o $@

$(BUILD):
	mkdir $@

$(LIBUSPI): $(LIBUSPI_CFG)
	make -C $(LIBUSPI_DIR)

$(LIBUSPI_CFG):
	@if [ -f $@ ] ; then \
		>&2 echo "################################################" ;\
		>&2 echo "WARNING! You seem to have a custom configuration" ; \
			"of $(LIBUSPI_CFG)." ; \
		>&2 echo "################################################" ; \
	else \
		echo "RASPPI = 1" > $@ ; \
		echo "PREFIX = arm-none-eabi-" >> $@ ;\
	fi

install: $(TARGET) $(SDCARD_DEVICE)
	mkdir -p ./mntpt
	sudo /usr/bin/mount $(SDCARD_DEVICE) ./mntpt -o nosuid,uid=$$(id -u)
	cp "$(TARGET)" "./mntpt/"
	sync
	sudo /usr/bin/umount $(SDCARD_DEVICE)
	rmdir mntpt

checkinstall:
	mkdir ./mntpt
	sudo /usr/bin/mount $(SDCARD_DEVICE) ./mntpt -o nosuid,uid=$$(id -u)
	@md5sum $(TARGET)
	@md5sum ./mntpt/$(TARGET)
	sudo /usr/bin/umount $(SDCARD_DEVICE)
	rmdir ./mntpt
		

clean:
	rm -rf $(BUILD)
	rm -f $(TARGET)
	make -C $(LIBUSPI_DIR) clean
	rm -f $(LIBUSPI_CFG)
