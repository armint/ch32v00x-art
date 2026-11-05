#pragma once

#include "SerialWritable.h"
#include "SerialReadable.h"

// A base class, first in the base list, so the buffers exist before SerialWritable and SerialReadable bind to them.
struct SerialBuffers {
   RingBuffer<uint8_t, 128> txRing;
   RingBuffer<uint8_t, 32> rxRing;
};

class Serial : private SerialBuffers, public SerialWritable, public SerialReadable {

   public:
   using Writable::write;
   using Readable::read;
   Serial();
   void initialize();
   void handleInterrupt();
};

extern Serial serial;
