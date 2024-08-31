ARCH=$(shell $(CC) -dumpmachine | awk 'BEGIN { FS = "-" } ; { print $$1 }')
BIT=$(shell getconf LONG_BIT)
#$(warning $(ARCH) $(origin ARCH))
#$(info $(ARCH))

export DEBUG=1
export LOG_USE_COLOR=1

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))
# $(info $(mkfile_path))
# $(info $(mkfile_dir))
# $(info $(current_dir))
# $(info $(PWD))

PRJROOT=$(mkfile_dir)
SRCPATH=$(PRJROOT)src
BINPATH=$(PRJROOT)bin/$(ARCH)
TESTPATH=$(PRJROOT)tests
DISTBASE=$(PRJROOT)dist
DISTPATH=$(DISTBASE)/$(ARCH)
DISTARCH=csim-$(ARCH).tgz

.DEFAULT_GOAL := default
.PHONY: default clean prepare tests dist dist-clean dist-pack

default: prepare tests


daemon:
	+@$(MAKE) -C $(DAEMONSRCPATH)

# tests-clean:
# 	+@$(MAKE) -C $(TESTPATH) clean

# tests:
# 	+@$(MAKE) -C $(TESTPATH)

prepare:
	-@mkdir -p $(BINPATH)

clean: dist-clean
#	+@$(MAKE) -C $(TESTPATH) clean
	+@$(MAKE) -C $(DAEMONSRCPATH) clean
	-@rm -rf $(BINPATH)/* > /dev/null 2>&1 || true

dist-clean:
	-@rm -rf $(DISTPATH)/* > /dev/null 2>&1 || true

dist:
	-@mkdir -p $(DISTPATH)
	-@cp $(BINPATH)/csim $(DISTPATH)

dist-pack:
	-@tar --directory $(DISTBASE) -czvf $(DISTBASE)/$(DISTARCH) \
		`basename $(DISTPATH)` \
		> /dev/null 2>&1
