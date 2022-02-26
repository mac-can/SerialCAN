/*
 *  Controler Area Network - Lawicel SLCAN Protocol (Serial-Line CAN)
 *
 *  Copyright (C) 2016,2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *
 *  This module is part of the SourceMedley repository.
 *
 *  This module is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this module.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file        slcan.c
 *
 *  @brief       Lawicel SLCAN protocol.
 *
 *  @author      $Author: eris $
 *
 *  @version     $Rev: 690 $
 *
 *  @addtogroup  slcan
 *  @{
 */
#include "slcan.h"
#include "serial.h"
#include "queue.h"
#include "buffer.h"
#include "logger.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>


/*  -----------  options  ------------------------------------------------
 */

#if (OPTION_SLCAN_DEBUG_LEVEL > 0)
#define SLCAN_DEBUG_ERROR(...)  log_printf(__VA_ARGS__)
#else
#define SLCAN_DEBUG_ERROR(...)  while (0)
#endif

#if (OPTION_SLCAN_DEBUG_LEVEL > 1)
#define SLCAN_DEBUG_INFO(...)  log_printf(__VA_ARGS__)
#else
#define SLCAN_DEBUG_INFO(...)  while (0)
#endif

#if (OPTION_SLCAN_DEBUG_LEVEL > 2)
#define SLCAN_DEBUG_ASYNC(...)  log_async(__VA_ARGS__)
#define SLCAN_DEBUG_SYNC(...)  log_sync(__VA_ARGS__)
#undef SLCAN_DEBUG_INFO
#define SLCAN_DEBUG_INFO(...)  while (0)
#else
#define SLCAN_DEBUG_ASYNC(...)  while (0)
#define SLCAN_DEBUG_SYNC(...)  while (0)
#endif

#if (OPTION_SLCAN_DYLIB != 0)
__attribute__((constructor))
static void _initializer() {
#if (OPTION_SLCAN_DEBUG_LEVEL != 0)
    fprintf(stdout, "[%s] [%s]\n", __FILE__, __FUNCTION__);
#endif
}
__attribute__((destructor))
static void _finalizer() {
#if (OPTION_SLCAN_DEBUG_LEVEL != 0)
    fprintf(stdout, "[%s] [%s]\n", __FILE__, __FUNCTION__);
#endif
}
#define EXPORT  __attribute__((visibility("default")))
#else
#define EXPORT
#endif

/*  -----------  defines  ------------------------------------------------
 */

#if (0)
#define BCD2CHR(x)  ((((x) & 0xF) < 0xA) ? ('0' + ((x) & 0xF)) : ('7' + ((x) & 0xF)))
#define CHR2BCD(x)  ((('0' <= (x)) && ((x) <= '9')) ? ((x) - '0') : \
                    ((('A' <= (x)) && ((x) <= 'F')) ? (10 + (x) - 'A') : \
                    ((('a' <= (x)) && ((x) <= 'f')) ? (10 + (x) - 'a') : 0xFF)))
#error Function-like macro did not work here!
#else
#define BCD2CHR(x)  (uint8_t)bcd2chr((uint8_t)(x))
#define CHR2BCD(x)  (uint8_t)chr2bcd((uint8_t)(x))
#endif
#define MAX_DLC(l)  (((l) < CAN_LEN_MAX) ? (l) : (CAN_DLC_MAX))

#define BUFFER_SIZE 128U
#define RESPONSE_TIMEOUT  100U
#define TRANSMIT_TIMEOUT  1000U


/*  -----------  types  --------------------------------------------------
 */

typedef struct slcan_t_ {
    sio_port_t port;
    buffer_t response;
    queue_t messages;
    uint8_t buffer[BUFFER_SIZE];
    size_t index;
} slcan_t;


/*  -----------  prototypes  ---------------------------------------------
 */

static int send_command(slcan_t *slcan, const uint8_t *request, size_t nbytes,
                        uint8_t *response, size_t maxbytes, uint16_t timeout);
