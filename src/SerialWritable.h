#pragma once

#include <cstdarg>
#include <cstddef>

#include <etl/circular_buffer.h>

#include <wch_device.h>

#include <peripherals/Usart.h>
#include <streams/Readable.h>
#include <streams/Writable.h>

/**
 * This class is used to write data to, and read data from a uart. It uses interrupts and buffers to minimize the load
 * on the processor and to make sure real-time processes are not disturbed.
 */
class SerialWritable : public Writable {

   protected:
   Usart usart;
   etl::circular_buffer_ext<uint8_t> txBuffer;

   virtual std::expected<void, Writable::Error> flush(size_t count);

   public:
   using Writable::write;

   // The rule of five.
   SerialWritable(const SerialWritable &) = delete;
   SerialWritable(SerialWritable &&) = delete;
   SerialWritable &operator=(const SerialWritable &) = delete;
   SerialWritable &operator=(SerialWritable &&) = delete;

   SerialWritable(USART_TypeDef *usartTypeDef, std::span<uint8_t> txBuffer);

   ~SerialWritable() = default;

   virtual std::expected<size_t, Writable::Error> write(std::span<const uint8_t> data) override;
   virtual std::expected<void, Writable::Error> flush() override;

   void handleInterrupt();
};
