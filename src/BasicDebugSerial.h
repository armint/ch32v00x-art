
#pragma once
#include "BasicSerialWritable.h"


class BasicDebugSerial : public BasicSerialWritable {
   public:
   using Writable::write;
   BasicDebugSerial();
   void initialize();
   ~BasicDebugSerial() = default;
};

extern BasicDebugSerial basicDebugSerial;
