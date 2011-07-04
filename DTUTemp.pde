#define TEMP_LOC 7
#define TEMP_OFFSET -0.03
#define TEMP_R1  2.7
#define TEMP_VIN 5.0

PROGMEM short temp_const[20] = {990, 1040, 1146, 1260, 1381, 1510, 1646, 
                    1790, 1941, 2100, 2267, 2441, 2623, 2812,
                    3009, 3214, 3426, 3643, 3855};

float getTemp() {
  // Resistance From -60 to 120C

  float Vout = 0, temp = 0;
  short R2 = 0;
  Vout = (TEMP_VIN / 1023.0) * analogvals[TEMP_LOC];
  R2 = (short)(((TEMP_R1 * ((TEMP_VIN / Vout ) - 1)) + TEMP_OFFSET)*1000);
  for(i=1;i<20;i++) {
    if (pgm_read_word_near(temp_const + (i-1)) < R2 && pgm_read_word_near(temp_const + i) >= R2)
      break;
  }
  temp = (i*10) - 70 + (10 - ((float)(pgm_read_word_near(temp_const + i)-R2) / (float)(pgm_read_word_near(temp_const + i)-pgm_read_word_near(temp_const + (i-1))))*10); 
  return temp;
}
