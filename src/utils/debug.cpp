#include "debug.hpp"

#include "godot_cpp/variant/utility_functions.hpp"

#include "utils/macros.hpp"

using namespace godot;

#ifdef DEBUG_ENABLED
  void Debug::_bind_methods() {
    SORUS_BIND_STATIC_PROPERTY_BOOL(verbose);
  }

  #define VERBOSE_CHECK(m_verbose) \
    if (m_verbose && !Debug::verbose) { \
      return; \
    } else ((void)0)

  void debug_set_verbose(bool p_verbose) {
    Debug::verbose = p_verbose;
  }

  bool debug_get_verbose() {
    return Debug::verbose;
  }

  void debug_print(const String &p_message, bool p_verbose) {
    VERBOSE_CHECK(p_verbose);
    UtilityFunctions::print(p_message);
  }

  void debug_print_rich(const String &p_message, bool p_verbose) {
    VERBOSE_CHECK(p_verbose);
    UtilityFunctions::print_rich(p_message);
  }

  void debug_print_err(const String &p_message, bool p_verbose) {
    VERBOSE_CHECK(p_verbose);
    UtilityFunctions::printerr(p_message);
  }

  void debug_printt(const String &p_message, bool p_verbose) {
    VERBOSE_CHECK(p_verbose);
    UtilityFunctions::printt(p_message);
  }

  void debug_prints(const String &p_message, bool p_verbose) {
    VERBOSE_CHECK(p_verbose);
    UtilityFunctions::prints(p_message);
  }

  void debug_print_raw(const String &p_message, bool p_verbose) {
    VERBOSE_CHECK(p_verbose);
    UtilityFunctions::printraw(p_message);
  }

  void debug_print_verbose(const String &p_message, bool p_verbose) {
    VERBOSE_CHECK(p_verbose);
    UtilityFunctions::print_verbose(p_message);
  }

  void debug_push_error(const String &p_message, bool p_verbose) {
    VERBOSE_CHECK(p_verbose);
    UtilityFunctions::push_error(p_message);
  }

  void debug_push_warning(const String &p_message, bool p_verbose) {
    VERBOSE_CHECK(p_verbose);
    UtilityFunctions::push_warning(p_message);
  }

#else

  void Debug::_bind_methods() {}

#endif // DEBUG_ENABLED