# Teensyduino Core Library
# http://www.pjrc.com/teensy/
# Copyright (c) 2017 PJRC.COM, LLC.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# 1. The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# 2. If the Software is incorporated into a build system that allows
# selection among a list of target devices, then similar target
# devices manufactured by PJRC.COM must be included in the list of
# target devices and selectable in the same manner.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# The name of your project (used to name the compiled .hex file)
TARGET = BTS
BUILDDIR = build

# Those that specify a NO_ARDUINO environment variable will
# be able to use this Makefile with no Arduino dependency.
# Please note that if ARDUINOPATH was set, it will override
# the NO_ARDUINO behaviour.
NO_ARDUINO = 1
ifndef NO_ARDUINO
# Path to your arduino installation
ARDUINOPATH ?= ../../../../..
endif

# configurable options
OPTIONS = -DF_CPU=72000000 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -DUSING_MAKEFILE

# options needed by many Arduino libraries to configure for Teensy 3.x
OPTIONS += -D__$(MCU)__ -DARDUINO=10805 -DTEENSYDUINO=141

# use "cortex-m4" for Teensy 3.x
# use "cortex-m0plus" for Teensy LC
#CPUARCH = cortex-m4
#CPUARCH = cortex-m0plus
COREPATH = cores/teensy3
CPPFLAGS += -D__MK20DX256__ -mcpu=cortex-m4
LDSCRIPT = cores/teensy3/mk20dx256.ld
LDFLAGS += -mcpu=cortex-m4 -T $(LDSCRIPT)


# Other Makefiles and project templates for Teensy 3.x:
#
# https://github.com/apmorton/teensy-template
# https://github.com/xxxajk/Arduino_Makefile_master
# https://github.com/JonHylands/uCee


#************************************************************************
# Location of Teensyduino utilities, Toolchain, and Arduino Libraries.
# To use this makefile without Arduino, copy the resources from these
# locations and edit the pathnames.  The rest of Arduino is not needed.
#************************************************************************

# Default to the normal GNU/Linux compiler path if NO_ARDUINO
# and ARDUINOPATH was not set.
#COMPILERPATH ?= /home/conor/arduino-1.8.5/hardware/tools/arm/bin
COMPILERPATH ?= /usr/bin/

#************************************************************************
# Settings below this point usually do not need to be edited
#************************************************************************

# CPPFLAGS = compiler options for C and C++
CPPFLAGS = -Wall -g -Os -mthumb -MMD $(OPTIONS) -I.

# compiler options for C++ only
CXXFLAGS = -std=gnu++14 -felide-constructors -fno-exceptions -fno-rtti

# compiler options for C only
CFLAGS =

# linker options
LDFLAGS = -Os -Wl,--gc-sections,--defsym=__rtc_localtime=0 --specs=nano.specs -mthumb -T$(LDSCRIPT)

# additional libraries to link
LIBS = -lm

# names for the compiler programs
CC = $(COMPILERPATH)/arm-none-eabi-gcc
CXX = $(COMPILERPATH)/arm-none-eabi-g++
OBJCOPY = $(COMPILERPATH)/arm-none-eabi-objcopy
SIZE = $(COMPILERPATH)/arm-none-eabi-size

# automatically create lists of the sources and objects

# adding teensy cores for T3.2
CPP_FILES += $(filter-out COREPATH/main.cpp, $(wildcard COREPATH*.cpp))
C_FILES += $(wildcard COREPATH*.c)

# add RN52 lib and ILI lib
EXTRALIBS = ILI9341_t3 SPI RN52 text
CPP_FILES += $(foreach lib, $(EXTRALIBS), $(wildcard /$(lib)/*.cpp))
C_FILES += $(foreach lib, $(EXTRALIBS), $(wildcard /$(lib)/*.c))

# add obj names
OBJSFILES := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o)
OBJS := $(foreach obj,$(OBJS_FILES), $(BUILDDIR)/$(obj))

all: hex

build: $(TARGET).elf

hex: $(TARGET).hex

post_compile: $(TARGET).hex
	$(Q)$(TOOLS_PATH)/teensy_post_compile -file="$(basename $<)" -path="$(call cygpath-win,$(CURDIR))" -tools="$(TOOLS_PATH)"

reboot:
	$(Q)-$(TOOLS_PATH)/teensy_reboot

upload: post_compile reboot

$(BUILDDIR)/%.o: %.c
	@echo "[CC]\t$<"
	$(Q)mkdir -p "$(dir $@)"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) $(L_INC) -o "$@" -c "$<"

$(BUILDDIR)/%.o: %.ino
	@echo "[CXX]\t$<"
	$(Q)mkdir -p "$(dir $@)"
	$(Q)$(CC) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o "$@" -c -x c++ -include Arduino.h "$<"

$(BUILDDIR)/%.o: %.cpp
	@echo "[CXX]\t$<"
	@mkdir -p "$(dir $@)"
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o "$@" -c "$<"

$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@echo "[LD]\t$@"
	$(Q)$(CC) $(LDFLAGS) -o "$@" -Wl,-Map,$(TARGET).map $(OBJS) $(LIBS)

%.hex: %.elf
	@echo "[HEX]\t$@"
	$(Q)$(SIZE) "$<"
	$(Q)$(OBJCOPY) -O ihex -R .eeprom "$<" "$@"

# compiler generated dependency info
-include $(OBJS:.o=.d)

clean:
	@echo Cleaning...
	$(Q)rm -rf "$(BUILDDIR)"
	$(Q)rm -f "$(TARGET).elf" "$(TARGET).hex" "$(TARGET).map"