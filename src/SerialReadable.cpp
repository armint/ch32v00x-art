#include "SerialReadable.h"
#include <utils/Interrupt.h>
#include <utils/Timeout.h>

SerialReadable::SerialReadable(USART_TypeDef *usartTypeDef, art::span<uint8_t> rxBuffer)
   : usart(usartTypeDef)
   , rxBuffer(rxBuffer.data(), rxBuffer.size()) {
}

void SerialReadable::handleInterrupt() {
   if(usart.isRxRegisterNotEmpty() && usart.isRxNotEmptyInterruptEnabled()) {
      uint8_t data = usart.getData();
      if(!rxBuffer.full()) {
         rxBuffer.push(data);
      }
   }
}

art::expected<char, Readable::Error> SerialReadable::read() {
   utils::Timeout timeout(timeout_);
   while(true) {
      bool empty;
      {
         Interrupt::CriticalSection ics;
         empty = rxBuffer.empty();
      }

      if(!empty) {
         Interrupt::CriticalSection ics;
         char c = static_cast<char>(rxBuffer.front());
         rxBuffer.pop();
         return c;
      }

      if(timeout_ != 0 && timeout.hasExpired()) {
         return art::unexpected(Timeout);
      }
   }
}

bool SerialReadable::available() {
   Interrupt::CriticalSection ics;
   return !rxBuffer.empty();
}
