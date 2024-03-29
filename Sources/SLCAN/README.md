### Lawicel SLCAN Protocol (Serial-Line CAN)

_Copyright &copy; 2016,2020-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)_ \
_All rights reserved._

# SLCAN

Implementation of Lawicel SLCAN protocol.

## SLCAN API

```C
/** @brief       creates a port instance for communication with a SLCAN compatible
 *               serial device (constructor).
 *
 *  @param[in]   queueSize  - size of the reception queue (number of messages)
 *
 *  @returns     a pointer to a SLCAN instance if successful, or NULL on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENOMEM  - out of memory (insufficient storage space)
 */
slcan_port_t slcan_create(size_t queueSize);


/** @brief       destroys the port instance (destructor).
 *
 *  @remarks     An established connection will be terminated by this.
 *
 *  @remarks     Prior to this the CAN controller is set into INIT mode and
 *               the CAN channel is closed (via command 'Close Channel').
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV  - no such device (invalid port instance)
 *  @retval      EFAULT  - bad address (release of resources)
 */
int slcan_destroy(slcan_port_t port);


/** @brief       establishes a connection with the serial communication device.
 *
 *  @remarks     The SLCAN protocol is check by retrieving version information
 *               of the CAN channel (via command 'HW/SW Version').
 *
 *  @remarks     Precautionary the CAN controller is set into INIT mode and
 *               the CAN channel is closed (via command 'Close Channel').
 *
 *  @param[in]   port    - pointer to a SLCAN instance
 *  @param[in]   device  - name of the serial device
 *  @param[in]   attr    - serial port attributes (optional)
 *
 *  @returns     a file descriptor if successful, or a negative value on error.
 *
 *  @remarks     On Windows, the communication port number (zero based) is returned.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV   - no such device (invalid port instance)
 *  @retval      EINVAL   - invalid argument (device name is NULL)
 *  @retval      EALREADY - already connected with the serial device
 *  @retval      'errno'  - error code from called system functions:
 *                          'open', 'tcsetattr', 'pthread_create'
 */
int slcan_connect(slcan_port_t port, const char *device, const sio_attr_t *attr);


/** @brief       terminates the connection with the serial communication device.
 *
 *  @remarks     Prior to this the CAN controller is set into INIT mode and
 *               the CAN channel is closed (via command 'Close Channel').
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV   - no such device (invalid port instance)
 *  @retval      EBADF    - bad file descriptor (device not connected)
 *  @retval      'errno'  - error code from called system functions:
 *                          'pthread_cancel', 'tcflush', 'close'
 */
int slcan_disconnect(slcan_port_t port);


/** @brief       returns the serial communication attributes (baudrate, etc.).
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *  @param[out]  attr  - serial communication attributes
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV   - no such device (invalid port instance)
 *  @retval      EINVAL   - invalid argument (attr is NULL)
 */
int slcan_get_attr(slcan_port_t port, slcan_attr_t *attr);


/** @brief       setup with standard CAN bit-rates.
 *
 *  @remarks     This command is only active if the CAN channel is closed.
 *
 *  @param[in]   port   - pointer to a SLCAN instance
 *  @param[in]   index  -
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EINVAL    - invalid argument (index)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
int slcan_setup_bitrate(slcan_port_t port, uint8_t index);


/** @brief       setup with BTR0/BTR1 CAN bit-rates; see SJA1000 datasheet.
 *
 *  @remarks     This command is only active if the CAN channel is closed.
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *  @param[in]   btr   - SJA1000 bit-timing register
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EINVAL    - invalid argument (...)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
int slcan_setup_btr(slcan_port_t port, uint16_t btr);


/** @brief       opens the CAN channel.
 *
 *  @remarks     This command is only active if the CAN channel is closed and
 *               has been set up prior with either the 'Setup Bitrate' or
 *               'Setup BTR' command (i.e. initiated).
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EINVAL    - invalid argument (...)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
int slcan_open_channel(slcan_port_t port);


/** @brief       closes the CAN channel.
 *
 *  @remarks     This command is only active if the CAN channel is open.
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EINVAL    - invalid argument (...)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
int slcan_close_channel(slcan_port_t port);


/** @brief       transmits a CAN message.
 *
 *  @remarks     This command is only active if the CAN channel is open.
 *
 *  @param[in]   port     - pointer to a SLCAN instance
 *  @param[in]   message  - pointer to the message to be sent
 *  @param[in]   timeout  - (not implemented yet)
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EINVAL    - invalid argument (message)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
int slcan_write_message(slcan_port_t port, const slcan_message_t *message, uint16_t timeout);


/** @brief       read one message from the message queue, if any.
 *
 *  @param[in]   port     - pointer to a SLCAN instance
 *  @param[out]  message  - pointer to a message buffer
 *  @param[in]   timeout  - time to wait for the reception of a message:
 *                               0 means the function returns immediately,
 *                               65535 means blocking read, and any other
 *                               value means the time to wait im milliseconds
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      -30  - when the message queue is empty (CAN API compatible)
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV  - no such device (invalid port instance)
 *  @retval      EINVAL  - invalid argument (message)
 *  @retval      ENOMSG  - no data available (message queue empty)
 *  @retval      ENOSPC  - no space left (message queue overflow)
 *
 *  @remarks     If a message has been successfully read from the message queue,
 *               the value ENOSPC in the system variable 'errno' indicates that
 *               a message queue overflow has occurred and that at least one
 *               CAN message has been lost.
 */
int slcan_read_message(slcan_port_t port, slcan_message_t *message, uint16_t timeout);


/** @brief       read status flags.
 *
 *  @remarks     This command is only active if the CAN channel is open.
 *
 *  @param[in]   port   - pointer to a SLCAN instance
 *  @param[out]  flags  - channel status flags
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
int slcan_status_flags(slcan_port_t port, slcan_flags_t *flags);


/** @brief       sets Acceptance Code Register (ACn Register of SJA1000).
 *
 *  @remarks     This command is only active if the CAN channel is initiated
 *               and not opened.
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *  @param[in]   code  - acceptance code register
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
int slcan_acceptance_code(slcan_port_t port, uint32_t code);


/** @brief       sets Acceptance Mask Register (AMn Register of SJA1000).
 *
 *  @remarks     This command is only active if the CAN channel is initiated
 *               and not opened.
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *  @param[in]   mask  - acceptance mask register
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
int slcan_acceptance_mask(slcan_port_t port, uint32_t mask);


/** @brief       get version number of both SLCAN hardware and software.
 *
 *  @remarks     This command is active always.
 *
 *  @param[in]   port      - pointer to a SLCAN instance
 *  @param[out]  hardware  - hardware version (8-bit: <major>.<minor>)
 *  @param[out]  software  - software version (8-bit: <major>.<minor>)
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
int slcan_version_number(slcan_port_t port, uint8_t *hardware, uint8_t *software);


/** @brief       get serial number of the SLCAN device.
 *
 *  @remarks     This command is active always.
 *
 *  @param[in]   port   - pointer to a SLCAN instance
 *  @param[out]  number - serial number (32-bit: 8 bytes)
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
int slcan_serial_number(slcan_port_t port, uint32_t *number);


/** @brief       signal all waiting objects, if any.
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *
 *  @returns     0 if successful, or a negative value on error.
 */
int slcan_signal(slcan_port_t port);


/** @brief       retrieves version information of the SLCAN API as a
 *               zero-terminated string.
 *
 *  @param[out]  version_no - major and minor version number (optional)
 *  @param[out]  patch_no   - patch number (optional)
 *  @param[out]  build_no   - build number (optional)
 * 
 *  @returns     pointer to a zero-terminated string, or NULL on error.
 */
char *slcan_api_version(uint16_t *version_no, uint8_t *patch_no, uint32_t *build_no);
```

## This and That

The documentation of the SLCAN protocol can be found on [Lawicel CANUSB product page](https://www.canusb.com/products/canusb).

### Dual-License

This work is dual-licensed under the terms of the BSD 2-Clause "Simplified" License and under the terms of the GNU General Public License v3.0 (or any later version).
You can choose between one of them if you use this work in whole or in part.

`SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later`

### Trademarks

Mac and macOS are trademarks of Apple Inc., registered in the U.S. and other countries. \
Windows is a registered trademarks of Microsoft Corporation in the United States and/or other countries. \
POSIX is a registered of the Institute of Electrical and Electronic Engineers, Inc. \
Linux is a registered trademark of Linus Torvalds. \
Cygwin is a registered trademark of Red Hat, Inc. \
All other company, product and service names mentioned herein may be trademarks, registered trademarks, or service marks of their respective owners.

### Hazard Note

_If you connect your CAN device to a real CAN network when using this library, you might damage your application._

### Contact

E-Mail: mailto://info@uv-software.com \
Internet: https://www.uv-software.com
