#ifndef EV_BASE_H
#define EV_BASE_H

#include "core/platform_detection.hpp"

#ifdef EV_DEBUG
	#if defined(EV_PLATFORM_WINDOWS)
		#define EV_DEBUGBREAK() __debugbreak()
	#elif defined(EV_PLATFORM_LINUX)
		#include <signal.h>
		#define EV_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
	#define EV_ENABLE_ASSERTS
#else
	#define EV_DEBUGBREAK()
#endif

#define EV_EXPAND_MACRO(x) x
#define EV_STRINGIFY_MACRO(x) #x

#include "core/exceptions.hpp"
#include "core/log.hpp"

#define EV_CHECK_THROW(expr, message) if(!(expr)) throw ::ev2::engine_exception{"[" + std::filesystem::path(__FILE__).filename().string() + ":" + std::to_string(__LINE__) + "]:" + message}

#endif