static bool encode_message(const slcan_message_t *message, uint8_t *buffer, size_t *nbytes);
static bool decode_message(slcan_message_t *message, const uint8_t *buffer, size_t nbytes);
static void reception_loop(const void *port, const uint8_t *buffer, size_t nbytes);


/*  -----------  variables  ----------------------------------------------
 */


/*  -----------  functions  ----------------------------------------------
 */

static inline uint8_t bcd2chr(uint8_t x) {
    if ((x & 0xF) < 0xA)
        return (uint8_t)('0' + (x & 0xF));
    else
        return (uint8_t)('7' + (x & 0xF));
}

static inline uint8_t chr2bcd(uint8_t x) {
    if (('0' <= x) && (x <= '9'))
        return (uint8_t)(x - '0');
    else if (('A' <= x) && (x <= 'F'))
        return (uint8_t)(10 + x - 'A');
    else if (('a' <= x) && (x <= 'f'))
        return (uint8_t)(10 + x - 'a');
    else
        return (uint8_t)(0xFF);
}

EXPORT
slcan_port_t slcan_create(size_t queueSize) {
    slcan_t *slcan = (slcan_t*)NULL;

    /* reset errno variable */
    errno = 0;
    /* C language constructor */
    if ((slcan = (slcan_t*)malloc(sizeof(slcan_t))) != NULL) {
        (void)memset(slcan, 0x00, sizeof(slcan_t));
        /* create a serial port instance */
        slcan->port = sio_create(reception_loop, (void*)slcan);
        if (!slcan->port) {
            /* errno set */
            free(slcan);
            return NULL;
        }
        /* create a buffer for response messages */
        slcan->response = buffer_create(BUFFER_SIZE);
        if (!slcan->response) {
            /* errno set */
            (void)sio_destroy(slcan->port);
            free(slcan);
            return NULL;
        }
        /* create a message queue for CAN messages */
        slcan->messages = queue_create(queueSize, sizeof(slcan_message_t));
        if (!slcan->messages) {
            /* errno set */
            (void)buffer_destroy(slcan->response);
            (void)sio_destroy(slcan->port);
            free(slcan);
            return NULL;
        }
        /* initialize reception buffer */
        slcan->index = 0U;
    }
    /* return a pointer to the instance */
    return (slcan_port_t)slcan;

}

EXPORT
int slcan_destroy(slcan_port_t port) {
    slcan_t *slcan = (slcan_t*)port;

    /* sanity check */
    errno = 0;
    if (!slcan) {
        errno = ENODEV;
        return -1;
    }
    /* close opened port (if any) and */
    (void)slcan_disconnect(port);
    /* destroy serial port instance */
    if (slcan->port)
        (void)sio_destroy(slcan->port);
    if (slcan->response)
        (void)buffer_destroy(slcan->response);
    if (slcan->messages)
        (void)queue_destroy(slcan->messages);
    /* C language destructor */
    free(slcan);
    return 0;
}

EXPORT
int slcan_signal(slcan_port_t port) {
    slcan_t *slcan = (slcan_t*)port;

    /* sanity check */
    errno = 0;
    if (!slcan) {
        errno = ENODEV;
        return -1;
    }
    /* signal all waiting objects */
    if (slcan->port)
        (void)sio_signal(slcan->port);
    if (slcan->response)
        (void)buffer_signal(slcan->response);
    if (slcan->messages)
        (void)queue_signal(slcan->messages);
    SLCAN_DEBUG_INFO("slcan_signal\n");
    return 0;
}

EXPORT
int slcan_connect(slcan_port_t port, const char *device) {
    slcan_t *slcan = (slcan_t*)port;
//    uint8_t cr = 0xAU;
//    uint8_t waste[42];
    int res = -1;

    /* sanity check */
    errno = 0;
    if (!slcan || !slcan->port) {
        errno = ENODEV;
        return -1;
    }
    /* note: NULL pointer check is done by serial interface.
     */
    /* reset reception buffer */
    slcan->index = 0U;
    /* connect to the serial port */
    res = sio_connect(slcan->port, device, NULL);
    /* send three [CR] to purge the data terminal */
#if (0)
    // FIXME: this does not work
    for (int i = 0; i < 3; i++)
        if (sio_transmit(slcan->port, &cr, 1) == 1)
            (void)buffer_get(slcan->response, waste, 42, RESPONSE_TIMEOUT / 3);
#endif
    /* returns the file descriptor on success */
    return res;
}

