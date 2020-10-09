# SPDX-License-Identifier: LGPL-2.1-only
#
# Makefile - Do recursively call all Makefiles found in sub directories
#
# Copyright (C) 2001-2020  PEAK System-Technik GmbH
#
# Contact: <linux@peak-system.com>
# Author:  Stephane Grosjean <s.grosjean@peak-system.com>
#
PWD := $(shell pwd)

export PCANBASIC_ROOT := $(shell cd $(PWD)/../pcanbasic; pwd)

all clean install uninstall xeno rtai:
# if any Makefile failed, stop building all...
	@for m in `find . -mindepth 2 -name Makefile`; do\
		$(MAKE) -C `dirname $$m` $@; \
		if [ $$? -ne 0 ]; then exit 1; fi; \
	done
