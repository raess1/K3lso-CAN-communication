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
wget https://www.peak-system.com/fileadmin/media/linux/files/peak-linux-driver-8.10.2.tar.gz
tar -xzf peak-linux-driver-8.10.2.tar.gz # Untar the compressed tarball file
cd peak-linux-driver-8.10.2/ 
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

