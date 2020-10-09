/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * @file pcbcore.c
 * @brief PCAN Basic core
 * $Id:
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
#include "pcbcore.h"

#include <stdio.h>		/* snprintf */
#include <stdlib.h>		/* NULL */
#include <string.h>		/* memset, etc. */
#include <sys/queue.h>	/* LIST_ENTRY, etc. */
#include <errno.h>		/* to handle errno returned by libpcanfd */
#include <unistd.h>		/* usleep */
#include <ctype.h>		/* isspace */

/* NOTE: the new PCANBasic API uses libpcanfd source code, here is why:
 *  - to avoid code duplication. libpcanfd and pcanbasic both use
 *    [__ioctl, fopen, etc.] functions to control CAN devices.
 * 	- to get updates sooner, libpcanfd gets updated of driver
 * 	  changes before pcanbasic.
 *
 * The API does not link the lib but use its source code instead,
 * because we can't include the old libpcan library as it
 * defines the same prototypes as PCANBasic (like CAN_write, etc.).
 *
 * But via source code, since PCANFD_OLD_STYLE_API is not defined,
 * the libpcan is not included automatically by libpcanfd.
 */
#include "pcanfd.h"	/* force local header */
#include "libpcanfd.h"

#include "../PCANBasic.h"	/* PCANBasic types and defines */
#include "pcaninfo.h"		/* discovers PCAN devices */
#include "pcanlog.h"		/* used to debug lib */
#include "resource.h"		/* used to translate error msgs */
#include "pcblog.h"			/* pcanbasic-logger used by get/set_value */
#include "pcbtrace.h"		/* pcanbasic-logger used by get/set_value */
#include "version.h"		/* API version */

#if !defined(LOG_LEVEL)
#if !defined(_DEBUG)
#define LOG_LEVEL		LVL_NORMAL
#else
#define LOG_LEVEL		LVL_DEBUG
#endif
#endif 
#if !defined(LOG_FILE)
#define LOG_FILE		NULL
#endif 
#if !defined(LOG_SHOW_TIME)
#define LOG_SHOW_TIME	(LOG_LEVEL == LVL_DEBUG)
#endif 

/* DEFINES	*/

/**
 * Default value for parameter 'bitrate_adapting'
 */
#define DEFAULT_PARAM_BITRATE_ADAPTING	PCAN_PARAMETER_OFF
/**
 * Default value for parameter 'listen_only'
 */
#define DEFAULT_PARAM_LISTEN_ONLY		PCAN_PARAMETER_OFF
/**
 * Default value for parameter 'rcv_status'
 */
#define DEFAULT_PARAM_RCV_STATUS		PCAN_PARAMETER_ON
/**
 * Minimum time elapsed (in µs) before refreshing the struct pcaninfo devices
 */
#define PCANINFO_TIME_REFRESH		100000

/**
 * Maximum size for hardware name
 */
#define HARDWARE_NAME_MAX_SIZE			32

/**
 * @defgroup FD_PARAM_INIT key parameters used in CAN FD initialization string
 *
 * @{
 */
#define FD_PARAM_INIT_CLOCK_MHZ		"f_clock_mhz"
#define FD_PARAM_INIT_CLOCK_HZ		"f_clock"
#define FD_PARAM_INIT_NOM_BRP		"nom_brp"
#define FD_PARAM_INIT_NOM_TSEG1		"nom_tseg1"
#define FD_PARAM_INIT_NOM_TSEG2		"nom_tseg2"
#define FD_PARAM_INIT_NOM_SJW		"nom_sjw"
#define FD_PARAM_INIT_DATA_BRP		"data_brp"
#define FD_PARAM_INIT_DATA_TSEG1	"data_tseg1"
#define FD_PARAM_INIT_DATA_TSEG2	"data_tseg2"
#define FD_PARAM_INIT_DATA_SJW		"data_sjw"
/** @} */

/* PRIVATE TYPES	*/
/**
 * Stores information on an initialized PCANBasic channel.
 * This structure maps a TPCANHandle to a file descriptor,
 * and implements linked-list feature.
 */
struct _pcanbasic_channel {
	TPCANHandle channel;		/**< CAN channel. */
	TPCANBaudrate btr0btr1; 	/**< Nominal bit rate as BTR0BTR1. */
	TPCANBitrateFD bitratefd;	/**< String configuration for nominal & data bit rates. */
	int fd;						/**< File descriptor, if 0 then channel is not initialized. */
	uint fd_flags;				/**< File descriptor's flags for libpcanfd. */
	__u8 bitrate_adapting;		/**< Allows initialization with already opened channels. */
	__u8 busoff_reset;			/**< Automatically resets bus when busoff. */
	__u8 listen_only;			/**< Initializes CAN with the mode listen-only. */
	__u8 rcv_status;			/**< Receive status, if 0 can mutes CAN reception (CAN_Read always returns QRCVEMPTY). */
	struct pcaninfo *pinfo;		/**< Pointer to sysfs info structure. */
	SLIST_ENTRY(_pcanbasic_channel) entries;	/**< Single linked list. */

	struct pcbtrace_ctx	tracer;	/**< PCANBasic tracing context. */
};
typedef struct _pcanbasic_channel pcanbasic_channel;
/**
 * PCANBASIC Core persistent data
 */
struct _pcanbasic_core {
	int initialized;				/**< States if structure (SLIST especially) was initialized. */
	struct timeval last_update;		/**< Time of the last pcaninfo hw update (avoid unnecessary updates). */
	struct pcaninfo_list *devices;	/**< Known pcan devices list. */
	SLIST_HEAD(PCANBASIC_channel_SLIST, _pcanbasic_channel) channels;	/* First element of the linked list of initialized channels. */
};
typedef struct _pcanbasic_core pcanbasic_core;

/*	PRIVATE FUNCTIONS DEFINITIONS	*/
/**
* @fn char *pcanbasic_ltrim(char *s)
* @brief Removes leading whitespaces.
*/
static char *pcanbasic_ltrim(char *s);
/**
* @fn char *pcanbasic_rtrim(char *s)
* @brief Removes trailing whitespaces.
*/
static char *pcanbasic_rtrim(char *s);
/**
* @fn char *pcanbasic_trim(char *s)
* @brief Removes leading and trailing whitespaces.
*/
static char *pcanbasic_trim(char *s);

/**
 * @fn void pcanbasic_init(void)
 * @brief Initializes PCANBasic persistent/private data.
 */
static void pcanbasic_init(void);
/**
 * @fn void pcanbasic_refresh_hw(void)
 * @brief Updates struct pcaninfo device list.
 */
static void pcanbasic_refresh_hw(void);
/**
 * @fn void pcanbasic_atexit(void)
 * @brief Cleans up API's objects.
 */
static void pcanbasic_atexit(void);
/**
 * @fn PCANBASIC_channel * pcanbasic_get_channel(TPCANHandle channel)
 * @brief Returns the PCANBasic channel structure based on the channel handle.
 *
 * @param channel The handle of a previously initialized channel.
 * @param opened States if the searched channel must be already initialized (or only pre-initialized).
 * @return An PCANBASIC_channel structure or NULL if the channel was not initialized.
 */
static pcanbasic_channel * pcanbasic_get_channel(TPCANHandle channel, __u8 opened);
/**
 * @fn struct pcaninfo * pcanbasic_get_device(TPCANHandle channel, uint hwtype, uint base, uint irq)
 * @brief Returns the struct pcaninfo corresponding to channel handle and more.
 *
 * @param channel Channel handle.
 * @param hwtype Hardware type.
 * @param base I/O port.
 * @param irq Interrupt.
 * @return Pointer to a pcaninfo structure or NULL if not found.
 */
static struct pcaninfo * pcanbasic_get_device(TPCANHandle channel, uint hwtype, uint base, uint irq);

/**
 * @fn void pcanbasic_get_hw(TPCANHandle channel, enum pcaninfo_hw *hw, uint *index)
 * @brief Gets the harware type and index corresponding to a channel handle.
 *
 * @param[in] channel Channel handle.
 * @param[out] hw Buffer to store the hardware type of the channel.
 * @param[out] index Buffer to store the index of the channel (ex. 3 for PCAN_USBBUS3).
 */
static void pcanbasic_get_hw(TPCANHandle channel, enum pcaninfo_hw *hw, uint *index);
/**
 * @fn TPCANStatus pcanbasic_errno_to_status(int err)
 * @brief Returns the TPCANStatus corresponding to an errno value.
 *
 * @param err An 'errno' value.
 * @return TPCANStatus corresponding to an errno value.
 */
static TPCANStatus pcanbasic_errno_to_status(int err);

/**
 * @fn TPCANStatus pcanbasic_errno_to_status(int err, int ctx)
 * @brief Returns the TPCANStatus corresponding to an errno value.
 *
 * @param err An 'errno' value.
 * @param ctx An 'context' value, see PCB_CTX_xxx defines below.
 * @return TPCANStatus corresponding to an errno value.
 */
static TPCANStatus pcanbasic_errno_to_status_ctx(int err, int ctx);
#define PCB_CTX_READ 1
#define PCB_CTX_WRITE 2
/**
 * @fn pcanbasic_channel* pcanbasic_create_channel(TPCANHandle channel, __u8 add_to_list)
 * @brief Allocates and initializes a pcanbasic_channel structure.
 *
 * @param channel Channel handle.
 * @param add_to_list states if the structure is added to PCANBasic persistent data (g_basiccore) (1=yes, 0=no).
 * @return a  pointer to a PCANBASIC_channel structrue or NULL.
 */
static pcanbasic_channel* pcanbasic_create_channel(TPCANHandle channel, __u8 add_to_list);

/**
 * @fn TPCANStatus pcanbasic_bus_state_to_condition(enum pcanfd_status	bus_state)
 * @brief Returns the channel condition based on a pcanfd_status enum.
 *
 * @param bus_state The bus state to analyze.
 * @return The channel condition corresponding to the bus_state.
 */
static TPCANStatus pcanbasic_bus_state_to_condition(enum pcanfd_status bus_state);
/**
 * @fn __u8 pcanbasic_get_filter(pcanbasic_channel * pchan)
 * @brief Returns the filter status of a channel.
 *
 * @param pchan Channel structure get filter status from.
 * @return The filter status (OPENED, CLOSED or CUSTOM).
 */
static __u8 pcanbasic_get_filter(pcanbasic_channel * pchan);
/**
 * @fn TPCANStatus pcanbasic_get_condition(TPCANHandle channel)
 * @brief Returns the channel's condition.
 *
 * @param channel Channel handle to get condition from.
 * @return The channel's condition (AVAILABLE, OCCUPIED, UNAVAILABLE).
 */
