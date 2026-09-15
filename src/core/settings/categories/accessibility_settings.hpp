#pragma once

#include "godot_cpp/classes/wrapped.hpp"
#include "reactive/reactive.hpp"

class AccessibilitySettings : public godot::Resource {
  GDCLASS(AccessibilitySettings, godot::Resource);

  public:
    #define TEXT_SIZE_HINT_STR "Small,Normal,Large"
    #define TEXT_SIZE_ITEMS() \
      TEXT_SIZE_ITEM(TEXT_SIZE_SMALL, "Small") \
      TEXT_SIZE_ITEM(TEXT_SIZE_NORMAL, "Normal") \
      TEXT_SIZE_ITEM(TEXT_SIZE_LARGE, "Large") \
      TEXT_SIZE_ITEM(TEXT_SIZE_MAX, "Unknown")

    enum TextSize {
      #define TEXT_SIZE_ITEM(m_id, m_name) m_id,
        TEXT_SIZE_ITEMS()
      #undef TEXT_SIZE_ITEM
    };

    #define COLOR_BLINDNESS_HINT_STR "None,Protanopia,Deuteranopia,Tritanopia"
    #define COLOR_BLINDNESS_ITEMS() \
      COLOR_BLINDNESS_ITEM(COLOR_BLINDNESS_NONE, "None") \
      COLOR_BLINDNESS_ITEM(COLOR_BLINDNESS_PROTANOPIA, "Protanopia") \
      COLOR_BLINDNESS_ITEM(COLOR_BLINDNESS_DEUTERANOPIA, "Deuteranopia") \
      COLOR_BLINDNESS_ITEM(COLOR_BLINDNESS_TRITANOPIA, "Tritanopia") \
      COLOR_BLINDNESS_ITEM(COLOR_BLINDNESS_MAX, "Unknown")

    enum ColorBlindness{
      #define COLOR_BLINDNESS_ITEM(m_id, m_name) m_id,
        COLOR_BLINDNESS_ITEMS()
      #undef COLOR_BLINDNESS_ITEM
    };

  protected:
    static void _bind_methods();

  public:
    static godot::String text_size_to_str(TextSize p_text_size);
    static godot::String color_blindness_to_str(ColorBlindness p_color_blindness);

    // void copy_from(const godot::Ref<AccessibilitySettings> p_accessibility_settings);
    // static godot::Ref<AccessibilitySettings> create_default();

    AccessibilitySettings() = default;
    ~AccessibilitySettings() = default;
};

VARIANT_ENUM_CAST(AccessibilitySettings::TextSize);
VARIANT_ENUM_CAST(AccessibilitySettings::ColorBlindness);

inline constexpr const char *to_string(AccessibilitySettings::TextSize p_text_size) {
  switch (p_text_size) {
    #define TEXT_SIZE_ITEM(m_id, m_name) case AccessibilitySettings::m_id: return m_name;
      TEXT_SIZE_ITEMS()
    #undef TEXT_SIZE_ITEM
    default: return "Unknown";
  }
}

inline constexpr const char *to_string(AccessibilitySettings::ColorBlindness p_color_blindness) {
  switch (p_color_blindness) {
    #define COLOR_BLINDNESS_ITEM(m_id, m_name) case AccessibilitySettings::m_id: return m_name;
      COLOR_BLINDNESS_ITEMS()
    #undef COLOR_BLINDNESS_ITEM
    default: return "Unknown";
  }
}
