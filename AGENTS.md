# Agent Instructions for ch32v00x-art

## Purpose
`ch32v00x-art` provides reusable, hardware-specific implementations of `libArT` interfaces for the WCH CH32V microcontrollers. It bridges the gap between the hardware abstraction (`ch32v00x-system`) and the generic utility library (`libArT`).

## Key Components
- **Serial Streams**: `SerialWritable` and `SerialReadable` provide interrupt-driven, buffered I/O.
- **Debug Serial**: `DebugSerial` and `BasicDebugSerial` are convenience classes for system logging.
- **Full Duplex**: `FullDuplexSerial` integrates both reading and writing.

## Guidelines
- **Dependencies**: When adding new components, ensure they depend on both `libArT` (for interfaces) and `ch32v00x-system` (for hardware). The `embedded template library` (ETL) is a dependency and should be considered before creating similar classes.
- **Style & Linting**: Follow the `libArT` stream pattern (returning `std::expected`) mechanically. Before completing tasks, format code using the provided CMake target: `cmake --build <build_dir> --target clangformat`.
- **Memory**: Ensure memory usage is static or stack-based; heap memory usage is strictly prohibited.