static TPCANStatus pcanbasic_get_condition(TPCANHandle channel);
/**
 * @fn int pcanbasic_get_features(TPCANHandle channel)
 * @brief Returns the channel's features (ex. FD capable or not).
 *
 * @param channel Channel handle to get information from.
 * @return The channel's features (FEATURE_FD_CAPABLE).
 */
static int pcanbasic_get_features(TPCANHandle channel);

/**
 * @fn TPCANStatus pcanbasic_get_version(pcanbasic_channel * pchan, char * buf, int size)
 * @brief Returns the channel's driver version.
 *
 * @param pchan Pointer to an initialized channel.
 * @param buf Buffer to store the version.
 * @param size Size of the buffer.
 * @return A TPCANStatus error code.
 */
static TPCANStatus pcanbasic_get_version(pcanbasic_channel * pchan, char * buf, int size);

/**
 * @fn int pcanbasic_parse_fd_init(struct pcanfd_init * pfdi, TPCANBitrateFD fdbitrate)
 * @brief Parses and fills a pcanfd_init struct based on an FD bitrate string
 *
 * @param pfdi Buffer to store the initialization values.
 * @param fdbitrate FD-Bitrate-Initialization string to be parsed.
 * @return 0 if no error, an errno otherwise.
 */
static int pcanbasic_parse_fd_init(struct pcanfd_init * pfdi, TPCANBitrateFD fdbitrate);
/**
 * @fn TPCANStatus pcanbasic_read_common(TPCANHandle channel, TPCANMsgFD* message, struct timeval *t)
 * @brief A common function to read CAN messages (CAN20 or CANFD message).
 *
 * @param channel Channel handle to read from.
 * @param[out] message Buffer to store the message (stored in a TPCANMsgFD struct even if it is a CAN20 message).
 * @param[out] t Buffer to store the timestamp of the message (NULL to skip).
 * @return A TPCANStatus error code.
 */
static TPCANStatus pcanbasic_read_common(TPCANHandle channel, TPCANMsgFD* message, struct timeval *t);
/**
 * @fn TPCANStatus pcanbasic_write_common(TPCANHandle channel, TPCANMsgFD* message)
 * @brief A common function to write CAN messages (CAN20 or CANFD message).
 *
 * @param channel Channel handle to write to.
 * @param[in] message Pointer to a TPCANMsgFD holding the message to write (either a CAN20 of CANFD message).
 * @return A TPCANStatus error code.
 */
static TPCANStatus pcanbasic_write_common(TPCANHandle channel, TPCANMsgFD* message);

/* PRIVATE VARIABLES	*/
/**
 * Stores persistent PCANBasic data
 */
static pcanbasic_core g_basiccore;

/*	PRIVATE FUNCTIONS	*/
char *pcanbasic_ltrim(char *s)
{
	while (isspace(*s)) s++;
	return s;
}

char *pcanbasic_rtrim(char *s)
{
	char* back = s + strlen(s);
	while (isspace(*--back));
	*(back + 1) = '\0';
	return s;
}

char *pcanbasic_trim(char *s)
{
	return pcanbasic_rtrim(pcanbasic_ltrim(s));
}

void pcanbasic_init(void) {
	pcanlog_set(LOG_LEVEL, LOG_FILE, LOG_SHOW_TIME);
	pcanlog_log(LVL_VERBOSE, "Initializing PCAN-Basic API...\n");
	SLIST_INIT(&g_basiccore.channels);
	g_basiccore.devices = NULL;
	pcanbasic_refresh_hw();
	g_basiccore.initialized = 1;
	atexit(pcanbasic_atexit);
}

void pcanbasic_refresh_hw(void) {
	if (g_basiccore.devices) {
		free(g_basiccore.devices);
		g_basiccore.devices = NULL;
	}
	pcanlog_log(LVL_VERBOSE, "Refreshing hardware device list...\n");
	pcaninfo_get(&g_basiccore.devices, 1);
	gettimeofday(&g_basiccore.last_update, NULL);
}

void pcanbasic_atexit(void) {
	pcanbasic_channel *plist;

	/* assert API is initialized */
	if (!g_basiccore.initialized) {
		return;
	}
	pcanlog_log(LVL_VERBOSE, "Cleaning up PCAN-Basic API...\n");
	/* look through initialized channels */
	for (plist = g_basiccore.channels.slh_first; plist != NULL; plist = plist->entries.sle_next) {
		pcanbasic_uninitialize(plist->channel);
	}
	if (g_basiccore.devices) {
		free(g_basiccore.devices);
		g_basiccore.devices = NULL;
		memset(&g_basiccore, 0, sizeof(g_basiccore));
	}
}

pcanbasic_channel * pcanbasic_get_channel(TPCANHandle channel, __u8 opened) {
	pcanbasic_channel *plist;

	/* assert API is initialized */
	if (!g_basiccore.initialized) {
		pcanbasic_init();
		return NULL;
	}
	/* look through initialized channels */
	for (plist = g_basiccore.channels.slh_first; plist != NULL; plist = plist->entries.sle_next) {
		if (plist->channel == channel) {
			if (opened)
				return (plist->fd > -1) ? plist : NULL;
			else
				return plist;
		}
	}
	return NULL;
}

void pcanbasic_free_channel(pcanbasic_channel * pchan) {
	if (pchan == NULL)
		return;
	if (pchan->fd > -1) {
		// check pending tx msgs
		struct pcanfd_state fds;
		pcanfd_get_state(pchan->fd, &fds);
		if (fds.tx_pending_msgs > 0) {
			// wait some time to transmit any pending msgs
			usleep(50000);
		}
		pcanfd_close(pchan->fd);
		pchan->fd = -1;
	}
	if (pchan->bitratefd) {
		free(pchan->bitratefd);
		pchan->bitratefd = NULL;
	}
	if (pchan->pinfo) {
		free(pchan->pinfo);
		pchan->pinfo = NULL;
	}
	pcbtrace_close(&pchan->tracer);
	free(pchan);
}

void pcanbasic_get_hw(TPCANHandle channel, enum pcaninfo_hw *hw, uint *index) {
	/* get the device's hardware category and minor/index from the channel */
	switch (channel) {
	case PCAN_ISABUS1:
	case PCAN_ISABUS2:
	case PCAN_ISABUS3:
	case PCAN_ISABUS4:
	case PCAN_ISABUS5:
	case PCAN_ISABUS6:
		*hw = PCANINFO_HW_ISA;
		*index = channel - PCAN_ISABUS1 + 1;
		break;
	case PCAN_DNGBUS1:
		*hw = PCANINFO_HW_DNG;
		*index = channel - PCAN_DNGBUS1 + 1;
		break;
	case PCAN_USBBUS1:
	case PCAN_USBBUS2:
	case PCAN_USBBUS3:
	case PCAN_USBBUS4:
	case PCAN_USBBUS5:
	case PCAN_USBBUS6:
	case PCAN_USBBUS7:
	case PCAN_USBBUS8:
		*hw = PCANINFO_HW_USB;
		*index = channel - PCAN_USBBUS1 + 1;
		break;
	case PCAN_USBBUS9:
	case PCAN_USBBUS10:
	case PCAN_USBBUS11:
	case PCAN_USBBUS12:
	case PCAN_USBBUS13:
	case PCAN_USBBUS14:
	case PCAN_USBBUS15:
	case PCAN_USBBUS16:
		*hw = PCANINFO_HW_USB;
		*index = channel - PCAN_USBBUS9 + 9;
		break;
	case PCAN_PCCBUS1:
	case PCAN_PCCBUS2:
		*hw = PCANINFO_HW_PCC;
		*index = channel - PCAN_PCCBUS1 + 1;
		break;
	case PCAN_PCIBUS1:
	case PCAN_PCIBUS2:
	case PCAN_PCIBUS3:
	case PCAN_PCIBUS4:
	case PCAN_PCIBUS5:
	case PCAN_PCIBUS6:
	case PCAN_PCIBUS7:
	case PCAN_PCIBUS8:
		*hw = PCANINFO_HW_PCI;
		*index = channel - PCAN_PCIBUS1 + 1;
		break;
	case PCAN_PCIBUS9:
	case PCAN_PCIBUS10:
	case PCAN_PCIBUS11:
	case PCAN_PCIBUS12:
	case PCAN_PCIBUS13:
	case PCAN_PCIBUS14:
	case PCAN_PCIBUS15:
	case PCAN_PCIBUS16:
		*hw = PCANINFO_HW_PCI;
		*index = channel - PCAN_PCIBUS9 + 9;
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
		*hw = PCANINFO_HW_LAN;
		*index = channel - PCAN_LANBUS1 + 1;
		break;
	default:
		*hw = PCANINFO_HW_NONE;
		*index = 0;
		break;
	}
}

struct pcaninfo * pcanbasic_get_device(TPCANHandle channel, uint hwtype, uint base, uint irq) {
	int i;
	struct pcaninfo * pinfo;
	enum pcaninfo_hw hw;
	uint index, count;
	struct timeval t, tsub;

	/* get time to see if device hw info needs an update */
	gettimeofday(&t, NULL);
	timersub(&t, &g_basiccore.last_update, &tsub);
	if (tsub.tv_sec > 0 || tsub.tv_usec > PCANINFO_TIME_REFRESH)
		pcanbasic_refresh_hw();
	/* get the device's hardware category and minor/index from the channel */
	pcanbasic_get_hw(channel, &hw, &index);
	/* detection algorithm depends on HW being plug'n play */
	switch(hw) {
	case PCANINFO_HW_DNG:
	case PCANINFO_HW_ISA:
		/* loop through all known devices and find matching device */
		for (i = 0; i < g_basiccore.devices->length; i++) {
			pinfo =  &g_basiccore.devices->infos[i];
			if (pinfo->hwtype == hwtype &&
					pinfo->base == base &&
					pinfo->irq == irq)
				return pinfo;
		}
		break;
	default:
		count = 0;
		/* loop through all known devices */
		for (i = 0; i < g_basiccore.devices->length; i++) {
			pinfo =  &g_basiccore.devices->infos[i];
			/* select only devices with the same category */
			if (pinfo->hwcategory == hw) {
				/* the idea is to match the index of the channel
				 * and the "pseudo-detection index" of the device
				 * within that category: ex. PCAN_USBBUS3 will match
				 * the third USB device detected in sysfs.
				 * Remember that in linux each channel of a harware
				 * is seen as a device, so a PCAN-USB-PRO device
				 * (2 channels) is seen in as two different 2 devices.
				 */
				count++;
				if (count == index) {
					return pinfo;
				}
			}
		}
		break;
	}
	return NULL;
}

