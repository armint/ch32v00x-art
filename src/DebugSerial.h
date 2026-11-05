
#pragma once

#include "SerialWritable.h"

class DebugSerial : public SerialWritable {
   RingBuffer<uint8_t, 128> txBuffer;

   public:
   using Writable::write;
   friend void USART1_IRQHandler();
   DebugSerial();
   void initialize();
};

extern DebugSerial debugSerial;
