# *************************************************************************************************
# Machine flags

CC_CMACH	= -mmcu=cc430f6137
CC_DMACH	= -D__MSP430_6137__ -DMRFI_CC430 -D__CC430F6137__


# *************************************************************************************************
# Build flags

CFLAGS		+= $(CC_CMACH) $(CC_DMACH) -Wall
CFLAGS		+= -fno-force-addr -finline-limit=1 -fno-schedule-insns
CFLAGS		+= -fshort-enums -Wl,-Map=output.map
LDFLAGS		=

# Release flags (Use dead code elimination flags. @see: http://gcc.gnu.org/ml/gcc-help/2003-08/msg00128.html)
CFLAGS_REL	+= -Os -fdata-sections -ffunction-sections -fomit-frame-pointer
LDFLAGS_REL	+= -Wl,--gc-sections -Wl,-s

# Debud flags
CFLAGS_DBG	+= -O1 -ggdb
LDFLAGS_DBG	+= -Wl,--gc-sections


# *************************************************************************************************
# Linker flags and include directories

INCLUDES	+= -I ./ -I gcc/ -I /usr/local/msp430/include/ -I /usr/local/lib/gcc/msp430/4.6.3/include


# *************************************************************************************************
# Build tools
 
CC 			= msp430-gcc
LD 			= msp430-ld
AS 			= msp430-as
AR 			= msp430-ar
