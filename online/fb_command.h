/*
Octopus-ReEL - Realtime Encephalography Laboratory Network
   Copyright (C) 2007-2025 Barkin Ilhan

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.

 Contact info:
 E-Mail:  barkin@unrlabs.org
 Website: http://icon.unrlabs.org/staff/barkin/
 Repo:    https://github.com/4e0n/
*/

/*
 * fb_command.h
 *
 * Octopus-ReEL - Realtime Encephalography Laboratory Network
 * 
 * This header defines the inter-process communication (IPC) protocol between
 * the frontend daemon and the acquisition backend in the Octopus-ReEL system.
 * It consists of command identifiers and the `fb_command` message structure.
 *
 * Copyright (C) 2007-2025 Barkin Ilhan
 * License: GNU GPL v3 or later – see <https://www.gnu.org/licenses/>.
 */

#ifndef FB_COMMAND_H
#define FB_COMMAND_H

/* ============================== */
/*       ACQUISITION CONTROL      */
/* ============================== */

/* Start data acquisition in the backend. */
#define ACQ_START          (0x0001)

/* Stop data acquisition immediately. */
#define ACQ_STOP           (0x0002)

/* Custom command issued from frontend to backend. */
#define ACQ_CMD_F2B        (0x0003)

/* Custom command issued from backend to frontend. */
#define ACQ_CMD_B2F        (0x0004)

/* General alert raised by backend. Could indicate a system fault. */
#define ACQ_ALERT          (0x0005)

/* ============================== */
/*     SYNCHRONIZATION PROTOCOL   */
/* ============================== */

/* Backend initiates a data transfer synchronization. */
#define B2F_DATA_SYN       (0x1001)

/* Frontend acknowledges data sync request. */
#define F2B_DATA_ACK       (0x1002)

/* Frontend requests backend reset. */
#define F2B_RESET_SYN      (0x1003)

/* Backend confirms reset acknowledgment. */
#define B2F_RESET_ACK      (0x1004)

/* ============================== */
/*     BUFFER SIZE EXCHANGE       */
/* ============================== */

/* Frontend requests the current buffer size allocated in backend. */
#define F2B_GET_BUF_SIZE   (0x1005)

/* Backend responds with allocated buffer size. */
#define B2F_PUT_BUF_SIZE   (0x1006)

/* Frontend requests total data count (e.g., sample count). */
#define F2B_GET_TOTAL_COUNT (0x1007)

/* Backend responds with total data count. */
#define B2F_PUT_TOTAL_COUNT (0x1008)

/* ============================== */
/*         TRIGGER TESTING        */
/* ============================== */

/* Frontend sends a trigger signal to test stimulus delivery. */
#define F2B_TRIGTEST       (0x1009)

/* ============================== */
/*          ALERT TYPES           */
/* ============================== */

/*
 * ALERT_DATA_LOSS:
 * Indicates a buffer underrun condition where the frontend is not
 * pulling data fast enough, risking sample loss.
 */
#define ALERT_DATA_LOSS    (0x0001)

/* ============================== */
/*        COMMAND STRUCTURE       */
/* ============================== */

/**
 * fb_command - Generic message container exchanged between frontend and backend.
 *
 * This structure is used for command and control communication between
 * the frontend and backend modules. Depending on the value of `id`,
 * the meaning of `iparam[]` parameters may vary.
 *
 * @id: Command identifier (one of the defined macros like ACQ_START, F2B_GET_BUF_SIZE, etc.)
 * @iparam: Array of four optional integer parameters to carry additional command data.
 */
typedef struct _fb_command {
    unsigned short id;    /* Command ID */
    int iparam[4];        /* Command parameters (context-specific) */
} fb_command;

#endif /* FB_COMMAND_H */
