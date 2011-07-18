import serial, time, string

ser = serial.Serial("/dev/ttyUSB0", 9600, timeout=1)


def read_n_print(ser):
	line = ser.readline()
	if line:
		print line.strip()
		return line

def wait_until(ser, v):
	ok = 0
	l = ""
	while ok == 0:
		l = read_n_print(ser)
		if l and l.find(v) == 0:
			ok = 1
	return l

while 1:
	ser.write("ATE1\r\n");
	time.sleep(0.3)
	read_n_print(ser)
	ser.write("AT+CIPSTART=\"TCP\",\"46.4.106.217\",\"80\"\r\n");
	wait_until(ser, "CONNECT")
	ser.write("AT+CIPSEND\r\n")
	wait_until(ser, ">")
	ser.write("GET / HTTP/1.0\r\n\r\n");
	ser.write("\x1a")
	wait_until(ser, "SEND OK")
	time.sleep(0.3)	
	wait_until(ser, "CLOSED")
	#ser.write("AT+CIPCLOSE\r\n")
	time.sleep(0.1)
ser.close()
