# *************************************************************************************************
# Includes

include Common.mk


# *************************************************************************************************
# Variables

OUTDIR = build
SUBDIRS = config core drivers modules

PYTHON := $(shell which python2 || which python)
BASH := $(shell which bash || which bash)


# *************************************************************************************************
# Utils

COLS =`tput cols`
PAD = $(shell expr $(COLS) - 6)

# Colors
RED=`tput setaf 1`
GREEN=`tput setaf 2`
YELLOW=`tput setaf 3`
NORMAL=`tput sgr0`

# Macro to display compilation error message
CHECK_ERRORS = @if test -e tmp.errors; \
	then printf "$(RED)[ERR]$(NORMAL)\n"; \
	elif test -s tmp.log; \
		then printf "$(YELLOW)[WARN]$(NORMAL)\n"; \
	else printf "$(GREEN)[OK]$(NORMAL)\n" ;	fi; \
	cat tmp.log >> Build.log; \
	rm -f tmp.errors; \
	rm -f tmp.log;


# *************************************************************************************************
# Targets

.PHONY: all
.PHONY: clear
.PHONY: clean
.PHONY: config

.PHONY: install
.PHONY: run
.PHONY: debug

.PHONY: depend
.PHONY: force

.PHONY: doc
.PHONY: latexdoc


# *************************************************************************************************
# Build list of sources and objects to build

SRCS := $(wildcard *.c)
$(foreach subdir, $(SUBDIRS), \
	$(eval SRCS := $(SRCS) $(wildcard $(subdir)/*.c)) \
)

OBJS := $(patsubst %.c, build/%.o, $(SRCS))


# *************************************************************************************************
# Append specific CFLAGS/LDFLAGS

ifneq ($(wildcard config/config.h), )
	
	DEBUG := $(shell grep "^\#define CONFIG_DEBUG" config/config.h)
	
	ifeq ($(DEBUG),)
		TARGET	:= RELEASE
		CFLAGS	+= $(CFLAGS_REL)
		LDFLAGS	+= $(LDFLAGS_REL)
	else
		TARGET	:= DEBUG
		CFLAGS	+= $(CFLAGS_DBG)
		LDFLAGS	+= $(LDFLAGS_DBG)
		MEMPYFLAGS := -d
	endif	
	
endif


# *************************************************************************************************
# Main rule

all: required depend $(OUTDIR)/openchronos.txt


# *************************************************************************************************
# Required files rules

required: config/config.h config/rtca_now.h $(OUTDIR)/config/modinit.o

config/modinit.c:
	@echo "Please do a 'make config' first!" && false

config/config.h:
	@echo "Please do a 'make config' first!" && false


# *************************************************************************************************
# Dependencies rules

depend: Dependencies.mk

Dependencies.mk: $(SRCS)
	@printf "%-${PAD}s" "(GEN) Checking dependencies..."
	@touch $@
	@makedepend $(INCLUDES) -Y -f $@ -- $(CFLAGS) $^ 2> tmp.log || touch tmp.errors
	$(CHECK_ERRORS)
	@rm -f $@.bak

ifneq ($(wildcard Dependencies.mk), )
	include Dependencies.mk
endif


# *************************************************************************************************
# Rebuild if CFLAGS changed 
# 	(use the .cflags file to store the last CFLAGS)

config/openchronos.cflags: force
	@echo "$(CFLAGS)" | cmp -s - $@ || echo "$(CFLAGS)" > $@

$(OBJS): config/openchronos.cflags


# *************************************************************************************************
# Basic rules

$(OUTDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@printf "%-${PAD}s" "(CC) $<"
	@$(CC) $(CFLAGS) $(SPEC_FLAGS) $(INCLUDES) -c $< -o $@ 2>> tmp.log || touch tmp.errors
	$(CHECK_ERRORS)


# *************************************************************************************************
# Rebuild all every time because system time changed

config/rtca_now.h: force
	@printf "(GEN) $@\n"
	@$(BASH) ./tools/update_rtca_now.sh

$(OBJS): config/rtca_now.h


# *************************************************************************************************
# Specific rules

$(OUTDIR)/openchronos.elf: $(OUTDIR)/core/even_in_range.o $(OBJS)
	@printf "\n%-${PAD}s" "Building $@ as target $(TARGET)..."
	@$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -o $@ $+ 2>> tmp.log || touch tmp.errors
	$(CHECK_ERRORS)
	@rm -f output.map	

$(OUTDIR)/openchronos.txt: $(OUTDIR)/openchronos.elf
	@$(PYTHON) tools/firmware/memory.py -i $< -o $@ $(MEMPYFLAGS)

$(OUTDIR)/core/even_in_range.o: core/even_in_range.s
	@mkdir -p $(dir $@)
	@printf "%-${PAD}s" "(CC) $<"
	@$(AS) -c $< -o $@ 2>> tmp.log || touch tmp.errors
	$(CHECK_ERRORS)

$(OUTDIR)/config/modinit.o: SPEC_FLAGS = -Wno-implicit-function-declaration


# *************************************************************************************************
# Top rules

config:
	@mkdir -p config
	@$(PYTHON) tools/config/config.py
	@$(PYTHON) tools/config/make_modinit.py

install: $(OUTDIR)/openchronos.txt
ifeq ($(method), usb)
	@echo "Installing the new firmware via USB..."
	mspdebug rf2500 "prog $(OUTDIR)/openchronos.elf"
else
	@echo "Installing the new firmware via RF..."
	sudo $(PYTHON) contrib/ChronosTool.py rfbsl $<
endif

clear: clean
	@rm -Rf ./config/

clean:
	@rm -Rf ./$(OUTDIR)/
	@rm -f Dependencies.mk Build.log

debug:
	@echo "Starting mspdebug..."
	mspdebug rf2500 gdb
	
run:
	@echo "Running the firmware using mspdebug..."
	mspdebug rf2500 run
	
doc:
	@rm -Rf doc
	@mkdir -p doc
	doxygen Doxyfile
	
latexdoc: doc
	@make -f .doc/latex/Makefile pdf
