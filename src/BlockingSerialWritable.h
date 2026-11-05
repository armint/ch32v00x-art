
#pragma once
#include <wch_device.h>
#include "peripherals/Usart.h"
#include "streams/Writable.h"


class BlockingSerialWritable : public Writable {
   protected:
   hw::Usart usart;

   public:
   using Writable::write;

   // The rule of five.
   BlockingSerialWritable(const BlockingSerialWritable &) = delete;
   BlockingSerialWritable(BlockingSerialWritable &&) = delete;
   BlockingSerialWritable &operator=(const BlockingSerialWritable &) = delete;
   BlockingSerialWritable &operator=(BlockingSerialWritable &&) = delete;

   BlockingSerialWritable(USART_TypeDef *usartTypeDef);

   ~BlockingSerialWritable() override = default;

   art::expected<size_t, Writable::Error> write(art::span<const uint8_t> data) override;
   art::expected<void, Writable::Error> flush() override;
};