TPCANStatus pcanbasic_errno_to_status(int err) {
	TPCANStatus sts;
	switch(err) {
	case EAGAIN: /* same as case EWOULDBLOCK: */
		sts = PCAN_ERROR_CAUTION;
		break;
	case EBADF:
		sts = PCAN_ERROR_ILLHW;
		break;
	case ENETDOWN:
		sts = PCAN_ERROR_BUSOFF;
		break;
	case EBADMSG:
	case EINVAL:
		sts = PCAN_ERROR_ILLPARAMVAL;
		break;
	case EOPNOTSUPP:
		sts = PCAN_ERROR_ILLOPERATION;
		break;
	default:
		pcanlog_log(LVL_NORMAL, "Error unhandled errno (%d / 0x%x)\n.", err, err);
		sts = PCAN_ERROR_UNKNOWN;
		break;
	}
	return sts;
}

TPCANStatus pcanbasic_errno_to_status_ctx(int err, int ctx) {
	switch(err) {
	case EAGAIN:
		switch (ctx) {
		case PCB_CTX_READ:
			return PCAN_ERROR_QRCVEMPTY;
		case PCB_CTX_WRITE:
			return PCAN_ERROR_QXMTFULL;
		}
	}
	return pcanbasic_errno_to_status(err);
}

pcanbasic_channel* pcanbasic_create_channel(TPCANHandle channel, __u8 add_to_list) {
	pcanbasic_channel* pchan;

	pchan = (pcanbasic_channel*) calloc(1, sizeof(pcanbasic_channel));
	if (pchan == NULL) {
		return NULL;
	}
	pchan->pinfo = (struct pcaninfo*) calloc(1, sizeof(struct pcaninfo));
	if (pchan->pinfo == NULL) {
		free(pchan);
		return NULL;
	}
	pchan->channel = channel;
	pchan->fd = -1;
	pchan->bitrate_adapting = DEFAULT_PARAM_BITRATE_ADAPTING;
	pchan->listen_only = DEFAULT_PARAM_LISTEN_ONLY;
	pchan->rcv_status = DEFAULT_PARAM_RCV_STATUS;
	pcbtrace_set_defaults(&pchan->tracer);
	pchan->tracer.pinfo = pchan->pinfo;
	if (add_to_list)
		SLIST_INSERT_HEAD(&g_basiccore.channels, pchan, entries);
	return pchan;
}

TPCANStatus pcanbasic_bus_state_to_condition(enum pcanfd_status	bus_state) {
	TPCANStatus sts;

	switch(bus_state) {
	case PCANFD_ERROR_WARNING:
 		sts = PCAN_ERROR_BUSLIGHT;
 		break;
 	case PCANFD_ERROR_PASSIVE:
 		sts = PCAN_ERROR_BUSHEAVY;
 		break;
 	case PCANFD_ERROR_BUSOFF:
 		sts = PCAN_ERROR_BUSOFF;
 		break;
 	case PCANFD_ERROR_ACTIVE:
 	default:
 		sts = PCAN_ERROR_OK;
 		break;
 	}
	return sts;
}

__u8 pcanbasic_get_filter(pcanbasic_channel * pchan) {
	int i, ires;
	__u8 res, bclosed;
	struct pcanfd_msg_filters *pfml;
	struct pcanfd_state fds;

	pfml = NULL;
	memset(&fds, 0, sizeof(fds));
	pcanfd_get_state(pchan->fd, &fds);
	res = PCAN_FILTER_CLOSE;

	if (fds.filters_counter > 0) {
		pfml = calloc(1, sizeof(*pfml) + fds.filters_counter * sizeof(pfml->list[0]));
		pfml->count = fds.filters_counter;
		ires = pcanfd_get_filters(pchan->fd, pfml);
		if (!ires) {
			bclosed = 1;
			/* the driver check for all filters
			 * and if one match the message is allowed.
			 * So an FILTER_OPEN occurs if there is no filter or
			 * if a filter allow all CAN IDs.
			 * Likewise it is FILTER_CLOSE if no filter allow any
			 * CAN message */
			for (i = 0; i < pfml->count; i++) {
				if (pfml->list[i].id_from == 0) {
					if (pfml->list[i].msg_flags & PCANFD_MSG_EXT) {
						if (pfml->list[i].id_to == CAN_MAX_EXTENDED_ID) {
							res = PCAN_FILTER_OPEN;
							goto pcanbasic_get_filter_free;
						}
					}
					else if (pfml->list[i].id_to == CAN_MAX_STANDARD_ID) {
						res = PCAN_FILTER_OPEN;
						goto pcanbasic_get_filter_free;
					}
				}
				if (pfml->list[i].id_from <= pfml->list[i].id_to) {
					bclosed = 0;
				}
			}
			if (bclosed)
				res = PCAN_FILTER_CLOSE;
			else
				res = PCAN_FILTER_CUSTOM;
		}
	}
	else
		res = PCAN_FILTER_OPEN;

pcanbasic_get_filter_free:
	if (pfml)
		free(pfml);
	return res;
}

TPCANStatus pcanbasic_get_condition(TPCANHandle channel) {
	TPCANStatus sts;
	pcanbasic_channel *pchan;
	struct pcaninfo *pci;

	/* get initialized channel only:
	 * if channel is pre-initialized, relying on sysfs is best as
	 * it may have been initialized by another application */
	pchan = pcanbasic_get_channel(channel, 1);
	if (pchan == NULL) {
		/* get device via sysfs */
		pci = pcanbasic_get_device(channel, 0, 0, 0);
		if (pci)
			sts = (pci->bus_state == 0) ? PCAN_CHANNEL_AVAILABLE : PCAN_CHANNEL_OCCUPIED;
		else
			sts = PCAN_CHANNEL_UNAVAILABLE;
	}
	else {
		if (pchan->fd > -1)
			sts = PCAN_CHANNEL_OCCUPIED;
		else
			sts = PCAN_CHANNEL_AVAILABLE;
	}
	return sts;
}

int pcanbasic_get_features(TPCANHandle channel) {
	int value, ibuf, ires;
	pcanbasic_channel *pchan;
	struct pcaninfo *pci;

	value = 0;
	/* search for an initialized channel */
	pchan = pcanbasic_get_channel(channel, 1);
	if (pchan == NULL) {
		/* get device via sysfs */
		pci = pcanbasic_get_device(channel, 0, 0, 0);
	}
	else
		pci = pchan->pinfo;
	if (pci != NULL) {
		value = ((pci->availflag & PCANINFO_FLAG_DATA_BITRATE) == PCANINFO_FLAG_DATA_BITRATE) ? FEATURE_FD_CAPABLE : 0;
		if (pchan != NULL) {
			// get inter-frame delay
			ibuf = 0;
			ires = pcanfd_get_option(pchan->fd, PCANFD_OPT_IFRAME_DELAYUS, &ibuf, sizeof(ibuf));
			if (ires >= 0)
				value |= FEATURE_DELAY_CAPABLE;
		}
	}
	return value;
}

TPCANStatus pcanbasic_get_version(pcanbasic_channel * pchan, char * buf, int size) {
	struct pcanfd_state fds;
	int ires;

	ires = pcanfd_get_state(pchan->fd, &fds);
	if (ires == 0) {
		snprintf(buf, size, "%d.%d.%d", fds.ver_major, fds.ver_minor, fds.ver_subminor);
		return PCAN_ERROR_OK;
	}
	return PCAN_ERROR_UNKNOWN;
}

__u8 pcanbasic_get_fd_dlc(int len) {
	if (len < 0)
		return 0;
	if (len <= 8)
		return len;
	if (len <= 12)
		return 9;
	if (len <= 16)
		return 10;
	if (len <= 20)
		return 11;
	if (len <= 24)
		return 12;
	if (len <= 32)
		return 13;
	if (len <= 48)
		return 14;
	if (len <= 64)
		return 15;
	return 0x0F;
}

int pcanbasic_get_fd_len(__u8 dlc) {
	dlc = dlc & 0x0F;
	if (dlc <= 8)
		return dlc;
	switch(dlc) {
	case 9:
		return 12;
	case 10:
		return 16;
	case 11:
		return 20;
	case 12:
		return 24;
	case 13:
		return 32;
	case 14:
		return 48;
	case 15:
	default:
		return 64;
	}
}

int pcanbasic_parse_fd_init(struct pcanfd_init * pfdi, TPCANBitrateFD fdbitrate) {
	char * sfd_init;
	char * tok, *saveptr1, *saveptr2;
	char * skey, *sval;
	__u32 val;

	if (pfdi == NULL || fdbitrate == NULL) {
		return EINVAL;
	}

	/* Init string example :
	 * f_clock_mhz=20, nom_brp=5, nom_tseg1=2, nom_tseg2=1, nom_sjw=1, data_brp=2, data_tseg1=3, data_tseg2=1, data_sjw=1
	 */
	memset(pfdi, 0, sizeof(*pfdi));
	sfd_init = strndup(fdbitrate, 500);
	if (sfd_init == NULL)
		return ENOMEM;
	pcanlog_log(LVL_DEBUG, "Parsing FD string: '%s'.\n", sfd_init);
	tok = strtok_r(sfd_init, ",", &saveptr1);
	while (tok) {
		pcanlog_log(LVL_DEBUG, "Parsing key/value pair: '%s'.\n", tok);
		skey = strtok_r(tok, "=", &saveptr2);
		if (skey == NULL)
			continue;
		skey = pcanbasic_trim(skey);
		sval = strtok_r(NULL, "=", &saveptr2);
		if (sval == NULL)
			continue;
		sval = pcanbasic_trim(sval);
		val = strtoul(sval, NULL, 0);
		pcanlog_log(LVL_DEBUG, "Parsing key/value pair: '%s' = '%s'.\n", skey, sval);
		if(strcmp(skey, FD_PARAM_INIT_CLOCK_HZ) == 0) {
			pfdi->clock_Hz = val;
		}
		else if(strcmp(skey, FD_PARAM_INIT_CLOCK_MHZ) == 0) {
			pfdi->clock_Hz = val * 1000000;
		}
		else if(strcmp(skey, FD_PARAM_INIT_NOM_BRP) == 0) {
			pfdi->nominal.brp = val;
		}
		else if(strcmp(skey, FD_PARAM_INIT_NOM_TSEG1) == 0) {
			pfdi->nominal.tseg1 = val;
		}
		else if(strcmp(skey, FD_PARAM_INIT_NOM_TSEG2) == 0) {
			pfdi->nominal.tseg2 = val;
		}
		else if(strcmp(skey, FD_PARAM_INIT_NOM_SJW) == 0) {
			pfdi->nominal.sjw = val;
		}
		else if(strcmp(skey, FD_PARAM_INIT_DATA_BRP) == 0) {
			pfdi->data.brp = val;
		}
		else if(strcmp(skey, FD_PARAM_INIT_DATA_TSEG1) == 0) {
			pfdi->data.tseg1 = val;
		}
		else if(strcmp(skey, FD_PARAM_INIT_DATA_TSEG2) == 0) {
			pfdi->data.tseg2 = val;
		}
		else if(strcmp(skey, FD_PARAM_INIT_DATA_SJW) == 0) {
			pfdi->data.sjw = val;
		}
		tok = strtok_r(NULL, ",", &saveptr1);
	}
	free(sfd_init);
	return 0;
}

