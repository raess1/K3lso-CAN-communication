package peak.can;

import peak.can.basic.TPCANHandle;
import peak.can.basic.TPCANType;

/**
 * The ChannelItem class wraps a PCANBasic Channel within his TPCANHandle, his TPCANType (for Non-PNP devices only)
 * and a boolean property which indicates if it is in working state.
 *
 * @version 1.10
 * @LastChange $Date: 2016-05-13 14:54:19 +0200 (ven., 13 mai 2016) $
 * @author Jonathan Urban/Uwe Wilhelm/Fabrice Vergnaud
 *
 * @Copyright (C) 1999-2014  PEAK-System Technik GmbH, Darmstadt
 * more Info at http://www.peak-system.com
 */
public class ChannelItem
{

    protected TPCANHandle handle;
    private TPCANType type;
    private boolean canFd = false;
    private boolean working = false;

    /**
     * Default Constructor
     */
    public ChannelItem()
    {
    }

    /**
     * Constructor
     * @param handle The wrapped PCANHandle
     * @param isCanFd States if CAN FD is enabled
     */
    public ChannelItem(TPCANHandle handle, boolean isCanFd)
    {
        this.handle = handle;
        this.canFd = isCanFd;
    }

    /**
     * Constructor
     *
     * @param tPCANHandle The wrapped PCANHandle
     * @param tPCANType The wrapped PCAN Hardware Type (for Non-PNP devices only)
     * @param isCanFd States if CAN FD is enabled
     */
    public ChannelItem(TPCANHandle tPCANHandle, TPCANType tPCANType, boolean isCanFd)
    {
        this.handle = tPCANHandle;
        this.type = tPCANType;
        this.canFd = isCanFd;
    }

    /**
     * Gets the PCAN Hardware Type (for Non-PNP devices only)
     * @return The TPCANType
     */
    public TPCANType getType()
    {
        return type;
    }

    /**
     * Sets the PCAN Hardware Type (for Non-PNP devices only)
     * @param type PCAN Hardware Type
     */
    public void setType(TPCANType type)
    {
        this.type = type;
    }

    @Override
    public String toString()
    {
        String str = handle.toString();
        if (working)
            str += " Working";
        else
            str += " In Pause";
        return str;
    }

    /**
     * Gets the PCAN Handle
     * @return The handle
     */
    public TPCANHandle getHandle()
    {
        return handle;
    }
    /**
     * Sets the PCAN Handle
     * @param handle PCAN Handle
     */
    public void setHandle(TPCANHandle handle)
    {
        this.handle = handle;
    }

    /**
     * Indicates if Handle is Working
     * @return true if handle is working, false if not
     */
    public boolean getWorking()
    {
        return working;
    }

    /**
     * Sets PCAN Handle state
     * @param working true if handle is working, false if not
     */
    public void setWorking(boolean working)
    {
        this.working = working;
    }

    /**
     * @return the isCanFd
     */
    public boolean isCanFd() {
        return canFd;
    }

    /**
     * @param isCanFd the isCanFd to set
     */
    public void setCanFd(boolean isCanFd) {
        this.canFd = isCanFd;
    }
}
