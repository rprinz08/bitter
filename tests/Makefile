ARCH=$(shell $(CC) -dumpmachine | awk 'BEGIN { FS = "-" } ; { print $$1 }')
BIT=$(shell getconf LONG_BIT)
#$(warning $(ARCH) $(origin ARCH))
#$(info $(ARCH))

ifeq ($(DEBUG),1)
	AUX_CFLAGS=-g
	AUX_CFLAGS+=-DDEBUG
	AUX_CFLAGS+=-Wall
	AUX_LDFLAGS=
else
	AUX_LDFLAGS=
endif

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))
# $(info $(mkfile_path))
# $(info $(mkfile_dir))
# $(info $(current_dir))
# $(info $(PWD))

SRCPATH=$(mkfile_dir)
OBJPATH_BASE=$(SRCPATH).obj
OBJPATH=$(OBJPATH_BASE)/$(ARCH)
OBJECTS=$(OBJPATH)/test_bitter.o \
		$(OBJPATH)/main.o
DEP=$(OBJECTS:.o=.d)
-include $(DEP)
BINPATH=$(mkfile_dir)../bin/$(ARCH)
EXECUTABLE=$(SRCPATH)/runtests.exe

CFLAGS=-std=gnu11 $(AUX_CFLAGS) -DARCH='"$(ARCH)"' -MD -fPIC -Wall \
	-I$(mkfile_dir)../include
LDFLAGS=-L$(BINPATH) \
	$(AUX_LDFLAGS) \
	-lcmocka -lm -lbitter

.DEFAULT_GOAL := default
.PHONY: default clean prepare

default: prepare $(EXECUTABLE)


$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $(EXECUTABLE) $(OBJECTS) $(LDFLAGS)

$(OBJPATH)/%.o: $(SRCPATH)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

prepare:
	-@mkdir -p $(OBJPATH)

clean:
	-@rm -rf $(OBJPATH)/* > /dev/null 2>&1 || true
#	-@rmdir $(OBJPATH)
	-@rm -f $(EXECUTABLE) > /dev/null 2>&1 || true
