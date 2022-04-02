# blinky-f0/tools

Development tools for building, testing, and debugging.

#### Device Test Agent (DTA)
I use WSL as my primary development environment.
This works really great for me, but there is one drawback: no access to USB ports from WSL.
I work around this by using a separate Linux host (an RPi or Intel NUC)
to which the device target is connected via USB.  I can then ssh into this remote host,
load an image onto my target using openocd, and access the UARTs on the device.

#### load-remote-target
Load a given application binary to the target device.
Assumes the target device is connected to a remote host.
This script uses ssh to access the remote host.
The remote host is specified to ssh by an environment variable named "DTA_HOST".
This script assumes:
- the user has access to the remote host through an ssh key.
- the binary to be loaded exists already on the remote host in /tmp.

Typically the build script will automatically deloy the binary image to the remote host when the build is successful.

