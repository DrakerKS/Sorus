#pragma once

#include "godot_cpp/core/error_macros.hpp"

#define INVALID_RESOURCE_MSG(m_resource_type) vformat("Provided '%s' resource is invalid (nullptr)", m_resource_type)

/**
* Prints `m_msg`.
*
* Ensures `m_cond` is false.
* If `m_cond` is true, a warning is printed and the current function continues.
*/
#define WARN_COND(m_cond, m_msg) \
	if (unlikely(m_cond)) { \
		::godot::_err_print_error(FUNCTION_STR, __FILE__, __LINE__, "Condition \"" _STR(m_cond) "\" is true. ", m_msg, false, true); \
	} else \
		((void)0)

/**
* Same as `WARN_COND` but also notifies the editor.
*/
#define WARN_COND_ED(m_cond, m_msg) \
	if (unlikely(m_cond)) { \
		::godot::_err_print_error(FUNCTION_STR, __FILE__, __LINE__, "Condition \"" _STR(m_cond) "\" is true. ", m_msg, true, true); \
	} else \
		((void)0)

/**
* Prints `m_msg`.
*
* Ensures `m_cond` is false.
* If `m_cond` is true, a warning is printed and the current function returns.
*/
#define WARN_FAIL_COND(m_cond, m_msg) \
	if (unlikely(m_cond)) { \
		::godot::_err_print_error(FUNCTION_STR, __FILE__, __LINE__, "Condition \"" _STR(m_cond) "\" is true. ", m_msg, false, true); \
		return; \
	} else \
		((void)0)

/**
* Same as `WARN_FAIL_COND` but also notifies the editor.
*/
#define WARN_FAIL_COND_ED(m_cond, m_msg) \
	if (unlikely(m_cond)) { \
		::godot::_err_print_error(FUNCTION_STR, __FILE__, __LINE__, "Condition \"" _STR(m_cond) "\" is true.", m_msg, true, true); \
		return; \
	} else \
		((void)0)

/**
 * Prints `m_msg`.
 *
 * Ensures `m_cond` is false.
 * If `m_cond` is true, a warning is printed and the current function returns `m_retval`.
 */
#define WARN_FAIL_COND_V(m_cond, m_retval, m_msg) \
	if (unlikely(m_cond)) { \
		::godot::_err_print_error(FUNCTION_STR, __FILE__, __LINE__, "Condition \"" _STR(m_cond) "\" is true. Returning: " _STR(m_retval), m_msg, false, true); \
		return m_retval; \
	} else \
		((void)0)

/**
* Same as `WARN_FAIL_COND_V` but also notifies the editor.
*/
#define WARN_FAIL_COND_V_ED(m_cond, m_retval, m_msg) \
	if (unlikely(m_cond)) { \
		::godot::_err_print_error(FUNCTION_STR, __FILE__, __LINE__, "Condition \"" _STR(m_cond) "\" is true. Returning: " _STR(m_retval), m_msg, true, true); \
		return m_retval; \
	} else \
		((void)0)
