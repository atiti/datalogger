// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}

// 1) Sets the date and time on the ds1307
// 2) Starts the clock
// 3) Sets hour mode to 24 hour clock
// Assumes you're passing in valid numbers, 
// Probably need to put in checks for valid numbers.
void setDateDs1307()                
{
  // Use of (byte) type casting and ascii math to achieve result.  
  second = (byte) ((Serial.read() - 48) * 10 + (Serial.read() - 48)); 
  minute = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
  hour  = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
  dayOfWeek = (byte) (Serial.read() - 48);
  dayOfMonth = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
  month = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
  year= (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
  Wire.beginTransmission(clockAddress);
  Wire.send(0x00);
  Wire.send(decToBcd(second));  // 0 to bit 7 starts the clock
  Wire.send(decToBcd(minute));
  Wire.send(decToBcd(hour));    // If you want 12 hour am/pm you need to set
  // bit 6 (also need to change readDateDs1307)
  Wire.send(decToBcd(dayOfWeek));
  Wire.send(decToBcd(dayOfMonth));
  Wire.send(decToBcd(month));
  Wire.send(decToBcd(year));
  Wire.endTransmission();
}
// Gets the date and time from the ds1307 and prints result
void getDateDs1307() {
  // Reset the register pointer
  Wire.beginTransmission(clockAddress);
  Wire.send(0x00);
  Wire.endTransmission();

  Wire.requestFrom(clockAddress, 7);

  // A few of these need masks because certain bits are control bits
  second     = bcdToDec(Wire.receive() & 0x7f);
  minute     = bcdToDec(Wire.receive());
  
  // Need to change this if 12 hour am/pm
  hour       = bcdToDec(Wire.receive() & 0x3f);  
  dayOfWeek  = bcdToDec(Wire.receive());
  dayOfMonth = bcdToDec(Wire.receive());
  month      = bcdToDec(Wire.receive());
  year       = bcdToDec(Wire.receive());

  Serial.print(hour, DEC);
  Serial.print(":");
  Serial.print(minute, DEC);
  Serial.print(":");
  Serial.print(second, DEC);
  Serial.print("  ");
  Serial.print(month, DEC);
  Serial.print("/");
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(year, DEC);

}
