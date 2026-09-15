#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"

class Autobinder : public godot::Resource {
  GDCLASS(Autobinder, godot::Resource);

protected:
  static void _bind_methods();

public:
  static void bind(godot::Node *p_container, godot::Resource *p_resource);
};

class ReactiveBinding : public godot::RefCounted {
  GDCLASS(ReactiveBinding, godot::RefCounted);

protected:
  static void _bind_methods();

public:
  static const constexpr char *const CALLBACK_REACTIVE_CHANGED = "on_reactive_changed";
  void on_reactive_changed(const godot::String &p_property_name, const godot::Node *p_node, const godot::Ref<godot::Resource> p_resource);

  static const constexpr char *const CALLBACK_WIDGET_CHANGED = "on_widget_changed";
  void on_widget_changed(const godot::Variant &p_value, const godot::Node *p_node, const godot::Ref<godot::Resource> p_resource);

  ReactiveBinding() = default;
  ~ReactiveBinding() = default;
};