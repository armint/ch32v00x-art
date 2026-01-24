#include <wch_device.h>

#include "DebugSerial.h"

#include <peripherals/GpioPin.h>
#include <peripherals/Usart.h>

DebugSerial::DebugSerial()
   : SerialWritable(USART1, txBuffer) {}

void DebugSerial::initialize() {
   GpioPin dbgTx(GPIOD, 5);
   dbgTx.configureOutput(GpioPin::OutputMode::PushPull, GpioPin::OutputSpeed::Output10MHz, true);
   usart.configure(115200, USARTParity::None, UsartStopBits::StopBits_1);
   usart.enableTx();
   usart.enable();
   NVIC_EnableIRQ(USART1_IRQn);
}

DebugSerial debugSerial;