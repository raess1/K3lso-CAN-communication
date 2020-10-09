/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * @file main.c
 * $Id:
 *
 * Merely handles all API entry points (from Windows) by
 * calling corresponding Linux functions.
 * PCANBasic Logging features are handled here.
 *
 * Copyright (C) 2001-2020  PEAK System-Technik GmbH <www.peak-system.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * PCAN is a registered Trademark of PEAK-System Germany GmbH
 *
 * Contact:      <linux@peak-system.com>
 * Maintainer:   Fabrice Vergnaud <f.vergnaud@peak-system.com>
 */

#include "pcaninfo.h"
#include "pcanlog.h"
#include "pcbcore.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <dirent.h>

#include "version.h"

#define BUF_SIZE	100

static const char *exec_name;

/* Definition of command-line options. */
static void print_info(void);
static int print_usage(int error);
static void print_help(void);
static void print_version(void);


/* Flag set by '--verbose'. */
static int verbose_flag;
/* Flag set by '--debug'. */
static int debug_flag;

static struct option long_options[] = {
	/* These options set a flag. */
	{ "debug", no_argument, &debug_flag, 1 },
	{ "help", no_argument, 0, 'h' },
	{ "verbose", no_argument, &verbose_flag, 1 },
	{ 0, 0, 0, 0 }
};

char * pretty_tpcanhandle(TPCANHandle channel, char * buf, int size) {
	switch (channel) {
	case PCAN_ISABUS1:
	case PCAN_ISABUS2:
	case PCAN_ISABUS3:
	case PCAN_ISABUS4:
	case PCAN_ISABUS5:
	case PCAN_ISABUS6:
		snprintf(buf, size, "PCAN_ISABUS%d", channel - PCAN_ISABUS1 + 1);
		break;
	case PCAN_DNGBUS1:
		snprintf(buf, size, "PCAN_DNGBUS%d", channel - PCAN_DNGBUS1 + 1);
		break;
	case PCAN_USBBUS1:
	case PCAN_USBBUS2:
	case PCAN_USBBUS3:
	case PCAN_USBBUS4:
	case PCAN_USBBUS5:
	case PCAN_USBBUS6:
	case PCAN_USBBUS7:
	case PCAN_USBBUS8:
		snprintf(buf, size, "PCAN_USBBUS%d", channel - PCAN_USBBUS1 + 1);
		break;
	case PCAN_USBBUS9:
	case PCAN_USBBUS10:
	case PCAN_USBBUS11:
	case PCAN_USBBUS12:
	case PCAN_USBBUS13:
	case PCAN_USBBUS14:
	case PCAN_USBBUS15:
	case PCAN_USBBUS16:
		snprintf(buf, size, "PCAN_USBBUS%d", channel - PCAN_USBBUS9 + 9);
		break;
	case PCAN_PCCBUS1:
	case PCAN_PCCBUS2:
		snprintf(buf, size, "PCAN_PCCBUS%d", channel - PCAN_PCCBUS1 + 1);
		break;
	case PCAN_PCIBUS1:
	case PCAN_PCIBUS2:
	case PCAN_PCIBUS3:
	case PCAN_PCIBUS4:
	case PCAN_PCIBUS5:
	case PCAN_PCIBUS6:
	case PCAN_PCIBUS7:
	case PCAN_PCIBUS8:
		snprintf(buf, size, "PCAN_PCIBUS%d", channel - PCAN_PCIBUS1 + 1);
		break;
	case PCAN_PCIBUS9:
	case PCAN_PCIBUS10:
	case PCAN_PCIBUS11:
	case PCAN_PCIBUS12:
	case PCAN_PCIBUS13:
	case PCAN_PCIBUS14:
	case PCAN_PCIBUS15:
	case PCAN_PCIBUS16:
		snprintf(buf, size, "PCAN_PCIBUS%d", channel - PCAN_PCIBUS9 + 9);
		break;
	case PCAN_LANBUS1:
	case PCAN_LANBUS2:
	case PCAN_LANBUS3:
	case PCAN_LANBUS4:
	case PCAN_LANBUS5:
	case PCAN_LANBUS6:
	case PCAN_LANBUS7:
	case PCAN_LANBUS8:
	case PCAN_LANBUS9:
	case PCAN_LANBUS10:
	case PCAN_LANBUS11:
	case PCAN_LANBUS12:
	case PCAN_LANBUS13:
	case PCAN_LANBUS14:
	case PCAN_LANBUS15:
	case PCAN_LANBUS16:
		snprintf(buf, size, "PCAN_LANBUS%d", channel - PCAN_LANBUS1 + 9);
		break;
	default:
		snprintf(buf, size, "PCAN_NONEBUS");
		break;
	}
	return buf;
}

