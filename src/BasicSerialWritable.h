
#pragma once
#include <wch_device.h>
#include "peripherals/Usart.h"
#include "streams/Writable.h"


class BasicSerialWritable : public Writable {
   protected:
   Usart usart;

   public:
   using Writable::write;

   // The rule of five.
   BasicSerialWritable(const BasicSerialWritable &) = delete;
   BasicSerialWritable(BasicSerialWritable &&) = delete;
   BasicSerialWritable &operator=(const BasicSerialWritable &) = delete;
   BasicSerialWritable &operator=(BasicSerialWritable &&) = delete;

   BasicSerialWritable(USART_TypeDef *usartTypeDef);

   ~BasicSerialWritable() override = default;

   art::expected<size_t, Writable::Error> write(art::span<const uint8_t> data) override;
   art::expected<void, Writable::Error> flush() override;
};
