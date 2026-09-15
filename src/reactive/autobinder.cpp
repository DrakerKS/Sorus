#include "autobinder.hpp"

#include "godot_cpp/classes/ref.hpp"

// #include "godot_cpp/classes/button.hpp"
// #include "godot_cpp/classes/check_box.hpp"
// #include "godot_cpp/classes/check_button.hpp"
// #include "godot_cpp/classes/code_edit.hpp"
// #include "godot_cpp/classes/color_picker.hpp"
// #include "godot_cpp/classes/color_picker_button.hpp"
// #include "godot_cpp/classes/color_rect.hpp"
// #include "godot_cpp/classes/h_slider.hpp"
// #include "godot_cpp/classes/label.hpp"
// #include "godot_cpp/classes/line_edit.hpp"
// #include "godot_cpp/classes/link_button.hpp"
// #include "godot_cpp/classes/nine_patch_rect.hpp"
// #include "godot_cpp/classes/option_button.hpp"
// #include "godot_cpp/classes/progress_bar.hpp"
// #include "godot_cpp/classes/rich_text_label.hpp"
// #include "godot_cpp/classes/spin_box.hpp"
// #include "godot_cpp/classes/tab_bar.hpp"
// #include "godot_cpp/classes/text_edit.hpp"
// #include "godot_cpp/classes/texture_button.hpp"
// #include "godot_cpp/classes/texture_progress_bar.hpp"
// #include "godot_cpp/classes/texture_rect.hpp"
// #include "godot_cpp/classes/v_slider.hpp"
// #include "godot_cpp/classes/video_stream_player.hpp"
// #include "godot_cpp/variant/array.hpp"
// #include "godot_cpp/variant/callable.hpp"
// #include "godot_cpp/variant/dictionary.hpp"
// #include "godot_cpp/variant/string.hpp"
// #include "godot_cpp/variant/typed_array.hpp"
// #include "godot_cpp/variant/variant.hpp"

#include "reactive/reactive.hpp"
#include "utils/debug.hpp"
#include "utils/macros.hpp"

using namespace godot;

// enum widget_target_idx {
//   TARGET_VALUE = 0,
//   TARGET_SIGNAL = 1,
//   TARGET_SIGNAL_ARGC = 2
// };

// static const Dictionary& get_widget_target_lookup() {
//   static Dictionary table;

//   if (table.is_empty()) {
//     // Color branch <Color> - 1-arg signal
//     table[ColorPickerButton::get_class_static()] = Array::make("color", "color_changed", 1);
//     table[ColorRect::get_class_static()] = Array::make("color", "", 0);
//     table[ColorPicker::get_class_static()] = Array::make("color", "color_changed", 1);
//     // Text related branch <String> - 1-arg signal
//     table[LineEdit::get_class_static()] = Array::make("text", "text_changed", 1);
//     table[TextEdit::get_class_static()] = Array::make("text", "text_changed", 1);
//     table[CodeEdit::get_class_static()] = Array::make("text", "text_changed", 1);
//     // Button branch <Bool> - 0-arg signal
//     table[Button::get_class_static()] = Array::make("button_pressed", "pressed", 0);
//     table[CheckBox::get_class_static()] = Array::make("button_pressed", "toggled", 0);
//     table[CheckButton::get_class_static()] = Array::make("button_pressed", "toggled", 0);
//     table[LinkButton::get_class_static()] = Array::make("button_pressed", "pressed", 0);
//     table[TextureButton::get_class_static()] = Array::make("button_pressed", "pressed", 0);
//     // Button branch <Int> - 1-arg signal
//     table[OptionButton::get_class_static()] = Array::make("selected", "item_selected", 1);
//     // Range branch <Float/Int> - 1-arg signal
//     table[ProgressBar::get_class_static()] = Array::make("value", "value_changed", 1);
//     table[SpinBox::get_class_static()] = Array::make("value", "value_changed", 1);
//     table[TextureProgressBar::get_class_static()] = Array::make("value", "value_changed", 1);
//     table[HSlider::get_class_static()] = Array::make("value", "value_changed", 1);
//     table[VSlider::get_class_static()] = Array::make("value", "value_changed", 1);
//     // ScrollBar branch <Float> - 1-arg signal
//     table[HScrollBar::get_class_static()] = Array::make("value", "value_changed", 1);
//     table[VScrollBar::get_class_static()] = Array::make("value", "value_changed", 1);
//     // Standalones <String> - no signal
//     table[Label::get_class_static()] = Array::make("text", "", 0);
//     table[RichTextLabel::get_class_static()] = Array::make("text", "", 0);
//     // Standalones <Texture2D> - 1-arg signal
//     table[NinePatchRect::get_class_static()] = Array::make("texture", "texture_changed", 1);
//     table[TextureRect::get_class_static()] = Array::make("texture", "", 0);
//     // Standalones <Int> - 1-arg signal
//     table[TabBar::get_class_static()] = Array::make("current_tab", "tab_changed", 1);
//     table[TabContainer::get_class_static()] = Array::make("current_tab", "tab_changed", 1);
//     // Standalones <VideoStream> - no signal
//     table[VideoStreamPlayer::get_class_static()] = Array::make("stream", "", 0);
//   }

