/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * @file libpcanbasic.c
 * @brief PCAN Basic for LINUX API
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
 * Contribution: Thomas Haber <thomas@toem.de>
 */
#include "../PCANBasic.h"
#include "pcbcore.h"
#include "pcblog.h"

#include <stdio.h>

#define MAX_LOG 256		/* Max length of a log string */


TPCANStatus CAN_Initialize(
        TPCANHandle Channel,
        TPCANBaudrate Btr0Btr1,
        TPCANType HwType,
		DWORD IOPort,
		WORD Interrupt) {
	TPCANStatus sts;
	char szLog[MAX_LOG];

	/* logging */
	pcblog_write_entry("CAN_Initialize");
	snprintf(szLog, MAX_LOG,
			"Channel: 0x%02X, Btr0Btr1: %d, HwType: 0x%08X, IOPort: 0x%08X, Interrupt: 0x%08X",
			Channel, Btr0Btr1, IOPort, HwType, Interrupt);
	pcblog_write_param("CAN_Initialize", szLog);
	/* forward call */
	sts = pcanbasic_initialize(Channel, Btr0Btr1, IOPort, HwType, Interrupt);
	pcblog_write_exit("CAN_Initialize", sts);
	return sts;
}

TPCANStatus CAN_InitializeFD(
    TPCANHandle Channel,
	TPCANBitrateFD BitrateFD) {
	TPCANStatus sts;
	char szLog[MAX_LOG];

	/* logging */
	pcblog_write_entry("CAN_InitializeFD");
	snprintf(szLog, MAX_LOG, "Channel: 0x%02X, BitrateFD: {%s}", Channel, BitrateFD);
	pcblog_write_param("CAN_InitializeFD", szLog);
	/* forward call */
	sts = pcanbasic_initialize_fd(Channel, BitrateFD);
	pcblog_write_exit("CAN_InitializeFD", sts);
	return sts;
}

TPCANStatus CAN_Uninitialize(
        TPCANHandle Channel) {
	TPCANStatus sts;
	char szLog[MAX_LOG];

	/* logging */
	pcblog_write_entry("CAN_Uninitialize");
	snprintf(szLog, MAX_LOG, "Channel: 0x%02X", Channel);
	pcblog_write_param("CAN_Uninitialize", szLog);
	/* forward call */
	sts = pcanbasic_uninitialize(Channel);
	pcblog_write_exit("CAN_Uninitialize", sts);
	return sts;
}

TPCANStatus CAN_Reset(
        TPCANHandle Channel) {
	TPCANStatus sts;
	char szLog[MAX_LOG];

	/* logging */
	pcblog_write_entry("CAN_Reset");
	snprintf(szLog, MAX_LOG, "Channel: 0x%02X", Channel);
	pcblog_write_param("CAN_Reset", szLog);
	/* forward call */
	sts = pcanbasic_reset(Channel);
	pcblog_write_exit("CAN_Reset", sts);
	return sts;
}

TPCANStatus CAN_GetStatus(
        TPCANHandle Channel) {
	TPCANStatus sts;

	/* logging */
	char szLog[MAX_LOG];
	pcblog_write_entry("CAN_GetStatus");
	snprintf(szLog, MAX_LOG, "Channel: 0x%02X", Channel);
	pcblog_write_param("CAN_GetStatus", szLog);
	/* forward call */
	sts = pcanbasic_get_status(Channel);
	pcblog_write_exit("CAN_GetStatus", sts);
	return sts;
}

TPCANStatus CAN_Read(
        TPCANHandle Channel,
        TPCANMsg* MessageBuffer,
        TPCANTimestamp* TimestampBuffer) {
	TPCANStatus sts;
	char szLog[MAX_LOG];

	/* logging */
	pcblog_write_entry("CAN_Read");
	snprintf(szLog, MAX_LOG,
			"Channel: 0x%02X, MessageBuffer: 0x%p, TimestampBuffer: 0x%p",
			Channel, MessageBuffer, TimestampBuffer);
	pcblog_write_param("CAN_Read", szLog);
	/* forward call */
	sts = pcanbasic_read(Channel, MessageBuffer, TimestampBuffer);
	pcblog_write_exit("CAN_Read", sts);
	return sts;
}

TPCANStatus CAN_ReadFD(
    TPCANHandle Channel,
	TPCANMsgFD* MessageBuffer,
	TPCANTimestampFD *TimestampBuffer) {
	TPCANStatus sts;
	char szLog[MAX_LOG];

	/* logging */
	pcblog_write_entry("CAN_Read");
	snprintf(szLog, MAX_LOG,
			"Channel: 0x%02X, MessageBuffer: 0x%p, TimestampBuffer: 0x%p",
			Channel, MessageBuffer, TimestampBuffer);
	pcblog_write_param("CAN_ReadFD", szLog);
	/* forward call */
	sts = pcanbasic_read_fd(Channel, MessageBuffer, TimestampBuffer);

	pcblog_write_exit("CAN_ReadFD", sts);
	return sts;
}

