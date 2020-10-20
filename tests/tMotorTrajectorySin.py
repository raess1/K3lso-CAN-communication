import numpy as np
import time

from PCANBasic import *

m_objPCANBasic = PCANBasic()
m_PcanHandle = PCAN_PCIBUS4

baudrate = PCAN_BAUD_1M
hwtype = PCAN_TYPE_ISA
ioport = 0x100
interrupt = 3

T_SPEED = 0.0
T_TORQUE =0.0
T_KP = 10.0
T_KD = 1.0

MOTOR_ID = 1

def getFormatedError(error):
        stsReturn = m_objPCANBasic.GetErrorText(error, 0)
        if stsReturn[0] != PCAN_ERROR_OK:
            return "An error occurred. Error-code's text ({0:X}h) couldn't be retrieved".format(error)
        else:
            return stsReturn[1]

def printCANMsg(msg):
    print('---CAN Message---')
    print(f'ID: {msg.ID}')
    print(f'LEN: {msg.LEN}')
    data_str = '['
    for i in range(msg.LEN):
        data_str += hex(msg.DATA[i])
        if i<(msg.LEN-1):
            data_str += ', '
    data_str += ']'
    print(data_str)
    print('-----------------')

def createCANMsg(id, data):
    CANMsg = TPCANMsg()
    CANMsg.ID = int(id)
    CANMsg.LEN = int(len(data))
    CANMsg.MSGTYPE =  PCAN_MESSAGE_STANDARD
    for i in range(CANMsg.LEN):
        CANMsg.DATA[i] = int(data[i])
    return CANMsg

def enableMotorMode(state):
    if state:
        msg = createCANMsg(MOTOR_ID, [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC])
    else:
        msg = createCANMsg(MOTOR_ID, [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD])
    # printCANMsg(msg)
    m_objPCANBasic.Write(m_PcanHandle, msg)

def sendCANMsg(pack):
    msg = createCANMsg(MOTOR_ID, pack)
    # printCANMsg(msg)
    m_objPCANBasic.Write(m_PcanHandle, msg)


def openCANPort():
    result = m_objPCANBasic.Initialize(m_PcanHandle,baudrate,hwtype,ioport,interrupt)
    if result!=PCAN_ERROR_OK:
        if result != PCAN_ERROR_CAUTION:
            print(f'Error!: {self.getFormatedError(result)}')
        else:
            print('The bitrate being used is different than the given one')
    return result

def closeCANPort():
    m_objPCANBasic.Uninitialize(m_PcanHandle)

def truncate_float(data, _min, _max):
    if data>_max:
        return _max
    if data<_min:
        return _min
    return data

def float_to_uint(x, x_min, x_max, bits):
    span = x_max - x_min
    offset = x_min
    return int( ( (x-offset) * ( float((1<<bits)-1) )/span) )

def makeTMotorPackage(p_des, v_des, t_ff, kp, kd):
    p_des = truncate_float(p_des, -95.5, 95.5)
    v_des = truncate_float(v_des, -30.0, 30.0)
    t_ff =  truncate_float(t_ff,  -18.0, 18.0)
    kp =    truncate_float(kp,      0.0, 500.0)
    kd =    truncate_float(kd,      0.0, 5.0)

    p_int = float_to_uint(p_des, -95.5, 95.5, 16)
    v_int = float_to_uint(v_des, -30.0, 30.0, 12)
    kp_int = float_to_uint(kp,  0.0, 500.0, 12)
    kd_int = float_to_uint(kd,  0.0, 5.0, 12)
    t_int = float_to_uint(t_ff, -18.0, 18.0, 12)

    package = bytearray(8*[0])

    package[0] = (p_int>>8)&0xFF
    package[1] = p_int&0xFF
    package[2] = (v_int>>4)&0xFF
    package[3] = ((v_int&0x0F)<<4) | ((kp_int>>8)&0x0F)
    package[4] = kp_int&0xFF
    package[5] = (kd_int>>4)&0xFF
    package[6] = ((kd_int&0xF)<<4) | ((t_int>>8)&0x0F)
    package[7] = t_int&0xFF

    return package

print('Opening CAN port...')
if openCANPort()!=PCAN_ERROR_OK:
    exit()
print('Port Opened')    

time.sleep(1)

print('Turning ON motor mode') 
enableMotorMode(True)
time.sleep(1)

start_time = time.time()
this_time = time.time() - start_time
i_time = 0

while(this_time<10):
    this_time = time.time() - start_time
    pos = np.sin(2*this_time)
    sendCANMsg( makeTMotorPackage(pos, T_SPEED, T_TORQUE, T_KP, T_KD) )
    i_time += 1

this_time = time.time() - start_time
freq = 1/(this_time/i_time)
print(f'Loop Freq = {freq}Hz')

print('Turning OFF motor mode') 
enableMotorMode(False)
time.sleep(1)

print('Closing CAN port...')
closeCANPort()
print('Port Closed')    
