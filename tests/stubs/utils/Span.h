#pragma once

#include <utils/compat/span.h>

/**
 * Stand-in for utils/Span.h of the system library. The host tests have no
 * system library build that selects hw::span, so hw::span is libArT's
 * art::span, the type that a consistent build selects.
 */
namespace hw {
using art::dynamic_extent;
using art::span;
} // namespace hw
