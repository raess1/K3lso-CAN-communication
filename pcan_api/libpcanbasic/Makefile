# SPDX-License-Identifier: LGPL-2.1-only
#
# Makefile - global Makefile for all Linux componenets of the pcanbasic package
#
# Copyright (C) 2001-2020  PEAK System-Technik GmbH
#
# Contact: <linux@peak-system.com>
# Author:  Stephane Grosjean <s.grosjean@peak-system.com>
#
define do-make
@make -C pcanbasic $1
@make -C pcaninfo $1
@make -C examples $1
endef

define make-all
$(call do-make, all)
endef

define make-clean
$(call do-make, clean)
endef

define make-install
$(call do-make, install)
endef

define make-uninstall
$(call do-make, uninstall)
endef

define make-xeno
$(call do-make, xeno)
endef

define make-rtai
$(call do-make, rtai)
endef

all:
	$(make-all)

clean:
	$(make-clean)

install:
	$(make-install)

uninstall:
	$(make-uninstall)

xeno:
	$(make-xeno)

rtai:
	$(make-rtai)
