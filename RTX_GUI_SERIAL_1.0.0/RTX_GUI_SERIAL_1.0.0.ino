//#define RTX_DEVICE_NAME "GUI_S"
//#define RTX_SOFTWARE_VERSION {1,0,0}
//#define RTX_DEVICE_IP 0
//
//#include "RTX_PROTOCAL.h"

uint8_t GUI_ModeLevel = 0;

uint8_t GUI_Mode = 1;
uint8_t GUI_Mode_Prev = 0;

void setup() {
  Serial.begin(115200);

}

void loop() {
  static uint8_t SlaveIP_Selected;
  switch (GUI_Mode) {
    case 1:
      {
        if (GUI_Mode_Prev != GUI_Mode) {
          GUI_Mode_Prev = GUI_Mode;
          //First time Entering this Mode:
          Serial.print(F("ENTER SLAVE IP: "));
        }
        if (Serial.available()) {
          //Check if next byte is a number
          if (ParseNext() == 1) {
            int16_t ParseSerial_Hold = ParseInt();
            //Make sure there is no more left:
            if (ParseNext() == 0) {
              if (ParseSerial_Hold != -1 && ParseSerial_Hold < 8) {
                SlaveIP_Selected = ParseSerial_Hold;
                Serial.println(SlaveIP_Selected);
                Serial.println();
                GUI_Mode = 2;
              }
            }
          }
          while (Serial.available()) Serial.read(); //Serial Flush
        }
      }
      break;
    case 2:
      {
        if (GUI_Mode_Prev != GUI_Mode) {
          GUI_Mode_Prev = GUI_Mode;
          //First time Entering this Mode:
          Serial.print(F("SELECT CMD: SYSTEM(S), CONFIG/DEBUG-BUFFERS(B), GENERAL-MESSAGE(M): "));
        }
        if (Serial.available()) {
          //Check if Next is a Char:
          if (ParseNext() == 2) {
            int16_t ParseSerial_Hold = ParseChar();
            //Make sure there is no more left:
            if (ParseNext() == 0) {
              //Make sure we do have a char:
              if (ParseSerial_Hold != -1) {
                switch (ParseSerial_Hold) {
                  case 'S':
                    //SYSTEM(S)
                    Serial.println(F("S"));
                    break;
                  case 'B':
                    //CONFIG/DEBUG-BUFFERS(B)
                    Serial.println(F("B"));
                    break;
                  case 'M':
                    //GENERAL-MESSAGE(M)
                    Serial.println(F("M"));
                    break;
                  case 'C':
                    //CANCEL(C)
                    Serial.println(F("CANCEL"));
                    break;
                  case 'E':
                    //CANCEL(E)
                    Serial.println(F("CANCEL"));
                    break;
                }
              }
            }
          }
          while (Serial.available()) Serial.read(); //Serial Flush
        }
      }
      break;
    case 3:
      break;
    case 4:
      break;
  }
}


//Returns The Int Found at the Serial Buffer. Returns -1 if no Int found:
int16_t ParseInt() {
  //Delay A little to make3 sure all incoming data made it:
  //TODO: Maybe an Unnecessary Delay? Could Cause Problems
  delay(2);

  int16_t ReturnVal = -1; //Set default as -1
  uint8_t BitCount = 0; //Keep Track of how many Number Bits we are recieving in the Serial Buffer
  uint8_t BitHold[5]; //Keep Track of all incoming numbers, max is signed 16 bit(32 some thousand) 5 digits

  while (Serial.available()) {
    //Make sure the next char is a number:
    if (Serial.peek() >= '0' && Serial.peek() <= '9') {
      //Cant go over 5 digits:
      if (BitCount < 5) {
        BitHold[BitCount] = (Serial.read() - '0');
        BitCount++;
      }
    }
    else {
      //Construct the Int and Return Value
      if (BitCount) {
        //Make Sure we got some kind of value Ready, Other wise return -1
        ReturnVal = 0;
        for (uint8_t X = 0; X < BitCount; X++) {
          uint16_t ValHold = BitHold[(BitCount - 1) - X];
          for (uint8_t Y = 0; Y < X; Y++) ValHold *= 10;
          ReturnVal += ValHold;
        }
      }
      return ReturnVal;
    }
  }
  return ReturnVal;
}

//Reads a single Char from buffer. Does not read numbers. Returns -1 if not a char or nothing available
int16_t ParseChar() {
  delay(2);
  if (Serial.peek() >= 65 && Serial.peek() <= 126) return Serial.read();
  return -1;
}

//Checks what is next availble:
uint8_t ParseNext() {
  if (Serial.peek() >= '0' && Serial.peek() <= '9') return 1; //int or Bin or HEX Available
  if (Serial.peek() >= 65 && Serial.peek() <= 126) return 2; //Char Available
  return 0; //Nothing or invalid byte available
}
