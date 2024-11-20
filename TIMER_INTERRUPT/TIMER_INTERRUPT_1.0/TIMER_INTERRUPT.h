//TIMER_INTERRUPT REV 1.0.0
//.h
#ifndef TIMER_INTERRUPT_h
#define TIMER_INTERRUPT_h

#include "Arduino.h"

/*KEYWORDS FOR DATASHEET
 * 
 * https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.
 *
 * CURRENT USES:
 * | TIMER 0:
 * | |delay()
 * | |millis()
 * | |micros()
 * | |PWM OUTPUTS 5,6
 * |
 * | TIMER 1:
 * | |SERVO LIB
 * | |PWM OUTPUTS 9,10
 * |  
 * | TIMER 2:
 * | |TONE LIB
 * | |PWM OUTPUT 3,11
 *
 * REGISTERS:
 * | TIMER 0:
 * | |TCNT0   TCNT0
 * | |OCR0A   OCR0A
 * | |OCR0B   OCR0B
 * | |TCCR0A  COM0A#, COM0B#, WGM0#
 * | |TCCR0B  FOC0A, FOC0B, WGM0#, CS0#
 * | |TIMSK0  OCIE0B, OCIE0A, TOIE0
 * | |TIFR0   OCF0B, OCF0A, TOV0
 * |
 * | TIMER 1:
 * | |TCNT1L  TCNT1L
 * | |TCNT1H  TCNT1H
 * | |OCR1AL  OCR1AL
 * | |OCR1AH  OCR1AH
 * | |OCR1BL  OCR1BL
 * | |OCR1BH  OCR1BH
 * | |TCCR1A  COM1A#, COM1B#, WGM1#
 * | |TCCR1B  ICNC1, ICES1, WGM1#, CS1#
 * | |TCCR1C  FOC1A, FOC1B
 * | |TIMSK1  ICIE1, OCIE1B, OCIE1A, TOIE1
 * | |TIFR1   ICF, OCF1B, OCF1A, TOV1
 * |
 * | TIMER 2:
 * | |TCNT2   TCNT2
 * | |OCR2A   OCR2A
 * | |OCR2B   OCR2B
 * | |TCCR2A  COM2A#, COM2B#, WGM2#
 * | |TCCR2B  FOC2A, FOC2B, WGM2#, CS2#
 * | |TIMSK2  OCIE2B, OCIE2A, TOIE2
 * | |TIFR2   OCF2B, OCF2A, TOV2
 */

class TIMER0_class {
  public:

    void CLEAR();
    //Set Methods
    void CNT(uint8_t);
    void OCR_A(uint8_t);
    void OCR_B(uint8_t);
    void WGM(uint8_t);
    void FOC_A(bool);
    void FOC_B(bool);
    void COM_A(uint8_t);
    void COM_B(uint8_t);
    void CS(uint8_t);
    void OCIE_A(bool);
    void OCIE_B(bool);
    void TOIE(bool);

    void TOV(bool);
    void OCF_A(bool);
    void OCF_B(bool);


    //Return Methods:
    uint8_t CNT();
    uint8_t OCR_A();
    uint8_t OCR_B();
    uint8_t WGM();
    bool FOC_A();
    bool FOC_B();
    uint8_t COM_A();
    uint8_t COM_B();
    uint8_t CS();
    bool OCIE_A();
    bool OCIE_B();
    bool TOIE();
    
    bool TOV();
    bool OCF_A();
    bool OCF_B();

    //Set Functions:
    void COMP_A(void(*FUNCT_POINTER)()) {
      _TIMER_COMP_A = FUNCT_POINTER;
    };
    void COMP_B(void(*FUNCT_POINTER)()) {
      _TIMER_COMP_B = FUNCT_POINTER;
    };
    void OVF(void(*FUNCT_POINTER)()) {
      _TIMER_OVF = FUNCT_POINTER;
    };

    void(*_TIMER_COMP_A)();
    void(*_TIMER_COMP_B)();
    void(*_TIMER_OVF)();

  private:

  
};


class TIMER1_class {
  public:

    void CLEAR();
    //Set Methods
    void CNT(uint16_t);
    void OCR_A(uint16_t);
    void OCR_B(uint16_t);
    void ICR(uint16_t);
    void WGM(uint8_t);
    void FOC_A(bool);
    void FOC_B(bool);
    void COM_A(uint8_t);
    void COM_B(uint8_t);
    void ICNC(bool);
    void ICES(bool);
    void CS(uint8_t);
    void OCIE_A(bool);
    void OCIE_B(bool);
    void TOIE(bool);
    void ICIE(bool);

