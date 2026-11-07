
#include "BlockingSerialWritable.h"

BlockingSerialWritable::BlockingSerialWritable(USART_TypeDef *usartTypeDef)
   : usart(usartTypeDef) {}
art::expected<size_t, Writable::Error> BlockingSerialWritable::write(art::span<const uint8_t> data) {
   for(auto c : data) {
      while(!usart.isTxRegisterEmpty()) {
         // Wait for register empty
      }
      usart.setData(c);
   }
   return data.size();
}

// Returns when the last stop bit is on the line. Call it before a USART clock change (a flash
// operation above 120 MHz halves the APB clocks) or before the USART is switched off.
art::expected<void, Writable::Error> BlockingSerialWritable::flush() {
   while(!usart.isTxRegisterEmpty() || !usart.isTransferComplete()) {
   }
   return {};
}