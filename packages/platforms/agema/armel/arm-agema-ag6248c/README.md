#How to run ONL in AGEMA AG6248C board

For the first step, it only support install the ONL to the USB and boot up.
It will be support to install the ONL to NandFlash next step.

Build the ONL
--------------------------------------------------------------------------
Please refer the $ONL/docs/Building.md

Install the ONL through ONIE
--------------------------------------------------------------------------
```
ONIE:/ # onie-discovery-stop 
discover: installer mode detected.
Stopping: discover... done.
ONIE:/ # 
ONIE:/ # ifconfig eth0 192.168.1.1   #configure the DUT IP address
ONIE:/ # tftp -r ONL-2.*_ARMEL_INSTALLED_INSTALLER -g 192.168.1.99 -b 10240
ONIE:/ # onie-nos-install ONL-2.*_ARMEL_INSTALLED_INSTALLER
```
Boot the ONL
--------------------------------------------------------------------------
Device will reboot automatically after install the ONL installer successfull.
Please Hit any key to enter into uboot mode when boot to uboot phase. We should
set some env to let the uboot load ONL.

--------------------------------------------------------------------------
```
Hit any key to stop autoboot:  0 
agema_ag6248c-poe->usb start
agema_ag6248c-poe->setenv onl_loadaddr 0x70000000; setenv onl_platform arm-agema-ag6248c-poe-r0; setenv onl_itb arm-agema-ag6248c-poe-r0.itb;
agema_ag6248c-poe->setenv bootargs console=ttyS0,115200 onl_platform=$onl_platform earlyprintk maxcpus=2 mem=1024M root=/dev/ram ethaddr=$ethaddr
agema_ag6248c-poe->ext2load usb 0:1 $onl_loadaddr $onl_itb;
agema_ag6248c-poe->bootm $onl_loadaddr#$onl_platform
```
--------------------------------------------------------------------------
Notice:
  If you want to run it on AG6248C NON-POE board, please use "setenv onl_platform arm-agema-ag6248c-r0"

Now it will start the ONL boot progress.