EXPORT
int slcan_disconnect(slcan_port_t port) {
    slcan_t *slcan = (slcan_t*)port;

    /* sanity check */
    errno = 0;
    if (!slcan || !slcan->port) {
        errno = ENODEV;
        return -1;
    }
    /* disconnect from serial port */
    (void)slcan_close_channel(port);
    return sio_disconnect(slcan->port);
}

EXPORT
int slcan_setup_bitrate(slcan_port_t port, uint8_t index) {
    slcan_t *slcan = (slcan_t*)port;
    uint8_t request[3] = {'S','\0','\r'};
    uint8_t response[1];
    int nbytes;
    int res = -1;

    /* sanity check */
    errno = 0;
    if (!slcan || !slcan->port) {
        errno = ENODEV;
        return -1;
    }
    if (index > 8) {
        errno = EINVAL;
        return -1;
    }
    /* bit-rate index:
     *     0 = 10 kbps
     *     1 = 20 kbps
     *     2 = 50 kbps
     *     3 = 100 kbps
     *     4 = 125 kbps
     *     5 = 250 kbps
     *     6 = 500 kbps
     *     7 = 800 kbps
     *     8 = 1000 kbps
     */
    request[1] = '0' + index;
    /* send command 'Setup with standard CAN bit-rates' */
    nbytes = send_command(slcan, request, 3, response, 1, RESPONSE_TIMEOUT);
    if ((nbytes == 1) && (response[0] == '\r')) {
        res = 0;
    } else if (nbytes >= 0) {
        /* note: Variable 'errno' is set by the called functions according
         *       to their result. On error they return a negative value.
         *       Receiving a wrong number of bytes will be interpreted as
         *       protocol error (EBADMSG).
         */
        errno = EBADMSG;
        res = -1;
    }
    SLCAN_DEBUG_INFO("slcan_setup_bitrate (%i)\n", res);
    return res;
}

EXPORT
int slcan_setup_btr(slcan_port_t port, uint16_t btr) {
    slcan_t *slcan = (slcan_t*)port;
    uint8_t request[6] = {'s','\0','\0','\0','\0','\r'};
    uint8_t response[1];
    int nbytes;
    int res = -1;

    /* sanity check */
    errno = 0;
    if (!slcan || !slcan->port) {
        errno = ENODEV;
        return -1;
    }
    /* convert btr into ASCI hex digits */
    request[1] = BCD2CHR(btr >> 12);
    request[2] = BCD2CHR(btr >> 8);
    request[3] = BCD2CHR(btr >> 4);
    request[4] = BCD2CHR(btr >> 0);
    /* send command 'Setup with BTR0/BTR1 CAN bit-rates' */
    nbytes = send_command(slcan, request, 6, response, 1, RESPONSE_TIMEOUT);
    if ((nbytes == 1) && (response[0] == '\r')) {
        res = 0;
    } else if (nbytes >= 0) {
        /* note: Variable 'errno' is set by the called functions according
         *       to their result. On error they return a negative value.
         *       Receiving a wrong number of bytes will be interpreted as
         *       protocol error (EBADMSG).
         */
        errno = EBADMSG;
        res = -1;
    }
    SLCAN_DEBUG_INFO("slcan_setup_btr (%i)\n", res);
    return res;
}

