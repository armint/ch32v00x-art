#pragma once

#include "SerialWritable.h"
#include "SerialReadable.h"

class Serial : public SerialWritable, public SerialReadable {
   uint8_t txBuffer[96];
   uint8_t rxBuffer[32];

   public:
   using Writable::write;
   using Readable::read;
   Serial();
   void initialize();
   void handleInterrupt();
};

extern Serial serial;
