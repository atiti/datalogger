#define AN_S0 2
#define AN_S1 3
#define AN_S2 5
#define AN_S3 4
#define AN_EN 6
#define AN_INP 14
#define AN_PULLUP LOW

unsigned char AOD[16] = {0, 0, 0, 0,
                         0, 0, 0, 1,
                         0, 0, 0, 0,
                         0, 0, 0, 0};

void Analog_init() {
  pinMode(AN_S0, OUTPUT);
  pinMode(AN_S1, OUTPUT);
  pinMode(AN_S2, OUTPUT);
  pinMode(AN_S3, OUTPUT);
  pinMode(AN_EN, OUTPUT);
  pinMode(AN_INP, INPUT);
  digitalWrite(AN_INP, AN_PULLUP);
}

void Analog_enable() {
  digitalWrite(AN_EN, LOW); 
}

void Analog_disable() {
  digitalWrite(AN_EN, HIGH);
}

void Analog_setPin(int pin, int doa) {
  AOD[pin] = doa;
}

int Analog_read(int pin) {
  int ret = 0;
  if ((pin & 0x1) > 0)
    digitalWrite(AN_S0, HIGH);
  else
    digitalWrite(AN_S0, LOW);
  if ((pin & 0x2) > 0)
    digitalWrite(AN_S1, HIGH);
  else
    digitalWrite(AN_S1, LOW);
  if ((pin & 0x4) > 0)
    digitalWrite(AN_S2, HIGH);
  else
    digitalWrite(AN_S2, LOW);
  if ((pin & 0x8) > 0)
    digitalWrite(AN_S3, HIGH);
  else
    digitalWrite(AN_S3, LOW);
  if (AOD[pin]) 
    ret = analogRead(AN_INP);
  else {
    ret = digitalRead(AN_INP);
  }
}

void Analog_readAll() {
 int i = 0;
#ifdef DEBUG
 float starttime = 0;
 if (debug_on) starttime = millis();
#endif
 for(i=0;i<16;i++) {
  analogvals[i] = Analog_read(i);
 }
#ifdef DEBUG
 if (debug_on) {
    Serial.print("Latency: ");
    Serial.print(millis()-starttime);
    Serial.println(" ms"); 
 }
#endif
}