EXPORT
int slcan_open_channel(slcan_port_t port) {
    slcan_t *slcan = (slcan_t*)port;
    uint8_t request[2] = {'O','\r'};
    uint8_t response[1];
    int nbytes;
    int res = -1;

    /* sanity check */
    errno = 0;
    if (!slcan || !slcan->port) {
        errno = ENODEV;
        return -1;
    }
    /* clear the message queue */
    (void)queue_clear(slcan->messages);  // FIXME: (?)
    /* send command 'Open the CAN channel' */
    nbytes = send_command(slcan, request, 2, response, 1, RESPONSE_TIMEOUT);
    if ((nbytes == 1) && (response[0] == '\r')) {
        res = 0;
    } else if (nbytes >= 0) {
        /* note: Variable 'errno' is set by the called functions according
         *       to their result. On error they return a negative value.
         *       Receiving a wrong number of bytes will be interpreted as
         *       protocol error (EBADMSG).
         */
        errno = EBADMSG;
        res = -1;
    }
    SLCAN_DEBUG_INFO("slcan_open_channel (%i)\n", res);
    return res;
}

EXPORT
int slcan_close_channel(slcan_port_t port) {
    slcan_t *slcan = (slcan_t*)port;
    uint8_t request[2] = {'C','\r'};
    uint8_t response[1];
    int nbytes;
    int res = -1;

    /* sanity check */
    errno = 0;
    if (!slcan || !slcan->port) {
        errno = ENODEV;
        return -1;
    }
    /* send command 'Close the CAN channel' */
    nbytes = send_command(slcan, request, 2, response, 1, RESPONSE_TIMEOUT);
    if ((nbytes == 1) && (response[0] == '\r')) {
        res = 0;
    } else if (nbytes >= 0) {
        /* note: Variable 'errno' is set by the called functions according
         *       to their result. On error they return a negative value.
         *       Receiving a wrong number of bytes will be interpreted as
         *       protocol error (EBADMSG).
         */
        errno = EBADMSG;
        res = -1;
    }
    SLCAN_DEBUG_INFO("slcan_close_channel (%i)\n", res);
    return res;
}

EXPORT
int slcan_write_message(slcan_port_t port, const slcan_message_t *message, uint16_t timeout) {
    slcan_t *slcan = (slcan_t*)port;
    uint8_t buffer[BUFFER_SIZE];
    size_t length;
    uint8_t response[2];
    int nbytes;
    int res = -1;
    (void)timeout;

    /* sanity check */
    errno = 0;
    if (!slcan || !slcan->port) {
        errno = ENODEV;
        return -1;
    }
    if (!message) {
        errno = EINVAL;
        return -1;
    }
    /* encode the CAN message */
    if (!encode_message(message, buffer, &length)) {
        errno = EFAULT;
        return -99;
    }
    /* clear pending response, if any */
    (void)buffer_clear(slcan->response);
    /* send CAN message to the device via serial port */
    nbytes = sio_transmit(slcan->port, buffer, length);
    if (nbytes == (int)length) {
        /* wait for response in the reception buffer */
        nbytes = buffer_get(slcan->response, (void*)response, 2, TRANSMIT_TIMEOUT);
        if ((nbytes == 2) && (response[1] == '\r') &&
            ((((response[0] == 'z') && ((buffer[0] == 't') || (buffer[0] == 'r')))) ||
             (((response[0] == 'Z') && ((buffer[0] == 'T') || (buffer[0] == 'R')))))) {
            res = 0;
        } else if (nbytes >= 0) {
            /* note: Variable 'errno' is set by the called functions according
             *       to their result. On error they return a negative value.
             *       Receiving a wrong number of bytes will be interpreted as
             *       protocol error (EBADMSG).
             */
             errno = EBADMSG;
            res = -1;
        }
    } else if (nbytes >= 0) {
        /* note: Variable 'errno' is set by the called functions according to
         *       their result. On error they return a negative value.
         *       When a wrong number of bytes has been transmitted this will
         *       be interpreted as the sender or the receiver is busy (EBUSY).
         */
        errno = EBUSY;
        res = -1;
    }
    SLCAN_DEBUG_INFO("slcan_write_message (%i)\n", res);
    return res;
}

