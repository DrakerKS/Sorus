#include "dynamic_property_info.hpp"

#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/editor_interface.hpp"
#include "godot_cpp/classes/editor_inspector.hpp"

#include "godot_cpp/variant/callable_method_pointer.hpp"

#include "utils/macros.hpp"
#include "utils/error_macros.hpp"
#include "utils/debug.hpp"
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

using namespace godot;

static String get_variant_type_hint_string() {
  static String result = "";

  if (result.is_empty()) {
    for (int i = 0; i < Variant::VARIANT_MAX; i++) {
      if (i > 0) {
        result += ",";
      }

      result += Variant::get_type_name(static_cast<Variant::Type>(i));
    }
  }

  return result;
}

struct PropertyHintEntry {
  PropertyHint hint;
  const char *name;
};

static constexpr PropertyHintEntry property_hints[] = {
  { PROPERTY_HINT_NONE, "None" },
  { PROPERTY_HINT_RANGE, "Range" },
  { PROPERTY_HINT_ENUM, "Enum" },
  { PROPERTY_HINT_ENUM_SUGGESTION, "EnumSuggestion" },
  { PROPERTY_HINT_EXP_EASING, "ExpEasing" },
  { PROPERTY_HINT_LINK, "Link" },
  { PROPERTY_HINT_FLAGS, "Flags" },
  { PROPERTY_HINT_LAYERS_2D_RENDER, "Layers2DRender" },
  { PROPERTY_HINT_LAYERS_2D_PHYSICS, "Layers2DPhysics" },
  { PROPERTY_HINT_LAYERS_2D_NAVIGATION, "Layers2DNavigation" },
  { PROPERTY_HINT_LAYERS_3D_RENDER, "Layers3DRender" },
  { PROPERTY_HINT_LAYERS_3D_PHYSICS, "Layers3DPhysics" },
  { PROPERTY_HINT_LAYERS_3D_NAVIGATION, "Layers3DNavigation" },
  { PROPERTY_HINT_LAYERS_AVOIDANCE, "LayersAvoidance" },
  { PROPERTY_HINT_FILE, "File" },
  { PROPERTY_HINT_DIR, "Dir" },
  { PROPERTY_HINT_GLOBAL_FILE, "GlobalFile" },
  { PROPERTY_HINT_GLOBAL_DIR, "GlobalDir" },
  { PROPERTY_HINT_RESOURCE_TYPE, "ResourceType" },
  { PROPERTY_HINT_MULTILINE_TEXT, "MultilineText" },
  { PROPERTY_HINT_EXPRESSION, "Expression" },
  { PROPERTY_HINT_PLACEHOLDER_TEXT, "PlaceholderText" },
  { PROPERTY_HINT_COLOR_NO_ALPHA, "ColorNoAlpha" },
  { PROPERTY_HINT_OBJECT_ID, "ObjectId" },
  { PROPERTY_HINT_TYPE_STRING, "TypeString" },
  { PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE, "NodePathToEditedNode" },
  { PROPERTY_HINT_OBJECT_TOO_BIG, "ObjectTooBig" },
  { PROPERTY_HINT_NODE_PATH_VALID_TYPES, "NodePathValidTypes" },
  { PROPERTY_HINT_SAVE_FILE, "SaveFile" },
  { PROPERTY_HINT_GLOBAL_SAVE_FILE, "GlobalSaveFile" },
  { PROPERTY_HINT_INT_IS_OBJECTID, "IntIsObjectId" },
  { PROPERTY_HINT_INT_IS_POINTER, "IntIsPointer" },
  { PROPERTY_HINT_ARRAY_TYPE, "ArrayType" },
  { PROPERTY_HINT_DICTIONARY_TYPE, "DictionaryType" },
  { PROPERTY_HINT_LOCALE_ID, "LocaleId" },
  { PROPERTY_HINT_LOCALIZABLE_STRING, "LocalizableString" },
  { PROPERTY_HINT_NODE_TYPE, "NodeType" },
  { PROPERTY_HINT_HIDE_QUATERNION_EDIT, "HideQuaternionEdit" },
  { PROPERTY_HINT_PASSWORD, "Password" },
  { PROPERTY_HINT_TOOL_BUTTON, "ToolButton" },
  { PROPERTY_HINT_ONESHOT, "Oneshot" },
  { PROPERTY_HINT_GROUP_ENABLE, "GroupEnable" },
  { PROPERTY_HINT_INPUT_NAME, "InputName" },
  { PROPERTY_HINT_FILE_PATH, "FilePath" }
};