    void TOV(bool);
    void OCF_A(bool);
    void OCF_B(bool);
    void ICF(bool);


    //Return Methods:
    uint16_t CNT();
    uint16_t OCR_A();
    uint16_t OCR_B();
    uint16_t ICR();
    uint8_t WGM();
    bool FOC_A();
    bool FOC_B();
    uint8_t COM_A();
    uint8_t COM_B();
    bool ICNC();
    bool ICES();
    uint8_t CS();
    bool OCIE_A();
    bool OCIE_B();
    bool TOIE();
    bool ICIE();

    bool TOV();
    bool OCF_A();
    bool OCF_B();
    bool ICF();

    //Set Functions:
    void COMP_A(void(*FUNCT_POINTER)()) {
      _TIMER_COMP_A = FUNCT_POINTER;
    };
    void COMP_B(void(*FUNCT_POINTER)()) {
      _TIMER_COMP_B = FUNCT_POINTER;
    };
    void OVF(void(*FUNCT_POINTER)()) {
      _TIMER_OVF = FUNCT_POINTER;
    };
    void CAPT(void(*FUNCT_POINTER)()) {
      _TIMER_CAPT = FUNCT_POINTER;
    };

    void(*_TIMER_COMP_A)();
    void(*_TIMER_COMP_B)();
    void(*_TIMER_OVF)();
    void(*_TIMER_CAPT)();

  private:
};


class TIMER2_class {
  public:

    void CLEAR();
    //Set Methods
    void CNT(uint8_t);
    void OCR_A(uint8_t);
    void OCR_B(uint8_t);
    void WGM(uint8_t);
    void FOC_A(bool);
    void FOC_B(bool);
    void COM_A(uint8_t);
    void COM_B(uint8_t);
    void CS(uint8_t);
    void OCIE_A(bool);
    void OCIE_B(bool);
    void TOIE(bool);

    void TOV(bool);
    void OCF_A(bool);
    void OCF_B(bool);


    //Return Methods:
    uint8_t CNT();
    uint8_t OCR_A();
    uint8_t OCR_B();
    uint8_t WGM();
    bool FOC_A();
    bool FOC_B();
    uint8_t COM_A();
    uint8_t COM_B();
    uint8_t CS();
    bool OCIE_A();
    bool OCIE_B();
    bool TOIE();

    bool TOV();
    bool OCF_A();
    bool OCF_B();

    //Set Functions:
    void COMP_A(void(*FUNCT_POINTER)()) {
      _TIMER_COMP_A = FUNCT_POINTER;
    };
    void COMP_B(void(*FUNCT_POINTER)()) {
      _TIMER_COMP_B = FUNCT_POINTER;
    };
    void OVF(void(*FUNCT_POINTER)()) {
      _TIMER_OVF = FUNCT_POINTER;
    };

    void(*_TIMER_COMP_A)();
    void(*_TIMER_COMP_B)();
    void(*_TIMER_OVF)();

  private:
};

//.cpp
//#include "TIMER_INTERRUPT.h"
//#include "Arduino.h"

void TIMER0_class::CLEAR() {
  TCNT0 = 0;

  OCR0A = 0;
  OCR0B = 0;

  TCCR0A = 0;
  TCCR0B = 0;

  TIMSK0 = 0;

  TIFR0 = 0;
}

void TIMER0_class::CNT(uint8_t VAL) {
  TCNT0 = VAL;
}
void TIMER0_class::OCR_A(uint8_t VAL) {
  OCR0A = VAL;
}
void TIMER0_class::OCR_B(uint8_t VAL) {
  OCR0B = VAL;
}
void TIMER0_class::WGM(uint8_t VAL) {
  TCCR0A &= B11111100;
  TCCR0B &= B11110111;
  TCCR0A |= (VAL & B00000011);
  TCCR0B |= (VAL & B00000100) << 1;
}
void TIMER0_class::FOC_A(bool VAL) {
  TCCR0B &= B01111111;
  TCCR0B |= VAL << 7;
}
void TIMER0_class::FOC_B(bool VAL) {
  TCCR0B &= B10111111;
  TCCR0B |= VAL << 6;
}
void TIMER0_class::COM_A(uint8_t VAL) {
  TCCR0A &= B00111111;
  TCCR0A |= (VAL & B00000011) << 6;
}
void TIMER0_class::COM_B(uint8_t VAL) {
  TCCR0A &= B11001111;
  TCCR0A |= (VAL & B00000011) << 4;
}
void TIMER0_class::CS(uint8_t VAL) {
  TCCR0B &= B11111000;
  TCCR0B |= (VAL & B00000001);
  TCCR0B |= (VAL & B00000010);
  TCCR0B |= (VAL & B00000100);
}
void TIMER0_class::OCIE_A(bool VAL) {
  TIMSK0 &= B11111101;
  TIMSK0 |= VAL << 1;
}
void TIMER0_class::OCIE_B(bool VAL) {
  TIMSK0 &= B11111011;
  TIMSK0 |= VAL << 2;
}
void TIMER0_class::TOIE(bool VAL) {
  TIMSK0 &= B11111110;
  TIMSK0 |= VAL << 0;
}

