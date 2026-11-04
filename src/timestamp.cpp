#include <peripherals/systick.h>
#include <utils/timestamp.h>

// libArT needs a monotonic millisecond clock (utils/timestamp.h). The SysTick
// runs at 1 kHz: the examples start it with SystemCoreClock / 1000.
timestamp_t timestamp_in_ms() {
   return getSysTick();
}
