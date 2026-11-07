#pragma once

#include <peripherals/GpioPin.h>

// USART1 pins without AFIO remap. CH32V00X: TX PD5, RX PD6.
// CH32V20X and CH32V30X: TX PA9, RX PA10.
#if defined(CH32V003) || defined(CH32V002) || defined(CH32V006)
inline hw::GpioPin usart1TxPin() {
   return { GPIOD, 5 };
}
inline hw::GpioPin usart1RxPin() {
   return { GPIOD, 6 };
}
#elif defined(CH32V20x_D8W) || defined(CH32V20x_D8) || defined(CH32V20x_D6) || defined(CH32V30x_D8C)
inline hw::GpioPin usart1TxPin() {
   return { GPIOA, 9 };
}
inline hw::GpioPin usart1RxPin() {
   return { GPIOA, 10 };
}
#else
#error "Usart1Pins.h: no USART1 pins for this device"
#endif
