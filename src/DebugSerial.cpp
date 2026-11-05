#include <wch_device.h>

#include "DebugSerial.h"
#include "Usart1Pins.h"

#include <peripherals/GpioPin.h>
#include <peripherals/Usart.h>

DebugSerial::DebugSerial()
   : SerialWritable(USART1, txRing) {}

void DebugSerial::initialize() {
   const hw::GpioPin dbgTx = usart1TxPin();
   dbgTx.configure(hw::GpioPin::Mode::Alternate, hw::GpioPin::OutputType::PushPull, hw::GpioPin::Pull::None,
                   hw::GpioPin::Speed::High);
   usart.configure(115200, hw::Usart::Parity::None, hw::Usart::StopBits::StopBits_1);
   usart.enableTx();
   usart.enable();
   NVIC_EnableIRQ(USART1_IRQn);
}

DebugSerial debugSerial;