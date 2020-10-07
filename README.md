# K3lso Quadruped

K3lso is a quadruped robot, powered by Jetson NX.  This is BLDC version.

Slack channel: https://join.slack.com/t/newworkspace-7gn1112/shared_invite/zt-hqn0zss3-pZaL3pkLpwtRk7jjMMkjIw

Visit the main repository: https://github.com/raess1/K3lso-Quadruped

# K3lso-CAN-communication

PCAN-M.2 CAN and CAN FD Interface for M.2 (PCIe)  https://www.peak-system.com/PCAN-M-2.473.0.html?&L=1

Check: Are CAN drivers part of your Linux environment?
grep PEAK_ /boot/config-`uname -r`

Check: Is the CAN device initialized?
lsmod | grep ^peak

...
