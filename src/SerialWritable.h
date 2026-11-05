#pragma once

#include <cstdarg>
#include <cstddef>

#include <utils/RingBuffer.h>

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
   hw::Usart usart;
   RingBufferBase<uint8_t> &txBuffer;

   virtual art::expected<void, Writable::Error> flush(size_t count);

   public:
   using Writable::write;

   // The rule of five.
   SerialWritable(const SerialWritable &) = delete;
   SerialWritable(SerialWritable &&) = delete;
   SerialWritable &operator=(const SerialWritable &) = delete;
   SerialWritable &operator=(SerialWritable &&) = delete;

   SerialWritable(USART_TypeDef *usartTypeDef, RingBufferBase<uint8_t> &txBuffer);

   ~SerialWritable() = default;

   virtual art::expected<size_t, Writable::Error> write(art::span<const uint8_t> data) override;
   virtual art::expected<void, Writable::Error> flush() override;

   void handleInterrupt();
};
