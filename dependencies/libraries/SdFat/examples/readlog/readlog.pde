/*
 * Read the logfile created by the eventlog.pde example.
 * Demo of pathnames and current working directory
 */
#include <SdFat.h>

// file system object
SdFat sd;

// define a serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
void setup() {
  char c;
  Serial.begin(9600);

  // Initialize SdFat - print detailed message for failure
  if (!sd.init()) sd.initErrorHalt();

  // set current working directory
  if (!sd.chdir("LOGS/2011/JAN/")) {
    sd.errorHalt("chdir failed. Did you run eventlog.pde?");
  }
  // open file in current working directory
  ifstream file("LOGFILE.TXT");
  
  if (!file.is_open()) sd.errorHalt("open failed");

  // copy the file to Serial
  while ((c = file.get()) >= 0) cout << c;
  
  cout << "Done" << endl;
}
//------------------------------------------------------------------------------
void loop() {}
