#pragma once

#include <cstdint>
#include <utils/compat/span.h>
#include <utils/compat/expected.h>

#include <etl/circular_buffer.h>

#include <wch_device.h>

#include <peripherals/Usart.h>
#include <streams/Readable.h>

/**
 * This class is used to read data from a uart. It uses interrupts and buffers to minimize the load
 * on the processor and to make sure real-time processes are not disturbed.
 */
class SerialReadable : public virtual Readable {

   protected:
   Usart usart;
   etl::circular_buffer_ext<uint8_t> rxBuffer;

   public:
   using Readable::read;

   // The rule of five.
   SerialReadable(const SerialReadable &) = delete;
   SerialReadable(SerialReadable &&) = delete;
   SerialReadable &operator=(const SerialReadable &) = delete;
   SerialReadable &operator=(SerialReadable &&) = delete;

   SerialReadable(USART_TypeDef *usartTypeDef, art::span<uint8_t> rxBuffer);

   ~SerialReadable() = default;

   virtual art::expected<char, Error> read() override;
   virtual bool available() override;

   void handleInterrupt();
};
