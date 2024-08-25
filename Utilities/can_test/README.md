__CAN Tester for CAN-over-Serial-Line Interfaces, Version 0.3__ \
Copyright &copy; 2005-2010 Uwe Vogt, UV Software, Friedrichshafen \
Copyright &copy; 2012-2024 Uwe Vogt, UV Software, Berlin

```
Usage: can_test <interface> [<option>...]
Options for receiver test (default test mode):
 -r, --receive                        count received messages until ^C is pressed
 -n, --number=<number>                check up-counting numbers starting with <number>
 -s, --stop                           stop on error (with option --number)
 -m, --mode=2.0                       CAN operation mode: CAN 2.0
     --shared                         shared CAN controller access (if supported)
     --listen-only                    monitor mode (listen-only mode)
     --error-frames                   allow reception of error frames
     --no-remote-frames               suppress remote frames (RTR frames)
     --no-extended-frames             suppress extended frames (29-bit identifier)
 -b, --baudrate=<baudrate>            CAN bit-timing in kbps (default=250), or
     --bitrate=<bit-rate>             CAN bit-rate settings (as key/value list)
 -v, --verbose                        show detailed bit-rate settings
Options for transmitter test:
 -t, --transmit=<time>                send messages for the given time in seconds, or
 -f, --frames=<number>,               alternatively send the given number of messages, or
     --random=<number>                optionally with random cycle time and data length
 -c, --cycle=<cycle>                  cycle time in milliseconds (default=0) or
 -u, --usec=<cycle>                   cycle time in microseconds (default=0)
 -d, --dlc=<length>                   send messages of given length (default=8)
 -i, --id=<can-id>                    use given identifier (default=100h)
 -n, --number=<number>                set first up-counting number (default=0)
 -m, --mode=2.0                       CAN operation mode: CAN 2.0
     --shared                         shared CAN controller access (if supported)
 -b, --baudrate=<baudrate>            CAN bit-timing in kbps (default=250), or
     --bitrate=<bit-rate>             CAN bit-rate settings (as key/value list)
 -v, --verbose                        show detailed bit-rate settings
Other options:
     --list-bitrates[=2.0]            list standard bit-rate settings and exit
 -h, --help                           display this help screen and exit
     --version                        show version information and exit
Arguments:
  <frames>       send this number of messages (frames), or
  <time>         send messages for the given time in seconds
  <msec>         cycle time in milliseconds (default=0), or 
  <usec>         cycle time in microseconds (default=0)
  <can-id>       send with given identifier (default=100h)
  <length>       send data of given length (default=8)
  <number>       set first up-counting number (default=0)
  <interface>    CAN interface board (list all with /LIST)
  <baudrate>     CAN baud rate index (default=3):
                 0 = 1000 kbps
                 1 = 800 kbps
                 2 = 500 kbps
                 3 = 250 kbps
                 4 = 125 kbps
                 5 = 100 kbps
                 6 = 50 kbps
                 7 = 20 kbps
                 8 = 10 kbps
  <bitrate>      comma-separated <key>=<value>-list:
                 f_clock=<value>      frequency in Hz or
                 f_clock_mhz=<value>  frequency in MHz
                 nom_brp=<value>      bit-rate prescaler (nominal)
                 nom_tseg1=<value>    time segment 1 (nominal)
                 nom_tseg2=<value>    time segment 2 (nominal)
                 nom_sjw=<value>      sync. jump width (nominal)
                 nom_sam=<value>      sampling (only SJA1000)
Hazard note:
  If you connect your CAN device to a real CAN network when using this program,
  you might damage your application.
```

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