//   return table;
// }

// static Ref<ReactiveBinding> & get_binding_dispatcher() {
//   static Ref<ReactiveBinding> binding_dispatcher;

//   if (binding_dispatcher.is_null()) {
//     binding_dispatcher.instantiate();
//   }

//   return binding_dispatcher;
// }

// void ReactiveBinding::_bind_methods() {
//   SORUS_BIND_METHOD(on_reactive_changed, "node", "resource");
//   SORUS_BIND_METHOD(on_widget_changed, "value", "node", "resource");
// }

// static void sync_widget(Node *p_node, const Ref<Resource> p_resource) {

//   const Dictionary &table = get_widget_target_lookup();

//   String class_name = p_node->get_class();
//   if (!table.has(class_name)){
//     return;
//   }

//   Variant value = p_resource->get(p_node->get_name());
//   if (value.get_type() == Variant::NIL){
//     return;
//   }

//   const Array &targets = Array(table[class_name]);
//   if (p_node->get(targets[TARGET_VALUE]) == value) {
//     return;
//   }

//   p_node->set_block_signals(true);
//   p_node->set(targets[TARGET_VALUE], value);
//   p_node->set_block_signals(false);

//   debug_print_rich(vformat(
//     COLOR_YELLOW("Widget syncing: %s with value '%s'"), p_node, value),
//     true
//   );
// }

// static void bind_reactive_signal(const Node *p_node, const Ref<Resource> p_resource) {
//   if (p_resource->has_connections(Reactive::CHANGED)) {
//     return;
//   }

//   Callable bound_callable = Callable(get_binding_dispatcher().ptr(), "on_reactive_changed").bind(p_node, p_resource);
//   p_resource->connect(Reactive::CHANGED, bound_callable);
//   debug_print_rich(vformat(
//     COLOR_LAVENDER("Reactive signal connection: %s -> %s: %s"), p_resource->get_class(), p_node, p_node->get_name())
//   );
// }

// static void bind_widget_signal(Node *p_node, const Ref<Resource> p_resource) {
//   const Dictionary &table = get_widget_target_lookup();

//   String widget_type = p_node->get_class();
//   if (!table.has(widget_type)) {
//     return;
//   }
  
//   const Array &targets = Array(table[widget_type]);
//   String target_signal_name = targets[TARGET_SIGNAL];
//   if (target_signal_name.is_empty() || !p_node->has_signal(target_signal_name)) {
//     return;
//   }

//   Variant prop = p_resource->get(p_node->get_name());
//   if (prop.get_type() == Variant::NIL) {
//     return;
//   }

//   Callable bound_callable = Callable(get_binding_dispatcher().ptr(), "on_widget_changed").bind(p_node, p_resource);
//   p_node->connect(target_signal_name, bound_callable);
//   debug_print_rich(vformat(
//     COLOR_BLUE("Widget signal connection: %s[%s] -> %s"), p_node, target_signal_name, p_resource->get_class())
//   );
// }

