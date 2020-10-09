import numpy as np
import matplotlib.pyplot as plt
import time

plt.axis([0, 100, 0, 10])

start_time = time.time()
this_time = time.time() - start_time

x = []
y = []

while(this_time<10):
    this_time = time.time() - start_time
    pos = np.sin(2*this_time)
    x.append(this_time)
    y.append(pos)
    plt.cla()
    plt.plot(x, y)
    plt.pause(0.001)