TPCANStatus pcanbasic_read_common(
        TPCANHandle channel,
		TPCANMsgFD* message,
		struct timeval *t) {
	TPCANStatus sts;
	pcanbasic_channel *pchan;
	struct pcanfd_msg msg;
	int ires;

	if (message == NULL) {
		sts = PCAN_ERROR_ILLPARAMVAL;
		goto pcanbasic_read_common_exit;
	}
	/* get initialized channel */
	pchan = pcanbasic_get_channel(channel, 1);
	if (pchan == NULL) {
		sts = PCAN_ERROR_INITIALIZE;
		goto pcanbasic_read_common_exit;
	}
	/* read msg via libpcanfd */
	ires = pcanfd_recv_msg(pchan->fd, &msg);
	/* SGr Notes: move return code test next to the function call */
	if (ires < 0) {
		sts = pcanbasic_errno_to_status_ctx(-ires, PCB_CTX_READ);
		goto pcanbasic_read_common_exit;
	}
	/* discard message if rcv_status is OFF */
	if (pchan->rcv_status == PCAN_PARAMETER_OFF) {
		sts = PCAN_ERROR_QRCVEMPTY;
		goto pcanbasic_read_common_exit;
	}
	sts = PCAN_ERROR_OK;
	/* convert msg to PCANBasic structure */
	memset(message, 0, sizeof(*message));
	message->ID = msg.id;
	message->DLC = pcanbasic_get_fd_dlc(msg.data_len);
	switch (msg.type) {
	case PCANFD_TYPE_CANFD_MSG:
		message->MSGTYPE |= PCAN_MESSAGE_FD;
		/* no break */
	case PCANFD_TYPE_CAN20_MSG:
		if (msg.data_len > sizeof(message->DATA))
			pcanlog_log(LVL_ALWAYS, "Received malformed CAN message (data_len=%d)", msg.data_len);
		memcpy(message->DATA, msg.data, msg.data_len);
		/* standard or extended CAN msg */
		if((msg.flags & PCANFD_MSG_EXT) == PCANFD_MSG_EXT)
			message->MSGTYPE |= PCAN_MESSAGE_EXTENDED;
		else
			message->MSGTYPE |= PCAN_MESSAGE_STANDARD;
		/* RTR msg ? */
		if((msg.flags & PCANFD_MSG_RTR) == PCANFD_MSG_RTR)
			message->MSGTYPE |= PCAN_MESSAGE_RTR;
		/* FD flags */
		if((msg.flags & PCANFD_MSG_BRS) == PCANFD_MSG_BRS)
			message->MSGTYPE |= PCAN_MESSAGE_BRS;
		if((msg.flags & PCANFD_MSG_ESI) == PCANFD_MSG_ESI)
			message->MSGTYPE |= PCANFD_MSG_ESI;
		break;
	case PCANFD_TYPE_STATUS:
		if (pchan->busoff_reset && (msg.flags & PCANFD_ERROR_BUS) && msg.id == PCANFD_ERROR_BUSOFF) {
			/* auto-reset */
			pcanbasic_reset(channel);
			sts = PCAN_ERROR_BUSOFF;
			goto pcanbasic_read_common_exit;
		}
		message->MSGTYPE = PCAN_MESSAGE_STATUS;
		message->DLC = 4;
		switch (msg.id) {
		case PCANFD_ERROR_WARNING:
			message->DATA[3] |= CAN_ERR_BUSLIGHT;
			break;
		case PCANFD_ERROR_PASSIVE:
			message->DATA[3] |= CAN_ERR_BUSHEAVY;
			break;
		case PCANFD_ERROR_BUSOFF:
			message->DATA[3] |= CAN_ERR_BUSOFF;
			break;
		case PCANFD_RX_EMPTY:
			message->DATA[3] |= CAN_ERR_QRCVEMPTY;
			break;
		case PCANFD_RX_OVERFLOW:
			message->DATA[3] |= CAN_ERR_OVERRUN;
			break;
		case PCANFD_TX_OVERFLOW:
			message->DATA[3] |= CAN_ERR_QXMTFULL;
			break;

		default:
		case PCANFD_TX_EMPTY:
			message->DATA[3] |= CAN_ERR_RESOURCE;
			break;

		case PCANFD_ERROR_ACTIVE:
			break;
		}
		break;
	case PCANFD_TYPE_ERROR_MSG:
		message->ID = 1 << msg.id;
		message->MSGTYPE = PCAN_MESSAGE_ERRFRAME;
		message->DATA[0] = (msg.flags & PCANFD_ERRMSG_RX) == PCANFD_ERRMSG_RX ? 1 : 0;
		message->DATA[1] = msg.data[0];
		message->DATA[2] = msg.ctrlr_data[0];
		message->DATA[3] = msg.ctrlr_data[1];
		break;
	}
	/* copy timestamp */
	if (t != NULL)
		*t = msg.timestamp;
	pcanlog_log(LVL_VERBOSE, "Read message: ID=0x%04x; TYPE=0x%02x; FLAGS=0x%02x; DATA=[0x%02x...].\n",
		msg.id, msg.type, msg.flags, msg.data[0]);
	/* trace message */
	pcbtrace_write_msg(&pchan->tracer, message, msg.data_len, &msg.timestamp, 1);

pcanbasic_read_common_exit:
	return sts;
}

TPCANStatus pcanbasic_write_common(
        TPCANHandle channel,
        TPCANMsgFD* message) {
	TPCANStatus sts;
	pcanbasic_channel *pchan;
	struct pcanfd_msg msg;
	struct timeval tv;
	int ires;

	if (message == NULL) {
		sts = PCAN_ERROR_ILLPARAMVAL;
		goto pcanbasic_write_exit;
	}

	/* get initialized channel */
	pchan = pcanbasic_get_channel(channel, 1);
	if (pchan == NULL) {
		sts = PCAN_ERROR_INITIALIZE;
		goto pcanbasic_write_exit;
	}
	/* convert message and send it */
	memset(&msg, 0, sizeof(msg));
	msg.id = message->ID;
	msg.data_len = pcanbasic_get_fd_len(message->DLC);
	memcpy(msg.data, message->DATA, msg.data_len);
	/* set message FD type */
	if ((message->MSGTYPE & PCAN_MESSAGE_FD) == PCAN_MESSAGE_FD)
		msg.type = PCANFD_TYPE_CANFD_MSG;
	else
		msg.type = PCANFD_TYPE_CAN20_MSG;
	/* set message type */
	if ((message->MSGTYPE & PCAN_MESSAGE_EXTENDED) == PCAN_MESSAGE_EXTENDED)
		msg.flags = PCANFD_MSG_EXT;
	else
		msg.flags = PCANFD_MSG_STD;
	/* set extra flags */
	if ((message->MSGTYPE & PCAN_MESSAGE_RTR) == PCAN_MESSAGE_RTR)
		msg.flags |= PCANFD_MSG_RTR;
	if ((message->MSGTYPE & PCAN_MESSAGE_BRS) == PCAN_MESSAGE_BRS)
		msg.flags |= PCANFD_MSG_BRS;
	pcanlog_log(LVL_VERBOSE, "Writing message: ID=0x%04x; TYPE=0x%02x; FLAGS=0x%02x; DATA=[0x%02x...].\n",
		msg.id, msg.type, msg.flags, msg.data[0]);
	ires = pcanfd_send_msg(pchan->fd, &msg);
	if (ires < 0) {
		sts = pcanbasic_errno_to_status_ctx(-ires, PCB_CTX_WRITE);
		/* check busoff auto reset */
		if(sts == PCANFD_ERROR_BUSOFF && pchan->busoff_reset)
			pcanbasic_reset(channel);
		goto pcanbasic_write_exit;
	}
	gettimeofday(&tv, NULL);
	pcbtrace_write_msg(&pchan->tracer, message, msg.data_len, &tv, 0);
	sts = PCAN_ERROR_OK;

pcanbasic_write_exit:
	return sts;

}

/* GLOBAL FUNCTIONS */
struct pcaninfo * pcanbasic_get_info(TPCANHandle channel) {
	pcanbasic_channel * pcbch;

	pcbch = pcanbasic_get_channel(channel, 1);
	if (pcbch)
		return pcbch->pinfo;
	return NULL;
}

TPCANHandle pcanbasic_get_handle(char * device, struct pcaninfo_list * plist) {
	TPCANHandle result = PCAN_NONEBUS;
	struct pcaninfo_list * pcil;
	struct pcaninfo * pci;
	enum pcaninfo_hw hw_count[PCANINFO_HW_COUNT];
	int i;

	if (device == NULL)
		return PCAN_NONEBUS;
	/* auto-load devices information if it is not provided */
	if (plist == NULL) {
		if (pcaninfo_get(&pcil, 1) != 0)
			return result;
	}
	else
		pcil = plist;
	/* loop through PCANInfo devices and count their number by category:
	 * 	ex. PCAN_PCIBUS3 = the third PCAN PCI device */
	memset(&hw_count, 0, sizeof(hw_count));
	for (i = 0; i < pcil->length; i++) {
		pci = &pcil->infos[i];
		/* load pci information if it was not done yet */
		if (!(pci->availflag & PCANINFO_FLAG_INITIALIZED))
			pcaninfo_update(pci);
		hw_count[pci->hwcategory]++;

		/* SGr Notes: "dev" is not part of sysfs in RT world: do the
		 * cmp on the device path instead */
		//if (strcmp(pci->dev, device) == 0)
		if (strcmp(pci->path, device) == 0)
		{
			// device found, will exit for loop
			i = pcil->length;
			// compute handle
			switch(pci->hwcategory) {
			case PCANINFO_HW_DNG:
				result = PCAN_DNGBUS1 - 1 + hw_count[pci->hwcategory];
				break;
			case PCANINFO_HW_ISA:
				result = PCAN_ISABUS1 - 1 + hw_count[pci->hwcategory];
				break;
			case PCANINFO_HW_PCC:
				result = PCAN_PCCBUS1 - 1 + hw_count[pci->hwcategory];
				break;
			case PCANINFO_HW_PCI:
				if (hw_count[pci->hwcategory] < 9)
					result = PCAN_PCIBUS1 - 1 + hw_count[pci->hwcategory];
				else
					result = PCAN_PCIBUS9 - 9 + hw_count[pci->hwcategory];
				break;
			case PCANINFO_HW_USB:
				if (hw_count[pci->hwcategory] < 9)
					result = PCAN_USBBUS1 - 1 + hw_count[pci->hwcategory];
				else
					result = PCAN_USBBUS9 - 9 + hw_count[pci->hwcategory];
				break;
			case PCANINFO_HW_LAN:
				result = PCAN_LANBUS1 - 1 + hw_count[pci->hwcategory];
				break;
			case PCANINFO_HW_PEAKCAN:
			case PCANINFO_HW_VIRTUAL:
			case PCANINFO_HW_NONE:
				result = PCAN_NONEBUS;
				break;
			}
		}
	}
	/* release local allocation */
	if (plist == NULL && pcil)
		free(pcil);

	return result;
}

