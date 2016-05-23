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

ELF = $(BUILD)output.elf

CFLAGS = -O2 -std=c++11 -mfpu=vfp -mfloat-abi=hard -march=armv6zk \
	-mtune=arm1176jzf-s -nostartfiles -Wall -Wextra -pedantic -static \
	-I $(SOURCE) -I $(USPI_INCLUDE_DIR)
CLIBS = -L$(LIBUSPI_DIR) -luspi
HARDWARE= -DHW_PI_1B
BIN_CFLAGS= --specs=nosys.specs

OBJS = \
		$(BUILD)cinit.o \
		$(BUILD)init.o \
		$(BUILD)kernel.o \
		$(BUILD)uspi_interface.o \
		$(BUILD)mailbox.o \
		$(BUILD)interrupts.o \
		$(BUILD)assert.o \
		$(BUILD)process.o \
		$(BUILD)malloc.o \
		$(BUILD)gpio.o \
		$(BUILD)sleep.o \
		$(BUILD)Bytes.o \
		$(BUILD)ipv4.o \
		$(BUILD)udp.o \
		$(BUILD)icmp.o \
		$(BUILD)arp.o \
		$(BUILD)logger.o \
		$(BUILD)format.o \
		$(BUILD)networkCore.o \
		$(BUILD)atomic.o \
		$(BUILD)filesystem.o \
		$(BUILD)fs_populator.o \
		$(BUILD)svc.o

LIBUSPI=uspi/lib/libuspi.a
LIBUSPI_CFG=uspi/Config.mk
LIBUSPI_DIR=uspi/lib/
USPI_INCLUDE_DIR=uspi/include

all: $(TARGET) $(LIST)

$(LIST): $(ELF)
	$(OD) -d $< > $@

$(TARGET): $(ELF)
	$(OC) $< -O binary $@

$(ELF): $(LIBUSPI) $(OBJS) $(LINKER)
	$(CC) $(CFLAGS) $(HARDWARE) $(BIN_CFLAGS) \
		$(OBJS) \
		$(CLIBS) \
		-T $(LINKER) \
		-o $@

$(BUILD)init.o: $(SOURCE)init.s $(BUILD)
	$(CC) $(CFLAGS) $(HARDWARE) -c $< -o $@

$(BUILD)%.o: $(SOURCE)%.cpp $(BUILD)
	$(CC) $(CFLAGS) $(HARDWARE) -c $< -o $@

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
