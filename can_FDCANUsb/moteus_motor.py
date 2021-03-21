import serial
import io
import struct
import enum
import time

MP_INT8 = 0
MP_INT16 = 1
MP_INT32 = 2
MP_F32 = 3

MP_WRITE_BASE = 0x00
MP_READ_BASE = 0x10
MP_REPLY_BASE = 0x20
MP_WRITE_ERROR = 0x30
MP_READ_ERROR = 0x31
MP_NOP = 0x50

_TYPE_STRUCTS = {
    MP_INT8: struct.Struct('<b'),
    MP_INT16: struct.Struct('<h'),
    MP_INT32: struct.Struct('<i'),
    MP_F32: struct.Struct('<f'),
}

MOTEUS_REG_MODE = 0x000
MOTEUS_REG_POSITION = 0x001
MOTEUS_REG_VELOCITY = 0x002
MOTEUS_REG_TORQUE = 0x003
MOTEUS_REG_Q_A = 0x004
MOTEUS_REG_D_A = 0x005
MOTEUS_REG_V = 0x00d
MOTEUS_REG_TEMP_C = 0x00e
MOTEUS_REG_FAULT = 0x00f

MOTEUS_REG_POS_POSITION = 0x20
MOTEUS_REG_POS_VELOCITY = 0x21
MOTEUS_REG_POS_TORQUE = 0x22

class MoteusMode(enum.IntEnum):
    STOPPED = 0
    FAULT = 1
    PWM = 5
    VOLTAGE = 6
    VOLTAGE_FOC = 7
    VOLTAGE_DQ = 8
    CURRENT = 9
    POSITION = 10
    TIMEOUT = 11
    ZERO_VEL = 12

def hexify(data):
    return ''.join(['{:02x}'.format(x) for x in data])


def dehexify(data):
    result = b''
    for i in range(0, len(data), 2):
        result += bytes([int(data[i:i+2], 16)])
    return result

def readline(stream):
    result = bytearray()
    while True:
        char = stream.read(1)
        if char == b'\n':
            if len(result):
                return result
        else:
            result += char

def read_varuint(stream):
    result = 0
    shift = 0

    for i in range(5):
        data = stream.read(1)
        if len(data) < 1:
            return None
        this_byte, = struct.unpack('<B', data)
        result |= (this_byte & 0x7f) << shift
        shift += 7

        if (this_byte & 0x80) == 0:
            return result

    assert False


def read_type(stream, field_type):
    s = _TYPE_STRUCTS[field_type]
    data = stream.read(s.size)
    return s.unpack(data)[0]


def parse_register_reply(data):
    stream = io.BytesIO(data)
    result = {}

    while True:
        opcode = read_varuint(stream)
        if opcode is None:
            break
        opcode_base = opcode & ~0x0f
        if opcode_base == MP_REPLY_BASE:
            field_type = (opcode & 0x0c) >> 2
            size = opcode & 0x03
            if size == 0:
                size = read_varuint(stream)
            start_reg = read_varuint(stream)
            for i in range(size):
                result[start_reg + i] = read_type(stream, field_type)
        elif opcode_base == MP_WRITE_ERROR:
            reg = read_varuint(stream)
            err = read_varuint(stream)
            result[reg] = 'werr {}'.format(err)
        elif opcode_base == MP_READ_ERROR:
            reg = read_varuint(stream)
            err = read_varuint(stream)
            result[reg] = 'rerr {}'.format(err)
        elif opcode_base == MP_NOP:
            pass
        else:
            # Unknown opcode.  Just bail.
            break

    return result

class FDCANUsb:
    def __init__(self, port='/dev/fdcanusb'):
        # self.serial = serial.Serial(port=port)
        pass

    def send_can_frame(self, target, frame, reply):
        data = "can send {:02x}{:02x} {}\n".format( 0x80 if reply else 0x00, target, hexify(frame) ).encode('latin1')
        # self.serial.write(data)
        # print(data)
        # Read (and discard) the fdcanusb response.
        ok_response = readline(self.serial).decode('utf-8')
        if not ok_response.startswith("OK"):
            raise RuntimeError("fdcanusb responded with: " + ok_response)

class MoteusMotor:
    def __init__(self, id, canbus):
        self.id = id
        self.canbus = canbus
        ### Motor Status
        self.angle_deg = 0.0
        self.velocity_dps = 0.0
        ### Init Motor
        self.set_torque(False)
    
    def set_position(self, angle_deg):
        self.angle_deg = angle_deg
        self.velocity_dps = 5.0
        raw_frame = self._construct_position()
        self.canbus.send_can_frame(target=self.id, frame=raw_frame, reply=True)

        # Read the devices response.
        device = readline(self.canbus.serial).decode('utf-8')
        if not device.startswith("rcv"):
            raise RuntimeError("unexpected response")
        fields = device.split(" ")
        response = dehexify(fields[2])
        response_data = parse_register_reply(response)
        # print(response_data)
    
    def set_torque(self, torque):
        if torque:
            self.canbus.send_can_frame(target=self.id, frame=self._construct_position_on(), reply=False)
        else:
            self.canbus.send_can_frame(target=self.id, frame=self._construct_stop(), reply=False)
    
    def _construct_stop(self):
        buf = io.BytesIO()
        buf.write(struct.pack(
            "<bbb",
            0x01,  # write int8 1x
            MOTEUS_REG_MODE,
            MoteusMode.STOPPED))
        return buf.getvalue()
    
    def _construct_position_on(self):
        buf = io.BytesIO()
        buf.write(struct.pack(
            "<bbb",
            0x01,  # write int8 1x
            MOTEUS_REG_MODE,
            MoteusMode.POSITION))
        return buf.getvalue()

    def _construct_position(self):
        buf = io.BytesIO()
        buf.write(struct.pack(
            "<bbb",
            0x01,  # write int8 1x
            MOTEUS_REG_MODE,
            MoteusMode.POSITION))
        buf.write(struct.pack(
            "<bbfff",
            0x0f,  # write float32 3x
            MOTEUS_REG_POS_POSITION,
            self.angle_deg / 360.0,  # position
            0.0,  # velocity
            0.0,  # feedforward torque
            ))
        buf.write(struct.pack(
            "<bbb",
            0x1c,  # read float32 (variable number)
            4,     # 4 registers
            0x00   # starting at 0
            ))
        buf.write(struct.pack(
            "<bb",
            0x13,  # read int8 3x
            MOTEUS_REG_V))
        return buf.getvalue()

canbus = FDCANUsb()
motors = {}
motors[60] = MoteusMotor(id=60, canbus=canbus)
motors[61] = MoteusMotor(id=61, canbus=canbus)
motors[62] = MoteusMotor(id=62, canbus=canbus)

prev_time = time.time()
num_samples = 100

for i in range(num_samples):
    motors[60].set_position(0)
    motors[61].set_position(0)
    motors[62].set_position(0)

loop_time = time.time()-prev_time
iter_time = loop_time/num_samples
freq = 1/iter_time

print(freq)
