#include "accessibility_settings.hpp"

#include "utils/macros.hpp"
#include "utils/error_macros.hpp"

using namespace godot;

void AccessibilitySettings::_bind_methods() {
	#define TEXT_SIZE_ITEM(m_id, m_name) BIND_ENUM_CONSTANT(m_id);
			TEXT_SIZE_ITEMS();
	#undef TEXT_SIZE_ITEM

	#define COLOR_BLINDNESS_ITEM(m_id, m_name) BIND_ENUM_CONSTANT(m_id)
		COLOR_BLINDNESS_ITEMS();
	#undef COLOR_BLINDNESS_ITEM



	SORUS_BIND_STATIC_METHOD(text_size_to_str);
	SORUS_BIND_STATIC_METHOD(color_blindness_to_str);
}

String AccessibilitySettings::text_size_to_str(TextSize p_text_size) {
	return ::to_string(p_text_size);
}

String AccessibilitySettings::color_blindness_to_str(ColorBlindness p_color_blindness) {
	return ::to_string(p_color_blindness);
}