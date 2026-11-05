#include <wch_device.h>

#include "Serial.h"

#include <peripherals/GpioPin.h>
#include <peripherals/Usart.h>

Serial::Serial()
   : SerialWritable(USART1, txRing)
   , SerialReadable(USART1, rxRing) {}

void Serial::initialize() {
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

void Serial::handleInterrupt() {
   SerialWritable::handleInterrupt();
   SerialReadable::handleInterrupt();
}

Serial serial;
