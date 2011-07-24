int del=1;
int c = 0;
int wt = 1000;
void setup() {
 pinMode(2, OUTPUT); 
 pinMode(13, OUTPUT);
}

void loop() {

  digitalWrite(2, HIGH);
  digitalWrite(13, HIGH);
  delay(del);
  digitalWrite(2, LOW);
  digitalWrite(13, LOW);
  delay(del);
  if (c < wt) {
    del+=10;
    if (del >= 500) {
      del = 1;
      wt = 1000;
    }
    c++;
    wt--;
  }
  else {
    c = 0;
  }

}
