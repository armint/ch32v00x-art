#include <wch_device.h>

#include "FullDuplexSerial.h"

#include <peripherals/GpioPin.h>
#include <peripherals/Usart.h>

FullDuplexSerial::FullDuplexSerial()
   : SerialWritable(USART1, txBuffer)
   , SerialReadable(USART1, rxBuffer) {}

void FullDuplexSerial::initialize() {
   GpioPin dbgTx(GPIOD, 5);
   dbgTx.configureOutput(GpioPin::OutputMode::PushPull, GpioPin::OutputSpeed::Output10MHz, true);

   GpioPin dbgRx(GPIOD, 6);
   dbgRx.configureInput(GpioPin::InputMode::Floating);

   SerialWritable::usart.configure(115200, USARTParity::None, UsartStopBits::StopBits_1);
   SerialWritable::usart.enableTx();
   SerialWritable::usart.enableRx();
   SerialWritable::usart.enable();

   SerialWritable::usart.enableRxNotEmptyInterrupt();

   NVIC_EnableIRQ(USART1_IRQn);
}

void FullDuplexSerial::handleInterrupt() {
   SerialWritable::handleInterrupt();
   SerialReadable::handleInterrupt();
}

FullDuplexSerial fullDuplexSerial;
