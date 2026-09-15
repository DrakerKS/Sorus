#include "reactive.hpp"

using namespace godot;

// +----------------------------------------------------------------------------------------+
// |======================================= REACTIVE =======================================|
// +----------------------------------------------------------------------------------------+

void Reactive::_bind_methods() {
  struct MethodInfo signal_info = 
    MethodInfo(
      Reactive::CHANGED,
      
      PropertyInfo(
        Variant::NIL,
        "value"
      )
    );
    signal_info.default_arguments.push_back(DEFVAL(Variant::NIL));

  ADD_SIGNAL(signal_info);
}

// +----------------------------------------------------------------------------------------+
// |=================================== REACTIVE_VARIANT ===================================|
// +----------------------------------------------------------------------------------------+

void ReactiveVariant::_bind_methods() {
  SORUS_BIND_METHOD(set);
  SORUS_BIND_METHOD(get);
  ADD_PROPERTY(
    PropertyInfo(
      Variant::NIL,
      VALUE_NAME
    ), 
    SORUS_METHOD_NAME(set), 
    SORUS_METHOD_NAME(get)
  );

  SORUS_BIND_METHOD(set_property_info);
  SORUS_BIND_METHOD(get_property_info);
  ADD_PROPERTY(
    PropertyInfo(
      Variant::OBJECT,
      DynamicPropertyInfo::MEMBER_PROPERTY_INFO,
      PROPERTY_HINT_RESOURCE_TYPE,
      DynamicPropertyInfo::get_class_static(),
      PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT
    ), 
    SORUS_METHOD_NAME(set_property_info),
    SORUS_METHOD_NAME(get_property_info)
  );

  SORUS_BIND_METHOD(on_dynamic_property_info_changed);
}

void ReactiveVariant::on_dynamic_property_info_changed() {
  notify_property_list_changed();
}

void ReactiveVariant::set_property_info(const Ref<DynamicPropertyInfo> &p_dynamic_property_info) {
  if (dynamic_property_info == nullptr) {
    dynamic_property_info = p_dynamic_property_info;
    dynamic_property_info->connect(
      "changed", 
      {this, SORUS_METHOD_NAME(on_dynamic_property_info_changed)}
    );
  }
}

Ref<DynamicPropertyInfo> ReactiveVariant::get_property_info() const {
  return dynamic_property_info;
}

bool ReactiveVariant::_property_can_revert(const StringName &p_name) const {
  return (p_name == StringName(DynamicPropertyInfo::MEMBER_PROPERTY_INFO));
}

bool ReactiveVariant::_property_get_revert(const StringName &p_name, Variant &r_property) const {
  if (p_name == StringName(DynamicPropertyInfo::MEMBER_PROPERTY_INFO)) {
    if (dynamic_property_info.is_valid()) {
      r_property = Variant(dynamic_property_info);
      return true;
    }
  }
  return false;
}

void ReactiveVariant::_validate_property(PropertyInfo &p_property) const {
  if (dynamic_property_info.is_null()) {
    return;
  }

  if (p_property.name == StringName(VALUE_NAME) && p_property.name == dynamic_property_info->get_name()) {
    p_property.type = dynamic_property_info->get_type();
    p_property.hint = dynamic_property_info->get_hint();
    p_property.hint_string = dynamic_property_info->get_hint_string();
    p_property.usage = dynamic_property_info->get_usage();
  }
}