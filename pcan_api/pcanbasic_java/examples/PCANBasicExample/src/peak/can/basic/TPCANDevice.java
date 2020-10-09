package peak.can.basic;

/**
 * Represents a PCAN device
 *
 * @version 1.9
 * @LastChange $Date: 2016-05-13 14:55:44 +0200 (ven., 13 mai 2016) $
 * @author Jonathan Urban/Uwe Wilhelm/Fabrice Vergnaud
 *
 * @Copyright (C) 1999-2012 PEAK-System Technik GmbH, Darmstadt more Info at
 * http://www.peak-system.com
 */
public enum TPCANDevice {

    /**
     * Undefined, unknown or not selected PCAN device value
     */
    PCAN_NONE((byte) 0x00),
    /**
     * PCAN Non-Plug'n'Play devices. NOT USED WITHIN PCAN-Basic API
     */
    PCAN_PEAKCAN((byte) 0x01),
    /**
     * PCAN-ISA, PCAN-PC/104, and PCAN-PC/104-Plus
     */
    PCAN_ISA((byte) 0x02),
    /**
     * PCAN-Dongle
     */
    PCAN_DNG((byte) 0x03),
    /**
     * PCAN-PCI, PCAN-cPCI, PCAN-miniPCI, and PCAN-PCI Express
     */
    PCAN_PCI((byte) 0x04),
    /**
     * PCAN-USB and PCAN-USB Pro
     */
    PCAN_USB((byte) 0x05),
    /**
     * PCAN-PC Card
     */
    PCAN_PCC((byte) 0x06),
    /**
     * PCAN Virtual hardware. NOT USED WITHIN PCAN-Basic API
     */
    PCAN_VIRTUAL((byte) 0x07),
    /**
     * PCAN Gateway devices
     */
    PCAN_LAN((byte) 0x08);

    private TPCANDevice(byte value) {
        this.value = value;
    }

    /**
     * The value of the CAN device
     * @return Byte value of the CAN device
     */
    public byte getValue() {
        return this.value;
    }
    private final byte value;
};
