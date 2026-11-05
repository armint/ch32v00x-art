
#pragma once

#include "SerialWritable.h"

// A base class, first in the base list, so the buffer exists before SerialWritable binds to it.
struct DebugSerialBuffers {
   RingBuffer<uint8_t, 128> txRing;
};

class DebugSerial : private DebugSerialBuffers, public SerialWritable {

   public:
   using Writable::write;
   friend void USART1_IRQHandler();
   DebugSerial();
   void initialize();
};

extern DebugSerial debugSerial;
