package peak.can.basic;

import java.util.EnumSet;

/**
 * Represents the type of a PCAN message
 *
 * @version 1.9
 * @LastChange $Date: 2016-05-13 14:55:44 +0200 (ven., 13 mai 2016) $
 * @author Fabrice Vergnaud
 *
 * @Copyright (C) 1999-2012 PEAK-System Technik GmbH, Darmstadt more Info at
 * http://www.peak-system.com
 */
public enum TPCANMessageType {

    /**
     * The PCAN message is a CAN Standard Frame (11-bit identifier)
     */
    PCAN_MESSAGE_STANDARD((byte) 0x00),
    /**
     * The PCAN message is a CAN Remote-Transfer-Request Frame
     */
    PCAN_MESSAGE_RTR((byte) 0x01),
    /**
     * The PCAN message is a CAN Extended Frame (29-bit identifier)
     */
    PCAN_MESSAGE_EXTENDED((byte) 0x02),
    /**
     * The PCAN message represents a FD frame in terms of CiA Specs
     */
    PCAN_MESSAGE_FD((byte) 0x04),
    /**
     * The PCAN message represents a FD bit rate switch (CAN data at a higher
     * bitrate)
     */
    PCAN_MESSAGE_BRS((byte) 0x08),
    /**
     * The PCAN message represents a FD error state indicator(CAN FD transmitter
     * was error active)
     */
    PCAN_MESSAGE_ESI((byte) 0x10),
    /**
     * The PCAN message represents a PCAN status message
     */
    PCAN_MESSAGE_STATUS((byte) 0x80);

    private TPCANMessageType(byte value) {
        this.value = value;
    }

    /**
     * The value of the message type
     * @return Value of the message type
     */
    public byte getValue() {
        return this.value;
    }
        
    private final byte value;
    
    /**
     * Gets the value of an EnumSet
     * @param type collection of TPCANMessageType
     * @return value of the EnumSet
     */
    public static byte getValue(EnumSet<TPCANMessageType> type)
    {
        byte result = 0;
        for (TPCANMessageType t : type)
            result |= t.value;
        return result;
    }
}
