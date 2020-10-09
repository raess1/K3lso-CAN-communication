package peak.can.basic;

/**
 * Defines the point of time at which a CAN message was received.
 *
 * @version 1.0
 * @LastChange $Date: 2016-05-13 14:55:44 +0200 (ven., 13 mai 2016) $
 * @author Jonathan Urban/Uwe Wilhelm/Fabrice Vergnaud
 *
 * @Copyright (C) 1999-2014  PEAK-System Technik GmbH, Darmstadt
 * more Info at http://www.peak-system.com
 */
public class TPCANTimestampFD
{
    private long _timestamp;

    /**
     * Default constructor
     */
    public TPCANTimestampFD()
    {
    }

    /**
     * @return the timestamp
     */
    public long getValue() {
        return _timestamp;
    }

    /**
     * @param timestamp the timestamp to set
     */
    public void setValue(long timestamp) {
        this._timestamp = timestamp;
    }
}