EXPORT
int slcan_read_message(slcan_port_t port, slcan_message_t *message, uint16_t timeout) {
    slcan_t *slcan = (slcan_t*)port;
    int res;

    /* sanity check */
    errno = 0;
    if (!slcan || !slcan->port) {
        errno = ENODEV;
        return -1;
    }
    if (!message) {
        errno = EINVAL;
        return -1;
    }
    /* get one message from the message queue, if any */
    res = queue_dequeue(slcan->messages, (void*)message, sizeof(slcan_message_t), timeout);
    if (res == (int)sizeof(slcan_message_t)) {
        /* note: On success value 0 will be returned (CAN API compatible).
         *       In case of a queue overflow variable 'errno' will be set.
         */
        if (queue_overflow(slcan->messages, NULL))
            errno = ENOSPC;
        res = 0;
    } else if (res >= 0) {
        /* note: The queue elements are of type void* and may be truncated
         *       when dequeue (or do not fit into the destination memory).
         *       In this case we will return 'No message received'.
         */
        errno = ENOMSG;
        res = -30;
    } else {
        /* note: CAN API compatible error codes will be returned on error. */
    }
    if (res != -30)  // when not empty
        SLCAN_DEBUG_INFO("slcan_read_message (%i)\n", res);
    return (int)res;
}

EXPORT
int slcan_status_flags(slcan_port_t port, slcan_flags_t *flags) {
    slcan_t *slcan = (slcan_t*)port;
    uint8_t request[2] = {'F','\r'};
    uint8_t response[4];
    int nbytes;
    int res = -1;

    /* sanity check */
    errno = 0;
    if (!slcan || !slcan->port) {
        errno = ENODEV;
        return -1;
    }
    /* send command 'Read Status Flags' */
    nbytes = send_command(slcan, request, 2, response, 4, RESPONSE_TIMEOUT);
    if ((nbytes == 4) && (response[0] == 'F') && (response[3] == '\r')) {
        if (flags) {
            flags->byte = (uint8_t)(CHR2BCD(response[1]) << 4);
            flags->byte |= (uint8_t)CHR2BCD(response[2]);
        }
        res = 0;
    } else if (nbytes >= 0) {
        /* note: Variable 'errno' is set by the called functions according
         *       to their result. On error they return a negative value.
         *       Receiving a wrong number of bytes will be interpreted as
         *       protocol error (EBADMSG).
         */
        errno = EBADMSG;
        res = -1;
    }
    SLCAN_DEBUG_INFO("slcan_status_flags (%i)\n", res);
    return res;
}

EXPORT
int slcan_acceptance_code(slcan_port_t port, uint32_t code) {
    slcan_t *slcan = (slcan_t*)port;
    uint8_t request[10] = {'M','\0','\0','\0','\0','\0','\0','\0','\0','\r'};
    uint8_t response[1];
    int nbytes;
    int res = -1;

    /* sanity check */
    errno = 0;
    if (!slcan || !slcan->port) {
        errno = ENODEV;
        return -1;
    }
    /* convert code into ASCI hex digits */
    request[1] = BCD2CHR(code >> 28);
    request[2] = BCD2CHR(code >> 24);
    request[3] = BCD2CHR(code >> 20);
    request[4] = BCD2CHR(code >> 16);
    request[5] = BCD2CHR(code >> 12);
    request[6] = BCD2CHR(code >> 8);
    request[7] = BCD2CHR(code >> 4);
    request[8] = BCD2CHR(code >> 0);
    /* send command 'Sets Acceptance Code Register' */
    nbytes = send_command(slcan, request, 10, response, 1, RESPONSE_TIMEOUT);
    if ((nbytes == 1) && (response[0] == '\r')) {
        res = 0;
    } else if (nbytes >= 0) {
        /* note: Variable 'errno' is set by the called functions according
         *       to their result. On error they return a negative value.
         *       Receiving a wrong number of bytes will be interpreted as
         *       protocol error (EBADMSG).
         */
        errno = EBADMSG;
        res = -1;
    }
    SLCAN_DEBUG_INFO("slcan_acceptance_code (%i)\n", res);
    return res;
}

