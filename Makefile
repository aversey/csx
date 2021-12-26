# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#     Compilation Options
# Debug mode [yes/no] (allowing to debug the library via gdb):
DEBUG   ?= no
# Specify your favourite C compiler here:
COMPILE ?= gcc
# Specify your include directory (headers location):
INCDIR  ?= /usr/include
# Specify your libraries directory:
LIBDIR  ?= /usr/lib
# Specify location of man pages on your machine:
MANDIR  ?= /usr/share/man
# Are you using Windows?
WINDOWS ?= no


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#     Preparations
# Compile as ANSI C code:
CFLAGS   = -xc -ansi -Wall -pedantic
# Specify linker to use the library:
LFLAGS   = -L$(BUILD) -lcsx
# Debug and optimisation (as well as -static for valgrind) are not compatible:
ifeq '$(DEBUG)' 'yes'
CFLAGS  += -g -O0
else
CFLAGS  += -O2
LFLAGS  += -static
endif

# Directories definitions:
INCLUDE  = include
BUILD    = build
SRCDIR   = src
EXADIR   = examples
MAN      = man
# Library itself:
LIB      = libcsx.a
# Modules:
EXTERNAL = $(foreach x,$(notdir $(wildcard $(INCLUDE)/*.h)),$(x:.h=))
INTERNAL = $(foreach x,$(notdir $(wildcard $(SRCDIR)/*.h)),$(x:.h=))

# Default target is library:
TARGET   = $(BUILD)/$(LIB)
# Determing needed object files:
EXTER_H  = $(foreach x,$(EXTERNAL:=.h),$(INCLUDE)/csx/$(x))
EXTER_C  = $(foreach x,$(EXTERNAL:=.c),$(SRCDIR)/$(x))
INTER_H  = $(foreach i,$(INTERNAL:=.h),$(SRCDIR)/$(i))
INTER_C  = $(INTER_H:.h=.c)
SRC      = $(INTER_C) $(EXTER_C)
OBJ      = $(foreach o,$(SRC:.c=.o),$(BUILD)/$(o))
# Example executables:
XSRC     = $(wildcard $(EXADIR)/*.c)
EXAMPLES = $(foreach x,$(XSRC:.c=),$(BUILD)/$(x))
# Dependency file:
DEPS     = deps.mk

SRCINC   = -I$(INCLUDE)
XINC     = -I$(INCLUDE)
SRCBUILD = $(BUILD)/$(SRCDIR)
XBLD     = $(BUILD)/$(EXADIR)


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#     Targets
.PHONY: all examples clean install uninstall

all: $(SRCBUILD) $(TARGET)

examples: all $(XBLD) $(EXAMPLES)

clean:
	rm -f $(TARGET) $(OBJ) $(EXAMPLES) $(DEPS)

install: all
	cp -r $(INCLUDE)/* $(INCDIR)
	cp $(TARGET) $(LIBDIR)
	cp $(MAN)/* $(MANDIR)/man3

uninstall:
	rm -r $(INCDIR)/csx.h $(INCDIR)/csx
	rm $(LIBDIR)/$(LIB)
	rm $(MANDIR)/man3/csx.3 $(MANDIR)/man3/libcsx.3


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#     Compilation
ifneq '$(WINDOWS)' 'yes'
-include $(DEPS)
endif

# Packing object files into library:
$(TARGET): $(OBJ)
	ar -rcs $@ $^

# Compile object files from corresponding source:
$(BUILD)/%.o: %.c
	$(COMPILE) $(CFLAGS) $(SRCINC) -c $< -o $@

# Compile executables from corresponding sources and library:
$(BUILD)/%: %.c $(TARGET)
	$(COMPILE) $(CFLAGS) $(XINC) $< $(LFLAGS) -o $@

# Create build directories, if no such:
ifneq '$(WINDOWS)' 'yes'
$(SRCBUILD) $(XBLD):
	mkdir -p $@
endif

# Generate dependency file, adding corresponding build prefixes:
$(DEPS): $(SRC) $(EXASRC) $(EXTER_H) $(INTER_H) $(INCLUDE)/csx.h
	$(COMPILE) $(SRCINC) $(SRC) -MM | sed '/^ /!s#^#$(SRCBUILD)/#' >$@
	$(COMPILE) $(XINC) $(XSRC) -MM | sed '/^ /!s#^#$(XBLD)/#;s/\.o//' >>$@
