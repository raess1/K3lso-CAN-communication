package peak.can;

import java.util.EnumSet;
import java.util.Vector;
import java.util.Random;
import peak.can.basic.TPCANMsg;
import peak.can.basic.PCANBasic;
import peak.can.basic.TPCANHandle;
import peak.can.basic.TPCANMessageType;
import peak.can.basic.TPCANMsgFD;
import peak.can.basic.TPCANStatus;

/**
 * The CANSendThread class extends Thread class and is used to send CAN Messages.
 *
 * @version 1.10
 * @LastChange $Date: 2016-05-13 14:54:19 +0200 (ven., 13 mai 2016) $
 * @author Jonathan Urban/Uwe Wilhelm/Fabrice Vergnaud
 *
 * @Copyright (C) 1999-2014  PEAK-System Technik GmbH, Darmstadt
 * more Info at http://www.peak-system.com
 */
public class CANSendThread extends Thread 
{
    // PCANBasic instance used to call read functions
    private PCANBasic pcanBasic;
    // Collection which stores all connected channels
    private Vector<ChannelItem> connectedChannelCollection = null;
    // Used to send CAN Messages with  Interval
    private int SendInterval;
    
    //Local CAN variables
    private TPCANMsg canMessage = null;
    private TPCANMsgFD canMessageFd = null;
    private TPCANStatus ret;
    // Random Generator
    Random randomGenerator;

    public int getSendInterval()
    {
        return SendInterval;
    }

    public void setSendInterval(int interval)
    {
        SendInterval = interval;
    }

    /**
     *
     * @param pcanbasic PCANBasic instance used to call read functions
     * @param connectedChannelCollection Reference to the collection which stores all connected channels
     */
    public CANSendThread(PCANBasic pcanbasic, Vector<ChannelItem> connectedChannelCollection)
    {
        this.pcanBasic = pcanbasic;
        this.connectedChannelCollection = connectedChannelCollection;
        
         // Create new CAN Message
        this.canMessage = new TPCANMsg();          
        this.canMessage.setID((int)1024); // HEX 400
        this.canMessage.setLength((byte)8); // 8 Byte
        this.canMessage.setType(TPCANMessageType.PCAN_MESSAGE_STANDARD); // Std. CAN Frame       
        this.randomGenerator = new Random();
         // Create new CAN FD Message
        this.canMessageFd = new TPCANMsgFD();          
        this.canMessageFd.setID((int)1024); // HEX 400
        this.canMessageFd.setDlc((byte)14); // 42 Byte
        this.canMessageFd.setType(EnumSet.of(TPCANMessageType.PCAN_MESSAGE_STANDARD, TPCANMessageType.PCAN_MESSAGE_FD));

    }  

    protected void finalize() throws Throwable
    {
      // Free local variables
       canMessage = null;
       randomGenerator = null;
     }    
    
    /**
     * Starts thread process
     */
    public void run()
    {
        while (true)
        {
        	synchronized (connectedChannelCollection) {
	            // Process each connected channel
	            for (ChannelItem item : connectedChannelCollection)
	            {
	                if ((item != MarkAllChannelItem.getInstance()) && (item.getWorking()))
	                    // Call the PCANBasic Send Function
	                    callAPIFunctionSend(item.getHandle(), item.isCanFd());
	            }
        	}

            // Sleep Time
            try
            {
                Thread.sleep(SendInterval);
            }
            catch (InterruptedException e)
            {
                return;
            }
        }
    }

    /**
     * Calls the PCANBasic Send Function 
     *
     * @param handle The handle of a PCAN Channel
     * @param isCanFd Channel is initialized in CAN FD mode
     */
    public void callAPIFunctionSend(TPCANHandle handle, boolean isCanFd)
    {
        byte Data;
        try
        {
            int length;
            Data = (byte)randomGenerator.nextInt(249);
            if (!isCanFd) {
                length = canMessage.getLength();
                Data = (byte)randomGenerator.nextInt(249);
                for (int i = 0; i < length; i++)
                    this.canMessage.getData()[i]= (byte)(Data + i);   

                // We execute the "Write" function of the PCANBasic
                ret = pcanBasic.Write(handle, canMessage);
            } else {
                length = canMessageFd.getLengthFromDLC();
                for (int i = 0; i < length; i++)
                    this.canMessageFd.getData()[i]= (byte)(Data + i);   

                // We execute the "Write" function of the PCANBasic
                ret = pcanBasic.WriteFD(handle, canMessageFd);                
            }
            //Process result
            if (ret == TPCANStatus.PCAN_ERROR_OK)
            {
              //Critical Area
              synchronized (Application.token)
              {
                //Put Message In the dataRowCollection
              }
            }

        }
        catch (Exception e)
        {
            System.out.println("CANSendThread Exception:" + e.getMessage());
            e.printStackTrace();
            System.exit(0);
        }
    }
}
