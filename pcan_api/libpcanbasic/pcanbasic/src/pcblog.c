/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * @file pcblog.c
 * @brief Logger for PCANBasic API
 *
 * $Id: pcblog.c 6126 2020-03-04 16:00:13Z Stephane $
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

#include "pcblog.h"

/*
 * INCLUDES
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "../PCANBasic.h"

/*
 * DEFINES
 */
/** Max length of a log string */
#define MAX_LOG		256
/** Default log file name */
#define LOG_FILE	"PCANBasic.log"

#define PCBLOG_DEFAULT_PATH	"."

/*
 * STRUCTURES AND TYPES
 */
/** Structure to keep track of the logger's context */
struct pcanbasic_logger {
	int initialized;	/**< states if logger was initialized */
	char path[PCAN_LOG_MAX_PATH];	/**< directory path to store logs */
	int enabled; 		/**< logger's status (0 disabled; 1 enabled) */
	int flags;			/**< logger's configuration (see PCAN_LOG_xxx) */
	int fd; 			/**< file descriptor */
};


/** PRIVATE VARIABLES */
/** stores the context of the PCANBasic logger */
static struct pcanbasic_logger g_pcblog = {0, PCBLOG_DEFAULT_PATH, 0, LOG_FUNCTION_DEFAULT, -1};

/** PRIVATE FUNCTIONS DECLARATIONS */
/**
 * @fn void pcblog_close(void)
 * @brief Closes the PCANBasic log file.
 */
static void pcblog_close(void);
/**
 * @fn void pcblog_check(void)
 * @brief Checks if the PCANBasic log file is opened and do so if required.
 */
static void pcblog_check(void);
/**
 * @fn void pcblog_atexit(void)
 * @brief A callback function to close log file when application stops.
 */
static void pcblog_atexit(void);
/**
 * @fn void pcblog_write_opened(void)
 * @brief Appends a default header when a log file is opened.
 */
static void pcblog_write_opened(void);
/**
 * @fn void pcblog_write_opened(void)
 * @brief Appends a default footer when a log file is closed.
 */
static void pcblog_write_closed(void);
/**
 * @fn void pcblog_write_unsafe(const char * s)
 * @brief Appends a message to the PCANBasic log.
 *
 * @param[in] s message to append
 */
static void pcblog_write_unsafe(const char * msg);
/**
 * @fn void pcblog_write_unsafe(const char * s, unsigned long len)
 * @brief Appends a message to the PCANBasic log.
 *
 * @param[in] s message to append
 * @param[in] len size of the message
 */
static void pcblog_nwrite_unsafe(const char * s, unsigned long len);

/* PRIVATE FUNCTIONS */
void pcblog_close() {
	if (g_pcblog.fd >= 0) {
		pcblog_write_closed();
		close(g_pcblog.fd);
		g_pcblog.fd = -1;
	}
}

void pcblog_check() {
	/* check/initialize logger */
	if (!g_pcblog.initialized) {
		atexit(pcblog_atexit);
		g_pcblog.initialized = 1;
	}
	/* open file */
	if (g_pcblog.enabled && g_pcblog.fd < 0) {
		char szFileName[PCAN_LOG_MAX_PATH + 16];
		sprintf(szFileName, "%s/%s", g_pcblog.path, LOG_FILE);
		g_pcblog.fd = open(szFileName, O_WRONLY | O_APPEND | O_CREAT, 0644);
		if (g_pcblog.fd >= 0)
			pcblog_write_opened();
	}
}

void pcblog_atexit(void) {
	pcblog_close();
	g_pcblog.enabled = 0;
}

void pcblog_write_opened() {
	pcblog_write_unsafe("«____________________________________»");
	pcblog_write_unsafe("«           PCAN-Basic Log           »");
	pcblog_write_unsafe("«____________________________________»");
}

void pcblog_write_closed() {
	pcblog_write_unsafe("«____________________________________»");
	pcblog_write_unsafe("«            ############            »");
	pcblog_write_unsafe("«____________________________________»");
}

void pcblog_write_unsafe(const char * s) {
	pcblog_nwrite_unsafe(s, strlen(s));
}