static String get_property_hint_hint_string() {
  static String result {};

  if (result.is_empty()) {
    for (auto &entry : property_hints) {
      if (not result.is_empty()) {
        result += ",";
      }

      result += entry.name;
    }
  }

  return result;
}

struct PropertyUsageEntry {
  PropertyUsageFlags usage;
  const char *name;
};

static constexpr PropertyUsageEntry property_usage_flags[] = {
  // { PROPERTY_USAGE_NONE, "None" },
  { PROPERTY_USAGE_DEFAULT, "Default" },
  { PROPERTY_USAGE_STORAGE, "Storage" },
  { PROPERTY_USAGE_EDITOR, "Editor" },
  { PROPERTY_USAGE_INTERNAL, "Internal" },
  { PROPERTY_USAGE_CHECKABLE, "Checkable" },
  { PROPERTY_USAGE_CHECKED, "Checked" },
  { PROPERTY_USAGE_GROUP, "Group" },
  { PROPERTY_USAGE_CATEGORY, "Category" },
  { PROPERTY_USAGE_SUBGROUP, "Subgroup" },
  { PROPERTY_USAGE_CLASS_IS_BITFIELD, "ClassIsBitfield" },
  { PROPERTY_USAGE_NO_INSTANCE_STATE, "NoInstanceState" },
  { PROPERTY_USAGE_RESTART_IF_CHANGED, "RestartIfChanged" },
  { PROPERTY_USAGE_SCRIPT_VARIABLE, "ScriptVariable" },
  { PROPERTY_USAGE_STORE_IF_NULL, "StoreIfNull" },
  { PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED, "UpdateAllIfModified" },
  { PROPERTY_USAGE_SCRIPT_DEFAULT_VALUE, "ScriptDefaultValue" },
  { PROPERTY_USAGE_CLASS_IS_ENUM, "ClassIsEnum" },
  { PROPERTY_USAGE_NIL_IS_VARIANT, "NilIsVariant" },
  { PROPERTY_USAGE_ARRAY, "Array" },
  { PROPERTY_USAGE_ALWAYS_DUPLICATE, "AlwaysDuplicate" },
  { PROPERTY_USAGE_NEVER_DUPLICATE, "NeverDuplicate" },
  { PROPERTY_USAGE_HIGH_END_GFX, "HighEndGfx" },
  { PROPERTY_USAGE_NODE_PATH_FROM_SCENE_ROOT, "NodePathFromSceneRoot" },
  { PROPERTY_USAGE_RESOURCE_NOT_PERSISTENT, "ResourceNotPersistent" },
  { PROPERTY_USAGE_KEYING_INCREMENTS, "KeyingIncrements" },
  { PROPERTY_USAGE_DEFERRED_SET_RESOURCE, "DeferredSetResource" },
  { PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT, "EditorInstantiateObject" },
  { PROPERTY_USAGE_EDITOR_BASIC_SETTING, "EditorBasicSetting" },
  { PROPERTY_USAGE_READ_ONLY, "ReadOnly" },
  { PROPERTY_USAGE_SECRET, "Secret" },
};

static String get_property_usage_flags_hint_string() {
  static String result {};

  if (result.is_empty()) {
    for (auto &entry : property_usage_flags) {
      if (not result.is_empty()) {
        result += ",";
      }

      result += entry.name;
      result += ":";
      result += String::num_uint64(entry.usage);
    }
  }

  return result;
}

// +-----------------------------------------------------------------------------------------+
// |================================= DYNAMIC_PROPERTY_INFO =================================|
// +-----------------------------------------------------------------------------------------+

