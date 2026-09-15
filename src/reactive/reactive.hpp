#pragma once

#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/core/property_info.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_extras/dynamic_property_info.hpp"

#include "utils/debug.hpp"
#include "utils/macros.hpp"
#include "utils/error_macros.hpp"

// +----------------------------------------------------------------------------------------+
// |======================================= REACTIVE =======================================|
// +----------------------------------------------------------------------------------------+

class Reactive : public godot::Resource {
  GDCLASS(Reactive, godot::Resource);

  protected:
    static void _bind_methods();

  public:
    static constexpr const char *const CHANGED = "reactive_changed";

    Reactive() = default;
    ~Reactive() = default;
};

// +----------------------------------------------------------------------------------------+
// |==================================== REACTIVE_VALUE ====================================|
// +----------------------------------------------------------------------------------------+

template<typename T>
class ReactiveValue : public Reactive {
  protected:
    T value{};

  public:
    static constexpr const char *const VALUE_NAME = "value";

    void set(const T &p_value) {
      if (value == p_value) {
        return;
      }

      value = p_value;
      emit_signal(Reactive::CHANGED);
    }
    T get() {
      return value;
    }

    ReactiveValue() = default;
    ~ReactiveValue() = default;
};

// +----------------------------------------------------------------------------------------+
// |=================================== REACTIVE_VARIANT ===================================|
// +----------------------------------------------------------------------------------------+

class ReactiveVariant : public ReactiveValue<godot::Variant> {
  GDCLASS(ReactiveVariant, ReactiveValue<godot::Variant>);

  private:
    godot::Ref<godot::DynamicPropertyInfo> dynamic_property_info;

  protected:
    static void _bind_methods();

  public:
    void on_dynamic_property_info_changed();

    void set_property_info(const godot::Ref<godot::DynamicPropertyInfo> &p_dynamic_property_info);
    godot::Ref<godot::DynamicPropertyInfo> get_property_info() const;

    bool _property_can_revert(const godot::StringName &p_name) const;
    bool _property_get_revert(const godot::StringName &p_name, godot::Variant &r_property) const;

    void _validate_property(godot::PropertyInfo &p_property) const;

    ReactiveVariant() = default;
    ~ReactiveVariant() = default;
};