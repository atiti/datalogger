import time, sys
import numpy as np
import matplotlib.pyplot as plt
import string, thread, serial

ser = serial.Serial("/dev/ttyUSB0", 57600, timeout=1)

gt = 0
gy = 0 
xdata, ydata = [], []

num_data = 0

def process_serial():
	global xdata, ydata, ds,gt,gy
	global num_data
	while 1:
		l = ser.readline()
		if l:
			l = string.split(l, "\t")
			if len(l) > 1 and l[0][0] == "T":
				ts = l[0][1:]
				v = l[1].split(":")[1]
				gy = int(v)
				gt += 1
				print l

fig = plt.figure()
ax = fig.add_subplot(2,2,1)
bx = fig.add_subplot(2,2,2)
line, = ax.plot([], [], animated=True, lw=2)
line2, = bx.plot([], [], animated=True, lw=2)
ax.set_ylim(-100, 1100)
ax.set_xlim(0, 100)
ax.grid()
bx.set_ylim(-100, 1100)
bx.set_xlim(0, 100)
bx.grid()
def run(*args):
    global gy,gt,xdata,ydata
    background = fig.canvas.copy_from_bbox(ax.bbox)
    # for profiling
    tstart = time.time()

    while 1:
        # restore the clean slate background
        fig.canvas.restore_region(background)
        # update the data
	#if gt >= 100:
	#	xdata = []
	#	ydata = []
	xdata.append(gt)
 	ydata.append(gy)
        xmin, xmax = ax.get_xlim()
        if gt>=xmax:
            	ax.set_xlim(xmin, 2*xmax)
            	fig.canvas.draw()
            	background = fig.canvas.copy_from_bbox(ax.bbox)

        line.set_data(xdata, ydata)

       	# just draw the animated artist
        ax.draw_artist(line)
        # just redraw the axes rectangle
        fig.canvas.blit(ax.bbox)

        if run.cnt==1000:
           	# print the timing info and quit
            	print 'FPS:' , 1000/(time.time()-tstart)
            	#sys.exit()
	    	run.cnt = 0

        run.cnt += 1
run.cnt = 0

thread.start_new_thread(process_serial, ())

manager = plt.get_current_fig_manager()
manager.window.after(100, run)

plt.show()