void DynamicPropertyInfo::_bind_methods() {
  SORUS_BIND_PROPERTY_ENUM_STRING(property_select, PLACEHOLDER_PROPERTY_SELECT);
  SORUS_BIND_PROPERTY_ENUM(type, get_variant_type_hint_string());
  SORUS_BIND_PROPERTY_ENUM(hint,get_property_hint_hint_string());
  SORUS_BIND_PROPERTY_STRING(hint_string);

  ClassDB::add_property_group(DynamicPropertyInfo::get_class_static(), "Usage bitfield", "usage");
  SORUS_BIND_PROPERTY_FLAGS(usage, get_property_usage_flags_hint_string());

  SORUS_BIND_PROPERTY_DICTIONARY(
    property_info_dict,
    vformat(
      "%d:;%d/%d:%s", 
      Variant::Type::STRING_NAME,
      Variant::Type::OBJECT,
      PROPERTY_HINT_RESOURCE_TYPE,
      DynamicPropertyInfo::get_class_static()
    ),
    PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY
  );
}

void DynamicPropertyInfo::_validate_property(PropertyInfo &p_property) const {
  if (not is_root) {
    if (p_property.name == StringName{member_property_select} || p_property.name == StringName{SORUS_MEMBER_NAME(property_info_dict)}) {
      p_property.usage = PROPERTY_USAGE_NONE;
    }
    return;
  }

  debug_print_rich(vformat("VALIDATE >>> %s[%s]", this, p_property.name), true);

  if (p_property.name != StringName(member_property_select)) {
    if (this->get_property_select() == StringName{PLACEHOLDER_PROPERTY_SELECT}) {
      p_property.usage = p_property.usage | PROPERTY_USAGE_READ_ONLY;
    }
    return;
  }

  String hint_string = get_property_names_hint_string();
  p_property.hint_string = hint_string;
}

String DynamicPropertyInfo::_to_string() const {
  return vformat("%s<%s#%s>", 
    this->get_class_static(), 
    this->get_parent_class_static(), 
    String::num_uint64(get_instance_id())
  );
}

void DynamicPropertyInfo::set_property_info_dict(Dictionary p_property_info_dict) {
  property_info_dict = p_property_info_dict;
}

Dictionary DynamicPropertyInfo::get_property_info_dict() const {
  return property_info_dict;
}

String DynamicPropertyInfo::get_property_names_hint_string() const {
  Array keys = property_info_dict.keys();
  String out {};

  for (int i = 0; i < keys.size(); i++) {
    if (not out.is_empty()) {
      out += ",";
    }

    out += (StringName)keys[i];
  }

  return out;
}

#define DPI_SETTER(m_type, m_member) \
  void DynamicPropertyInfo::SORUS_SETTER_TOKEN(m_member)(m_type P_TOKEN(m_member)) { \
    if (m_member == P_TOKEN(m_member)) \
      return; \
    m_member = P_TOKEN(m_member); \
  } END_MACRO()

#define DPI_GETTER(m_type, m_member) \
  m_type DynamicPropertyInfo::SORUS_GETTER_TOKEN(m_member)() const { \
    return static_cast<m_type>(m_member); \
  } END_MACRO()

    DPI_SETTER(Variant::Type, type);
    DPI_GETTER(Variant::Type, type);

    void DynamicPropertyInfo::set_property_select(StringName p_property_select) {
      if (property_select == p_property_select) {
        return;
      }
      property_select = p_property_select;

      Ref<DynamicPropertyInfo> serialized_dpi = 
        property_info_dict.get(
          p_property_select, 
          nullptr
        );

      if (serialized_dpi.is_valid()) {
        type = serialized_dpi->get_type();
        hint = serialized_dpi->get_hint();
        hint_string = serialized_dpi->get_hint_string();
        usage = serialized_dpi->get_usage();
      }

      if (is_root) {
        this->emit_changed();
      }
    }

    StringName DynamicPropertyInfo::get_property_select() const {
      return property_select;
    }

    DPI_SETTER(uint32_t, hint);
    DPI_GETTER(uint32_t, hint);

    DPI_SETTER(String, hint_string);
    DPI_GETTER(String, hint_string);

    DPI_SETTER(uint32_t, usage);
    DPI_GETTER(uint32_t, usage);

#undef DPI_SETTER
#undef DPI_GETTER

// +----------------------------------------------------------------------------------------+
// |======================== DYNAMIC_PROPERTY_INFO_INSPECTOR_PLUGIN ========================|
// +----------------------------------------------------------------------------------------+

