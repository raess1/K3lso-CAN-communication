package peak.can;

/**
 * The MutableInteger class wraps a value of the primitive type int in an object. An object of type Integer contains a single field whose type is int.
 * This class is used because Java.Lang.Integer objects are defined as Imutable. Yet, we need to maintain object reference when it's passed to the JNI library using.
 * So, we defined a simple Class which extends Java.Lang.Object to resolve the problematic.
 *
 * @version 1.9
 * @LastChange $Date: 2016-05-13 14:54:19 +0200 (ven., 13 mai 2016) $
 * @author Jonathan Urban/Uwe Wilhelm
 *
 * @Copyright (C) 1999-2014  PEAK-System Technik GmbH, Darmstadt
 * more Info at http://www.peak-system.com
 */
public class MutableInteger
{

    /**
     * Constructor
     * @param value int value
     */
    public MutableInteger(int value)
    {
        this.value = value;
    }

    /**
     * Constructor parsing the string argument as a integer
     * @param value integer as string
     */
    public MutableInteger(String value)
    {
        this.value = Integer.parseInt(value);
    }

    /**
     * Gets integer value
     * @return integer value
     */
    public int getValue()
    {
        return value;
    }

    /**
     * Sets integer value
     * @param value Integer value
     */
    public void setValue(int value)
    {
        this.value = value;
    }
    public int value;

    /**
     * Overrides toString() to display int value
     * @return MutableInteger's value as a string
     */
    @Override
    public String toString()
    {
        return Integer.toString(value);
    }      
}