/* PCANBASIC API	*/
TPCANStatus pcanbasic_initialize(
        TPCANHandle channel,
        TPCANBaudrate btr0btr1,
        TPCANType hwtype,
		DWORD base,
		WORD irq) {
	TPCANStatus sts;
	pcanbasic_channel *pchan;
	struct pcaninfo * pinfo;
	__u8 inserted;


	inserted = 0;
	/* check if channel exists */
	pchan = pcanbasic_get_channel(channel, 0);
	if (pchan != NULL) {
		/* channel is already initialized */
		if (pchan->fd > -1) {
			sts = PCAN_ERROR_INITIALIZE;
			/* initialized with different bitrate */
			if (pchan->btr0btr1 != btr0btr1 && pchan->bitrate_adapting)
				sts = PCAN_ERROR_CAUTION;
			goto pcanbasic_initialize_exit;
		}
		/* channel is not initialized */
		else {
			/* this means that flags were set for initialization
			 * jump directly after malloc */
			inserted = 1;
			goto pcanbasic_initialize_malloc_post;
		}
	}
	/* allocate and initialize a new PCANBASIC_channel */
	pchan = pcanbasic_create_channel(channel, 0);
	if (pchan == NULL) {
		errno = ENOMEM;
		sts = PCAN_ERROR_UNKNOWN;
		goto pcanbasic_initialize_exit;
	}

pcanbasic_initialize_malloc_post:
	sts = PCAN_ERROR_OK;
	/* find a corresponding device */
	pinfo = pcanbasic_get_device(channel, hwtype, base, irq);
	if (pinfo == NULL) {
		sts = PCAN_ERROR_NODRIVER;
		if (inserted)
			SLIST_REMOVE(&g_basiccore.channels, pchan, _pcanbasic_channel, entries);
		pcanbasic_free_channel(pchan);
		goto pcanbasic_initialize_exit;
	}
	/* complete struct initialization */
	pchan->btr0btr1 = btr0btr1;
	memcpy(pchan->pinfo, pinfo, sizeof(*pinfo));
	/* check if CAN channel is used by another application */
	if (pchan->pinfo->bus_state != 0) {
		/* check bitrate adaptation */
		if (!pchan->bitrate_adapting) {
			/* not allowed, compare btr0btr1 */
			if (pchan->pinfo->btr0btr1 != btr0btr1) {
				sts = PCAN_ERROR_INITIALIZE;
				if (inserted)
					SLIST_REMOVE(&g_basiccore.channels, pchan, _pcanbasic_channel, entries);
				pcanbasic_free_channel(pchan);
				goto pcanbasic_initialize_exit;
			}
		}
		else {
			sts = PCAN_ERROR_CAUTION;
		}
	}

	pchan->fd_flags = OFD_BITRATE | OFD_BTR0BTR1 | OFD_NONBLOCKING;
	if (pchan->listen_only == PCAN_PARAMETER_ON)
		pchan->fd_flags |= PCANFD_INIT_LISTEN_ONLY;
	pchan->fd = pcanfd_open(pchan->pinfo->path, pchan->fd_flags, pchan->btr0btr1);
	if (pchan->fd < 0) {
		sts = PCAN_ERROR_ILLOPERATION;
		if (inserted)
			SLIST_REMOVE(&g_basiccore.channels, pchan, _pcanbasic_channel, entries);
		pcanbasic_free_channel(pchan);
		goto pcanbasic_initialize_exit;
	}
	/* insert new channel info in list */
	if (!inserted)
		SLIST_INSERT_HEAD(&g_basiccore.channels, pchan, entries);
	/* remove previously set filters */
	pcanfd_del_filters(pchan->fd);
	/* refresh pcaninfo struct to update bitrate information */
	if (sts == PCAN_ERROR_OK)
		pcaninfo_update(pchan->pinfo);

pcanbasic_initialize_exit:
	return sts;
}

TPCANStatus pcanbasic_initialize_fd(
    TPCANHandle channel,
	TPCANBitrateFD bitratefd) {
	TPCANStatus sts;
	pcanbasic_channel *pchan;
	struct pcaninfo * pinfo;
	__u8 inserted;
	struct pcanfd_init fdi;

	inserted = 0;
	/* check if channel exists */
	pchan = pcanbasic_get_channel(channel, 0);
	if (pchan != NULL) {
		/* channel is already initialized */
		if (pchan->fd > -1) {
			sts = PCAN_ERROR_INITIALIZE;
			/* initialized with different bitrate */
			if (pchan->bitrate_adapting && strcmp(pchan->bitratefd, bitratefd) == 0)
				sts = PCAN_ERROR_CAUTION;
			goto pcanbasic_initialize_fd_exit;
		}
		/* channel is not initialized */
		else {
			/* this means that flags were set for initialization
			 * jump directly after malloc */
			inserted = 1;
			goto pcanbasic_initialize_malloc_post;
		}
	}
	/* allocate and initialize a new PCANBASIC_channel */
	pchan = pcanbasic_create_channel(channel, 0);
	if (pchan == NULL) {
		errno = ENOMEM;
		sts = PCAN_ERROR_UNKNOWN;
		goto pcanbasic_initialize_fd_exit;
	}

pcanbasic_initialize_malloc_post:
	sts = PCAN_ERROR_OK;
	/* find a corresponding device */
	pinfo = pcanbasic_get_device(channel, 0, 0, 0);
	if (pinfo == NULL) {
		sts = PCAN_ERROR_NODRIVER;
		if (inserted)
			SLIST_REMOVE(&g_basiccore.channels, pchan, _pcanbasic_channel, entries);
		pcanbasic_free_channel(pchan);
		goto pcanbasic_initialize_fd_exit;
	}
	/* complete struct initialization */
	if (pcanbasic_parse_fd_init(&fdi, bitratefd) != 0) {
		sts = PCAN_ERROR_INITIALIZE;
		if (inserted)
			SLIST_REMOVE(&g_basiccore.channels, pchan, _pcanbasic_channel, entries);
		pcanbasic_free_channel(pchan);
		goto pcanbasic_initialize_fd_exit;
	}
	if (pchan->bitratefd)
		free(pchan->bitratefd);
	pchan->bitratefd = strdup(bitratefd);
	memcpy(pchan->pinfo, pinfo, sizeof(*pinfo));
	/* check if CAN channel is used by another application */
	if (pchan->pinfo->bus_state != 0) {
		/* check bitrate adaptation */
		if (!pchan->bitrate_adapting) {
			/* not allowed, compare bitratefd */
			if (pchan->pinfo->nom_bitrate != fdi.nominal.bitrate &&
				pchan->pinfo->data_bitrate != fdi.data.bitrate) {
				sts = PCAN_ERROR_INITIALIZE;
				if (inserted)
					SLIST_REMOVE(&g_basiccore.channels, pchan, _pcanbasic_channel, entries);
				pcanbasic_free_channel(pchan);
				goto pcanbasic_initialize_fd_exit;
			}
		}
		else {
			sts = PCAN_ERROR_CAUTION;
		}
	}

	pchan->fd_flags = OFD_BITRATE | OFD_DBITRATE | OFD_BRPTSEGSJW | OFD_CLOCKHZ | OFD_NONBLOCKING;
	if (pchan->listen_only == PCAN_PARAMETER_ON)
		pchan->fd_flags |= PCANFD_INIT_LISTEN_ONLY;
	pchan->fd = pcanfd_open(pchan->pinfo->path, pchan->fd_flags,
			fdi.nominal.brp, fdi.nominal.tseg1, fdi.nominal.tseg2, fdi.nominal.sjw,
			fdi.data.brp, fdi.data.tseg1, fdi.data.tseg2, fdi.data.sjw,
			fdi.clock_Hz);
	if (pchan->fd < 0) {
		sts = PCAN_ERROR_ILLOPERATION;
		if (inserted)
			SLIST_REMOVE(&g_basiccore.channels, pchan, _pcanbasic_channel, entries);
		pcanbasic_free_channel(pchan);
		goto pcanbasic_initialize_fd_exit;
	}
	/* insert new channel info in list */
	if (!inserted)
		SLIST_INSERT_HEAD(&g_basiccore.channels, pchan, entries);
	/* remove previously set filters */
	pcanfd_del_filters(pchan->fd);
	/* refresh pcaninfo struct to update bitrate information */
	if (sts == PCAN_ERROR_OK)
		pcaninfo_update(pchan->pinfo);

pcanbasic_initialize_fd_exit:
	return sts;
}

TPCANStatus pcanbasic_uninitialize(
        TPCANHandle channel) {
	TPCANStatus sts;
	pcanbasic_channel *pchan;

	/* PCAN_NONEBUS clears all channels */
	if (channel == PCAN_NONEBUS) {
		pcanbasic_atexit();
		sts = PCAN_ERROR_OK;
		goto pcanbasic_uninitialize_exit;
	}
	/* get channel */
	pchan = pcanbasic_get_channel(channel, 0);
	if (pchan == NULL) {
		sts = PCAN_ERROR_INITIALIZE;
		goto pcanbasic_uninitialize_exit;
	}
	/* close and remove channel from initialized channels' list */
	SLIST_REMOVE(&g_basiccore.channels, pchan, _pcanbasic_channel, entries);
	pcanbasic_free_channel(pchan);
	sts = PCAN_ERROR_OK;

pcanbasic_uninitialize_exit:
	return sts;
}

TPCANStatus pcanbasic_reset(
        TPCANHandle channel) {
	TPCANStatus sts;
	pcanbasic_channel *pchan;
	struct pcanfd_init pfdinit;

	/* get channel */
	pchan = pcanbasic_get_channel(channel, 1);
	if (pchan == NULL) {
		sts = PCAN_ERROR_INITIALIZE;
		goto pcanbasic_reset_exit;
	}
	/* get fd initialization to restore it later */
	pcanfd_get_init(pchan->fd, &pfdinit);
	if (pchan->listen_only)
		pfdinit.flags |= PCANFD_INIT_LISTEN_ONLY;
	/* close and open file descriptor */
	pcanfd_close(pchan->fd);
	pchan->fd = pcanfd_open(pchan->pinfo->path, OFD_NONBLOCKING);	/* no flag as we will use set_init */
	if (pchan->fd < 0) {
		sts = PCAN_ERROR_ILLOPERATION;
		goto pcanbasic_reset_exit;
	}
	/* re-set config */
	if (pcanfd_set_init(pchan->fd, &pfdinit) < 0) {
		sts = PCAN_ERROR_ILLOPERATION;
		goto pcanbasic_reset_exit;
	}
	sts = PCAN_ERROR_OK;

pcanbasic_reset_exit:
	return sts;
}

