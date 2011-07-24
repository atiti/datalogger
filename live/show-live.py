import serial, time, string, thread
import numpy as np
import matplotlib
matplotlib.use('GTKAgg') # do this before importing pylab

import matplotlib.pyplot as plt

fig = plt.figure()

ax = fig.add_subplot(111)

ser = serial.Serial("/dev/ttyUSB0", 57600, timeout=1)

x = np.arange(0, 100, 1)
y = np.arange(0, 100, 1)
gy = 0
ci = 0
gplot, = ax.plot(x, y);

def process_serial():
	global gy
	global ci
	global y
	while 1:
		l = ser.readline()
		if l:
			l = string.split(l, "\t")
			if len(l) > 1 and l[0][0] == "T":
				ts = l[0][1:]
				v = l[2].split(":")[1]
				gy = int(v)
				y[ci] = gy
				ci += 1
				if (ci > 100):
					ci = 0

def animate():
    global gy
    global ci
    global gplot
    #print dir(gplot)
    #gplot.set_ydata(gy)  # update the data
    ax.draw_artist(gplot)
    fig.canvas.draw()                         # redraw the canvas
    return True

thread.start_new_thread(process_serial, ())
import gobject
print 'adding idle'
gobject.idle_add(animate)
print 'showing'
plt.show()

