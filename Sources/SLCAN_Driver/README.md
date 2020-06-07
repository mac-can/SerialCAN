### CAN-over-Serial-Line (Lawicel SLCAN Protocol)

_Copyright &copy; 2016,2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)_

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
extern slcan_port_t slcan_create(size_t queueSize);


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
extern int slcan_destroy(slcan_port_t port);


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
 *  !param[in]   param   - serial port attributes
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV   - no such device (invalid port instance)
 *  @retval      EINVAL   - invalid argument (device name is NULL)
 *  @retval      EALREADY - already connected with the serial device
 *  @retval      'errno'  - error code from called system functions:
 *                          'open', 'tcsetattr', 'pthread_create'
 */
extern int slcan_connect(slcan_port_t port, const char *device);


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
extern int slcan_disconnect(slcan_port_t port);


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
extern int slcan_setup_bitrate(slcan_port_t port, uint8_t index);


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
extern int slcan_setup_btr(slcan_port_t port, uint16_t btr);


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
extern int slcan_open_channel(slcan_port_t port);


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
extern int slcan_close_channel(slcan_port_t port);


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
extern int slcan_write_message(slcan_port_t port, const slcan_message_t *message, uint16_t timeout);


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
extern int slcan_read_message(slcan_port_t port, slcan_message_t *message, uint16_t timeout);


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
extern int slcan_status_flags(slcan_port_t port, slcan_flags_t *flags);


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
extern int slcan_acceptance_code(slcan_port_t port, uint32_t code);


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
extern int slcan_acceptance_mask(slcan_port_t port, uint32_t mask);


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
extern int slcan_version_number(slcan_port_t port, uint8_t *hardware, uint8_t *software);


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
extern int slcan_serial_number(slcan_port_t port, uint32_t *number);
```

## This and That

### SourceMedley Repo

The modules in this folder are from the SourceMedley repo which is a collection of reusable sources for C and C++ projects.

### License

SourceMedley is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SourceMedley is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with SourceMedley.  If not, see <http://www.gnu.org/licenses/>.

### Trademarks

Mac and macOS are trademarks of Apple Inc., registered in the U.S. and other countries. \
Windows is a registered trademarks of Microsoft Corporation in the United States and/or other countries. \
POSIX is a registered of the Institute of Electrical and Electronic Engineers, Inc. \
Linux is a registered trademark of Linus Torvalds.

### Contact

Uwe Vogt \
UV Software \
Chausseestrasse 33a \
10115 Berlin \
Germany

E-Mail: mailto://info@mac.can.com \
Internet: https://www.mac-can.com

##### *Have a lot of fun!*