void print_info(void) {
	printf("'pcaninfo' lists all known PCAN devices and outputs information for each one.\n");

}

int print_usage(int error) {
	return fprintf(error ? stderr : stdout, "Usage: %s [OPTION] [device_name_1] [device_name_2] [...]\n", exec_name);
}

void print_help() {
	printf("  -h, --help				show this help\n");
	printf("  -g, --debug				display debug messages\n");
	printf("  -v, --verbose				display more messages\n");
}

void print_version() {
	printf(("%s version %d.%d.%d.%d\n\n"), exec_name, VERSION_MAJOR, 
		VERSION_MINOR, VERSION_PATCH, VERSION_BUILD);
}
	   
int main(int argc, char * argv[]) {
	PCANLOG_LEVEL log_lvl;
	char * device;
	int c;
	struct pcaninfo_list *pcilist;
	int i, j, ires, doprint;
	char buf[BUF_SIZE];
	TPCANHandle hdl;

	/* get exec name */
	exec_name = strrchr(argv[0], '/');
	if (!exec_name)
		exec_name = argv[0];
	else
		++exec_name;
	/* initialization */
	device = NULL;
	log_lvl = LVL_NORMAL;

	/* parse command arguments */
	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "d:hgv", long_options, &option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;
		switch (c) {
		case 0:
			/* If this option set a flag, do nothing else now. */
			if (long_options[option_index].flag != 0)
				break;
			break;
		case 'g':
			debug_flag = 1;
			break;
		case 'v':
			verbose_flag = 1;
			break;
		case 'h':
			print_info();
			print_version();
			print_usage(0);
			print_help();
			exit(0);
		default:
			print_usage(0);
			print_help();
			exit(0);
		}
	}

	if (verbose_flag)
		log_lvl = LVL_VERBOSE;
	if (debug_flag)
		log_lvl = LVL_DEBUG;
	pcanlog_set(log_lvl, 0, log_lvl == LVL_DEBUG);

	/* assume remaining command line arguments are devices
	 * to look for and output */
	ires = pcaninfo_get(&pcilist, 1);
	if (ires != 0) {
		return -1;
	}
	if (pcilist->version[0] != 0)
		fprintf(stdout, "PCAN driver version: %s\n\n", pcilist->version);
	else
		fprintf(stdout, "PCAN driver not found\n\n");

	/* try to match arg and existing device name */
	for (i = 0; i < pcilist->length; i++) {
		if (optind >= argc)
			doprint = 1;
		else {
			/* search if device is requested in args */
			doprint = 0;
			for (j = optind; j < argc; j++) {
				device = argv[j];
				if (strstr(pcilist->infos[i].name, device) != 0) {
					doprint = 1;
				}
				else if (strstr(pcilist->infos[i].path, device) != 0) {
					doprint = 1;
				}
			}
		}
		if (doprint) {
			pcaninfo_output(&pcilist->infos[i]);
			hdl = pcanbasic_get_handle(pcilist->infos[i].path, pcilist);
			fprintf(stdout, "  \t- TPCANHandle: \"%s\" (0x%03x)\n", pretty_tpcanhandle(hdl, buf, BUF_SIZE), hdl);
			fprintf(stdout, "  \t-----------------\n\n");
			ires = 1;
		}
	}

	free(pcilist);

	return 0;
}
