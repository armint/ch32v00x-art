#pragma once

#include "SerialWritable.h"
#include "SerialReadable.h"

class Serial : public SerialWritable, public SerialReadable {
   RingBuffer<uint8_t, 128> txBuffer;
   RingBuffer<uint8_t, 32> rxBuffer;

   public:
   using Writable::write;
   using Readable::read;
   Serial();
   void initialize();
   void handleInterrupt();
};

extern Serial serial;
