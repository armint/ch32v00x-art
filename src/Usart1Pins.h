#pragma once

#include <peripherals/GpioPin.h>

// USART1 pins without AFIO remap. CH32V00X: TX PD5, RX PD6.
// CH32V20X and CH32V30X: TX PA9, RX PA10 (the STM32F1 pins).
#if defined(CH32V003) || defined(CH32V002) || defined(CH32V006)
inline hw::GpioPin usart1TxPin() {
   return { GPIOD, 5 };
}
inline hw::GpioPin usart1RxPin() {
   return { GPIOD, 6 };
}
#else
inline hw::GpioPin usart1TxPin() {
   return { GPIOA, 9 };
}
inline hw::GpioPin usart1RxPin() {
   return { GPIOA, 10 };
}
#endif
