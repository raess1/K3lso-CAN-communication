package peak.can;

/**
 * The MarkAllChannelItem class use an Singleton Design Pattern to represent all connected ChannelItem.
 * It has the undefined/default value for a PCAN bus (PCAN_NONEBUS) and it is labeled "All" in a JComponent.
 *
 * @version 1.9
 * @LastChange $Date: 2016-05-13 14:55:44 +0200 (ven., 13 mai 2016) $
 * @author Jonathan Urban/Uwe Wilhelm
 *
 * @Copyright (C) 1999-2014  PEAK-System Technik GmbH, Darmstadt
 * more Info at http://www.peak-system.com
 */

import peak.can.basic.TPCANHandle;

public class MarkAllChannelItem extends ChannelItem
{

    // Static instance
    private static MarkAllChannelItem instance;

    /**
     * Default Constructor
     */
    public MarkAllChannelItem()
    {
        // Sets the PCANHandle PCAN_NONEBUS
        this.handle = TPCANHandle.PCAN_NONEBUS;
    }

    /**
     * Gets Singleton
     * @return single instance
     */
    public static MarkAllChannelItem getInstance()
    {
        if (null == instance)
            instance = new MarkAllChannelItem();
        return instance;
    }

    /**
     * Gets a label to represent the singleton
     * @return label
     */
    @Override
    public String toString()
    {
        return "ALL";
    }
}
