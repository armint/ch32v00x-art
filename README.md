# CH32V00x-ArT Library

The **CH32V00x-ArT** library provides reusable components that bridge the **libArT** utility library and the **ch32v00x-system** HAL. It specifically implements stream abstractions for serial communication on WCH CH32V microcontrollers.

## Features

- **Interrupt-driven Serial I/O**: Efficient, non-blocking serial communication using the libArT `RingBuffer` (`utils/RingBuffer.h`).
- **Stream Implementation**: Implements `art::Writable` and `art::Readable` interfaces from `libArT`.
- **Debug Utilities**:
    - `DebugSerial`: Buffered, interrupt-driven serial output for logging.
    - `BasicDebugSerial`: Minimal, blocking serial output for early boot or low-resource scenarios.
- **Full Duplex Support**: `Serial` for simultaneous transmit and receive.

## Core Components

- `SerialWritable`: Buffered UART transmission using interrupts.
- `SerialReadable`: Buffered UART reception using interrupts.
- `DebugSerial`: High-level wrapper for serial debugging.
- `BlockingSerialWritable`: Simple polling-based UART transmission.
- `Serial`: Combines buffered reading and writing.

## Dependencies

- **art-core**: Provides the base `Readable` and `Writable` interfaces and the `RingBuffer` for the serial buffers.
- **ch32v00x-system**: Provides peripheral access to USART and GPIO. Use a version with the names of `system-stm32` (`hw::GpioPin`, `getUptimeTicks()`), from [MR !17](https://gitlab.vandertogt.org/riscv/ch32v00x-system/-/merge_requests/17) on.

## Timestamps

`timestamp_in_ms()` returns the time of the SysTick (system tick timer) in `ch32v00x-system`. This is the millisecond clock that libArT uses for timeouts.

The application must start the SysTick with `SysTick_Config(SystemCoreClock / 1000)`, which gives a 1 ms tick. Without this call, the tick does not advance and libArT timeouts never expire.

## Usage in CMake

This library is intended to be used as a dependency in the `ch32v00x-examples` project or other projects using these MCUs.

```cmake
add_subdirectory(dependencies/ch32v00x-art)
target_link_libraries(${PROJECT_NAME} PRIVATE ch32v00x-art)
```

## License

ch32v00x-art uses the MIT license. See [LICENSE](LICENSE).
