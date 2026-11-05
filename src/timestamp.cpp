#include <utils/Timekeeping.h>
#include <utils/timestamp.h>

// libArT needs a monotonic millisecond clock (utils/timestamp.h). The application
// must start the SysTick with SysTick_Config(SystemCoreClock / 1000), or the tick never advances.
timestamp_t timestamp_in_ms() {
   return getUptimeTicks();
}
