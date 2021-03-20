# K3lso Quadruped

K3lso is a quadruped robot, powered by Jetson NX.  This is BLDC version.

Slack channel: https://join.slack.com/t/newworkspace-7gn1112/shared_invite/zt-hqn0zss3-pZaL3pkLpwtRk7jjMMkjIw

Visit the main repository: https://github.com/raess1/K3lso-Quadruped

# K3lso-CAN-communication

PCAN-M.2 CAN and CAN FD Interface for M.2 (PCIe)  https://www.peak-system.com/PCAN-M-2.473.0.html?&L=1



Check: Are CAN drivers part of your Linux environment?
``` bash
grep PEAK_ /boot/config-`uname -r`
```

Check: Is the CAN device initialized?
``` bash
lsmod | grep ^peak

ls -l /dev/pcan*

```

How to install PCAN-View via repository

Download and install the following file peak-system.list from the PEAK-System website:
``` bash
wget -q http://www.peak-system.com/debian/dists/`lsb_release -cs`/peak-system.list -O- | sudo tee /etc/apt/sources.list.d/peak-system.list
```
Then, download and install the PEAK-System public key for apt-secure, so that the repository is trusted:
``` bash
wget -q http://www.peak-system.com/debian/peak-system-public-key.asc -O- | sudo apt-key add -
```

Install pcanview-ncurses and other dependencies:
``` bash
sudo apt-get update
sudo apt-get install pcanview-ncurses
sudo apt-get install libpopt-dev
```


Install PCAN-M.2 Driver for Linux (Ubuntu 18.04)
Driver installation:
``` bash
cd && mkdir -p ~/src/peak && cd ~/src/peak
wget https://www.peak-system.com/fileadmin/media/linux/files/peak-linux-driver-8.11.0.tar.gz
tar -xzf peak-linux-driver-8.11.0.tar.gz # Untar the compressed tarball file
cd peak-linux-driver-8.11.0/ 
make -C driver PCI=PCI_SUPPORT PCIEC=PCIEC_SUPPORT DNG=NO_DONGLE_SUPPORT USB=NO_USB_SUPPORT ISA=NO_ISA_SUPPORT PCC=NO_PCCARD_SUPPORT
sudo make -C driver install
make -C lib && sudo make -C lib install
make -C test && sudo make -C test install # test utilities (optional)
sudo modprobe pcan # driver loading
```

Launch PCAN-View for testing CAN communication:
``` bash
pcanview
```

uninstall PCAN-M.2 Driver for Linux (Ubuntu 18.04)
``` bash
cd ~/src/peak/peak-linux-driver-8.10.2/ 
sudo make uninstall
rmmod pcan
sudo reboot
```

SocketCan : Adapter such as the PEAK-CAN-FD use a 80MHz clock. The following timings have been observed to work:
``` bash
ip link set can0 up type can \
  tq 12 prop-seg 25 phase-seg1 25 phase-seg2 29 sjw 10 \
  dtq 12 dprop-seg 6 dphase-seg1 2 dphase-seg2 7 dsjw 12 \
  restart-ms 1000 fd on
```

PEAK
Tseg1 = Prop_Seq + Phase_Seq1 (some CAN controllers have separate fields, we only use a single value here)
Tseg2 = Phase_Seq2


Timings:
f_clock_mhz=80
nom_brp=1
nom_Tseq1 = 50
nom_Tseq2 = 29
nom_sjw=10
data_brp=1
data_tseg1=8
data_tseg2=7
data_sjw=12




``` bash
More info here:
```





export some more properties of the device
``` bash
for f in /sys/class/pcan/pcanpcifd1/*; do [ -f $f ] && echo -n "`basename $f` = " && cat $f; done 
```

lspcan overview of the PC CAN interfaces. The "-i" option displays static properties of devices nodes. with –T –t –s –f refreshes the screen every second with a detailed view 
``` bash
./lspcan -T -t -i
```









## test Directory  (need make test and make install)

``` bash
 pcanfdtst –-help
```



# TX TEST  (connecting to channels together. In this case CAN1 & CAN2)
`(p36 .2 CAN Driver for Linux v8 User manual)`
Write CAN FD (non-ISO) frames with extended ID 0x123 and 24 data bytes at a nominal bitrate of 
1 Mbit/s and data bitrate of 5 Mbit/s, using the 80 MHz clock of the 2nd PCIFD interface and the 
1st PCI interface. 
``` bash
pcanfdtst tx --fd-non-iso -n 10 -ie 0x123 -l 24 -b 1M -d 5M -c 80M /dev/pcanpcifd1 
/dev/pcanpcifd0
```
![111](https://user-images.githubusercontent.com/6362413/111870441-115e9900-8985-11eb-8618-77ddae632b36.PNG)





# RX TEST  (connecting to channels together. In this case CAN1 & CAN2)
`(p37 .5 CAN Driver for Linux v8 User manual)`
Read the same bus, but from the 1st /dev/pcanpcifd1 interface:
``` bash
pcanfdtst rx --fd-non-iso -b 1M -d 5M -c 80M /dev/pcanpcifd1
```
![2222](https://user-images.githubusercontent.com/6362413/111870445-16234d00-8985-11eb-83e3-049b2d0bed51.PNG)




# notes 
1. A Channel that is initialized using CAN_InitializeFD must use CAN_ReadFD and CAN_WriteFD for communication. Calling CAN_Read and/or CAN_Write will result in a PCAN_ERROR_ILLOPERATION error. 
2. As indicated by its name, the CAN_InitializeFD function initiates a FD capable PCAN Channel, preparing it for communicate within the CAN bus connected to it. Calls to the API functions will fail if they are used with a Channel handle, different than PCAN_NONEBUS, that has not been initialized yet. Each initialized channel should be released when it is not needed anymore. 