static const Variant *get_default_variant_values() {
  struct DefaultVariantValues{
    Variant values[Variant::Type::VARIANT_MAX];

    DefaultVariantValues () {
      values[Variant::Type::NIL] = Variant();

      values[Variant::Type::BOOL] = false;
      values[Variant::Type::INT] = 0;
      values[Variant::Type::FLOAT] = 0.0f;
      values[Variant::Type::STRING] = String();

      values[Variant::Type::VECTOR2] = Vector2();
      values[Variant::Type::VECTOR2I] = Vector2i();
      values[Variant::Type::RECT2] = Rect2();
      values[Variant::Type::RECT2I] = Rect2i();
      values[Variant::Type::VECTOR3] = Vector3();
      values[Variant::Type::VECTOR3I] = Vector3i();
      values[Variant::Type::TRANSFORM2D] = Transform2D();
      values[Variant::Type::VECTOR4] = Vector4();
      values[Variant::Type::VECTOR4I] = Vector4i();
      values[Variant::Type::PLANE] = Plane();
      values[Variant::Type::QUATERNION] = Quaternion();
      values[Variant::Type::AABB] = AABB();
      values[Variant::Type::BASIS] = Basis();
      values[Variant::Type::TRANSFORM3D] = Transform3D();
      values[Variant::Type::PROJECTION] = Projection();

      values[Variant::Type::COLOR] = Color();
      values[Variant::Type::STRING_NAME] = StringName();
      values[Variant::Type::NODE_PATH] = NodePath();
      values[Variant::Type::RID] = RID();
      values[Variant::Type::OBJECT] = Variant();
      values[Variant::Type::CALLABLE] = Callable();
      values[Variant::Type::SIGNAL] = Signal();
      values[Variant::Type::DICTIONARY] = Dictionary();
      values[Variant::Type::ARRAY] = Array();

      values[Variant::Type::PACKED_BYTE_ARRAY] = PackedByteArray();
      values[Variant::Type::PACKED_INT32_ARRAY] = PackedInt32Array();
      values[Variant::Type::PACKED_INT64_ARRAY] = PackedInt64Array();
      values[Variant::Type::PACKED_FLOAT32_ARRAY] = PackedFloat32Array();
      values[Variant::Type::PACKED_FLOAT64_ARRAY] = PackedFloat64Array();
      values[Variant::Type::PACKED_STRING_ARRAY] = PackedStringArray();
      values[Variant::Type::PACKED_VECTOR2_ARRAY] = PackedVector2Array();
      values[Variant::Type::PACKED_VECTOR3_ARRAY] = PackedVector3Array();
      values[Variant::Type::PACKED_COLOR_ARRAY] = PackedColorArray();
      values[Variant::Type::PACKED_VECTOR4_ARRAY] = PackedVector4Array();
    }
  };
	
  static struct DefaultVariantValues default_variant_values;
	return default_variant_values.values;
}

/**
Rules:

  Hint changed, no hint string reffers to the case where hint is changed to X hint value and hint_string is empty
  Hint changed reffers to the case where hint is changed to X hint value, wether or not the hint_string is valid comes later in the case, this case always has priority
  Hint string changed reffers to the case where hint_string has been changed and hint has stayed the same since last time

  - RANGE:
    - Hint changed, no hint_string: not valid
    - Hint changed: max value in hint_string
    - Hint string changed: Check if current value is valid and return it if true, as hint changed if not
  - ENUM:
    - Hint changed, no hint_string: not valid
    - Hint changed: first value in hint_string
    - Hint string changed: Check if current value is valid and return it if true, as hint changed if not
  - FLAGS:
    - Hint changed, no hint_string: not valid
    - Hint changed: set to 0 no flags active
    - Hint string changed: Check if current value is valid and return it if true, as hint changed if not
  - COLOR_NO_ALPHA:
    - Hint changed, no hint_string: ? Documentation says nothing about hint_string: "● PROPERTY_HINT_COLOR_NO_ALPHA = 21 Sugiere que una propiedad de tipo Color debe editarse sin afectar su transparencia (Color.a no es editable)."
    - Hint changed: set to Color()
    - Hint string changed: ?
*/

