HIDRAW-POKE - Linux HIDRAW Test Tool
====================================

This is a simple test utility for the linux hidraw module, allowing for a
simple command line utility to read and write raw HID reports from/to an
attached HID device.

By default, HID devices should generate an ephemeral hidraw device entry
in `/dev/hidraw*`, which can be used to control the HID device directly,
bypassing the normal HID report parser and HID higher layers in the
kernel.

Functionality:
--------------
This utility is intended to issue single control request read and writes
only, rather than regular reads and writes over the normal I/O. Not all
devices will support HID read and write report requests over the control
channel, instead of the regular data channel.

Usage:
------

Example usage:
```
hidraw-poke --write feature --id 2 --device /dev/hidraw0 01 02 ab cd
hidraw-poke --read input --id 5 --device /dev/hidraw0 --length 6
```

See `hidraw-poke --help` for more usage information.

Important Note - Kernel 5.11 or Newer Required:
-----------------------------------------------
Kernel versions older than 5.11 only support a limited set of ioctls,
for reading and writing Feature reports via the control channel only.

To read and write Input and Output reports via the control channel,
Linux 5.11 on newer must be used.
