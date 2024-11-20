#define TIMER_2_EN
#define TIMER_1_EN
#include"TIMER_INTERRUPT.h"
void setup() {
pinMode(13,OUTPUT);
  cli();
  TIMER_1.CLEAR();
  TIMER_1.WGM(2);
  TIMER_2.CLEAR();
  TIMER_2.WGM(2);
  TIMER_2.OCR_A(255);
  TIMER_2.CS(5);
  TIMER_2.OCIE_A(true);
  TIMER_2.COMP_A(Blink);
  sei();
//  Serial.begin(115200);
//  Serial.println();
//  for (uint8_t X = 0; X < 8; X++) Serial.print(bitRead(TCNT2, 7 - X));
//  Serial.println();
//  for (uint8_t X = 0; X < 8; X++) Serial.print(bitRead(OCR2A, 7 - X));
//  Serial.println();
//  for (uint8_t X = 0; X < 8; X++) Serial.print(bitRead(OCR2B, 7 - X));
//  Serial.println();
//  for (uint8_t X = 0; X < 8; X++) Serial.print(bitRead(TCCR2A, 7 - X));
//  Serial.println();
//  for (uint8_t X = 0; X < 8; X++) Serial.print(bitRead(TCCR2B, 7 - X));
//  Serial.println();
//  for (uint8_t X = 0; X < 8; X++) Serial.print(bitRead(TIMSK2, 7 - X));
//  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:

}

void Blink() {
  digitalWrite(13, !digitalRead(13));
}