static Variant normalize_range(const PropertyInfo &p_property_info, const Variant &p_value) {
  Variant::Type type = p_property_info.type;
  if (type != Variant::INT && type != Variant::FLOAT) {
    return {};
  }

  /** From range hint_string "min,max,..." ensure at least min and max values */
  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.size() < 2) {
    return {};
  }

  #define NORMALIZE_TYPE(m_type) do { \
    if (not hint_string_entries[0].JOIN(is_valid,m_type)() || not hint_string_entries[1].JOIN(is_valid,m_type)()) { \
      return {}; \
    } \
    if (p_value == Variant()) { \
      return hint_string_entries[1].JOIN(to,m_type)(); \
    } \
    return CLAMP((m_type)p_value, hint_string_entries[0].JOIN(to,m_type)(), hint_string_entries[1].JOIN(to,m_type)()); \
   } while(false)

  if (type == Variant::INT) {
    NORMALIZE_TYPE(int);
  } else {
    NORMALIZE_TYPE(float);
  }

  #undef NORMALIZE_TYPE

  return {};
}

static Variant normalize_enum(const PropertyInfo &p_property_info, const Variant &p_value) {
  Variant::Type type = p_property_info.type;
  if (type != Variant::INT && type != Variant::STRING && type != Variant::STRING_NAME) {
    return {};
  }

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {};
  }

  String first_entry = hint_string_entries[0];
  int64_t first_entry_numeric;

  if (type != Variant::INT) {
    if (hint_string_entries.has(String(p_value))) {
      return {p_value};
    }

    return first_entry;
  }

  /** In hint_string's PackedStringArray ["a","b:10","c","d","e:10", "f:20"] get numerics by slicing every 1th (0-wise) element */
  int64_t last_visited;
  bool first_visit = true;

  for (String &entry : hint_string_entries) {
    String entry_numeric = entry.get_slice(":", 1);

    if (entry_numeric == entry || entry_numeric.is_empty()) {
      last_visited = first_visit ? 0 : last_visited + 1;
    } else {
      last_visited = entry_numeric.to_int();
    }

    if (first_visit) {
      first_entry_numeric = last_visited;
      first_visit = false;
    }

    if ((int64_t)p_value == last_visited) {
      return {p_value};
    }
  }

  return {first_entry_numeric};
}

static Variant normalize_flags(const PropertyInfo &p_property_info, const Variant &p_value) {
  if (p_property_info.type != Variant::INT) {
    return {};
  }

  if (p_value == Variant()) {
    return {0};
  }

  /** For flags hint strings entry position is absolute truth if no specific value is provided "A:16,B,C" @A = 16, B = 2, C = 4 */
  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {};
  }

  uint32_t current_bit = 0;
  uint32_t bit_mask = 0;
  for (String& entry : hint_string_entries) {
    String bit_value_str = entry.get_slice(":", 1);
    uint32_t bit_value;

    if (bit_value_str == entry || bit_value_str.is_empty()) {
      bit_value = 1u << current_bit;
    } else {
      bit_value = bit_value_str.to_int();
    }

    bit_mask |= (uint32_t)p_value & bit_value;
    current_bit++;
  }

  return {bit_mask};
}

static Variant normalize_value(const PropertyInfo &p_property_info, const Variant& p_value = Variant()) {
  Variant out {};

  Variant::Type type = p_property_info.type;
  if (type == Variant::ARRAY) {
    /** Call some shi for arrays */
  }

  if (type == Variant::DICTIONARY) {
    /** Call some shi for dictionaries */
  }

  if (type == Variant::OBJECT) {
    /** Call some shi for objects */
  }

  switch (p_property_info.hint) {
    case PROPERTY_HINT_RANGE:
      out = normalize_range(p_property_info, p_value);
      break;
    case PROPERTY_HINT_ENUM:
      out = normalize_enum(p_property_info, p_value);
      break;
    case PROPERTY_HINT_FLAGS:
      out = normalize_flags(p_property_info, p_value);
      break;
    /** @todo handle other hint cases */
    default:
      /** @todo maybe if p_value's type is already p_property_info.type is better to return the same value */
      break;
  }

  if (out != Variant()) {
    return out;
  }

  return get_default_variant_values()[p_property_info.type];
}

