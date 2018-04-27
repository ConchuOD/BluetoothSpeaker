# Modified for BTS 2018.
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

# the name of the project
TARGET = BTS
BUILDDIR = build
MCU = MK20DX256

# add RN52, SPI & TFT lib and the TFT font
EXTRALIBS = ILI9341_t3 SPI RN52 text

# configurable options
OPTIONS = -DF_CPU=72000000 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -DUSING_MAKEFILE

# options needed by many Arduino libraries to configure for Teensy 3.x
OPTIONS += -D__$(MCU)__  -DARDUINO=10805 -DTEENSYDUINO=141

# use "cortex-m4" for Teensy 3.x
# use "mk20dx256" for Teensy 3.2
COREPATH = $(abspath cores/teensy3)
CPPFLAGS += -mcpu=cortex-m4
MCU_LD = cores/teensy3/mk20dx256.ld
LDFLAGS = -mcpu=cortex-m4 -T $(MCU_LD)

#************************************************************************
# Location of Teensyduino utilities, Toolchain, and Arduino Libraries.
# To use this makefile without Arduino, copy the resources from these
# locations and edit the pathnames.  The rest of Arduino is not needed.
#************************************************************************

# normal GNU/Linux compiler path
COMPILERPATH ?= /usr/bin/

# teensy tools locations
TOOLSPATH = $(abspath /home/conor/arduino-1.8.5/hardware/tools)

#************************************************************************
# Settings below this point usually do not need to be edited
#************************************************************************

# CPPFLAGS = compiler options for C and C++
CPPFLAGS += -Wall -g -Os -mthumb -MMD $(OPTIONS) -I. -I$(COREPATH) -I$(COREPATH)/avr -IILI9341_t3 -ISPI -IRN52 -Itext

# compiler options for C++ only
CXXFLAGS += -std=gnu++14 -felide-constructors -fno-exceptions -fno-rtti

# compiler options for C only
CFLAGS +=

# linker options
LDFLAGS += -Os -Wl,--gc-sections,--defsym=__rtc_localtime=0 --specs=nano.specs -mthumb

# additional libraries to link
LIBS += -lm

# names for the compiler programs
CC = $(COMPILERPATH)/arm-none-eabi-gcc
CXX = $(COMPILERPATH)/arm-none-eabi-g++
OBJCOPY = $(COMPILERPATH)/arm-none-eabi-objcopy
SIZE = $(COMPILERPATH)/arm-none-eabi-size

# automatically create lists of the sources and objects
CPP_FILES = BTS.cpp

# adding teensy cores
CPP_FILES += $(filter-out $(COREPATH)/main.cpp, $(wildcard $(COREPATH)/*.cpp))
C_FILES = $(wildcard $(COREPATH)/*.c)

# add RN52, SPI & TFT lib and the TFT font
CPP_FILES += $(foreach lib,$(EXTRALIBS),$(wildcard $(lib)/*.cpp))
C_FILES += $(foreach lib, $(EXTRALIBS),$(wildcard $(lib)/*.c))
A_FILES += $(wildcard $(COREPATH)/*.S)
# add obj names
OBJS := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o) $(A_FILES:.S=.o)

#************************************************************************
# PJRC Make section - implicit rules.
#************************************************************************

all: $(TARGET).hex

$(TARGET).elf: $(OBJS) $(MCU_LD)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.hex: %.elf
	$(SIZE) $<
	$(OBJCOPY) -O ihex -R .eeprom $< $@
ifneq (,$(wildcard $(TOOLSPATH)))
	$(TOOLSPATH)/teensy_post_compile -file=$(basename $@) -path=$(shell pwd) -tools=$(TOOLSPATH)
	-$(TOOLSPATH)/teensy_reboot
endif

# compiler generated dependency info
-include $(OBJS:.o=.d)

clean:
	rm -f *.o *.d $(TARGET).elf $(TARGET).hex

print-%  : ; @echo $* = $($*)

#************************************************************************
# JonHylands/uCee Make section
#************************************************************************    

#OBJSFILES := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o)
#OBJS := $(foreach obj,$(OBJS_FILES), $(BUILDDIR)/$(obj))

#all: hex

#build: $(TARGET).elf

#hex: $(TARGET).hex

#post_compile: $(TARGET).hex
#	$(Q)$(TOOLS_PATH)/teensy_post_compile -file="$(basename $<)" -path="$(call cygpath-win,$(CURDIR))" -tools="$(TOOLS_PATH)"

#reboot:
#	$(Q)-$(TOOLS_PATH)/teensy_reboot

#upload: post_compile reboot

#$(BUILDDIR)/%.o: %.c
#	@echo "[CC]\t$<"
#	$(Q)mkdir -p "$(dir $@)"
#	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) $(L_INC) -o "$@" -c "$<"

#$(BUILDDIR)/%.o: %.ino
#	@echo "[CXX]\t$<"
#	$(Q)mkdir -p "$(dir $@)"
#	$(Q)$(CC) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o "$@" -c -x c++ -include Arduino.h "$<"

#$(BUILDDIR)/%.o: %.cpp
#	@echo "[CXX]\t$<"
#	@mkdir -p "$(dir $@)"
#	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o "$@" -c "$<"

#$(TARGET).elf: $(OBJS) $(MCU_LD)
#	@echo "[LD]\t$@"
#	$(Q)$(CC) $(LDFLAGS) -o "$@" -Wl,-Map,$(TARGET).map $(OBJS) $(LIBS)

#%.hex: %.elf
#	@echo "[HEX]\t$@"
#	$(Q)$(SIZE) "$<"
#	$(Q)$(OBJCOPY) -O ihex -R .eeprom "$<" "$@"

# compiler generated dependency info
#-include $(OBJS:.o=.d)

#clean:
#	@echo Cleaning...
#	$(Q)rm -rf "$(BUILDDIR)"
#	$(Q)rm -f "$(TARGET).elf" "$(TARGET).hex" "$(TARGET).map"