EXPORT
int slcan_acceptance_mask(slcan_port_t port, uint32_t mask) {
    slcan_t *slcan = (slcan_t*)port;
    uint8_t request[10] = {'m','\0','\0','\0','\0','\0','\0','\0','\0','\r'};
    uint8_t response[1];
    int nbytes;
    int res = -1;

    /* sanity check */
    errno = 0;
    if (!slcan || !slcan->port) {
        errno = ENODEV;
        return -1;
    }
    /* convert mask into ASCI hex digits */
    request[1] = BCD2CHR(mask >> 28);
    request[2] = BCD2CHR(mask >> 24);
    request[3] = BCD2CHR(mask >> 20);
    request[4] = BCD2CHR(mask >> 16);
    request[5] = BCD2CHR(mask >> 12);
    request[6] = BCD2CHR(mask >> 8);
    request[7] = BCD2CHR(mask >> 4);
    request[8] = BCD2CHR(mask >> 0);
    /* send command 'Sets Acceptance Mask Register' */
    nbytes = send_command(slcan, request, 10, response, 1, RESPONSE_TIMEOUT);
    if ((nbytes == 1) && (response[0] == '\r')) {
        res = 0;
    } else if (nbytes >= 0) {
        /* note: Variable 'errno' is set by the called functions according
         *       to their result. On error they return a negative value.
         *       Receiving a wrong number of bytes will be interpreted as
         *       protocol error (EBADMSG).
         */
        errno = EBADMSG;
        res = -1;
    }
    SLCAN_DEBUG_INFO("slcan_acceptance_mask (%i)\n", res);
    return res;
}

EXPORT
int slcan_version_number(slcan_port_t port, uint8_t *hardware, uint8_t *software) {
    slcan_t *slcan = (slcan_t*)port;
    uint8_t request[2] = {'V','\r'};
    uint8_t response[6];
    int nbytes;
    int res = -1;

    /* sanity check */
    errno = 0;
    if (!slcan || !slcan->port) {
        errno = ENODEV;
        return -1;
    }
    /* send command 'Get Version number of both CANUSB hardware and software' */
    nbytes = send_command(slcan, request, 2, response, 6, RESPONSE_TIMEOUT);
    if ((nbytes == 6) && (response[0] == 'V') && (response[5] == '\r')) {
        if (hardware) {
            *hardware = (uint8_t)(CHR2BCD(response[1]) << 4);
            *hardware |= (uint8_t)CHR2BCD(response[2]);
        }
        if (software) {
            *software = (uint8_t)(CHR2BCD(response[3]) << 4);
            *software |= (uint8_t)CHR2BCD(response[4]);
        }
        res = 0;
    } else if (nbytes >= 0) {
        /* note: Variable 'errno' is set by the called functions according
         *       to their result. On error they return a negative value.
         *       Receiving a wrong number of bytes will be interpreted as
         *       protocol error (EBADMSG).
         */
        errno = EBADMSG;
        res = -1;
    }
    SLCAN_DEBUG_INFO("slcan_version_number (%i)\n", res);
    return res;

}

EXPORT
int slcan_serial_number(slcan_port_t port, uint32_t *number) {
    slcan_t *slcan = (slcan_t*)port;
    uint8_t request[2] = {'N','\r'};
    uint8_t response[6];
    int nbytes;
    int res = -1;

    /* sanity check */
    errno = 0;
    if (!slcan || !slcan->port) {
        errno = ENODEV;
        return -1;
    }
    /* send command 'Get Serial number of the CANUSB' */
    nbytes = send_command(slcan, request, 2, response, 6, RESPONSE_TIMEOUT);
    if ((nbytes == 6) && (response[0] == 'N') && (response[5] == '\r')) {
        if (number) {
            *number = (uint32_t)(response[0] << 24);
            *number |= (uint32_t)(response[1] << 16);
            *number |= (uint32_t)(response[2] << 8);
            *number |= (uint32_t)response[3];
        }
        res = 0;
    } else if (nbytes >= 0) {
        /* note: Variable 'errno' is set by the called functions according
         *       to their result. On error they return a negative value.
         *       Receiving a wrong number of bytes will be interpreted as
         *       protocol error (EBADMSG).
         */
        errno = EBADMSG;
        res = -1;
    }
    SLCAN_DEBUG_INFO("slcan_serial_number (%i)\n", res);
    return res;
}

