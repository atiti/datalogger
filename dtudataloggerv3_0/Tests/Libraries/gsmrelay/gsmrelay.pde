//Serial Relay - Arduino will patch a 
//serial link between the computer and the GPRS Shield
//at 19200 bps 8-N-1
//Computer is connected to Hardware UART
//GPRS Shield is connected to the Software UART 
 
#include <NewSoftSerial.h>
 
NewSoftSerial mySerial(7, 8);

int a = 0, b = 0, i=0;
void setup()
{
  mySerial.begin(9600);               // the GPRS baud rate   
  Serial.begin(9600);                 // the GPRS baud rate   

}
 
void loop()
{
    if(a = Serial.available())
    {
       for(i=0;i<a;i++)
         mySerial.print(Serial.read(), BYTE);
     }  
    else  if(b = mySerial.available())
    {
       for(i=0;i<b;i++)
         Serial.print(mySerial.read(), BYTE);
     }   
} 
