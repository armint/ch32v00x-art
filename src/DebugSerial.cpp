#include <wch_device.h>

#include "DebugSerial.h"

#include <peripherals/GpioPin.h>
#include <peripherals/Usart.h>

DebugSerial::DebugSerial()
   : SerialWritable(USART1, txRing) {}

void DebugSerial::initialize() {
   const hw::GpioPin dbgTx(GPIOD, 5);
   dbgTx.configure(hw::GpioPin::Mode::Alternate, hw::GpioPin::OutputType::PushPull, hw::GpioPin::Pull::None,
                   hw::GpioPin::Speed::High);
   usart.configure(115200, USARTParity::None, UsartStopBits::StopBits_1);
   usart.enableTx();
   usart.enable();
   NVIC_EnableIRQ(USART1_IRQn);
}

DebugSerial debugSerial;