#include "settings_data.hpp"

using namespace godot;

void SettingsData::_bind_methods() {
  SORUS_BIND_PROPERTY_RESOURCE(accessibility_settings, AccessibilitySettings::get_class_static());

  SORUS_BIND_STATIC_METHOD(create_default);
}

Ref<SettingsData> SettingsData::create_default() {
  Ref<SettingsData> out;
  out.instantiate();

  // out->accessibility_settings = AccessibilitySettings::create_default();

  return out;
}