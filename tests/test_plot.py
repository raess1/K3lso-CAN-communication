# import numpy as np
# import matplotlib.pyplot as plt
# import time

# plt.axis([0, 100, 0, 10])

# start_time = time.time()
# this_time = time.time() - start_time

# x = []
# y = []

# while(this_time<10):
#     this_time = time.time() - start_time
#     pos = np.sin(2*this_time)
#     x.append(this_time)
#     y.append(pos)
#     plt.cla()
#     plt.plot(x, y)
#     plt.pause(0.001)

import numpy as np
import matplotlib.pyplot as plt
import time

plt.axis([0, 100, 0, 10])

start_time = time.time()
this_time = time.time() - start_time

x = []
y = []

period = 4.0
max_angle = 3.1416

while(this_time<10):
    this_time = time.time() - start_time
    t = this_time/period - np.floor(this_time/period)
    if t<0.25:
        pos = max_angle*t*4.0
    elif t<0.5:
        pos = max_angle
    elif t<0.75:
        pos = -max_angle*t*4.0 + 3*max_angle
    else:
        pos = 0.0
    x.append(this_time)
    y.append(pos)
    plt.cla()
    plt.plot(x, y)
    plt.pause(0.001)