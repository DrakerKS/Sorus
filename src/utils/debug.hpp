#pragma once

#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/variant/variant.hpp"

#ifdef DEBUG_ENABLED
  #define DEBUG_PRINT(m_message, ...) \
    debug_print(m_message __VA_OPT__(,) __VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__)

  #define DEBUG_LOG_CALL() \
    print_line(vformat("%s",__PRETTY_FUNCTION__))

  void debug_set_verbose(bool p_verbose);
  bool debug_get_verbose();
  void debug_print(const godot::String &p_message, bool p_verbose = false, const godot::String &p_file = godot::String(), int p_line = 0, const godot::String &p_func = godot::String());
  void debug_print_rich(const godot::String &p_message, bool p_verbose = false);
  void debug_print_err(const godot::String &p_message, bool p_verbose = false);
  void debug_printt(const godot::String &p_message, bool p_verbose = false);
  void debug_prints(const godot::String &p_message, bool p_verbose = false);
  void debug_print_raw(const godot::String &p_message, bool p_verbose = false);
  void debug_print_verbose(const godot::String &p_message, bool p_verbose = false);
  void debug_push_error(const godot::String &p_message, bool p_verbose = false);
  void debug_push_warning(const godot::String &p_message, bool p_verbose = false);
#else
  #define debug_set_verbose(m_verbose) ((void)0)
  #define debug_get_verbose() ((void)0)
  #define debug_print(m_message, ...) ((void)0)
  #define debug_print_rich(m_message, ...) ((void)0)
  #define debug_print_err(m_message, ...) ((void)0)
  #define debug_printt(m_message, ...) ((void)0)
  #define debug_prints(m_message, ...) ((void)0)
  #define debug_print_raw(m_message, ...) ((void)0)
  #define debug_print_verbose(m_message, ...) ((void)0)
  #define debug_push_error(m_message, ...) ((void)0)
  #define debug_push_warning(m_message, ...) ((void)0)
#endif

// Separators
#define SEPARATOR               "----------------------------------------------------------------------------------------------------"
#define PADDING(m_str)          "---------- " m_str " ----------"
// Font modifiers
#define BOLD(m_str)             "[b]" m_str "[/b]"
#define ITALIC(m_str)           "[i]" m_str "[/i]"
#define UNDERLINED(m_str)       "[u]" m_str "[/u]"
#define STRIKED(m_str)          "[s]" m_str "[/s]"
// Color modifiers
#define COLOR_RED(m_str)        "[color=#fb4934ff]" m_str "[/color]"
#define COLOR_PINK(m_str)       "[color=#d3869bff]" m_str "[/color]"
#define COLOR_GREEN(m_str)      "[color=#63c259ff]" m_str "[/color]"
#define COLOR_OLIVE(m_str)      "[color=#b8bb26ff]" m_str "[/color]"
#define COLOR_LAVENDER(m_str)   "[color=#a3a3f5ff]" m_str "[/color]"
#define COLOR_BLUE(m_str)       "[color=#458588ff]" m_str "[/color]"
#define COLOR_YELLOW(m_str)     "[color=#fabd2fff]" m_str "[/color]"
#define COLOR_WHITE(m_str)      "[color=#eadab1ff]" m_str "[/color]"

inline godot::String INDENT(const godot::String& p_string, int p_amount) {
  return godot::String(" ").repeat(p_amount * 2) + p_string;
}

class Debug : public godot::Object {
  GDCLASS(Debug, godot::Object);

protected:
  static void _bind_methods();

#ifdef DEBUG_ENABLED
private:
  inline static bool verbose = false;

  friend void debug_set_verbose(bool p_verbose);
  friend bool debug_get_verbose();
  friend void debug_print(const godot::String &p_message, bool p_verbose);
  friend void debug_print_rich(const godot::String &p_message, bool p_verbose);
  friend void debug_print_err(const godot::String &p_message, bool p_verbose);
  friend void debug_printt(const godot::String &p_message, bool p_verbose);
  friend void debug_prints(const godot::String &p_message, bool p_verbose);
  friend void debug_print_raw(const godot::String &p_message, bool p_verbose);
  friend void debug_print_verbose(const godot::String &p_message, bool p_verbose);
  friend void debug_push_error(const godot::String &p_message, bool p_verbose);
  friend void debug_push_warning(const godot::String &p_message, bool p_verbose);

  static void set_verbose(bool p_verbose) {
    debug_set_verbose(p_verbose);
  }

  static bool get_verbose() {
    return debug_get_verbose();
  }
#endif // DEBUG_ENABLED
};