void TIMER0_class::TOV(bool VAL) {
  TIFR0 &= B11111110;
  TIFR0 |= VAL << 0;
}
void TIMER0_class::OCF_A(bool VAL) {
  TIFR0 &= B11111101;
  TIFR0 |= VAL << 1;
}
void TIMER0_class::OCF_B(bool VAL) {
  TIFR0 &= B11111011;
  TIFR0 |= VAL << 2;
}

uint8_t TIMER0_class::CNT() {
  return TCNT0;
}
uint8_t TIMER0_class::OCR_A() {
  return OCR0A;
}
uint8_t TIMER0_class::OCR_B() {
  return OCR0B;
}
uint8_t TIMER0_class::WGM() {
  return (TCCR0A & B00000011) | ((TCCR0B & B00001000) >> 1);
}
bool TIMER0_class::FOC_A() {
  return (TCCR0B & B10000000);
}
bool TIMER0_class::FOC_B() {
  return (TCCR0B & B01000000);
}
uint8_t TIMER0_class::COM_A() {
  return (TCCR0A & B11000000) >> 6;
}
uint8_t TIMER0_class::COM_B() {
  return (TCCR0A & B00110000) >> 4;
}
uint8_t TIMER0_class::CS() {
  return (TCCR0B & B00000111);
}
bool TIMER0_class::OCIE_A() {
  return (TIMSK0 & B00000010);
}
bool TIMER0_class::OCIE_B() {
  return (TIMSK0 & B00000100);
}
bool TIMER0_class::TOIE() {
  return (TIMSK0 & B00000001);
}

bool TIMER0_class::TOV() {
return (TIFR0 & B00000001);
}
bool TIMER0_class::OCF_A() {
return (TIFR0 & B00000010);
}
bool TIMER0_class::OCF_B() {
return (TIFR0 & B00000100);
}



void TIMER1_class::CLEAR() {
  TCNT1L = 0;
  TCNT1H = 0;

  ICR1L = 0;
  ICR1H = 0;

  OCR1AL = 0;
  OCR1AH = 0;
  OCR1BL = 0;
  OCR1BH = 0;

  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;

  TIMSK1 = 0;

  TIFR1 = 0;
}


void TIMER1_class::CNT(uint16_t VAL) {
  TCNT1L = VAL;
  TCNT1H = VAL >> 8;
}
void TIMER1_class::OCR_A(uint16_t VAL) {
  OCR1AL = VAL;
  OCR1AH = VAL >> 8;
}
void TIMER1_class::OCR_B(uint16_t VAL) {
  OCR1BL = VAL;
  OCR1BH = VAL >> 8;
}
void TIMER1_class::ICR(uint16_t VAL) {
  ICR1L = VAL;
  ICR1H = VAL >> 8;
}
void TIMER1_class::WGM(uint8_t VAL) {
  TCCR1A &= B11111100;
  TCCR1B &= B11100111;
  TCCR1A |= (VAL & B00000011);
  TCCR1B |= (VAL & B00001100) << 1;
}
void TIMER1_class::FOC_A(bool VAL) {
  TCCR1C &= B01111111;
  TCCR1C |= VAL << 7;
}
void TIMER1_class::FOC_B(bool VAL) {
  TCCR1C &= B10111111;
  TCCR1C |= VAL << 6;
}
void TIMER1_class::COM_A(uint8_t VAL) {
  TCCR1A &= B00111111;
  TCCR1A |= ((VAL & B00000011) << 6);
}
void TIMER1_class::COM_B(uint8_t VAL) {
  TCCR1A &= B11001111;
  TCCR1A |= ((VAL & B00000011) << 4);
}
void TIMER1_class::ICNC(bool VAL) {
  TCCR1B &= B01111111;
  TCCR1B |= VAL << 7;
}
void TIMER1_class::ICES(bool VAL) {
  TCCR1B &= B10111111;
  TCCR1B |= VAL << 6;
}
void TIMER1_class::CS(uint8_t VAL) {
  TCCR1B &= B11111000;
  TCCR1B |= ((VAL & B00000111) << 0);
}
void TIMER1_class::OCIE_A(bool VAL) {
  TIMSK1 &= B11111101;
  TIMSK1 |= VAL << 1;
}
void TIMER1_class::OCIE_B(bool VAL) {
  TIMSK1 &= B11111011;
  TIMSK1 |= VAL << 2;
}
void TIMER1_class::TOIE(bool VAL) {
  TIMSK1 &= B11111110;
  TIMSK1 |= VAL << 0;
}
void TIMER1_class::ICIE(bool VAL) {
  TIMSK1 &= B11011111;
  TIMSK1 |= VAL << 5;
}

