package peak.can.basic;

/**
 * Parameter values definition
 *
 * @version 1.9
 * @LastChange $Date: 2016-05-13 14:55:44 +0200 (ven., 13 mai 2016) $
 * @author Jonathan Urban/Uwe Wilhelm/Fabrice Vergnaud
 *
 * @Copyright (C) 1999-2014  PEAK-System Technik GmbH, Darmstadt
 * more Info at http://www.peak-system.com
 */
public enum TPCANParameterValue
{
    /** The PCAN parameter is not set (inactive)*/
    PCAN_PARAMETER_OFF(0),
    /** The PCAN parameter is set (active)*/
    PCAN_PARAMETER_ON(1),
    
    /** The PCAN filter is closed. No messages will be received*/
    PCAN_FILTER_CLOSE(0),
    /** The PCAN filter is fully opened. All messages will be received*/
    PCAN_FILTER_OPEN(1),
    /** The PCAN filter is custom configured. Only registered messages will be received*/
    PCAN_FILTER_CUSTOM(2),
    
    /** The PCAN-Channel handle is illegal*/
    PCAN_CHANNEL_UNAVAILABLE(0),
    /** The PCAN-Channel handle is valid to connect/initialize*/
    PCAN_CHANNEL_AVAILABLE(1),
    /** The PCAN-Channel handle is valid, but is already being used*/
    PCAN_CHANNEL_OCCUPIED(2),
    /** The PCAN-Channel handle is already being used by a PCAN-View application, but is available to connect*/
    PCAN_CHANNEL_PCANVIEW(PCAN_CHANNEL_AVAILABLE.value | PCAN_CHANNEL_OCCUPIED.value),
    
    /** Logs system exceptions / errors*/
    LOG_FUNCTION_DEFAULT(0x00),
    /** Logs the entries to the PCAN-Basic API functions*/
    LOG_FUNCTION_ENTRY(0x01),
    /** Logs the parameters passed to the PCAN-Basic API functions*/
    LOG_FUNCTION_PARAMETERS(0x02),
    /** Logs the exits from the PCAN-Basic API functions*/
    LOG_FUNCTION_LEAVE(0x04),
    /** Logs the CAN messages passed to the CAN_Write function*/
    LOG_FUNCTION_WRITE(0x08),
    /** Logs the CAN messages received within the CAN_Read function*/
    LOG_FUNCTION_READ(0x10),
    /** Logs all possible information within the PCAN-Basic API functions*/
    LOG_FUNCTION_ALL(0xFFFF),
    
    /** A single file is written until it size reaches PCAN_TRACE_SIZE */
    TRACE_FILE_SINGLE(0x00),
    /** Traced data is distributed in several files with size PCAN_TRACE_SIZE */
    TRACE_FILE_SEGMENTED(0x01),
    /** Includes the date into the name of the trace file */
    TRACE_FILE_DATE(0x02),
    /** Includes the start time into the name of the trace file */
    TRACE_FILE_TIME(0x04),
    /** Causes the overwriting of available traces (same name) */
    TRACE_FILE_OVERWRITE(0x80),
    
    /**Device supports flexible data-rate (CAN-FD)*/
    FEATURE_FD_CAPABLE(0x01);
    
    private TPCANParameterValue(int value)
    {
        this.value = value;
    }

    /**
     * The value of the CAN parameter's value
     * @return Value of the CAN parameter's value
     */
    public int getValue()
    {
        return this.value;
    }

    /**
     * Parses a int value into the TPCANParameterValue PCAN_PARAMETER_ON or PCAN_PARAMETER_OFF
     * @param value parsed value
     * @return Corresponding TPCANParameterValue
     */
    static public TPCANParameterValue parseOnOff(int value)
    {
        if(value == TPCANParameterValue.PCAN_PARAMETER_ON.getValue())
            return TPCANParameterValue.PCAN_PARAMETER_ON;
        else if(value == TPCANParameterValue.PCAN_PARAMETER_OFF.getValue())
            return TPCANParameterValue.PCAN_PARAMETER_OFF;
        else
            return null;
    }
     /**
     * Parses a int value into the TPCANParameterValue PCAN_CHANNEL_AVAILABLE, PCAN_CHANNEL_ILLEGAL, PCAN_CHANNEL_OCCUPIED or PCAN_CHANNEL_UNOCCUPIED
     * @param value parsed value
     * @return Corresponding TPCANParameterValue
     */
    static public TPCANParameterValue parseCondition(int value)
    {
        if(value == TPCANParameterValue.PCAN_CHANNEL_AVAILABLE.getValue())
            return TPCANParameterValue.PCAN_CHANNEL_AVAILABLE;
        else if(value == TPCANParameterValue.PCAN_CHANNEL_UNAVAILABLE.getValue())
            return TPCANParameterValue.PCAN_CHANNEL_UNAVAILABLE;
        else if(value == TPCANParameterValue.PCAN_CHANNEL_OCCUPIED.getValue())
            return TPCANParameterValue.PCAN_CHANNEL_OCCUPIED;
        else if(value == TPCANParameterValue.PCAN_CHANNEL_PCANVIEW.getValue())
            return TPCANParameterValue.PCAN_CHANNEL_PCANVIEW;
        else
            return null;
    }
    /**
     * Parses a int value into the TPCANParameterValue PCAN_FILTER_CLOSE, PCAN_FILTER_OPEN or PCAN_FILTER_CUSTOM.
     * @param value parsed value
     * @return Corresponding TPCANParameterValue
     */
    static public TPCANParameterValue parseFilterStatus(int value)
    {
        if(value == TPCANParameterValue.PCAN_FILTER_CLOSE.getValue())
            return TPCANParameterValue.PCAN_FILTER_CLOSE;
        else if(value == TPCANParameterValue.PCAN_FILTER_OPEN.getValue())
            return TPCANParameterValue.PCAN_FILTER_OPEN;
        else if(value == TPCANParameterValue.PCAN_FILTER_CUSTOM.getValue())
            return TPCANParameterValue.PCAN_FILTER_CUSTOM;
        else
            return null;
    }
    private final int value;
};
