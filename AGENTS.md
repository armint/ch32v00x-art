# Agent Instructions for ch32v00x-art

## Purpose
`ch32v00x-art` provides reusable, hardware-specific implementations of `libArT` interfaces for the WCH CH32V microcontrollers. It bridges the gap between the hardware abstraction (`ch32v00x-system`) and the generic utility library (`libArT`).

## Key Components
- **Serial Streams**: `SerialWritable` and `SerialReadable` provide interrupt-driven, buffered I/O.
- **Debug Serial**: `DebugSerial` and `BasicDebugSerial` are convenience classes for system logging.
- **Full Duplex**: `Serial` integrates both reading and writing.

## Guidelines
- **Dependencies**: When adding new components, ensure they depend on both `libArT` (for interfaces) and `ch32v00x-system` (for hardware). Use the libArT utilities (for example `RingBuffer` in `utils/RingBuffer.h`) before creating similar classes. Do not use the Embedded Template Library (ETL).
- **Names**: Call the public names of `ch32v00x-system`. A class or function that does the same job as one in `libart-stm32` has the same name. A function that does a different job gets a different name. Do not change the behavior or the signature of a function only to get the same name. Do not add a wrapper or an alias with a different name.
- **No References to STM32 Libraries in Docs and Comments**: `README.md` says once that the names try to agree with `libart-stm32`. Do not refer to `system-stm32`, `libart-stm32` or the STM32 classes anywhere else in `README.md` or the code comments. Do not compare a class with them, and do not write what they have or do not have. Describe each class on its own terms. A reference to the STM32F1 register encoding is a hardware fact and is allowed.
- **Hardware Tests in Docs**: Most functions of the library have run on hardware, because a project needed them. Do not write in the docs which functions have a recorded hardware test. Write a known hardware limit or a hardware behavior that the code depends on.
- **Documentation**: Write the prose in ASD-STE100 Simplified Technical English (STE): instructions of 20 words or fewer, descriptions of 25 words or fewer, active voice, simple tenses, and one term for each concept.
- **Style & Linting**: Follow the `libArT` stream pattern (returning `art::expected`) mechanically. Before completing tasks, format code using the provided CMake target: `cmake --build <build_dir> --target clangformat`.
- **Memory**: Ensure memory usage is static or stack-based; heap memory usage is strictly prohibited.