void TIMER1_class::TOV(bool VAL) {
  TIFR1 &= B11111110;
  TIFR1 |= VAL << 0;
}
void TIMER1_class::OCF_A(bool VAL) {
  TIFR1 &= B11111101;
  TIFR1 |= VAL << 1;
}
void TIMER1_class::OCF_B(bool VAL) {
  TIFR1 &= B11111011;
  TIFR1 |= VAL << 2;
}
void TIMER1_class::ICF(bool VAL) {
  TIFR1 &= B11011111;
  TIFR1 |= VAL << 5;
}


uint16_t TIMER1_class::CNT() {
  return TCNT1L | (TCNT1H << 8);
}
uint16_t TIMER1_class::OCR_A() {
  return OCR1AL | (OCR1AH << 8);
}
uint16_t TIMER1_class::OCR_B() {
  return OCR1BL | (OCR1BH << 8);
}
uint16_t TIMER1_class::ICR() {
  return ICR1L | (ICR1L << 8);
}
uint8_t TIMER1_class::WGM() {
  return (TCCR1A & B00000011) | ((TCCR1B & B00011000) >> 1);
}
bool TIMER1_class::FOC_A() {
  return TCCR1C & B10000000;
}
bool TIMER1_class::FOC_B() {
return TCCR1C & B01000000;
}
uint8_t TIMER1_class::COM_A() {
return (TCCR1A & B11000000) >> 6;
}
uint8_t TIMER1_class::COM_B() {
return (TCCR1A & B00110000) >> 4;
}
bool TIMER1_class::ICNC() {
return (TCCR1B & B10000000);
}
bool TIMER1_class::ICES() {
return (TCCR1B & B01000000);
}
uint8_t TIMER1_class::CS() {
return (TCCR1B & B00000111);
}
bool TIMER1_class::OCIE_A() {
return (TIMSK1 & B00000010);
}
bool TIMER1_class::OCIE_B() {
return (TIMSK1 & B00000100);
}
bool TIMER1_class::TOIE() {
return (TIMSK1 & B00000001);
}
bool TIMER1_class::ICIE() {
return (TIMSK1 & B00100000);
}

bool TIMER1_class::TOV() {
return (TIFR1 & B00000001);
}
bool TIMER1_class::OCF_A() {
return (TIFR1 & B00000010);
}
bool TIMER1_class::OCF_B() {
return (TIFR1 & B00000100);
}
bool TIMER1_class::ICF() {
return (TIFR1 & B00100000);
}



void TIMER2_class::CLEAR() {
  TCNT2 = 0;

  OCR2A = 0;
  OCR2B = 0;

  TCCR2A = 0;
  TCCR2B = 0;

  TIMSK2 = 0;

  TIFR2 = 0;
}

