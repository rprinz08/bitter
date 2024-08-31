ARCH=$(shell $(CC) -dumpmachine | awk 'BEGIN { FS = "-" } ; { print $$1 }')
BIT=$(shell getconf LONG_BIT)
GIT_VERSION := "$(shell git describe --abbrev=7 --dirty --always --tags)"
#$(warning $(GIT_VERSION) $(origin GIT_VERSION))
#$(warning $(ARCH) $(origin ARCH))
#$(info $(ARCH))

ifeq ($(DEBUG),1)
	AUX_CFLAGS=-g
	AUX_CFLAGS+=-DDEBUG
	AUX_LDFLAGS=
    $(info ************ DEBUG VERSION ************)
else
#	AUX_CFLAGS=-O2 -mcpu=cortex-a53 -mtune=cortex-a53 -march=armv8-a
	AUX_CFLAGS=-O2
	AUX_LDFLAGS=
    $(info ************ RELEASE VERSION ***********)
endif

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))

SRCPATH=$(mkfile_dir)
OBJPATH=$(SRCPATH).obj/$(ARCH)
OBJECTS=$(OBJPATH)/tools.o \
		$(OBJPATH)/dump.o \
		$(OBJPATH)/bitter.o
DEP=$(OBJECTS:.o=.d)
-include $(DEP)
BINPATH=$(mkfile_dir)../bin/$(ARCH)
LIBRARY=$(BINPATH)/libbitter.so

CFLAGS=-std=gnu11 $(AUX_CFLAGS) \
	-DARCH='"$(ARCH)"' -DGIT_VERSION=\"$(GIT_VERSION)\" \
	-MD -fPIC -Wall \
	-I. -I$(SRCPATH)../include
LDFLAGS=-L$(BINPATH) $(AUX_LDFLAGS)

.DEFAULT_GOAL := default
.PHONY: default clean prepare

default: prepare $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(CC) -shared -o $(LIBRARY) $(OBJECTS) $(LDFLAGS)

$(OBJPATH)/%.o: $(SRCPATH)%.c
	$(CC) $(CFLAGS) -c $< -o $@

prepare:
	-@mkdir -p $(OBJPATH)
	-@mkdir -p $(BINPATH)

clean:
	-@rm -rf $(OBJPATH)/* > /dev/null 2>&1 || true
	-@rm -f $(LIBRARY) > /dev/null 2>&1 || true
