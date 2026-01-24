
#pragma once

#include "SerialWritable.h"

class DebugSerial : public SerialWritable {
   uint8_t txBuffer[96];

   public:
   using Writable::write;
   friend void USART1_IRQHandler();
   DebugSerial();
   void initialize();
};

extern DebugSerial debugSerial;