// // +----------------------------------------------------------------------------------------+
// // |====================================== AUTOBINDER ======================================|
// // +----------------------------------------------------------------------------------------+

void Autobinder::_bind_methods() {
  SORUS_BIND_STATIC_METHOD(bind, "container", "resource");
}

static Ref<Resource> get_inner_resource(const Node *p_node, const Ref<Resource> p_resource) {
  Variant prop = p_resource->get(p_node->get_name());

  if (prop.get_type() == Variant::NIL) {
    return p_resource;
  }

  Ref<Resource> out(prop);
  if (out.is_null()) {
    debug_print_err(vformat(("Null resource for '%s' property"), p_node->get_name()));
    return p_resource;
  }

  return out;
}

static void recursive_bind(Node *p_container, Resource *p_resource) {
  Ref<Resource> res_ref(p_resource);
  TypedArray<Node> children = p_container->get_children(false);

  for (int i = 0; i < children.size(); i++) {
    Node *node = Object::cast_to<Node>((Object*)(children[i]));
    if (node == nullptr){
      continue;
    }

    Ref<Resource> curr_res = get_inner_resource(node, res_ref);
    // bind_reactive_signal(node, curr_res);

    if (curr_res->is_class(Reactive::get_class_static())) {
      /** @todo containers as AccessibilitySettings or SettingsData shouldn't be Reactive no more, 
      only the actual props, though containers should still satisfy the name contract */
      curr_res->call("bind", node);
    }
    recursive_bind(node, curr_res.ptr());

    // sync_widget(node, curr_res);
    // bind_widget_signal(node, curr_res);
  }
}

void Autobinder::bind(Node *p_container, Resource *p_resource) {
  WARN_FAIL_COND_ED(p_container == nullptr, "Can't bind to a null node");
  WARN_FAIL_COND_ED(p_container == nullptr, "Can't bind to a null resource");

  debug_print_rich(vformat(
    COLOR_GREEN(PADDING("Autobinder::bind() process start at '%s' with resource '%s'")), p_container, p_resource->get_class())
  );

  recursive_bind(p_container, p_resource);

  debug_print_rich(vformat(
    COLOR_GREEN(PADDING("Autobinder::bind() process finished")))
  );
}

// // +----------------------------------------------------------------------------------------+
// // |=================================== REACTIVE_BINDING ===================================|
// // +----------------------------------------------------------------------------------------+


// void ReactiveBinding::on_reactive_changed(const String &p_property_name, const Node *p_node, const Ref<Resource> p_resource) {
//   Node *node = p_node->get_node_or_null(p_property_name);
//   if (node == nullptr) {
//     return;
//   }

//   const Variant &value = p_resource->get(p_property_name);
//   if (value.get_type() == Variant::NIL) {
//     debug_print_err(vformat(
//       "Can't obtain property '%s' from '%s' resource", 
//       p_property_name, p_resource->get_class())
//     );
//     return;
//   }

//   debug_print_rich(vformat(
//     COLOR_OLIVE("Reactive change: %s[%s] -> %s"), p_property_name, value, node), 
//     true
//   );
//   sync_widget(node, p_resource);
// }

// void ReactiveBinding::on_widget_changed(const Variant &p_value = Variant(), const Node *p_node = nullptr, const Ref<Resource> p_resource = Ref<Resource>()) {
//   const Dictionary &table = get_widget_target_lookup();
//   const Array &targets = Array(table[p_node->get_class()]);

//   const String &changed_property = targets[TARGET_VALUE];
//   const Variant &value = p_node->get(changed_property);

//   debug_print_rich(
//     vformat(COLOR_OLIVE("Widget change (%s[%s]): %s -> %s"), changed_property, value, p_node, p_resource->get_class()), 
//     true
//   );
//   p_resource->set(p_node->get_name(), value);
// }