TPCANStatus pcanbasic_get_status(
        TPCANHandle channel) {
	TPCANStatus sts;
	pcanbasic_channel *pchan;
	struct pcanfd_state fds;
	int ires;

	/* get channel */
	pchan = pcanbasic_get_channel(channel, 1);
	if (pchan == NULL) {
		sts = PCAN_ERROR_INITIALIZE;
		goto pcanbasic_get_status_exit;
	}
	/* read status and convert result */
	ires = pcanfd_get_state(pchan->fd, &fds);
	if (ires < 0) {
		sts = pcanbasic_errno_to_status(-ires);
		goto pcanbasic_get_status_exit;
	}
	sts = pcanbasic_bus_state_to_condition(fds.bus_state);

pcanbasic_get_status_exit:
	return sts;
}

TPCANStatus pcanbasic_read(
        TPCANHandle channel,
        TPCANMsg* message,
        TPCANTimestamp* timestamp) {
	TPCANStatus sts;
	TPCANMsgFD msgfd;
	struct timeval t;

	if (message == NULL) {
		sts = PCAN_ERROR_ILLPARAMVAL;
		goto pcanbasic_read_exit;
	}
	sts = pcanbasic_read_common(channel, &msgfd, &t);
	if (sts == PCAN_ERROR_OK) {
		/* convert TPCANMsgFD message to TPCANMsg */
		memset(message, 0, sizeof(*message));
		message->ID = msgfd.ID;
		memcpy(message->DATA, msgfd.DATA, 8);
		message->LEN = msgfd.DLC;
		message->MSGTYPE = msgfd.MSGTYPE;
		/* convert timestamp */
		if (timestamp != NULL) {
			/* overflow: idea is to get millis in 64bit
			 * and see if it match the 32b version
			 * if not then an overflow occured */
			__u64 millis = ((__u64) t.tv_sec) * 1000 + t.tv_usec / 1000;
			timestamp->micros = t.tv_usec % 1000;
			timestamp->millis = millis;
			if (timestamp->millis != millis)
				timestamp->millis_overflow = (millis - timestamp->millis) >> (sizeof(timestamp->millis)*8);
			else
				timestamp->millis_overflow = 0;
		}
	}

pcanbasic_read_exit:
	return sts;
}

TPCANStatus pcanbasic_read_fd(
    TPCANHandle channel,
	TPCANMsgFD* message,
	TPCANTimestampFD *timestamp) {
	TPCANStatus sts;
	struct timeval t;

	sts = pcanbasic_read_common(channel, message, &t);
	if (sts == PCAN_ERROR_OK && timestamp != NULL) {
		*timestamp = ((__u64) t.tv_sec) * 1000000 + t.tv_usec;
	}

	return sts;
}

TPCANStatus pcanbasic_write(
        TPCANHandle channel,
        TPCANMsg* message) {
	TPCANStatus sts;
	TPCANMsgFD msgfd;

	if (message == NULL) {
		sts = PCAN_ERROR_ILLPARAMVAL;
		goto pcanbasic_write_exit;
	}
	/* convert std message to a FD message and use a single common function */
	memset(&msgfd, 0, sizeof(msgfd));
	msgfd.ID = message->ID;
	msgfd.DLC = message->LEN;
	msgfd.MSGTYPE = message->MSGTYPE;
	memcpy(msgfd.DATA, message->DATA, message->LEN);
	sts = pcanbasic_write_common(channel, &msgfd);

pcanbasic_write_exit:
	return sts;
}

TPCANStatus pcanbasic_write_fd(
    TPCANHandle channel,
	TPCANMsgFD* message) {
	return pcanbasic_write_common(channel, message);
}

TPCANStatus pcanbasic_filter(
        TPCANHandle channel,
        DWORD from,
        DWORD to,
        TPCANMode mode) {
	TPCANStatus sts;
	pcanbasic_channel *pchan;
	struct pcanfd_msg_filter filter;
	int ires;

	/* get channel */
	pchan = pcanbasic_get_channel(channel, 1);
	if (pchan == NULL) {
		sts = PCAN_ERROR_INITIALIZE;
		goto pcanbasic_filter_exit;
	}

	filter.id_from = from;
	filter.id_to = to;
	switch (mode) {
	case PCAN_MODE_EXTENDED:
		filter.msg_flags = PCAN_MESSAGE_EXTENDED;
		break;
	case PCAN_MODE_STANDARD:
	default:
		filter.msg_flags = PCAN_MESSAGE_STANDARD;
		break;
	}
	ires = pcanfd_add_filter(pchan->fd, &filter);
	if (ires < 0) {
		sts = pcanbasic_errno_to_status(-ires);
		goto pcanbasic_filter_exit;
	}
	sts = PCAN_ERROR_OK;

pcanbasic_filter_exit:
	return sts;
}

TPCANStatus pcanbasic_get_value(
        TPCANHandle channel,
        TPCANParameter parameter,
        void* buffer,
        DWORD len) {
	TPCANStatus sts;
	pcanbasic_channel *pchan;
	char sversion[20];
	struct pcanfd_state state;
	struct __array_of_struct(pcanfd_msg_filter, 1);
	int ires;
	size_t size;
	__u8 ctmp;
	__u32 itmp;

	/* check parameter */
	if (buffer == NULL || len <= 0) {
		sts = PCAN_ERROR_ILLPARAMVAL;
		goto pcanbasic_get_value_exit;
	}

	ctmp = 0;
	itmp = 0;
	/* clear buffer to avoid memcpy side effects
	 * with a type bigger than the one required */
	memset(buffer, 0, len);

	/* handle parameter that do not require a channel */
	switch(parameter) {
	case PCAN_API_VERSION:
		size = sizeof(sversion);
		snprintf(sversion, size, "%d.%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_BUILD);
		size = strlen(sversion) * sizeof(sversion[0]);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, &sversion, size);
		goto pcanbasic_get_value_exit_ok;
		break;
	case PCAN_LISTEN_ONLY:
		/* can be called with an uninitialized channel */
		size = sizeof(ctmp);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		/* get channel (can be initialized or not) */
		pchan = pcanbasic_get_channel(channel, 0);
		ctmp = (pchan != NULL) ? pchan->listen_only : DEFAULT_PARAM_LISTEN_ONLY;
		memcpy(buffer, &ctmp, size);
		goto pcanbasic_get_value_exit_ok;
		break;
	case PCAN_LOG_LOCATION:
		if (len < 1) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		pcblog_get_location(buffer, len);
		goto pcanbasic_get_value_exit_ok;
		break;
	case PCAN_LOG_STATUS:
		size = sizeof(itmp);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		itmp = pcblog_get_status();
		memcpy(buffer, &itmp, size);
		goto pcanbasic_get_value_exit_ok;
		break;
	case PCAN_LOG_CONFIGURE:
		size = sizeof(itmp);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		itmp = pcblog_get_config();
		memcpy(buffer, &itmp, size);
		goto pcanbasic_get_value_exit_ok;
		break;
	case PCAN_LOG_TEXT:
		/* not possible */
		sts = PCAN_ERROR_ILLPARAMTYPE;
		goto pcanbasic_get_value_exit;
		break;
	case PCAN_RECEIVE_STATUS:
		/* can be called with an uninitialized channel */
		size = sizeof(ctmp);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		/* get channel (can be initialized or not) */
		pchan = pcanbasic_get_channel(channel, 0);
		ctmp = (pchan != NULL) ? pchan->rcv_status : DEFAULT_PARAM_RCV_STATUS;
		memcpy(buffer, &ctmp, size);
		goto pcanbasic_get_value_exit_ok;
		break;
	case PCAN_CHANNEL_CONDITION:
		/* can be called with an uninitialized channel */
		size = sizeof(TPCANStatus);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		sts = pcanbasic_get_condition(channel);
		memcpy(buffer, &sts, size);
		goto pcanbasic_get_value_exit_ok;
		break;
	case PCAN_CHANNEL_IDENTIFYING:
		/* not implemented in PCAN linux driver */
		sts = PCAN_ERROR_ILLPARAMTYPE;
		break;
	case PCAN_CHANNEL_FEATURES:
		/* can be called with an uninitialized channel */
		size = sizeof(TPCANStatus);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		ires = pcanbasic_get_features(channel);
		memcpy(buffer, &ires, size);
		goto pcanbasic_get_value_exit_ok;
		break;
	case PCAN_BITRATE_ADAPTING:
		/* can be called with an uninitialized channel */
		size = sizeof(ctmp);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		/* get channel (can be initialized or not) */
		pchan = pcanbasic_get_channel(channel, 0);
		ctmp = (pchan != NULL) ? pchan->bitrate_adapting : DEFAULT_PARAM_BITRATE_ADAPTING;
		memcpy(buffer, &ctmp, size);
		goto pcanbasic_get_value_exit_ok;
		break;
	}

	/* get initialized channel */
	pchan = pcanbasic_get_channel(channel, 1);
	if (pchan == NULL || pchan->fd < 0) {
		sts = PCAN_ERROR_INITIALIZE;
		goto pcanbasic_get_value_exit;
	}

	switch(parameter) {
	case PCAN_DEVICE_NUMBER:
		size = sizeof(itmp);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		ires = pcanfd_get_device_id(pchan->fd, (__u32*)&itmp);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, &itmp, size);
		break;
	case PCAN_5VOLTS_POWER:
		/* not supported by driver */
		sts = PCAN_ERROR_ILLPARAMTYPE;
		goto pcanbasic_get_value_exit;
		break;
	case PCAN_RECEIVE_EVENT:
		size = sizeof(itmp);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, &pchan->fd, size);
		break;
	case PCAN_MESSAGE_FILTER:
		size = sizeof(ctmp);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		ctmp = pcanbasic_get_filter(pchan);
		memcpy(buffer, &ctmp, size);
		break;
	case PCAN_CHANNEL_VERSION:
		sts = pcanbasic_get_version(pchan, buffer, len);
		goto pcanbasic_get_value_exit;
		break;
	case PCAN_BUSOFF_AUTORESET:
		size = sizeof(pchan->busoff_reset);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, &pchan->busoff_reset, size);
		break;
	case PCAN_HARDWARE_NAME:
		size = strnlen(pchan->pinfo->type, HARDWARE_NAME_MAX_SIZE);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, pchan->pinfo->type, size);
		break;
	case PCAN_CONTROLLER_NUMBER:
		ires = pcanfd_get_state(pchan->fd, &state);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_get_value_exit;
		}
		size = sizeof(state.channel_number);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, &state.channel_number, size);
		break;
	case PCAN_TRACE_LOCATION:
		size = strlen(pchan->tracer.directory);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, pchan->tracer.directory, size);
		break;
	case PCAN_TRACE_STATUS:
		size = sizeof(pchan->tracer.status);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, &pchan->tracer.status, size);
		break;
	case PCAN_TRACE_SIZE:
		size = sizeof(pchan->tracer.maxsize);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, &pchan->tracer.maxsize, size);
		break;
	case PCAN_TRACE_CONFIGURE:
		size = sizeof(pchan->tracer.flags);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, &pchan->tracer.flags, size);
		break;
	case PCAN_BITRATE_INFO:
		size = sizeof(pchan->pinfo->btr0btr1);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, &pchan->pinfo->btr0btr1, size);
		break;
	case PCAN_BITRATE_INFO_FD:
		size = 0;
		if (pchan->bitratefd != NULL)
			size = strlen(pchan->bitratefd);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		snprintf(buffer, size, "%s", pchan->bitratefd);
		break;
	case PCAN_BUSSPEED_NOMINAL:
		size = sizeof(pchan->pinfo->nom_bitrate);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, &pchan->pinfo->nom_bitrate, size);
		break;
	case PCAN_BUSSPEED_DATA:
		size = sizeof(pchan->pinfo->data_bitrate);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, &pchan->pinfo->data_bitrate, size);
		break;
	case PCAN_IP_ADDRESS:
	case PCAN_LAN_SERVICE_STATUS:
		/* not supported by driver */
		sts = PCAN_ERROR_NODRIVER;
		goto pcanbasic_get_value_exit;
		break;
	case PCAN_ALLOW_ERROR_FRAMES:
		size = sizeof(__u8);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		ires = pcanfd_get_option(pchan->fd, PCANFD_OPT_ALLOWED_MSGS, &itmp, sizeof(itmp));
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_get_value_exit;
		}
		itmp = (itmp & PCANFD_ALLOWED_MSG_ERROR) == PCANFD_ALLOWED_MSG_ERROR ?
				PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
		memcpy(buffer, &itmp, size);
		break;
	case PCAN_ALLOW_RTR_FRAMES:
		size = sizeof(__u8);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		ires = pcanfd_get_option(pchan->fd, PCANFD_OPT_ALLOWED_MSGS, &itmp, sizeof(itmp));
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_get_value_exit;
		}
		itmp = (itmp & PCANFD_ALLOWED_MSG_RTR) == PCANFD_ALLOWED_MSG_RTR ?
				PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
		memcpy(buffer, &itmp, size);
		break;
	case PCAN_ALLOW_STATUS_FRAMES:
		size = sizeof(__u8);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		ires = pcanfd_get_option(pchan->fd, PCANFD_OPT_ALLOWED_MSGS, &itmp, sizeof(itmp));
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_get_value_exit;
		}
		itmp = (itmp & PCANFD_ALLOWED_MSG_STATUS) == PCANFD_ALLOWED_MSG_STATUS ?
				PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
		memcpy(buffer, &itmp, size);
		break;
	case PCAN_INTERFRAME_DELAY:
		size = sizeof(__u32);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		ires = pcanfd_get_option(pchan->fd, PCANFD_OPT_IFRAME_DELAYUS, buffer, len);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_get_value_exit;
		}
		break;
	case PCAN_ACCEPTANCE_FILTER_11BIT:
		size = sizeof(__u64);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		ires = pcanfd_get_option(pchan->fd, PCANFD_OPT_ACC_FILTER_11B, buffer, len);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_get_value_exit;
		}
		break;
	case PCAN_ACCEPTANCE_FILTER_29BIT:
		size = sizeof(__u64);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		ires = pcanfd_get_option(pchan->fd, PCANFD_OPT_ACC_FILTER_29B, buffer, len);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_get_value_exit;
		}
		break;
	case PCAN_IO_DIGITAL_CONFIGURATION:
		size = sizeof(__u32);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		ires = pcanfd_get_option(pchan->fd, PCANFD_IO_DIGITAL_CFG, buffer, len);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_get_value_exit;
		}
		break;
	case PCAN_IO_DIGITAL_VALUE:
		size = sizeof(__u32);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		ires = pcanfd_get_option(pchan->fd, PCANFD_IO_DIGITAL_VAL, buffer, len);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_get_value_exit;
		}
		break;
	case PCAN_IO_ANALOG_VALUE:
		size = sizeof(__u32);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		ires = pcanfd_get_option(pchan->fd, PCANFD_IO_ANALOG_VAL, buffer, len);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_get_value_exit;
		}
		break;
	case PCAN_FIRMWARE_VERSION:
		size = sizeof(pchan->pinfo->adapter_version);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_get_value_exit;
		}
		memcpy(buffer, &pchan->pinfo->adapter_version, size);
		break;
	default:
		sts = PCAN_ERROR_UNKNOWN;
		goto pcanbasic_get_value_exit;
		break;
	}
