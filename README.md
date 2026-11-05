# CH32V00x-ArT Library

The **CH32V00x-ArT** library provides reusable components that bridge the **libArT** utility library and the **ch32v00x-system** HAL. It specifically implements stream abstractions for serial communication on WCH CH32V microcontrollers.

## Features

- **Interrupt-driven Serial I/O**: Efficient, non-blocking serial communication using `etl::circular_buffer`.
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

- **art-core**: Provides the base `Readable` and `Writable` interfaces.
- **ch32v00x-system**: Provides peripheral access to USART and GPIO.
- **etl**: Uses the Embedded Template Library for circular buffers.

## Timestamps

`timestamp_in_ms()` returns the time of the SysTick (system tick timer) in `ch32v00x-system`. This is the millisecond clock that libArT uses for timeouts.

The application must start the SysTick with `initSysTick(SystemCoreClock / 1000)`, which gives a 1 ms tick. Without this call, the tick does not advance and libArT timeouts never expire.

## Usage in CMake

This library is intended to be used as a dependency in the `ch32v00x-examples` project or other projects using these MCUs.

```cmake
add_subdirectory(dependencies/ch32v00x-art)
target_link_libraries(${PROJECT_NAME} PRIVATE ch32v00x-art)
```
