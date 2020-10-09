package peak.can.basic;

/**
 * Baud rate codes = BTR0/BTR1 register values for the CAN controller. You can
 * define your own Baud rate with the BTROBTR1 register. Take a look at
 * www.peak-system.com for our free software "BAUDTOOL" to calculate the
 * BTROBTR1 register for every baudrate and sample point.
 *
 * @version 1.9
 * @LastChange $Date: 2016-05-13 14:55:44 +0200 (ven., 13 mai 2016) $
 * @author Jonathan Urban/Uwe Wilhelm/Fabrice Vergnaud
 *
 * @Copyright (C) 1999-2014 PEAK-System Technik GmbH, Darmstadt more Info at
 * http://www.peak-system.com
 */
public enum TPCANBitrateFDValue {

    /**
     * Clock frequency in Herz (80000000, 60000000, 40000000, 30000000,
     * 24000000, 20000000)
     */
    PCAN_BR_CLOCK("f_clock"),
    /**
     * Clock frequency in Megaherz (80, 60, 40, 30, 24, 20)
     */
    PCAN_BR_CLOCK_MHZ("f_clock_mhz"),
    /**
     * Clock prescaler for nominal time quantum
     */
    PCAN_BR_NOM_BRP("nom_brp"),
    /**
     * TSEG1 segment for nominal bit rate in time quanta
     */
    PCAN_BR_NOM_TSEG1("nom_tseg1"),
    /**
     * TSEG2 segment for nominal bit rate in time quanta
     */
    PCAN_BR_NOM_TSEG2("nom_tseg2"),
    /**
     * Synchronization Jump Width for nominal bit rate in time quanta
     */
    PCAN_BR_NOM_SJW("nom_sjw"),
    /**
     * Sample point for nominal bit rate
     */
    PCAN_BR_NOM_SAMPLE("nom_sam"),
    /**
     * Clock prescaler for highspeed data time quantum
     */
    PCAN_BR_DATA_BRP("data_brp"),
    /**
     * TSEG1 segment for fast data bit rate in time quanta
     */
    PCAN_BR_DATA_TSEG1("data_tseg1"),
    /**
     * TSEG2 segment for fast data bit rate in time quanta
     */
    PCAN_BR_DATA_TSEG2("data_tseg2"),
    /**
     * Synchronization Jump Width for highspeed data bit rate in time quanta
     */
    PCAN_BR_DATA_SJW("data_sjw"),
    /**
     * Secondary sample point delay for highspeed data bitrate in cyles
     */
    PCAN_BR_DATA_SAMPLE("data_ssp_offset");

    private final String value;

    private TPCANBitrateFDValue(String value) {
        this.value = value;
    }

    /**
     * The string value of Bitrate FD string parameter
     * @return String corresponding to the bitrate FD string parameter
     */
    public String getValue() {
        return this.value;
    }
};