bool DynamicPropertyInfoInspectorPlugin::find_root_dpi(const Object *p_object, Ref<DynamicPropertyInfo> *r_dpi, StringName *r_dpi_name) {
  if (p_object == nullptr) {
    return false;
  }

  TypedArray<Dictionary> property_list  = p_object->get_property_list();

  for (int i = property_list.size() - 1; i >= 0; i--) {
    Dictionary current_prop = property_list[i];

    if (StringName{current_prop[N_HINT_STRING]} == DynamicPropertyInfo::get_class_static()) {
      Variant prop = p_object->get(current_prop[N_NAME]);
      if (r_dpi) {
        *r_dpi = prop;
      }
      if (r_dpi_name) {
        *r_dpi_name = current_prop[N_NAME];
      }
      
      return true;
    }
  }
  
  return false;
}

void DynamicPropertyInfoInspectorPlugin::on_dynamic_property_info_changed(Object *p_object, const Ref<DynamicPropertyInfo> &root_dpi) {
  if (p_object == nullptr) {
    return;
  }

  if (root_dpi.is_valid()) {
    Ref<DynamicPropertyInfo> previous = root_dpi->property_info_dict.get(root_dpi->property_select, nullptr);
    if (previous.is_null()) {
      return;
    }

    PropertyInfo root_pi = root_dpi->get_property_info();
    String property = root_dpi->get_property_select();

    if (root_dpi->type != previous->type || root_dpi->hint != previous->hint) {
      Variant new_value = normalize_value(root_pi);
      p_object->set(property, new_value);
    } 
    else if (root_dpi->hint_string != previous->hint_string) {
      Variant new_value = normalize_value(root_pi, p_object->get(property));
      p_object->set(property, new_value);
    }
  }

  p_object->call_deferred("notify_property_list_changed");
}

bool DynamicPropertyInfoInspectorPlugin::_can_handle(Object *p_object) const {
  debug_print_rich(vformat("CAN HANDLE >>> %s", p_object), true);

  if (p_object == nullptr) {
    return false;
  }

  if (creating_native_editor()) {
    return false;
  }

  if (p_object->is_class(DynamicPropertyInfo::get_class_static())) {
    return false;
  }

  return find_root_dpi(p_object);
}

void DynamicPropertyInfoInspectorPlugin::_parse_begin(Object *p_object) {
  if (p_object == nullptr) {
    return;
  }

  debug_print_rich(vformat("PARSE BEGIN >>> %s", p_object), true);

  root_dpi.unref();
  root_dpi_name = StringName{};

  if (not find_root_dpi(p_object, &root_dpi, &root_dpi_name)) {
    return;
  }

  if (root_dpi.is_null()) {
    root_dpi.instantiate();
    p_object->set(root_dpi_name, root_dpi);
  }

  Callable bound_callable = callable_mp_static(&DynamicPropertyInfoInspectorPlugin::on_dynamic_property_info_changed).bind(p_object, root_dpi);
  if (not root_dpi->is_connected(SIGNAL(root_dpi, changed), bound_callable)) {
    root_dpi->connect(SIGNAL(root_dpi, changed), bound_callable);
  }

  root_dpi->is_root = true;
}

