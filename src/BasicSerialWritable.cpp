
#include "BasicSerialWritable.h"

BasicSerialWritable::BasicSerialWritable(USART_TypeDef *usartTypeDef)
   : usart(usartTypeDef) {}
std::expected<size_t, Writable::Error> BasicSerialWritable::write(std::span<const uint8_t> data) {
   for(auto c : data) {
      while(!usart.isTxRegisterEmpty()) {
         // Wait for register empty
      }
      usart.setData(c);
   }
   return data.size();
}

std::expected<void, Writable::Error> BasicSerialWritable::flush() {
   return {};
}