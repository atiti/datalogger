/*
 * Open all files in the root dir and print their filename
 */
#include <SdFat.h>

SdFat sd;

SdFile file;

// define a serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
void setup() {
 char name[13];
 
 Serial.begin(9600);

 if (!sd.init()) sd.initErrorHalt();

 // open next file in root.  current working directory, cwd, is root
 while (file.openNext(sd.cwd(), O_READ)) {
   file.getFilename(name);
   cout << name << endl;
   file.close();
 }
 cout << "Done" << endl;
}
//------------------------------------------------------------------------------
void loop() {}
