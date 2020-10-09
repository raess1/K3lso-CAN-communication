package peak.can.basic;

import java.util.EnumSet;

/**
 * Defines a CAN message.
 *
 * @version 1.9
 * @LastChange $Date: 2016-05-13 14:55:44 +0200 (ven., 13 mai 2016) $
 * @author Jonathan Urban/Uwe Wilhelm/Fabrice Vergnaud
 *
 * @Copyright (C) 1999-2014 PEAK-System Technik GmbH, Darmstadt
 * more Info at http://www.peak-system.com
 */
public class TPCANMsg implements Cloneable
{

    /**
     * 11bit message type (standard)
     * @deprecated Use enum TPCANMessageType instead
     */
	@Deprecated
    static public final byte MSGTYPE_STANDARD = TPCANMessageType.PCAN_MESSAGE_STANDARD.getValue();
    /**
     * Remote request
     * @deprecated Use enum TPCANMessageType instead
     */
	@Deprecated
    static public final byte MSGTYPE_RTR =  TPCANMessageType.PCAN_MESSAGE_RTR.getValue();
    /**
     * 29bit message type (extended)
     * @deprecated Use enum TPCANMessageType instead
     */
	@Deprecated
    static public final byte MSGTYPE_EXTENDED =  TPCANMessageType.PCAN_MESSAGE_EXTENDED.getValue();
    
    private int _id;
    private byte _type;
    private byte _length;
    private byte _data[];

    /**
     * Default constructor
     */
    public TPCANMsg()
    {
        _data = new byte[8];
    }

    /**
     * Constructs a new message object.
     * @param id the message id
     * @param type the message type
     * @param length the message length
     * @param data the message data
     */
    public TPCANMsg(int id, byte type, byte length, byte[] data)
    {
        _id = id;
        _type = type;
        _length = length;
        _data = new byte[length];
        System.arraycopy(data, 0, _data, 0, length);
    }
    /**
     * Constructs a new message object.
     * @param id the message id
     * @param type the message type as an enumeration set
     * @param length the message length
     * @param data the message data
     */
    public TPCANMsg(int id, EnumSet<TPCANMessageType> type, byte length, byte[] data)
    {
        _id = id;
        _type = TPCANMessageType.getValue(type);
        _length = length;
        _data = new byte[length];
        System.arraycopy(data, 0, _data, 0, length);
    }

    /**
     * Sets the id of this message.
     * @param id the message id
     */
    public void setID(int id)
    {
        _id = id;
    }

    /**
     * Sets the data and length of this message.
     * @param data the message data
     * @param length the message length
     */
    public void setData(byte[] data, byte length)
    {
        _length = length;
        System.arraycopy(data, 0, _data, 0, length);
    }

    /**
     * Sets the length of this message.
     * @param length the length of the message
     */
    public void setLength(byte length)
    {
        _length = length;
    }

    /**
     * Sets the type of this message.
     * @param type the message type
     */
    public void setType(byte type)
    {
        _type = type;
    }
    /**
     * Sets the type of this message.
     * @param type the message type
     */
    public void setType(TPCANMessageType type)
    {
        _type = type.getValue();
    }
    /**
     * Sets the type of this message.
     * @param type the message type
     */
    public void setType(EnumSet<TPCANMessageType> type)
    {
        _type = TPCANMessageType.getValue(type);
    }

    /**
     * Gets the id of this message.
     * @return the message id
     */
    public int getID()
    {
        return _id;
    }

    /**
     * Gets the data of this message.
     * @return the message data
     */
    public byte[] getData()
    {
        return _data;
    }

    /**
     * Gets the length of this message.
     * @return the message length
     */
    public byte getLength()
    {
        return _length;
    }

    /**
     * Gets the type of this message.
     * @return the message type
     */
    public byte getType()
    {
        return _type;
    }

    /**
     * Clones this message object.
     * @return The cloned message object.
     */
    @Override
    public Object clone()
    {
        TPCANMsg msg = null;
        try
        {
            msg = (TPCANMsg) super.clone();
            msg._data = _data.clone();
        }
        catch (CloneNotSupportedException e)
        {
            System.out.println(e.getMessage());
        }
        return msg;
    }
}
