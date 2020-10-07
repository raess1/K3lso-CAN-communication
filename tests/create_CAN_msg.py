p_des = float( input('Enter Motor Position [-95.5 - 95.5]: ') )
v_des = float( input('Enter Motor Speed [-30.0 - 30.0]: ') )
t_ff = float( input('Enter Motor Torque [-18.0 - 18.0]: ') )
kp = float( input('Enter Kp [0.0 - 500.0]: ') )
kd = float( input('Enter Kd [0.0 - 5.0]: ') )

def truncate_float(data, _min, _max):
    if data>_max:
        return _max
    if data<_min:
        return _min
    return data

p_des = truncate_float(p_des, -95.5, 95.5)
v_des = truncate_float(v_des, -30.0, 30.0)
t_ff =  truncate_float(t_ff,  -18.0, 18.0)
kp =    truncate_float(kp,      0.0, 500.0)
kd =    truncate_float(kd,      0.0, 5.0)


def float_to_uint(x, x_min, x_max, bits):
    span = x_max - x_min
    offset = x_min
    return int( ( (x-offset) * ( float((1<<bits)-1) )/span) )

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

out_str = ''

for data in package:
    out_str += ("0x%0.2X" % data) + ' '

print('\nCAN Package:')
print(out_str)
print()