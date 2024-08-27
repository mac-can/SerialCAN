__CAN Monitor for CAN-over-Serial-Line Interfaces, Version 0.3__ \
Copyright &copy; 2007,2012-2024 by Uwe Vogt, UV Software, Berlin

```
Usage: can_moni <interface> [<option>...]
Options:
 -t, --time=(ZERO|ABS|REL)            absolute or relative time (default=0)
 -i  --id=(HEX|DEC|OCT)               display mode of CAN-IDs (default=HEX)
 -d, --data=(HEX|DEC|OCT)             display mode of data bytes (default=HEX)
 -a, --ascii=(ON|OFF)                 display data bytes in ASCII (default=ON)
 -x, --exclude=[~]<id-list>           exclude CAN-IDs: <id-list> = <id>[-<id>]{,<id>[-<id>]}
     --code=<id>                      acceptance code for 11-bit IDs (default=0x000)
     --mask=<id>                      acceptance mask for 11-bit IDs (default=0x000)
     --xtd-code=<id>                  acceptance code for 29-bit IDs (default=0x00000000)
     --xtd-mask=<id>                  acceptance mask for 29-bit IDs (default=0x00000000)
 -m, --mode=2.0                       CAN operation mode: CAN 2.0
     --shared                         shared CAN controller access (if supported)
     --listen-only                    monitor mode (listen-only mode)
     --error-frames                   allow reception of error frames
     --no-remote-frames               suppress remote frames (RTR frames)
     --no-extended-frames             suppress extended frames (29-bit identifier)
 -b, --baudrate=<baudrate>            CAN bit-timing in kbps (default=250), or
     --bitrate=<bit-rate>             CAN bit-rate settings (as key/value list)
 -v, --verbose                        show detailed bit-rate settings
 -z, --protocol=(Lawicel|CANable)     select SLCAN protocol (default=Lawicel)
     --list-bitrates[=2.0]            list standard bit-rate settings and exit
 -h, --help                           display this help screen and exit
     --version                        show version information and exit
Arguments:
  <id>           CAN identifier (11-bit)
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
