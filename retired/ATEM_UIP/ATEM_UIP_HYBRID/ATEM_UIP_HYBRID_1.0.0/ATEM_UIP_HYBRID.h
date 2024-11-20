//ATEM_UIP_HYBRID REV 1.0.0
//.h
#ifndef ATEM_UIP_HYBRID_h
#define ATEM_UIP_HYBRID_h

#include "Arduino.h"
#include "ATEM_UIP_FAST.h"
#include "ATEM_UIP_SLOW.h"

class ATEM_UIP_HYBRID {
  public:

    void Setup(IPAddress IP, uint16_t PortFast, uint16_t PortSlow) {
      ATEM_FAST.Setup(IP, PortFast);
      ATEM_SLOW.Setup(IP, PortSlow);
    }

    //Returns Connected State:
    uint8_t Run();

    byte TallyInput[8];
  
  private:

    ATEM_UIP_FAST ATEM_FAST;
    ATEM_UIP_SLOW ATEM_SLOW;

    byte PrevTallyInput_SLOW[8];
    byte PrevTallyInput_FAST[8];

};

//.cpp
//#include "ATEM_UIP_HYBRID.h"
//#include "Arduino.h"

uint8_t ATEM_UIP_HYBRID::Run() {

uint8_t Result = 0;
  
  Result = ATEM_FAST.Run();
  Result |= (ATEM_SLOW.Run() << 1);

  
  for(uint8_t X = 0; X < 8; X++) {
//    if(this->PrevTallyInput_SLOW[X] != ATEM_SLOW.TallyInput[X]) {
//      TallyInput[X] = ATEM_SLOW.TallyInput[X];
//    }
//    else 
    if(this->PrevTallyInput_FAST[X] != ATEM_FAST.TallyInput[X]){
      TallyInput[X] = ATEM_FAST.TallyInput[X];
    }
    PrevTallyInput_FAST[X] = ATEM_FAST.TallyInput[X];
    PrevTallyInput_SLOW[X] = ATEM_SLOW.TallyInput[X];
  }

  return Result;
}

#endif