pcanbasic_get_value_exit_ok:
	sts = PCAN_ERROR_OK;
pcanbasic_get_value_exit:
	return sts;
}

TPCANStatus pcanbasic_set_value(
        TPCANHandle channel,
        TPCANParameter parameter,
        void* buffer,
		DWORD len) {
	TPCANStatus sts;
	pcanbasic_channel *pchan;
	int ires;
	size_t size;
	__u8 ctmp;
	__u32 itmp;

	/* check parameter */
	if (buffer == NULL) {
		sts = PCAN_ERROR_ILLPARAMVAL;
		goto pcanbasic_set_value_exit;
	}

	itmp = 0;

	/* handle parameter that do not require a channel */
	switch(parameter) {
	case PCAN_LISTEN_ONLY:
		/* can be called with an uninitialized channel */
		size = sizeof(ctmp);
		/* allow only ON/OFF values */
		ctmp = *(__u8*)buffer;
		if (len > size)
			len = size;
		if (ctmp != PCAN_PARAMETER_ON &&
				ctmp != PCAN_PARAMETER_OFF) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		/* get channel (can be initialized or not) */
		pchan = pcanbasic_get_channel(channel, 0);
		if (pchan == NULL) {
			/* create channel to store INIT. value */
			pchan = pcanbasic_create_channel(channel, 1);
			if (pchan == NULL) {
				sts = PCAN_ERROR_UNKNOWN;
				goto pcanbasic_set_value_exit;
			}
		}
		pchan->listen_only = ctmp;
		if (pchan->fd > -1) {
			sts = pcanbasic_reset(channel);
			goto pcanbasic_set_value_exit;
		}
		goto pcanbasic_set_value_exit_ok;
		break;
	case PCAN_LOG_LOCATION:
		if (channel != PCAN_NONEBUS) {
			sts = PCAN_ERROR_ILLCLIENT;
			goto pcanbasic_set_value_exit;
		}
		pcblog_set_location(buffer);
		goto pcanbasic_set_value_exit_ok;
		break;
	case PCAN_LOG_STATUS:
		if (channel != PCAN_NONEBUS) {
			sts = PCAN_ERROR_ILLCLIENT;
			goto pcanbasic_set_value_exit;
		}
		size = sizeof(itmp);
		/* allow only ON/OFF values */
		itmp = *(__u32*) buffer;
		if (len > size)
			len = size;
		if (itmp != PCAN_PARAMETER_ON &&
				itmp != PCAN_PARAMETER_OFF) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		pcblog_set_status(itmp);
		goto pcanbasic_set_value_exit_ok;
		break;
	case PCAN_LOG_CONFIGURE:
		if (channel != PCAN_NONEBUS) {
			sts = PCAN_ERROR_ILLCLIENT;
			goto pcanbasic_set_value_exit;
		}
		size = sizeof(itmp);
		if (len > size)
			len = size;
		itmp = *(__u32*) buffer;
		pcblog_set_config(itmp);
		goto pcanbasic_set_value_exit_ok;
		break;
	case PCAN_LOG_TEXT:
		if (channel != PCAN_NONEBUS) {
			sts = PCAN_ERROR_ILLCLIENT;
			goto pcanbasic_set_value_exit;
		}
		pcblog_write(buffer, len);
		goto pcanbasic_set_value_exit_ok;
		break;
	case PCAN_RECEIVE_STATUS:
		/* can be called with an uninitialized channel */
		size = sizeof(ctmp);
		/* allow only ON/OFF values */
		ctmp = *(__u8*)buffer;
		if (len > size)
			len = size;
		if (ctmp != PCAN_PARAMETER_ON &&
				ctmp != PCAN_PARAMETER_OFF) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		/* get channel (can be initialized or not) */
		pchan = pcanbasic_get_channel(channel, 0);
		if (pchan == NULL) {
			/* create channel to store INIT. value */
			pchan = pcanbasic_create_channel(channel, 1);
			if (pchan == NULL) {
				sts = PCAN_ERROR_UNKNOWN;
				goto pcanbasic_set_value_exit;
			}
		}
		pchan->rcv_status =  ctmp;
		goto pcanbasic_set_value_exit_ok;
		break;
	case PCAN_CHANNEL_IDENTIFYING:
		//TODO PCAN_CHANNEL_IDENTIFYING: need driver impl. see with s.grosjean
		sts = PCAN_ERROR_UNKNOWN;
		goto pcanbasic_set_value_exit;
		break;
	case PCAN_BITRATE_ADAPTING:
		/* can be called with an uninitialized channel */
		size = sizeof(ctmp);
		/* allow only ON/OFF values */
		ctmp = *(__u8*)buffer;
		if (len > size)
			len = size;
		if (ctmp != PCAN_PARAMETER_ON &&
				ctmp != PCAN_PARAMETER_OFF) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		/* get channel (can be initialized or not) */
		pchan = pcanbasic_get_channel(channel, 0);
		if (pchan == NULL) {
			/* create channel to store INIT. value */
			pchan = pcanbasic_create_channel(channel, 1);
			if (pchan == NULL) {
				sts = PCAN_ERROR_UNKNOWN;
				goto pcanbasic_set_value_exit;
			}
		}
		pchan->bitrate_adapting = ctmp;
		goto pcanbasic_set_value_exit_ok;
		break;
	case PCAN_CHANNEL_CONDITION:
		/* set is not allowed */
		sts = PCAN_ERROR_ILLPARAMTYPE;
		goto pcanbasic_set_value_exit;
		break;
	}

	/* get initialized channel */
	pchan = pcanbasic_get_channel(channel, 1);
	if (pchan == NULL) {
		sts = PCAN_ERROR_INITIALIZE;
		goto pcanbasic_set_value_exit;
	}

	switch(parameter) {
	case PCAN_DEVICE_NUMBER:
		size = sizeof(itmp);
		if (len > size)
			len = size;
		memcpy(&itmp, buffer, len);
		ires = pcanfd_set_device_id(pchan->fd, itmp);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		break;
	case PCAN_5VOLTS_POWER:
		/* not supported by driver */
		sts = PCAN_ERROR_ILLPARAMTYPE;
		goto pcanbasic_set_value_exit;
		break;
	case PCAN_RECEIVE_EVENT:
		/* always enabled via pchan->fd, mark not supported */
		sts = PCAN_ERROR_ILLOPERATION;
		goto pcanbasic_set_value_exit;
		break;
	case PCAN_MESSAGE_FILTER:
		size = sizeof(ctmp);
		/* allow only ON/OFF values */
		ctmp = *(__u8*)buffer;
		if (len > size)
			len = size;
		if (ctmp != PCAN_FILTER_CLOSE &&
				ctmp != PCAN_FILTER_OPEN) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		/* deleting all, opens everything */
		pcanfd_del_filters(pchan->fd);
		if (ctmp == PCAN_FILTER_CLOSE) {
			/* if (id_from > id_to) everything is closed */
			struct pcanfd_msg_filter filter;
			filter.id_from = 1;
			filter.id_to = 0;
			filter.msg_flags = 0;
			pcanfd_add_filter(pchan->fd, &filter);
		}
		break;
	case PCAN_BUSOFF_AUTORESET:
		size = sizeof(pchan->busoff_reset);
		if (len > size)
			len = size;
		memcpy(&pchan->busoff_reset, buffer, size);
		break;
	case PCAN_TRACE_LOCATION:
		size = sizeof(pchan->tracer.directory);
		if (len > size)
			len = size;
		snprintf(pchan->tracer.directory, len, "%s", (char *)buffer);
		if (pchan->tracer.status == PCAN_PARAMETER_ON) {
			enum pcaninfo_hw hw;
			uint idx;
			pcbtrace_close(&pchan->tracer);
			pcanbasic_get_hw(pchan->channel, &hw, &idx);
			pcbtrace_open(&pchan->tracer, hw, idx);
		}
		break;
	case PCAN_TRACE_STATUS:
		size = sizeof(pchan->tracer.status);
		if (len > size)
			len = size;
		memcpy(&pchan->tracer.status, buffer, len);
		if (pchan->tracer.status == PCAN_PARAMETER_ON) {
			enum pcaninfo_hw hw;
			uint idx;
			pcanbasic_get_hw(pchan->channel, &hw, &idx);
			pcbtrace_open(&pchan->tracer, hw, idx);
		}
		else {
			pcbtrace_close(&pchan->tracer);
		}
		break;
	case PCAN_TRACE_SIZE:
		if (pchan->tracer.status == PCAN_PARAMETER_ON) {
			sts = PCAN_ERROR_ILLOPERATION;
			goto pcanbasic_set_value_exit;
		}
		size = sizeof(pchan->tracer.maxsize);
		if (len > size)
			len = size;
		memcpy(&pchan->tracer.maxsize, buffer, len);
		break;
	case PCAN_TRACE_CONFIGURE:
		if (pchan->tracer.status == PCAN_PARAMETER_ON) {
			sts = PCAN_ERROR_ILLOPERATION;
			goto pcanbasic_set_value_exit;
		}
		size = sizeof(pchan->tracer.flags);
		if (len > size)
			len = size;
		memcpy(&pchan->tracer.flags, buffer, len);
		break;

	case PCAN_ALLOW_ERROR_FRAMES:
		size = sizeof(__u8);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		/* get options to retrieve all flags */
		ires = pcanfd_get_option(pchan->fd, PCANFD_OPT_ALLOWED_MSGS, &itmp, sizeof(itmp));
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		/* change flag */
		if (*(__u8 *)buffer == PCAN_PARAMETER_ON)
			itmp |= PCANFD_ALLOWED_MSG_ERROR;
		else
			itmp = itmp & ~PCANFD_ALLOWED_MSG_ERROR;
		ires = pcanfd_set_option(pchan->fd, PCANFD_OPT_ALLOWED_MSGS, &itmp, sizeof(itmp));
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		break;
	case PCAN_ALLOW_RTR_FRAMES:
		size = sizeof(__u8);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		/* get options to retrieve all flags */
		ires = pcanfd_get_option(pchan->fd, PCANFD_OPT_ALLOWED_MSGS, &itmp, sizeof(itmp));
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		/* change flag */
		if (*(__u8 *)buffer == PCAN_PARAMETER_ON)
			itmp |= PCANFD_ALLOWED_MSG_RTR;
		else
			itmp = itmp & ~PCANFD_ALLOWED_MSG_RTR;
		ires = pcanfd_set_option(pchan->fd, PCANFD_OPT_ALLOWED_MSGS, &itmp, sizeof(itmp));
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		break;
	case PCAN_ALLOW_STATUS_FRAMES:
		size = sizeof(__u8);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		/* get options to retrieve all flags */
		ires = pcanfd_get_option(pchan->fd, PCANFD_OPT_ALLOWED_MSGS, &itmp, sizeof(itmp));
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		/* change flag */
		if (*(__u8 *)buffer == PCAN_PARAMETER_ON)
			itmp |= PCANFD_ALLOWED_MSG_STATUS;
		else
			itmp = itmp & ~PCANFD_ALLOWED_MSG_STATUS;
		ires = pcanfd_set_option(pchan->fd, PCANFD_OPT_ALLOWED_MSGS, &itmp, sizeof(itmp));
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		break;
	case PCAN_INTERFRAME_DELAY:
		size = sizeof(__u32);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		ires = pcanfd_set_option(pchan->fd, PCANFD_OPT_IFRAME_DELAYUS, buffer, len);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		break;
	case PCAN_ACCEPTANCE_FILTER_11BIT:
		size = sizeof(__u64);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		ires = pcanfd_set_option(pchan->fd, PCANFD_OPT_ACC_FILTER_11B, buffer, len);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		break;
	case PCAN_ACCEPTANCE_FILTER_29BIT:
		size = sizeof(__u64);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		ires = pcanfd_set_option(pchan->fd, PCANFD_OPT_ACC_FILTER_29B, buffer, len);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		break;
	case PCAN_IO_DIGITAL_CONFIGURATION:
		size = sizeof(__u32);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		ires = pcanfd_set_option(pchan->fd, PCANFD_IO_DIGITAL_CFG, buffer, len);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		break;
	case PCAN_IO_DIGITAL_VALUE:
		size = sizeof(__u32);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		ires = pcanfd_set_option(pchan->fd, PCANFD_IO_DIGITAL_VAL, buffer, len);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		break;
	case PCAN_IO_DIGITAL_SET:
		size = sizeof(__u32);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		ires = pcanfd_set_option(pchan->fd, PCANFD_IO_DIGITAL_SET, buffer, len);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		break;
	case PCAN_IO_DIGITAL_CLEAR:
		size = sizeof(__u32);
		if (len < size) {
			sts = PCAN_ERROR_ILLPARAMVAL;
			goto pcanbasic_set_value_exit;
		}
		ires = pcanfd_set_option(pchan->fd, PCANFD_IO_DIGITAL_CLR, buffer, len);
		if (ires < 0) {
			sts = pcanbasic_errno_to_status(-ires);
			goto pcanbasic_set_value_exit;
		}
		break;
	default:
		sts = PCAN_ERROR_ILLPARAMTYPE;
		goto pcanbasic_set_value_exit;
		break;
	}
pcanbasic_set_value_exit_ok:
	sts = PCAN_ERROR_OK;
pcanbasic_set_value_exit:
	return sts;
}