void TIMER2_class::CNT(uint8_t VAL) {
  TCNT2 = VAL;
}
void TIMER2_class::OCR_A(uint8_t VAL) {
  OCR2A = VAL;
}
void TIMER2_class::OCR_B(uint8_t VAL) {
  OCR2B = VAL;
}
void TIMER2_class::WGM(uint8_t VAL) {
  TCCR2A &= B11111100;
  TCCR2B &= B11110111;
  TCCR2A |= (VAL & B00000011);
  TCCR2B |= (VAL & B00000100) << 1;
}
void TIMER2_class::FOC_A(bool VAL) {
  TCCR2B &= B01111111;
  TCCR2B |= VAL << 7;
}
void TIMER2_class::FOC_B(bool VAL) {
  TCCR2B &= B10111111;
  TCCR2B |= VAL << 6;
}
void TIMER2_class::COM_A(uint8_t VAL) {
  TCCR2A &= B00111111;
  TCCR2A |= (VAL & B00000011) << 6;
}
void TIMER2_class::COM_B(uint8_t VAL) {
  TCCR2A &= B11001111;
  TCCR2A |= (VAL & B00000011) << 4;
}
void TIMER2_class::CS(uint8_t VAL) {
  TCCR2B &= B11111000;
  TCCR2B |= (VAL & B00000001);
  TCCR2B |= (VAL & B00000010);
  TCCR2B |= (VAL & B00000100);
}
void TIMER2_class::OCIE_A(bool VAL) {
  TIMSK2 &= B11111101;
  TIMSK2 |= VAL << 1;
}
void TIMER2_class::OCIE_B(bool VAL) {
  TIMSK2 &= B11111011;
  TIMSK2 |= VAL << 2;
}
void TIMER2_class::TOIE(bool VAL) {
  TIMSK2 &= B11111110;
  TIMSK2 |= VAL << 0;
}

void TIMER2_class::TOV(bool VAL) {
  TIFR2 &= B11111110;
  TIFR2 |= VAL << 0;
}
void TIMER2_class::OCF_A(bool VAL) {
  TIFR2 &= B11111101;
  TIFR2 |= VAL << 1;
}
void TIMER2_class::OCF_B(bool VAL) {
  TIFR2 &= B11111011;
  TIFR2 |= VAL << 2;
}

uint8_t TIMER2_class::CNT() {
  return TCNT2;
}
uint8_t TIMER2_class::OCR_A() {
  return OCR2A;
}
uint8_t TIMER2_class::OCR_B() {
  return OCR2B;
}
uint8_t TIMER2_class::WGM() {
  return (TCCR2A & B00000011) | ((TCCR2B & B00001000) >> 1);
}
bool TIMER2_class::FOC_A() {
  return (TCCR2B & B10000000);
}
bool TIMER2_class::FOC_B() {
  return (TCCR2B & B01000000);
}
uint8_t TIMER2_class::COM_A() {
  return (TCCR2A & B11000000) >> 6;
}
uint8_t TIMER2_class::COM_B() {
  return (TCCR2A & B00110000) >> 4;
}
uint8_t TIMER2_class::CS() {
  return (TCCR2B & B00000111);
}
bool TIMER2_class::OCIE_A() {
  return (TIMSK2 & B00000010);
}
bool TIMER2_class::OCIE_B() {
  return (TIMSK2 & B00000100);
}
bool TIMER2_class::TOIE() {
  return (TIMSK2 & B00000001);
}

bool TIMER2_class::TOV() {
return (TIFR2 & B00000001);
}
bool TIMER2_class::OCF_A() {
return (TIFR2 & B00000010);
}
bool TIMER2_class::OCF_B() {
return (TIFR2 & B00000100);
}

#ifdef TIMER_0_EN
TIMER0_class TIMER_0;

ISR(TIMER0_COMPA_vect) {
  TIMER_0._TIMER_COMP_A();
}
ISR(TIMER0_COMPB_vect) {
  TIMER_0._TIMER_COMP_B();
}
//ISR(TIMER0_OVF_vect) {
// TIMER_0._TIMER_OVF();
//}
#endif

#ifdef TIMER_1_EN
TIMER1_class TIMER_1;

ISR(TIMER1_COMPA_vect) {
  TIMER_1._TIMER_COMP_A();
}
ISR(TIMER1_COMPB_vect) {
  TIMER_1._TIMER_COMP_B();
}
ISR(TIMER1_OVF_vect) {
  TIMER_1._TIMER_OVF();
}
ISR(TIMER1_CAPT_vect) {
  TIMER_1._TIMER_CAPT();
}
#endif

#ifdef TIMER_2_EN
TIMER2_class TIMER_2;

ISR(TIMER2_COMPA_vect) {
  TIMER_2._TIMER_COMP_A();
}
ISR(TIMER2_COMPB_vect) {
  TIMER_2._TIMER_COMP_B();
}
ISR(TIMER2_OVF_vect) {
  TIMER_2._TIMER_OVF();
}
#endif

#endif
