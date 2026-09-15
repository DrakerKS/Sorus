#pragma once

#include "reactive/reactive.hpp"
#include "core/settings/categories/accessibility_settings.hpp"

class SettingsData : public godot::Resource {
  GDCLASS(SettingsData, godot::Resource);

  protected:
    static void _bind_methods();

  private:
    godot::Ref<AccessibilitySettings> accessibility_settings;

  public:
    static godot::Ref<SettingsData> create_default();

    SORUS_MEMBER_SETTER(const godot::Ref<AccessibilitySettings> &, accessibility_settings);
    SORUS_MEMBER_GETTER(godot::Ref<AccessibilitySettings>, accessibility_settings);
};