TPCANStatus CAN_Write(
        TPCANHandle Channel,
        TPCANMsg* MessageBuffer) {
	TPCANStatus sts;
	char szLog[MAX_LOG];

	/* logging */
	pcblog_write_entry("CAN_Write");
	snprintf(szLog, MAX_LOG, "Channel: 0x%02X, MessageBuffer: 0x%p", Channel,
			MessageBuffer);
	pcblog_write_param("CAN_Write", szLog);
	/* forward call */
	sts = pcanbasic_write(Channel, MessageBuffer);

	pcblog_write_exit("CAN_Write", sts);
	return sts;
}

TPCANStatus CAN_WriteFD(
    TPCANHandle Channel,
	TPCANMsgFD* MessageBuffer) {
	TPCANStatus sts;
	char szLog[MAX_LOG];

	/* logging */
	pcblog_write_entry("CAN_WriteFD");
	snprintf(szLog, MAX_LOG, "Channel: 0x%02X, MessageBuffer: 0x%p", Channel,
			MessageBuffer);
	pcblog_write_param("CAN_WriteFD", szLog);
	/* forward call */
	sts = pcanbasic_write_fd(Channel, MessageBuffer);
	pcblog_write_exit("CAN_WriteFD", sts);
	return sts;
}

TPCANStatus CAN_FilterMessages(
        TPCANHandle Channel,
        DWORD FromID,
        DWORD ToID,
        TPCANMode Mode) {
	TPCANStatus sts;
	char szLog[MAX_LOG];

	/* logging */
	pcblog_write_entry("CAN_FilterMessages");
	snprintf(szLog, MAX_LOG,
			"Channel: 0x%02X, FromID: 0x%08X, ToID: 0x%08X, Mode: 0x%08X",
			Channel, FromID, ToID, Mode);
	pcblog_write_param("CAN_FilterMessages", szLog);
	/* forward call */
	if (FromID > ToID)
		sts = pcanbasic_filter(Channel, ToID, FromID, Mode);
	else
		sts = pcanbasic_filter(Channel, FromID, ToID, Mode);

	pcblog_write_exit("CAN_FilterMessages", sts);
	return sts;
}

TPCANStatus CAN_GetValue(
        TPCANHandle Channel,
        TPCANParameter Parameter,
        void* Buffer,
        DWORD BufferLength) {
	TPCANStatus sts;
	char szLog[MAX_LOG];

	/* logging */
	pcblog_write_entry("CAN_GetValue");
	snprintf(szLog, MAX_LOG,
			"Channel: 0x%02X, Parameter: 0x%08X, Buffer: 0x%p, BufferLength: 0x%08X",
			Channel, Parameter, Buffer, BufferLength);
	pcblog_write_param("CAN_GetValue", szLog);
	/* forward call */
	sts = pcanbasic_get_value(Channel, Parameter, Buffer, BufferLength);
	pcblog_write_exit("CAN_GetValue", sts);
	return sts;
}

TPCANStatus CAN_SetValue(
        TPCANHandle Channel,
        TPCANParameter Parameter,
        void* Buffer,
		DWORD BufferLength) {
	TPCANStatus sts;
	char szLog[MAX_LOG];

	/* logging */
	pcblog_write_entry("CAN_SetValue");
	snprintf(szLog, MAX_LOG,
			"Channel: 0x%02X, Parameter: 0x%08X, Buffer: 0x%p, BufferLength: 0x%08X",
			Channel, Parameter, Buffer, BufferLength);
	pcblog_write_param("CAN_SetValue", szLog);
	/* forward call */
	sts = pcanbasic_set_value(Channel, Parameter, Buffer, BufferLength);
	pcblog_write_exit("CAN_SetValue", sts);
	return sts;
}

TPCANStatus CAN_GetErrorText(
        TPCANStatus Error,
        WORD Language,
        LPSTR Buffer) {
	TPCANStatus sts;
	char szLog[MAX_LOG];

	/* logging */
	pcblog_write_entry("CAN_GetErrorText");
	snprintf(szLog, MAX_LOG, "Error: 0x%08X, Language: 0x%08X, Buffer: 0x%p",
			Error, Language, Buffer);
	pcblog_write_param("CAN_GetErrorText", szLog);
	/* forward call */
	sts = pcanbasic_get_error_text(Error, Language, Buffer);
	pcblog_write_exit("CAN_GetErrorText", sts);
	return sts;
}
