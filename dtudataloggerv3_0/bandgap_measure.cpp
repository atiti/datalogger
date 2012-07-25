#include <Arduino.h>


// Find internal 1.1 reference voltage on AREF pin
void setup ()
{
//  ADMUX = _BV (REFS0) | _BV (REFS1);
    ADMUX = _BV (REFS1);
}
void loop () { }

