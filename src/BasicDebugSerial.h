
#pragma once
#include "BlockingSerialWritable.h"


class BasicDebugSerial : public BlockingSerialWritable {
   public:
   using Writable::write;
   BasicDebugSerial();
   void initialize();
   ~BasicDebugSerial() = default;
};

extern BasicDebugSerial basicDebugSerial;