void pcblog_nwrite_unsafe(const char * s, unsigned long len) {
	time_t t;
	struct tm * ptm;
	char *stime;
	ssize_t n;

	if (g_pcblog.fd < 0)
		return;
	/* create time info */
	time(&t);
	ptm = localtime(&t);
	stime = asctime(ptm);
	/* removes trailing '\n' */
	stime[strlen(stime) - 1] = 0;
	// write time
	n = 0;
	n += write(g_pcblog.fd, stime, strlen(stime));
	n += write(g_pcblog.fd, " - ", 3);
	// write message
	if (s)
		n += write(g_pcblog.fd, s, len);
	n += write(g_pcblog.fd, "\n", 1);
}

/* PUBLIC FUNCTIONS */
void pcblog_write(const char * msg, unsigned long len) {
	pcblog_check();
	pcblog_nwrite_unsafe(msg, len);
}

void pcblog_write_entry(const char *sfunc) {
	if (g_pcblog.enabled && (g_pcblog.flags & LOG_FUNCTION_ENTRY)) {
		char szMessage[MAX_LOG];
		sprintf(szMessage, "ENTRY      '%s'", sfunc);
		pcblog_write(szMessage, strlen(szMessage));
	}
}

void pcblog_write_param(const char *sfunc, const char *sparam) {
	if (g_pcblog.enabled && (g_pcblog.flags & LOG_FUNCTION_PARAMETERS)) {
		char szMessage[MAX_LOG];
		sprintf(szMessage, "PARAMETERS of %s: %s", sfunc, sparam);
		pcblog_write(szMessage, strlen(szMessage));
	}
}

void pcblog_write_exit(const char *sfunc, TPCANStatus sts) {
	if (g_pcblog.enabled && (g_pcblog.flags & LOG_FUNCTION_LEAVE)) {
		char szMessage[MAX_LOG];
		sprintf(szMessage, "EXIT       '%s' -   RESULT: 0x%02X", sfunc,
				(unsigned int) sts);
		pcblog_write(szMessage, strlen(szMessage));
	}
}

void pcblog_write_exception(const char *sfunc) {
	if (g_pcblog.enabled) {
		char szMessage[MAX_LOG];
		sprintf(szMessage, "EXCEPTION FOUND IN '%s'", sfunc);
		pcblog_write(szMessage, strlen(szMessage));
	}
}

void pcblog_write_can_msg(TPCANHandle channel, int direction, TPCANMsg* pmsg) {
	if (g_pcblog.enabled && (g_pcblog.flags &direction) && pmsg) {
		char szMessage[MAX_LOG];
		sprintf(
				szMessage,
				"CHANNEL    0x%02X (%s) ID=0x%X Len=%u, Data=0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
				(unsigned int) channel, direction == LOG_FUNCTION_WRITE ? "OUT"
						: "IN", (unsigned int) pmsg->ID,
				(unsigned int) pmsg->LEN,
				(unsigned int) pmsg->DATA[0],
				(unsigned int) pmsg->DATA[1],
				(unsigned int) pmsg->DATA[2],
				(unsigned int) pmsg->DATA[3],
				(unsigned int) pmsg->DATA[4],
				(unsigned int) pmsg->DATA[5],
				(unsigned int) pmsg->DATA[6],
				(unsigned int) pmsg->DATA[7]);
		pcblog_write(szMessage, strlen(szMessage));
	}
}

/* PUBLIC FUNCTIONS: getters/setters */
void pcblog_get_location(char *buffer, unsigned int size) {
	if (buffer == NULL || size == 0)
		return;
	snprintf(buffer, size, "%s", g_pcblog.path);
}
void pcblog_set_location(const char *buffer) {
	const char * stmp;

	if (buffer == NULL)
		stmp = PCBLOG_DEFAULT_PATH;
	else if (strnlen(buffer, PCAN_LOG_MAX_PATH) > 0)
		stmp = buffer;
	else
		stmp = PCBLOG_DEFAULT_PATH;
	snprintf(g_pcblog.path, PCAN_LOG_MAX_PATH, "%s", stmp);
	/* close log if any,
	 * it will be reopened automatically with the new path */
	pcblog_close();
}

int pcblog_get_status(void) {
	return g_pcblog.enabled;
}
void pcblog_set_status(int status) {
	g_pcblog.enabled = status;
}

int pcblog_get_config(void) {
	return g_pcblog.flags;
}
void pcblog_set_config(int flags) {
	g_pcblog.flags = flags;
}
