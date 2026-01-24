#include <wch_device.h>

#include "BasicDebugSerial.h"

#include "peripherals/GpioPin.h"

BasicDebugSerial::BasicDebugSerial()
   : BasicSerialWritable(USART1) {}

void BasicDebugSerial::initialize() {
   GpioPin dbgTx(GPIOD, 5);
   dbgTx.configureOutput(GpioPin::OutputMode::PushPull, GpioPin::OutputSpeed::Output10MHz, true);
   usart.configure(115200, USARTParity::None, UsartStopBits::StopBits_1);
   usart.enableTx();
   usart.enable();
}

BasicDebugSerial basicDebugSerial;