#include "SerialWritable.h"

SerialWritable::SerialWritable(USART_TypeDef *usartTypeDef, RingBufferBase<uint8_t> &txBuffer)
   : usart(usartTypeDef)
   , txBuffer(txBuffer) {
}


void SerialWritable::handleInterrupt() {
   if(usart.isTxRegisterEmpty() && usart.isTxEmptyInterruptEnabled()) {
      uint8_t c;
      if(txBuffer.try_pop(c)) {
         usart.setData(c);
      }
      if(txBuffer.empty()) {
         usart.disableTxEmptyInterrupt();
      }
   }
}


art::expected<void, Writable::Error> SerialWritable::flush(size_t count) {
   usart.disableTxEmptyInterrupt();
   utils::Timeout timeout(5000);
   size_t flushed = 0;
   while(true) {
      if(txBuffer.empty() || timeout.hasExpired() || flushed >= count) {
         break;
      }

      // The TX interrupt is off, so this is the only consumer.
      uint8_t c;
      if(usart.isTxRegisterEmpty() && txBuffer.try_pop(c)) {
         usart.setData(c);
         flushed++;
      }
   }

   if(!txBuffer.empty()) {
      usart.enableTxEmptyInterrupt();
   }

   if(timeout.hasExpired()) {
      return art::unexpected(CouldNotWrite);
   }
   return {};
}


art::expected<void, Writable::Error> SerialWritable::flush() {
   return flush(txBuffer.size());
}


art::expected<size_t, Writable::Error> SerialWritable::write(art::span<const uint8_t> data) {
   for(size_t i = 0; i < data.size(); i++) {
      uint8_t c = data[i];
      if(txBuffer.full()) {
         auto res = flush(1);
         if(!res) {
            return art::unexpected(res.error());
         }
      }
      static_cast<void>(txBuffer.try_push(c));
   }
   usart.enableTxEmptyInterrupt();
   return data.size();
}
