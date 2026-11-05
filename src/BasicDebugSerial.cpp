#include <wch_device.h>

#include "BasicDebugSerial.h"
#include "Usart1Pins.h"

#include "peripherals/GpioPin.h"

BasicDebugSerial::BasicDebugSerial()
   : BlockingSerialWritable(USART1) {}

void BasicDebugSerial::initialize() {
   const hw::GpioPin dbgTx = usart1TxPin();
   dbgTx.configure(hw::GpioPin::Mode::Alternate, hw::GpioPin::OutputType::PushPull, hw::GpioPin::Pull::None,
                   hw::GpioPin::Speed::High);
   usart.configure(115200, USARTParity::None, UsartStopBits::StopBits_1);
   usart.enableTx();
   usart.enable();
}

BasicDebugSerial basicDebugSerial;