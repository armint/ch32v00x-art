#include "SerialReadable.h"
#include <utils/Timeout.h>

SerialReadable::SerialReadable(USART_TypeDef *usartTypeDef, RingBufferBase<uint8_t> &rxBuffer)
   : usart(usartTypeDef)
   , rxBuffer(rxBuffer) {
}

void SerialReadable::handleInterrupt() {
   if(usart.isRxRegisterNotEmpty() && usart.isRxNotEmptyInterruptEnabled()) {
      uint8_t data = usart.getData();
      // A full buffer drops the byte.
      static_cast<void>(rxBuffer.try_push(data));
   }
}

art::expected<char, Readable::Error> SerialReadable::read() {
   utils::Timeout timeout(timeout_);
   while(true) {
      uint8_t c;
      if(rxBuffer.try_pop(c)) {
         return static_cast<char>(c);
      }

      if(timeout_ != 0 && timeout.hasExpired()) {
         return art::unexpected(Timeout);
      }
   }
}

bool SerialReadable::available() {
   return !rxBuffer.empty();
}
