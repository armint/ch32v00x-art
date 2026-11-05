#include "SerialWritable.h"
#include <utils/Interrupt.h>

SerialWritable::SerialWritable(USART_TypeDef *usartTypeDef, art::span<uint8_t> txBuffer)
   : usart(usartTypeDef)
   , txBuffer(txBuffer.data(), txBuffer.size()) {
}


void SerialWritable::handleInterrupt() {
   if(usart.isTxRegisterEmpty() && usart.isTxEmptyInterruptEnabled()) {
      if(!txBuffer.empty()) {
         usart.setData(txBuffer.front());
         txBuffer.pop();
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
      bool empty;
      {
         Interrupt::CriticalSection ics;
         empty = txBuffer.empty();
      }

      if(empty || timeout.hasExpired() || flushed >= count) {
         break;
      }

      if(usart.isTxRegisterEmpty()) {
         Interrupt::CriticalSection ics;
         usart.setData(txBuffer.front());
         txBuffer.pop();
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
      bool full;
      {
         Interrupt::CriticalSection ics;
         full = txBuffer.full();
      }
      if(full) {
         auto res = flush(1);
         if(!res) {
            return art::unexpected(res.error());
         }
      }
      Interrupt::CriticalSection ics;
      txBuffer.push(c);
   }
   usart.enableTxEmptyInterrupt();
   return data.size();
}