static int send_command(slcan_t *slcan, const uint8_t *request, size_t nbytes,
                        uint8_t *response, size_t maxbytes, uint16_t timeout) {
    int res;

    assert(slcan);
    assert(request);
    assert(response);

    /* clear pending response, if any */
    (void)buffer_clear(slcan->response);
    /* send request to the device via serial port */
    res = sio_transmit(slcan->port, request, nbytes);
    if (res == (int)nbytes) {
        /* wait for response in the reception buffer */
        res = buffer_get(slcan->response, (void*)response, maxbytes, timeout);
        /* note: Interpretation of the received data shall be done by the
         *       caller (e.g. EBADMSG).
         */
    } else if (res >= 0) {
        /* note: Variable 'errno' is set by the called functions according to
         *       their result. On error they return a negative value.
         *       When a wrong number of bytes has been transmitted this will
         *       be interpreted as the sender or the receiver is busy (EBUSY).
         */
        errno = EBUSY;
        res = -1;
    }
    /* return number of received bytes, or a negative value on error */
    return res;
}

static bool encode_message(const slcan_message_t *message, uint8_t *buffer, size_t *nbytes) {
    size_t index = 0;
    uint8_t i;

    assert(message);
    assert(buffer);
    assert(nbytes);

    if (!(message->can_id & CAN_XTD_FRAME)) {
        if(!(message->can_id & CAN_RTR_FRAME))
            buffer[index++] = (uint8_t)'t';
        else
            buffer[index++] = (uint8_t)'r';
        buffer[index++] = (uint8_t)BCD2CHR((message->can_id & CAN_STD_MASK) >> 8);
        buffer[index++] = (uint8_t)BCD2CHR((message->can_id & CAN_STD_MASK) >> 4);
        buffer[index++] = (uint8_t)BCD2CHR((message->can_id & CAN_STD_MASK) >> 0);
    } else {
        if(!(message->can_id & CAN_RTR_FRAME))
            buffer[index++] = (uint8_t)'T';
        else
            buffer[index++] = (uint8_t)'R';
        buffer[index++] = (uint8_t)BCD2CHR((message->can_id & CAN_STD_MASK) >> 28);
        buffer[index++] = (uint8_t)BCD2CHR((message->can_id & CAN_STD_MASK) >> 24);
        buffer[index++] = (uint8_t)BCD2CHR((message->can_id & CAN_STD_MASK) >> 20);
        buffer[index++] = (uint8_t)BCD2CHR((message->can_id & CAN_STD_MASK) >> 16);
        buffer[index++] = (uint8_t)BCD2CHR((message->can_id & CAN_STD_MASK) >> 12);
        buffer[index++] = (uint8_t)BCD2CHR((message->can_id & CAN_STD_MASK) >> 8);
        buffer[index++] = (uint8_t)BCD2CHR((message->can_id & CAN_STD_MASK) >> 4);
        buffer[index++] = (uint8_t)BCD2CHR((message->can_id & CAN_STD_MASK) >> 0);
    }
    if(!(message->can_id & CAN_RTR_FRAME)) {
        buffer[index++] = (uint8_t)BCD2CHR(MAX_DLC(message->can_dlc));
        for (i = 0; i < (uint8_t)MAX_DLC(message->can_dlc); i++) {
            buffer[index++] = (uint8_t)BCD2CHR(message->data[i] >> 4);
            buffer[index++] = (uint8_t)BCD2CHR(message->data[i] >> 0);
        }
    } else {
        buffer[index++] = (uint8_t)BCD2CHR(MAX_DLC(message->can_dlc));
    }
    buffer[index++] = (uint8_t)'\r';
    *nbytes = index;
    return true;
}