bool DynamicPropertyInfoInspectorPlugin::_parse_property(Object *p_object, Variant::Type p_type, const String &p_name, PropertyHint p_hint, const String &p_hint_string, BitField<PropertyUsageFlags> p_usage, bool p_wide) { 
  if (not p_name.begins_with(STD_DYNAMIC_PROP_PREFIX)) {
    return false;
  }

  if (p_object == nullptr) {
    return false;
  }

  if (root_dpi.is_null()) {
    return false;
  }

  Ref<Resource> prop = p_object->get(p_name);
  if (prop.is_valid() && prop->is_class(DynamicPropertyInfo::get_class_static())) {
    Ref<DynamicPropertyInfo> extra_dpi = prop;

    if (extra_dpi != root_dpi) {
      Callable bound_callable = callable_mp_static(&DynamicPropertyInfoInspectorPlugin::on_dynamic_property_info_changed).bind(p_object, root_dpi);

      if (extra_dpi->is_connected(SIGNAL(extra_dpi, changed), bound_callable)) {
        debug_print_rich(vformat(
          COLOR_YELLOW("Previous '%s' resource disconnected, the '%s' instance will be used instead from now"), extra_dpi, root_dpi),
          true
        );
        extra_dpi->disconnect(SIGNAL(extra_dpi, changed), bound_callable);
      }
      extra_dpi->is_root = false;
    }

    return false;
  }

  debug_print_rich(vformat("PARSE PROPERTY >>> %s", p_name), true);

  Ref<DynamicPropertyInfo> target_dpi {root_dpi};
  if (p_name != root_dpi->get_property_select()) {
    target_dpi = root_dpi->get_property_info_dict().get(p_name, nullptr);
    if (target_dpi.is_null()) {
      target_dpi = Ref<DynamicPropertyInfo>(memnew(DynamicPropertyInfo({Variant::NIL, p_name})));
    }
  }

  Ref<DynamicPropertyInfo> serialized_dpi = target_dpi->duplicate(true);
  serialized_dpi->is_root = false;
  serialized_dpi->property_info_dict.clear();

  root_dpi->property_info_dict[p_name] = serialized_dpi; /** @todo old removed props will still be serialized */

  debug_print_rich(vformat(
    COLOR_GREEN("Dynamic property '%s' updated in '%s::%s' instance"), p_name, p_object, root_dpi),
    true
  );
  
  creating_native_editor() = true;

  EditorProperty *editor = EditorInspector::instantiate_property_editor(
    p_object,
    target_dpi->get_type(), 
    p_name, 
    static_cast<PropertyHint>(target_dpi->get_hint()),
    target_dpi->get_hint_string(),
    target_dpi->get_usage(),
    p_wide
  );

  creating_native_editor() = false;

  editor->set_object_and_property(p_object, p_name);
  this->add_property_editor(p_name, editor);

  return true;
}

void DynamicPropertyInfoInspectorPlugin::_on_submit() {
  if (root_dpi.is_valid() && root_dpi->is_root) {
    root_dpi->emit_changed();
  }
}

static Node* find_root_dpi_inspector_node(Object *p_object, const Ref<DynamicPropertyInfo> &p_root_dpi, Node *p_node) {
  Node *out = nullptr;

  if (p_node->get_class() == "EditorPropertyResource") {
    EditorProperty *target = Object::cast_to<EditorProperty>(p_node);
    StringName target_name = target->get_edited_property();

    Ref<DynamicPropertyInfo> dpi = p_object->get(target_name);

    if (dpi.is_valid() && dpi == p_root_dpi) {
      return Object::cast_to<Node>(target);
    }
  }

  TypedArray<Node> children =  p_node->get_children();

  for (int i = 0; i < children.size(); i++) {
    Node *child = Object::cast_to<Node>((Object*)(children[i]));
    if (child == nullptr){
      continue;
    }

    out = find_root_dpi_inspector_node(p_object, p_root_dpi, child);

    if (out != nullptr) {
      return out;
    }
  }

  return out;
}

void DynamicPropertyInfoInspectorPlugin::_parse_end(Object *p_object) {
  if (p_object == nullptr) {
    return;
  }

  Button *submit_button = memnew(Button);
  submit_button->set_text("Apply");

  Node* root_dpi_inspector_node = find_root_dpi_inspector_node(p_object, root_dpi, EditorInterface::get_singleton()->get_inspector());
  EditorProperty *x = Object::cast_to<EditorProperty>(root_dpi_inspector_node);
  if (root_dpi_inspector_node != nullptr) {
    submit_button->connect("pressed", callable_mp(this, &DynamicPropertyInfoInspectorPlugin::_on_submit));

    TypedArray<Node> root_dpi_inspector_node_children = root_dpi_inspector_node->get_children();
    Object::cast_to<Node>((Object*)root_dpi_inspector_node_children[1])->add_child(submit_button);
  } else {
    memdelete(submit_button);
  }

  debug_print_rich(vformat(COLOR_GREEN("%s - Inspector updated"), p_object));
}

// +-----------------------------------------------------------------------------------------+
// |========================== DYNAMIC_PROPERTY_INFO_EDITOR_PLUGIN ==========================|
// +-----------------------------------------------------------------------------------------+

void DynamicPropertyInfoEditorPlugin::_enter_tree() {
  inspector_plugin.instantiate();
  add_inspector_plugin(inspector_plugin);
}

void DynamicPropertyInfoEditorPlugin::_exit_tree() {
  if (inspector_plugin.is_valid()) {
    remove_inspector_plugin(inspector_plugin);
    inspector_plugin.unref();
  }
}