TPCANStatus pcanbasic_get_error_text(
        TPCANStatus error,
        WORD language,
        LPSTR buffer) {
	TPCANStatus sts;

	sts = PCAN_ERROR_OK;

	switch (language) {
	case 0x00:
		language = IDS_STR_IND_LANG_EN;
		break;
	case 0x07:
		language = IDS_STR_IND_LANG_DE;
		break;
	case 0x09:
		language = IDS_STR_IND_LANG_EN;
		break;
	case 0x0A:
		language = IDS_STR_IND_LANG_ES;
		break;
	case 0x0C:
		language = IDS_STR_IND_LANG_FR;
		break;
	case 0x10:
		language = IDS_STR_IND_LANG_IT;
		break;
	default:
		language = IDS_STR_IND_LANG_EN;
		break;
	}

	switch (error) {
	case PCAN_ERROR_OK:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_OK]);
		break;
	case PCAN_ERROR_XMTFULL:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_XMTFULL]);
		break;
	case PCAN_ERROR_OVERRUN:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_OVERRUN]);
		break;
	case PCAN_ERROR_BUSLIGHT:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_BUSLIGHT]);
		break;
	case PCAN_ERROR_BUSHEAVY:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_BUSHEAVY]);
		break;
	case PCAN_ERROR_BUSOFF:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_BUSOFF]);
		break;
	case PCAN_ERROR_ANYBUSERR:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_ANYBUSERR]);
		break;
	case PCAN_ERROR_QRCVEMPTY:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_QRCVEMPTY]);
		break;
	case PCAN_ERROR_QOVERRUN:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_QOVERRUN]);
		break;
	case PCAN_ERROR_QXMTFULL:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_QXMTFULL]);
		break;
	case PCAN_ERROR_REGTEST:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_REGTEST]);
		break;
	case PCAN_ERROR_NODRIVER:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_NODRIVER]);
		break;
	case PCAN_ERROR_RESOURCE:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_RESOURCE]);
		break;
	case PCAN_ERROR_ILLPARAMTYPE:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_ILLPARAMTYPE]);
		break;
	case PCAN_ERROR_ILLPARAMVAL:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_ILLPARAMVAL]);
		break;
#if PCAN_ERROR_ILLCLIENT != PCAN_ERROR_ILLHANDLE
	case PCAN_ERROR_ILLHANDLE:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_ILLHANDLE]);
		break;
#endif
	case PCAN_ERROR_INITIALIZE:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_INITIALIZE]);
		break;
	case PCAN_ERROR_UNKNOWN:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_UNKNOW]);
		break;
	case PCAN_ERROR_HWINUSE:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_HWINUSE]);
		break;
	case PCAN_ERROR_NETINUSE:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_NETINUSE]);
		break;
	case PCAN_ERROR_ILLHW:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_ILLHW]);
		break;
	case PCAN_ERROR_ILLNET:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_ILLNET]);
		break;
	case PCAN_ERROR_ILLCLIENT:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_ILLCLIENT]);
		break;
	case PCAN_ERROR_ILLDATA:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_ILLDATA]);
		break;
	case PCAN_ERROR_ILLOPERATION:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_ILLOPERATION]);
		break;
	case PCAN_ERROR_BUSPASSIVE:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_BUSPASSIVE]);
		break;
	case PCAN_ERROR_CAUTION:
		strcpy(buffer, resource[language][IDS_STR_IND_ERR_CAUTION]);
		break;
	default:
		sprintf(buffer, "Undefined (0x%x)", error);
		sts = PCAN_ERROR_ILLPARAMVAL;
		break;
	}

	return sts;
}
