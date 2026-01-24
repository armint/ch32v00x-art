#pragma once

#include "SerialWritable.h"
#include "SerialReadable.h"

class FullDuplexSerial : public SerialWritable, public SerialReadable {
   uint8_t txBuffer[96];
   uint8_t rxBuffer[32];

   public:
   using Writable::write;
   using Readable::read;
   FullDuplexSerial();
   void initialize();
   void handleInterrupt();
};

extern FullDuplexSerial fullDuplexSerial;
