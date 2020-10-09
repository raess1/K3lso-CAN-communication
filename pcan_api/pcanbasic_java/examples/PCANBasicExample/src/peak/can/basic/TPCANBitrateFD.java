package peak.can.basic;

/**
 * FD bit rates for the CAN FD controller. You can
 * define your own bit rate with the FD string, as shown in this exemple:
 * "f_clock_mhz=20, nom_brp=5, nom_tseg1=2, nom_tseg2=1, nom_sjw=1, 
 * data_brp=2, data_tseg1=3, data_tseg2=1, data_sjw=1"
 *
 * @version 1.9
 * @LastChange $Date: 2016-05-13 14:55:44 +0200 (ven., 13 mai 2016) $
 * @author Jonathan Urban/Uwe Wilhelm/Fabrice Vergnaud
 *
 * @Copyright (C) 1999-2014 PEAK-System Technik GmbH, Darmstadt more Info at
 * http://www.peak-system.com
 */
public class TPCANBitrateFD {
    private String value;

    /**
     * Creates a CAN FD bitrate
     * @param value A CAN FD bitrate string (for instance:
     * "f_clock_mhz=20, nom_brp=5, nom_tseg1=2, nom_tseg2=1, nom_sjw=1, 
     *  data_brp=2, data_tseg1=3, data_tseg2=1, data_sjw=1")
     */
    public TPCANBitrateFD(String value) {
        this.value = value;
    }

    /**
     * Returns the string configuration of the bitrate code.
     * @return The bitrate string configuration
     */
    public String getValue() {
        return this.value;
    }
    /**
     * Sets string configuration of the bitrate code.
     * @param value The new bitrate string configuration
     */
    public void setValue(String value) {
        this.value = value;
    }
};
