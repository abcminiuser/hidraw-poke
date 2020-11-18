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

Important Note - Kernel Patch Required:
---------------------------------------
Also note that the Linux kernel currently only supports feature request
read and writes via the control channel only. This project uses
additional ioctls proposed in a as-yet unaccepted patch to add support
for reading and writing input and output reports via the same method.

If the kernel patch is not applied, only feature requests can be written
or read.

For the associated proposed kernel patch, [see here](https://github.com/abcminiuser/linux/tree/topic-linux-hidraw).
