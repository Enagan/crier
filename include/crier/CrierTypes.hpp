#ifndef CRIER_TYPES_HPP
#define CRIER_TYPES_HPP

#pragma once
#include <functional>

namespace crier {    
    enum class CallbackPriority { FIRST, ASAP, NORMAL };
    enum class UnhandledMessageBehaviour { Ignore, Enqueue };
    enum class InboundDispatching { Immediate, DispatchQueue };    
}

#endif 