static bool decode_message(slcan_message_t *message, const uint8_t *buffer, size_t nbytes) {
    int i = 0;
    size_t index = 0;
    size_t offset;
    uint8_t digit;
    uint32_t flags;

    assert(message);
    assert(buffer);
    assert(nbytes);

    (void)memset(message, 0x00, sizeof(slcan_message_t));

    /* (1) message flags: XTD and RTR */
    switch (buffer[index++]) {
        case 't': flags = CAN_STD_FRAME; offset = index + 3; break;
        case 'T': flags = CAN_XTD_FRAME; offset = index + 8; break;
        case 'r': flags = CAN_RTR_FRAME; offset = index + 3; break;
        case 'R': flags = CAN_RTR_FRAME | CAN_XTD_FRAME; offset = index + 8; break;
        default: return false;
    }
    if (index >= nbytes)
        return false;
    /* (2) CAN identifier: 11-bit or 29-bit */
    while ((index < offset) && (index < nbytes)) {
        digit = CHR2BCD(buffer[index++]);
        if (digit != 0xFF)
            message->can_id = (message->can_id << 4) | (uint32_t)digit;
        else
            return false;
    }
    if (index >= nbytes)
        return false;
    /* (!) ORing message flags (Linux-CAN compatible) */
    message->can_id |= flags;
    /* (3) Data Length Code: 0..8 */
    digit = CHR2BCD(buffer[index++]);
    if (digit <= CAN_DLC_MAX)
        message->can_dlc = (uint8_t)digit;
    else
        return false;
    if (index >= nbytes)
        return false;
    /* (4) message data: up to 8 bytes */
    if (!(flags & CAN_RTR_FRAME))
        offset = index + (size_t)(message->can_dlc * 2);
    else  /* note: no data in RTR frames! */
        offset = index;
    while ((index < offset) && (index < nbytes)) {
        digit = CHR2BCD(buffer[index++]);
        if ((digit != 0xFF) && (index < nbytes))
            message->data[i] = (uint8_t)digit;
        else
            return false;
        digit = CHR2BCD(buffer[index++]);
        if (digit != 0xFF)
            message->data[i] = (message->data[i] << 4) | (uint8_t)digit;
        else
            return false;
        i++;
    }
    if (index >= nbytes)
        return false;
    /* (5) ignore the rest: CR or time-stamp + CR */
    return true;
}

static void reception_loop(const void *port, const uint8_t *buffer, size_t nbytes) {
    slcan_t *slcan = (slcan_t*)port;
    slcan_message_t message;
    size_t index;

    if (slcan && buffer) {
        assert(slcan->response);
        assert(slcan->messages);
        for (index = 0; index < nbytes; index++) {
            /* get next byte (asynchronous reception) */
            if ((slcan->index + 1) < BUFFER_SIZE)
                slcan->buffer[slcan->index++] = buffer[index];
            if (buffer[index] == '\r') {
                /* positive ACKnowledge [CR] received */
                if ((slcan->buffer[0] == 't') || (slcan->buffer[0] == 'T') ||
                    (slcan->buffer[0] == 'r') || (slcan->buffer[0] == 'R')) {
                    /* message indication or confirmation? */
                    if (slcan->index > 2) {
                        /* new message received (indication) */
                        if (decode_message(&message, slcan->buffer, slcan->index))
                            (void)queue_enqueue(slcan->messages, &message, sizeof(slcan_message_t));
                    } else {
                        /* confirmation of a sent message received */
                        (void)buffer_put(slcan->response, slcan->buffer, slcan->index);
                    }
                } else {
                    /* response of a sent request received */
                    (void)buffer_put(slcan->response, slcan->buffer, slcan->index);
                }
                /* done: reset reception buffer */
                slcan->index = 0U;
            } else if (buffer[index] == '\a') {
                /* Negative ACKnowledge [BEL] received */
                (void)buffer_put(slcan->response, slcan->buffer, slcan->index);
                /* done: reset reception buffer */
                slcan->index = 0U;
            }
        }
    }
}

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
