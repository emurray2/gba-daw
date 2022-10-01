#
# metronome.mak
#
# Demonstrating a musical DAW that originated from a simple metronome program
#
# NOTE: for educational purposes only. For real work, use 
# devkitPro's templates ( $(DEVKITPRO)/examples/gba/template )
# or tonclib's templates ( $(TONCCODE)/lab/template )

PATH := $(DEVKITARM)/bin:$(PATH)

# --- Project details -------------------------------------------------

PROJ    := metronome
TITLE   := $(PROJ)

LIBTONC	:= ../../tonclib

INCLUDE  := -I$(DEVKITPRO)/libtonc/include
LIBPATHS := -L$(DEVKITPRO)/libtonc/lib

LIBS    := -ltonc

# ROM, IWRAM, asm sources
RCXXSRC   := $(PROJ).cpp
ICXXSRC   := 
SSRC    := 

# boot mode: cart/multiboot
bMB     := 0
# Generate asm for CPP files 
bGENASM := 0
# generate map file
bMAP    := 0

# === The rest need not be altered (probably) =========================

CXXSRC    := $(RCXXSRC) $(ICXXSRC)

RCXXOBJ   := $(RCXXSRC:.cpp=.o)
ICXXOBJ   := $(ICXXSRC:.cpp=.o)
CXXOBJ    := $(RCXXOBJ) $(ICXXOBJ)
SOBJ    := $(SSRC:.s=.o)

OBJ     := $(CXXOBJ) $(SOBJ)

ifeq ($(strip $(bMB)), 1)
TARGET  := $(PROJ).mb
SPECS   := -specs=gba_mb.specs
else
TARGET  := $(PROJ)
SPECS   := -specs=gba.specs
endif

# --- Compiling -------------------------------------------------------

CROSS   ?= arm-none-eabi-
AS      := $(CROSS)g++
CC      := $(CROSS)g++
LD      := $(CROSS)g++
OBJCOPY := $(CROSS)objcopy

ARCH    := -mthumb-interwork -mthumb
RARCH   := -mthumb-interwork -mthumb
IARCH   := -mthumb-interwork -marm

CXXBASE   := $(INCLUDE) -O2 -Wall -fno-strict-aliasing
RCXXFLAGS := $(CXXBASE) $(RARCH)
ICXXFLAGS := $(CXXBASE) $(IARCH) -mlong-calls
CXXFLAGS  := $(RCXXFLAGS)

ASFLAGS := -x assembler-with-cxx  -c -mthumb-interwork
LDFLAGS := $(ARCH) $(SPECS) $(LIBPATHS) $(LIBS)

ifeq ($(strip $(bMAP)), 1)
	LDFLAGS += -Wl,-Map,$(PROJ).map
endif

# === TARGETS =========================================================

.PHONY: build clean

# --- Build -----------------------------------------------------------

# --- Main target ---
build: depends $(TARGET).gba
	@echo done

# --- Translation ---
# ---.elf -> .gba ---
$(TARGET).gba : $(TARGET).elf
	@$(OBJCOPY) -v -O binary $< $@
	-@gbafix $@ -t$(TITLE)

# --- Linking -------------
# --- *.o *.a -> .elf -----
$(TARGET).elf : $(OBJ) 
	$(LD) $^ $(LDFLAGS) -o $@

# --- ROM compilation ---
# --- *.cpp -> *.o --------
$(RCXXOBJ) : %.o : %.cpp
	$(CC) $(RCXXFLAGS) -c $< -o $@
ifeq ($(strip $(bGENASM)), 1)
	$(CC) $(RCXXFLAGS) -fverbose-asm -S $<
endif

# --- IWRAM compilation ----
# *.iwram.c -> *.iwram.o ---
$(ICXXOBJ) : %.iwram.o : %.iwram.cpp
	$(CC) $(ICXXFLAGS) -c $< -o $@
ifeq ($(strip $(bGENASM)), 1)
	$(CC) $(ICXXFLAGS) -fverbose-asm -S $<
endif

# --- Assembling ---
# --- *.s -> *.o ---
$(SOBJ) : %.o : %.s
	$(AS) $(ASFLAGS) -c $< -o $@

# --- Header dependencies ---
# --- *.c -> .d -------------
depends:
	$(CC) -MM $(CXXFLAGS) -c $(ICXXSRC) $(RCXXSRC) > $(PROJ).d

-include $(PROJ).d

# --- Build Lib -------------------------------------------------------

buildlib : 
	$(MAKE) -C $(UDIR) -f tonclib.mak	buildlib

# --- Clean -----------------------------------------------------------

clean : 
	@rm -fv $(TARGET).gba
	@rm -fv $(TARGET).elf $(PROJ).map $(PROJ).d
	@rm -fv $(CXXSRC:.cpp=.s)
	@rm -fv $(OBJ)

#EOF
