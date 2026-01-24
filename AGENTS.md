# Agent Instructions for ch32v00x-art

## Purpose
`ch32v00x-art` provides reusable, hardware-specific implementations of `libArT` interfaces for the WCH CH32V microcontrollers. It bridges the gap between the hardware abstraction (`ch32v00x-system`) and the generic utility library (`libArT`).

## Key Components
- **Serial Streams**: `SerialWritable` and `SerialReadable` provide interrupt-driven, buffered I/O.
- **Debug Serial**: `DebugSerial` and `BasicDebugSerial` are convenience classes for system logging.
- **Full Duplex**: `FullDuplexSerial` integrates both reading and writing.

## Guidelines
- When adding new components, ensure they depend on both `libArT` (for interfaces) and `ch32v00x-system` (for hardware).
- Follow the `libArT` stream pattern (returning `std::expected`).
- Ensure memory usage is static or stack-based; no heap allocation is allowed.
