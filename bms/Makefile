#
# Build file for Formula Slug FS-2 bms
#

MBED_TOOLCHAIN := GCC_ARM
MBED_TARGET := LPC1768
MBED_BUILD_PROFILE := release

BUILD_DIR := cmake_build

DEPENDENCIES := mbed-os lib-mbed-ltc681x

.PHONY: all
all: build-release

#
# Dependency pulling
# 

.PHONY: deps
deps: $(DEPENDENCIES)

$(DEPENDENCIES):
	mbed-tools deploy

#
# Build profiles
#

$(BUILD_DIR):
	mbed-tools configure -t $(MBED_TOOLCHAIN) -m $(MBED_TARGET) -b $(MBED_BUILD_PROFILE)

.PHONY: build
build: deps $(BUILD_DIR)
	mbed-tools compile -t $(MBED_TOOLCHAIN) -m $(MBED_TARGET) -b $(MBED_BUILD_PROFILE)

.PHONY: build-debug
build-debug: MBED_BUILD_PROFILE=debug
build-debug: build

.PHONY: build-release
build-release: MBED_BUILD_PROFILE=release
build-release: build

#
# Flashing to devices
#

.PHONY: flash
flash: deps $(BUILD_DIR)
	mbed-tools compile -t $(MBED_TOOLCHAIN) -m $(MBED_TARGET) -b $(MBED_BUILD_PROFILE) -f

#
# STM32 F303 debug
#

.PHONY: debug
debug: build-debug
	arm-none-eabi-gdb -x .gdbinit ./$(BUILD_DIR)/$(MBED_TARGET)/$(MBED_BUILD_PROFILE)/$(MBED_TOOLCHAIN)/bms.elf

#
# Utilities
#

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: test
test:
	# Run all builds for testing
	make MBED_TARGET=LPC1768 MBED_BUILD_PROFILE=debug build
	make MBED_TARGET=LPC1768 MBED_BUILD_PROFILE=release build
