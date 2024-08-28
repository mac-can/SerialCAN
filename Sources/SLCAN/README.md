### Lawicel SLCAN Protocol (Serial-Line CAN)

_Copyright &copy; 2016,2020-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)_ \
_All rights reserved._

# SLCAN

Implementation of Lawicel SLCAN protocol.

## Lawicel SLCAN Protocol

### Supported commands

- `O[CR]` - Open the CAN channel
- `C[CR]` - Close the CAN channel
- `Sn[CR]` - Setup with standard CAN bit-rates (`0` = 10kbps, `1` = 20kbps, ..., `8` = 1000kbps)
- `sxxyy[CR]` -  Setup with BTR0/BTR1 CAN bit-rates (`xx` = BTR0, `yy` = BTR1)
- `tiiildd...[CR]` - Transmit a standard (11bit) CAN frame (`iii` = Id., `l` = DLC, `dd` = Data)
- `Tiiiiiiiildd...[CR]` - Transmit an extended (29bit) CAN frame  (`iiiiiiii` = Id., `l` = DLC, `dd` = Data)
- `riiil[CR]` - Transmit an standard RTR (11bit) CAN frame (`iii` = Id., `l` = DLC)
- `Riiiiiiiil[CR]` - Transmit an extended RTR (29bit) CAN frame (`iiiiiiii` = Id., `l` = DLC)
- `F[CR]` - Read Status Flags (returns 8-bit status register; see below)
- `Mxxxxxxxx[CR]` - Sets Acceptance Code Register (AC0, AC1, AC2 & AC3; LSB first)
- `mxxxxxxxx[CR]` - Sets Acceptance Mask Register (AM0, AM1, AM2 & AM3; LSB first)
- `V[CR]` - Get Version number of both CANUSB hardware and software (returns `Vhhss[CR]` - `hh` = Hw, `ss` = Sw)
- `N[CR]` - Get Serial number of the CANUSB (returns `Naaaa[CR]` - `aaaa` = S/N; alpha-numerical)

Note: Channel configuration commands must be sent before opening the channel. The channel must be opened before transmitting frames.

### CANable SLCAN Protocol (Option 1)

Supported commands

- `O` - Open channel
- `C` - Close channel
- `S0` - Set bitrate to 10k
- `S1` - Set bitrate to 20k
- `S2` - Set bitrate to 50k
- `S3` - Set bitrate to 100k
- `S4` - Set bitrate to 125k
- `S5` - Set bitrate to 250k
- `S6` - Set bitrate to 500k
- `S7` - Set bitrate to 750k
- `S8` - Set bitrate to 1M
- `M0` - Set mode to normal mode (default) *(not supported)*
- `M1` - Set mode to silent mode *(not supported)*
- `A0` - Disable automatic retransmission *(not supported)*
- `A1` - Enable automatic retransmission (default) *(not supported)*
- `TIIIIIIIILDD...` - Transmit data frame (Extended ID) [ID, length, data]
- `tIIILDD...` - Transmit data frame (Standard ID) [ID, length, data]
- `RIIIIIIIIL` - Transmit remote frame (Extended ID) [ID, length]
- `rIIIL` - Transmit remote frame (Standard ID) [ID, length]
- `V` - Returns firmware version and remote path as a string

Note: Channel configuration commands must be sent before opening the channel. The channel must be opened before transmitting frames.

**Note: The firmware currently does not provide any ACK/NACK feedback for serial commands.**

Note: The implementation currently does not support CAN FD commands and frame format.

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


/** @brief       enables or disables ACK/NACK feedback for serial commands.
 *               Defaults to ACK/NACK feedback enabled (Lawicel protocol).
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *  @param[in]   on    -
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV   - no such device (invalid port instance)
 */
int slcan_set_ack(slcan_port_t port, bool on);


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

The documentation of the SLCAN protocol can be found on [Lawicel CANUSB](https://www.canusb.com/products/canusb) product page.
For the CANable 2.0 adaptation, see the [CANable Firmware](https://github.com/normaldotcom/canable-fw) documentation on GitHub. 

### Dual-License

Except where otherwise noted, this work is dual-licensed under the terms of the BSD 2-Clause "Simplified" License
and under the terms of the GNU General Public License v3.0 (or any later version).
You can choose between one of them if you use these portions of this work in whole or in part.

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
