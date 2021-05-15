# K3lso-CAN-communication

![Blueprint](https://user-images.githubusercontent.com/6362413/114827198-ac9e2f00-9dc8-11eb-9a55-0b9ee85d9e90.png)


PCAN-M.2 CAN and CAN FD Interface for M.2 (PCIe)  https://www.peak-system.com/PCAN-M-2.473.0.html?&L=1

PCAN-miniPCIe FD https://www.peak-system.com/PCAN-miniPCIe-FD.464.0.html?&L=1



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





# Pcanview timings and settings for communication with moteus r4.5 controller.
GETTING STARTED

Mjbots have confirmed timings that are working with 80 MHz clock systems over SocketCan https://github.com/mjbots/moteus/blob/main/docs/reference.md#80-mhz-clock-systems

This timings will not work out-of-the running Peak Basic API since they use a different format.
SocketCan timings:
``` bash
ip link set can0 up type can \
  tq 12 prop-seg 25 phase-seg1 25 phase-seg2 29 sjw 10 \
  dtq 12 dprop-seg 6 dphase-seg1 2 dphase-seg2 7 dsjw 12 \
  restart-ms 1000 fd on
```

PEAK converting timings help
``` bash
f_clock_mhz = 80mhz (depending on device)
brp = 1
nom_Tseq1 = prop-seg + phase-seg1
nom_Tseq2 = phase-seg2
data_tseg1 = dprop-seg + dphase-seg1
data_tseg2 = dphase-seg2
nom_sjw = sjw
data_sjw  = dsjw
data_sjw = 1
```

With the help from above you will end up with someting like this:
``` bash
Timings:
f_clock_mhz=80
brp=1
nom_Tseq1 = 50
nom_Tseq2 = 29
nom_sjw=10
data_brp=1
data_tseg1=8
data_tseg2=7
data_sjw=12
```


First start Pcanview with (assuming you have it installed)
``` bash
pcanview 
```

Select Pcan Channel (for example  -CAN2 /dev/pcan1) and configure it with the following settings:
``` bash
Clock Frequency (Hz) = 80000000
Bitrate (bps) Nominal = 1000000
Bitrate (bps) Data = 5000000
Sample point (x100) Nominal = 6375
Sample point (x100) Data = 5625
Sync Jump Width Nominal = 10
Sync Jump Width Data = 12
```


### Test 1
To send a first test command to verify communication with moteus (Edit Transmit Message)
``` bash
ID: (hex) = 8001
Len = 3
Data: (hex) = 42 01 20
Cycle Time: (ms) = 0
[x] Paused 
- Message Type select the follwing: 
[x] Extended Frame
[x] CAN FD
[x] Bit Rate Swtich
Press OK
```

Under the Tx CAN-ID you can see your created Messages. to send one. simply select one and press (space).

In the Rx CAN-ID. you should now get a respons from moteus under data table with the following:
``` bash
41 01 00
```

### Test 2
Create two new Transmit Messages. (Edit Transmit Message)

first messange: Set postion to 0 and turn on torque.
``` bash
ID: (hex) = 8001
Len = 19
Data: (hex) = 01 00 0a 0e 20 00 00 00 00 00 00 00 00 11 00 1f 01 13 0d
Cycle Time: (ms) = 0
[x] Paused 
- Message Type select the follwing: 
[x] Extended Frame
[x] CAN FD
[x] Bit Rate Swtich
Press OK
```
Second messange: turn of torque
``` bash
ID: (hex) = 8001
Len = 3
Data: (hex) = 01 00 00
Cycle Time: (ms) = 0
[x] Paused 
- Message Type select the follwing: 
[x] Extended Frame
[x] CAN FD
[x] Bit Rate Swtich
Press OK
```
Send first messange and you should here the motor has applied torque.
RX respons: 
``` bash
21 00 00 2f 01 29 a3 1a 3d 33 e3 8a 3a 00 00 00 00 23 0d 32 1c 00 50 50
```
Send second messange stop applied torque.
RX respons: 
``` bash
0
```



Can notes:
SJW – abbreviation for Synchronization Jump Width – a quantity that determines how much a CAN controller is allowed to adjust its on-chip clock to run in sync with other nodes on the bus. A typical value would be some 10-20% of a bit.




## improving performance (test section)
To start we are hitting a loop Frequency at around = 650[hz] for one motor out of the box with the PCAN-M.2 Four Channel. Yes. this is not a great speed!
So lets improve it!


The pcan driver behavior can be configured to answer a bit quicker to the PCIe FD hw interrupts.
Open the pcan.conf with sudo.
``` bash
sudo gedit /etc/modprobe.d/pcan.conf
```
Lowering these values should increase the responsiveness of the driver.
```
# pcan - automatic made entry, begin --------
# if required add options and remove comment
# options pcan type=isa,sp
options pcan fdirqtl=1
options pcan fdirqcl=1
install pcan modprobe --ignore-install pcan
# pcan - automatic made entry, end ----------
```
Save the file and run the follwing. (which unloads and then reloads the drive) 

``` bash
$ sudo rmmod pcan
$ sudo modprobe pcan
```

With this new conf we are hitting around `2489.53[hz]`

Definition
- fdirqcl defines the limit of pending frames above which the HW generates a hw interrupt, while
- fdirqtl defines the time limit above which the HW generates a hw interrupt.

According to the manual (PAGE 11) https://www.peak-system.com/fileadmin/media/linux/files/PCAN-Driver-Linux_UserMan_eng.pdf

We can enable the driver the usage of MSI for the PCIe-based CAN FD cards.
![1](https://user-images.githubusercontent.com/6362413/112323778-6286c980-8cb2-11eb-9158-9b50120e15aa.PNG)

Lets try it! 

``` bash
sudo gedit /etc/modprobe.d/pcan.conf
```

add `options pcan fdusemsi=1`
```
# pcan - automatic made entry, begin --------
# if required add options and remove comment
# options pcan type=isa,sp
options pcan fdirqtl=1
options pcan fdirqcl=1
options pcan fdusemsi=1
install pcan modprobe --ignore-install pcan
# pcan - automatic made entry, end ----------
```
Save the file and run the follwing. (which unloads and then reloads the drive) 

``` bash
$ sudo rmmod pcan
$ sudo modprobe pcan
```

With this new config we are hitting around `3090.87[hz]` You can play around with this settings to see what you get. for example `options pcan fdusemsi=2`
 


1. Run via shell (not over GUI)
 Frequency = 3089.7[hz]
3. Adjust data size. (12bit vs 32bit).
4. Priority settings.
5. Adjust CPU performance test.
6. OS/Kernel optimization.
7. Driver nonblocking vs blocking
8. Run on isolated thread.


## cpuset Lets try some basic cpu isolation.

``` bash
cset shield --cpu 4-7
```

``` bash
cset shield --exec mycommand -- -arg1 -arg2
```
more to come.
realse shield.


Notes for myself:
for ID in $(seq 1 12); do echo "d pos nan 0 5" | moteus_tool -t $ID -c; done

##  Set a device-number for specific channel 

Check current device number (replace [pcanpcifd0] with results from ls -l /dev/pcan*)
``` bash
pcan-settings -f=/dev/pcanpcifd0 –d
```

Apply a device id to a channel (in this example id:10)
``` bash
pcan-settings -f=/dev/pcanpcifd0 –d 10
```

Confirm and assign ID:s to each channel. Then we can use `#define DEVICE "/dev/pcan-pcie_fd/devid=X` to call the joint


##  moteus_tool and tview configuration over python-can
`~/can.conf`

Add the following to the can.conf
```
[default]
interface = pcan
fd = True
f_clock_mhz = 80
nom_brp = 1
nom_tseg1 = 50
nom_tseg2 = 29
nom_sjw = 10
data_brp = 1
data_tseg1 = 8
data_tseg2 = 7
data_sjw = 12
```
Open terminal and type execute. Where [devices=x] is the motor ID and [PCAN_PCIBUS2] is your can bus. So please note that the ID most exist on the correct BUS.
`python3.7 -m moteus_gui.tview --devices=x --can-iface pcan --can-chan PCAN_PCIBUS2`



