#include <wch_device.h>

#include "Serial.h"
#include "Usart1Pins.h"

#include <peripherals/GpioPin.h>
#include <peripherals/Usart.h>

Serial::Serial()
   : SerialWritable(USART1, txRing)
   , SerialReadable(USART1, rxRing) {}

void Serial::initialize() {
   const hw::GpioPin dbgTx = usart1TxPin();
   dbgTx.configure(hw::GpioPin::Mode::Alternate, hw::GpioPin::OutputType::PushPull, hw::GpioPin::Pull::None,
                   hw::GpioPin::Speed::High);

   const hw::GpioPin dbgRx = usart1RxPin();
   dbgRx.configure(hw::GpioPin::Mode::Input);

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
