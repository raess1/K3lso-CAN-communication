#  Makefile
#
#  ~~~~~~~~~~~~
#
#  PCANBasic JNI Interface
#
#  ~~~~~~~~~~~~
#
#  ------------------------------------------------------------------
#  Last changed by:	$Author: Fabrice $
#  Last change: $Date: 2019-03-05 15:58:16 +0100 (mar., 05 mars 2019) $
#  Language: make
#  ------------------------------------------------------------------
#
#  Copyright (C) 1999-2018  PEAK-System Technik GmbH, Darmstadt
#  more Info at http://www.peak-system.com
# ------------------------------------------------------------------
#
# linux@peak-system.com
# www.peak-system.com
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#****************************************************************************

# Commands
CC	= gcc
LN	= ln -sf

# Paths and files
JAVA_HOME = /usr/lib/jvm/default
SRC     = src
INC     = -I. -I$(JAVA_HOME)/include/ -I$(JAVA_HOME)/include/linux
FILES   = $(SRC)/pcanbasic_jni.c
LIBPATH = /usr/lib
# default local PCAN-Basic root (if the library is not installed)
PATH_PCANBASIC ?= ../../libpcanbasic/pcanbasic
DEFAULT_PATH_PCANBASIC = /usr/include

# get build version
SED_GET_VERSION = 's/^\#.*[\t\f ]+([0-9]+)[\t\f \r\n]*/\1/'
VERSION_FILE = 'src/VersionNo.h'
MAJOR = $(shell cat $(VERSION_FILE) | grep VERSION_MAJOR | sed -re $(SED_GET_VERSION))
MINOR = $(shell cat $(VERSION_FILE) | grep VERSION_MINOR | sed -re $(SED_GET_VERSION))
PATCH = $(shell cat $(VERSION_FILE) | grep VERSION_PATCH | sed -re $(SED_GET_VERSION))

# Define targets
NAME = libpcanbasic_jni
NAME_OLD = libPCANBasic_JNI
EXT = .so
TARGET_SHORT = $(NAME)$(EXT)
TARGET  = $(TARGET_SHORT).$(MAJOR).$(MINOR).$(PATCH)
SONAME  = $(TARGET_SHORT).$(MAJOR)

# User application compile options 
DBG                = 
USERAPP_CFLAGS    ?= 
USERAPP_LIBS      ?= -lc -lm -lpcanbasic
USERAPP_LDFLAGS   ?= 

# Flags
CFLAGS = -fPIC -shared -O2 -Wl,-soname,$(TARGET_SHORT) $(DBG) $(USERAPP_CFLAGS) $(USERAPP_LIBS) $(INC)
CFLAGS += -Wall -Wcast-align -Wcast-qual -Wimplicit 
CFLAGS += -Wpointer-arith -Wswitch
CFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused

# Check if PCAN-Basic library is installed
PCANBASIC_STATUS = "PCAN-Basic header"
ifeq (,$(wildcard $(DEFAULT_PATH_PCANBASIC)/PCANBasic.h))
  PCANBASIC_STATUS += "not found, using local path: \'$(PATH_PCANBASIC)\'"
  # set local pathes to PCAN-Basic root
  INC 				+= -I$(PATH_PCANBASIC)
  USERAPP_CFLAGS	+= -L$(PATH_PCANBASIC) -Wl,-rpath $(PATH_PCANBASIC)
else
  PATH_PCANBASIC = $(DEFAULT_PATH_PCANBASIC)
  PCANBASIC_STATUS += "found in \'$(DEFAULT_PATH_PCANBASIC)\'."
endif


#********** entries *********************

all: message $(TARGET)

$(TARGET) : $(FILES) 
	@echo
	@echo "*** Compiling project:" 
ifeq (,$(wildcard $(PATH_PCANBASIC)/PCANBasic.h))
	$(error "Failed to locate PCAN-Basic header in '$(PATH_PCANBASIC)'")
endif
	$(CC) $(FILES) $(CFLAGS) -o $(TARGET)

clean:
	@echo
	@echo "*** Cleaning project:"
	-rm -f $(SRC)/*~ $(SRC)/*.o *~ *.so.* *.so

.PHONY: message
message:
	@ echo "***"
	@ echo "***"
	@ echo "*** target=$(TARGET_SHORT)" 
	@ echo "*** version=$(MAJOR).$(MINOR).$(PATCH)"
	@ echo "***"
	@ echo "*** $(CC) version=$(shell $(CC) -dumpversion)"
	@ echo "***"
	@ echo "*** JAVA_HOME is '$(JAVA_HOME)'"
	@ echo "*** $(PCANBASIC_STATUS)"
	
#********** this entry is reserved for root access only *********************
install:
	@echo
	@echo "*** Installing:"
	cp $(TARGET) $(LIBPATH)/$(TARGET)
	$(LN) $(LIBPATH)/$(TARGET) $(LIBPATH)/$(SONAME)
	$(LN) $(LIBPATH)/$(SONAME) $(LIBPATH)/$(TARGET_SHORT)
	/sbin/ldconfig
 
uninstall:
	@echo
	@echo "*** Uninstalling:"
	-rm $(LIBPATH)/$(TARGET)
	-rm $(LIBPATH)/$(SONAME)
	-rm $(LIBPATH)/$(TARGET_SHORT)	
	/sbin/ldconfig
  
uninstall-purge: uninstall
	-rm $(LIBPATH)/$(TARGET_SHORT).*
	-rm $(LIBPATH)/$(NAME_OLD)$(EXT)*
	/sbin/ldconfig

