#include "dynamic_property_info.hpp"
#include "godot_extras/globals.hpp"

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/editor_interface.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/resource_uid.hpp"

#include "godot_cpp/variant/callable_method_pointer.hpp"

#include "godot_cpp/templates/hash_map.hpp"
#include "godot_cpp/templates/pair.hpp"
#include "godot_cpp/templates/vector.hpp"

#include "utils/macros.hpp"
#include "utils/error_macros.hpp"
#include "utils/debug.hpp"
#include <type_traits>

using namespace godot;

struct PropertyUsageEntry {
  PropertyUsageFlags usage;
  const char *name;
};

static constexpr PropertyUsageEntry property_usage_flags[] = {
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

// +-----------------------------------------------------------------------------------------+
// |================================= DYNAMIC_PROPERTY_INFO =================================|
// +-----------------------------------------------------------------------------------------+

static String get_variant_type_hint_string() {
  static String result = "";

  if (result.is_empty()) {
    for (int i = 0; i < Variant::VARIANT_MAX; i++) {
      if (i > 0) {
        result += ",";
      }

      result += Variant::get_type_name(static_cast<Variant::Type>(i));
      Variant::get_type_by_name("INT");
    }
  }

  return result;
}

static String get_property_hint_hint_string() {
  static String hint_string {};
  if (not hint_string.is_empty()) {
    return hint_string;
  }

  Vector<NamedValue<PropertyHint>> entries = property_hint_named_values();
  // for (const NamedValue<PropertyHint> &entry : entries) {
  //   if (not hint_string.is_empty()) {
  //     hint_string += ",";
  //   }

  //   String name = entry.second;

  //   hint_string += name;
  //   hint_string += ":";
  //   hint_string += String::num_int64(entry.first);
  // }

  return hint_string;
}

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

void DynamicPropertyInfo::_bind_methods() {
  SORUS_BIND_PROPERTY_ENUM_STRING(property_select, PLACEHOLDER_PROPERTY_SELECT);
  SORUS_BIND_PROPERTY_ENUM(type, get_variant_type_hint_string());
  SORUS_BIND_PROPERTY_ENUM(hint,get_property_hint_hint_string());

  // SORUS_BIND_PROPERTY_STRING(hint_string);
  SORUS_BIND_PROPERTY_MULTILINE_TEXT(hint_string);

  ClassDB::add_property_group(DynamicPropertyInfo::get_class_static(), "Usage bitfield", member_usage);
  SORUS_BIND_PROPERTY_FLAGS(usage, get_property_usage_flags_hint_string());

  ClassDB::add_property_group(DynamicPropertyInfo::get_class_static(), "Properties", member_property_info_dict);
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

static Variant normalize_value(const PropertyInfo &p_property_info, const Object* p_object = nullptr, const Variant &p_value = Variant());

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

  if (p_property_info.hint == PROPERTY_HINT_ENUM_SUGGESTION && type == Variant::INT) {
    return {};
  }

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {};
  }

  String first_entry = hint_string_entries[0];

  if (type != Variant::INT) {
    if (hint_string_entries.has(String(p_value))) {
      return {p_value};
    }

    if (p_property_info.hint == PROPERTY_HINT_ENUM_SUGGESTION) {
      return {String()};
    }

    return first_entry;
  }

  /** In hint_string's PackedStringArray ["a","b:10","c","d","e:10", "f:20"] get numerics by slicing every 1th (0-wise) element */
  int64_t last_visited;
  bool first_visit = true;
  int64_t first_entry_numeric;

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

static Variant normalize_exp_easing(const PropertyInfo &p_property_info, const Variant &p_value) {
  if (p_property_info.type != Variant::FLOAT) { 
    return {};
  }

  if (p_value == Variant()) {
    return {0.0f};
  }

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",",false);
  if (hint_string_entries.has("positive_only") && (float)p_value < 0) {
    return Variant{};
  }

  return {(float)p_value};
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

static Variant normalize_file(const PropertyInfo &p_property_info, const Variant &p_value) {
  if (p_property_info.type != Variant::STRING) {
    return {};
  }

  if (p_value == Variant()) {
    return {};
  }

  String path = (String)p_value;
  if (path.begins_with("uid://")) {
    path = ResourceUID::uid_to_path(path);
  }

  if (not path.begins_with("/") && not path.begins_with("res://")) {
    return {};
  }

  Variant out = (p_property_info.hint == PROPERTY_HINT_GLOBAL_FILE || p_property_info.hint == PROPERTY_HINT_FILE_PATH) ? Variant{path} : Variant{p_value};

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {out};
  }

  for (String &entry : hint_string_entries) {
    String extension = entry.get_slice(".", 1);
    if (extension == entry || extension.is_empty()) {
      continue;
    }

    extension = "." + extension;
    if (entry.ends_with(extension)) {
      return {out};
    }
  }

  return {};
}

static Variant normalize_resource_type(const PropertyInfo &p_property_info, const Variant &p_value) {
  Variant::Type type = p_property_info.type;
  if (type != Variant::OBJECT) {
    return {};
  }

  if (p_value == Variant()) {
    return {};
  }

  Ref<Resource> out = p_value;
  if (out.is_null()) {
    return {};
  }

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {p_value};
  }

  for (String &entry : hint_string_entries) {
    if (out->is_class(entry)) {
      return {p_value};
    }
  }

  return {};
}

static Variant normalize_object_id(const PropertyInfo &p_property_info, const Variant &p_value) {
  /** @todo Implement */

  return {};
}

static Variant normalize_node_path_valid_types(const PropertyInfo &p_property_info, const Object* p_object, const Variant &p_value) {
  if (p_property_info.type != Variant::NODE_PATH) {
    return {};
  }

  if (p_value == Variant()) {
    return {};
  }

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {};
  }

  const Node *node = Object::cast_to<Node>(p_object);
  if (node == nullptr) {
    return {};
  }

  const Node *target = node->get_node_or_null((NodePath)p_value);
  if (target == nullptr) {
    return {};
  }

  for (String &entry : hint_string_entries) {
    if (target->is_class(entry)) {
      return {p_value};
    }
  }

  return {};
}

static Variant normalize_array_type(const PropertyInfo &p_property_info, const Object* p_object, const Variant &p_value) {
  if (p_property_info.type != Variant::ARRAY) {
    return {};
  }

  if (p_value == Variant()) {
    return {};
  }

  if (p_property_info.hint_string.is_empty()) {
    return {};
  }

  /**
    Example hint_strings: 
      subType/subTypeHint:subTypeHintString

      2/2:a,b,c,d -> int:Enum:a,b,c,d

    Not yet supported:
      28/31:1: -> Array:ArrayType:bool
      28:24/17:DynamicPropertyInfo -> Array:Object/ResourceType:DynamicPropertyInfo
      28:2/2:a,b,c,d -> Array:int/Enum:a,b,c,d
  */

  Variant::Type subtype;
  PropertyHint subtype_hint;
  String subtype_hint_string;

  int hint_subtype_separator = p_property_info.hint_string.find(":");
  if (hint_subtype_separator >= 0) {
    String subtype_string = p_property_info.hint_string.substr(0, hint_subtype_separator);
    int slash_pos = subtype_string.find("/");
    if (slash_pos >= 0) {
      subtype_hint = PropertyHint(subtype_string.substr(slash_pos + 1).to_int());
      subtype_string = subtype_string.substr(0, slash_pos);
    }

    subtype_hint_string = p_property_info.hint_string.substr(hint_subtype_separator + 1);
    subtype = Variant::Type(subtype_string.to_int());

    print_line("FLAG >>> ", subtype);
    print_line("FLAG >>> ", subtype_hint);
    print_line("FLAG >>> ", subtype_hint_string);
  } else {
    subtype = Variant::get_type_by_name(p_property_info.hint_string);
  
    if (subtype == Variant::VARIANT_MAX) {
      subtype = Variant::OBJECT;
      subtype_hint = PROPERTY_HINT_RESOURCE_TYPE;
      subtype_hint_string = p_property_info.hint_string;
    }
  }

  if (subtype == Variant::ARRAY || subtype == Variant::DICTIONARY) {
    
  }

  Array array = Array(p_value);
  if (array.is_empty()) {
    return {p_value};
  }

  for (int i = 0; i < array.size(); i++) {
     PropertyInfo sub_property_info = {
      subtype,
      String(p_property_info.name) + '[' + String::num_int64(i) + ']',
      subtype_hint,
      subtype_hint_string
    };

    Variant &curr = array[i];
    curr = normalize_value(sub_property_info, p_object, curr);
  }

  return {array};
}

static Variant normalize_string_type(const PropertyInfo &p_property_info, const Object* p_object, const Variant &p_value) {
  Variant::Type type = p_property_info.type;

  switch (type) {
    case Variant::ARRAY:
      return normalize_array_type(p_property_info, p_object, p_value);
      break;
    case Variant::DICTIONARY:
      break;
    case Variant::STRING:
    default:
      break;
  }

  return {};
}

static Variant normalize_node_type(const PropertyInfo &p_property_info, const Variant &p_value) {
  if (p_property_info.type != Variant::OBJECT) {
    return {};
  }

  if (p_value == Variant()) {
    return {};
  }

  Node *node = Object::cast_to<Node>((Object*)p_value);
  if (node == nullptr) {
    return {};
  }

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {p_value};
  }

  for (String &entry : hint_string_entries) {
    if (node->is_class(entry)) {
      return {p_value};
    }
  }

  return {};
}

static Variant normalize_value(const PropertyInfo &p_property_info, const Object* p_object /* = nullptr */, const Variant &p_value /* = Variant() */) {
  Variant out {};

  PropertyInfo pi {p_property_info};
  if (pi.type == Variant::ARRAY || pi.type == Variant::DICTIONARY) {
    if (pi.hint == PROPERTY_HINT_NONE) {
      pi.hint = PROPERTY_HINT_TYPE_STRING;
    }
  }

  Variant value {p_value};
  if (p_value == Variant() && p_object) {
    value = p_object->get(pi.name);
  }

  switch (pi.hint) {
    case PROPERTY_HINT_RANGE:
      out = normalize_range(pi, value);
      break;
    case PROPERTY_HINT_ENUM:
      out = normalize_enum(pi, value);
      break;
    case PROPERTY_HINT_ENUM_SUGGESTION:
      out = normalize_enum(pi, value);
      break;
    case PROPERTY_HINT_FLAGS:
      out = normalize_flags(pi, value);
      break;
    case PROPERTY_HINT_EXP_EASING:
      out = normalize_exp_easing(pi, value);
      break;
    case PROPERTY_HINT_FILE:
    case PROPERTY_HINT_GLOBAL_FILE:
    case PROPERTY_HINT_SAVE_FILE:
    case PROPERTY_HINT_GLOBAL_SAVE_FILE:
    case PROPERTY_HINT_FILE_PATH:
      out = normalize_file(pi, value);
      break;
    case PROPERTY_HINT_RESOURCE_TYPE:
      out = normalize_resource_type(pi, value);
      break;
    case PROPERTY_HINT_OBJECT_ID:
      /** @todo handle */
      break;
    case PROPERTY_HINT_TYPE_STRING:
      out = normalize_string_type(pi, p_object, value);
      break;
    case PROPERTY_HINT_NODE_PATH_VALID_TYPES:
      out = normalize_node_path_valid_types(pi, p_object, value);
      break;
    case PROPERTY_HINT_ARRAY_TYPE:
      out = normalize_array_type(pi, p_object, value);
      break;
    case PROPERTY_HINT_DICTIONARY_TYPE:
      /** @todo handle */
      break;
    case PROPERTY_HINT_NODE_TYPE:
      out = normalize_node_type(pi, value);
      break;
    case PROPERTY_HINT_INPUT_NAME:
      /** @todo handle */
      break;
    default:
      break;
  }

  if (out == Variant()) {
    out = get_default_variant_values()[pi.type];
  }

  bool use_quotes = pi.type == Variant::STRING || pi.type == Variant::STRING_NAME || pi.type == Variant::NODE_PATH;
  debug_print_rich(vformat(
    COLOR_GREEN("NORMALIZED VALUE [%s = %s] (%s(%s)/%s(%s))"), 
    pi.name, use_quotes? (Variant)vformat("\"%s\"", out) : out, Variant::get_type_name(pi.type), pi.type, "property_hint_lookup()[pi.hint]", pi.hint)
  );

  return out;
}

static bool find_root_dpi(const Object *p_object, Ref<DynamicPropertyInfo> *r_dpi = nullptr, StringName *r_dpi_name = nullptr) {
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

static size_t find_inspector_editor_properties(const String &p_property_name, TypedArray<EditorProperty> &r_ed_props, bool p_stop_at_first = false, Node *p_root_node = nullptr) {
  if (p_root_node == nullptr) {
    p_root_node = EditorInterface::get_singleton()->get_inspector();
  }

  if (p_stop_at_first && r_ed_props.size() > 0) {
    return r_ed_props.size();
  }

  EditorProperty *likely_target = Object::cast_to<EditorProperty>(p_root_node);
  if (likely_target != nullptr && likely_target->get_edited_property() == p_property_name) {
    r_ed_props.push_back(likely_target);
  } else {
    TypedArray<Node> children =  p_root_node->get_children();

    for (int i = 0; i < children.size(); i++) {
      Node *child = Object::cast_to<Node>((Object*)(children[i]));
      if (child == nullptr){
        continue;
      }

      find_inspector_editor_properties(p_property_name, r_ed_props, p_stop_at_first, child);
    }
  }

  return r_ed_props.size();
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

    /** @todo hint_string translator */

    PropertyInfo root_pi = root_dpi->get_property_info();
    String property = root_dpi->get_property_select();

    // root_pi.hint_string = normalize_hint_string(root_pi.hint_string);

    if (root_dpi->type != previous->type || root_dpi->hint != previous->hint) {
      Variant new_value = normalize_value(root_pi);
      p_object->set(property, new_value);
    } 
    else if (root_dpi->hint_string != previous->hint_string) {
      Variant new_value = normalize_value(root_pi, p_object);
      p_object->set(property, new_value);
    }
  }

  p_object->notify_property_list_changed();
}

void DynamicPropertyInfoInspectorPlugin::_on_submit() {
  if (root_dpi.is_valid() && root_dpi->is_root) {
    root_dpi->emit_changed();
  }
}

bool DynamicPropertyInfoInspectorPlugin::_can_handle(Object *p_object) const {
  if (p_object == nullptr) {
    return false;
  }

  if (creating_native_editor()) {
    return false;
  }

  debug_print_rich(vformat("CAN HANDLE >>> %s", p_object), true);

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

    // ----- Extra dpi EditorProperty test for foreign plugins -----
    // if (p_name == root_dpi_name) {
    //   creating_native_editor() = true;
    //   EditorProperty *editor = EditorInspector::instantiate_property_editor(
    //     p_object,
    //     Variant::INT, 
    //     p_name, 
    //     PropertyHint::PROPERTY_HINT_NONE,
    //     "",
    //     PROPERTY_USAGE_DEFAULT,
    //     p_wide
    //   );
    //   creating_native_editor() = false;
    //   editor->set_object_and_property(p_object, p_name);
    //   this->add_property_editor(p_name, editor);
    // }

    if (extra_dpi != root_dpi) {
      Callable bound_callable = callable_mp_static(&DynamicPropertyInfoInspectorPlugin::on_dynamic_property_info_changed).bind(p_object, root_dpi);

      if (p_name != root_dpi_name && extra_dpi->is_connected(SIGNAL(extra_dpi, changed), bound_callable)) {
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

  /** 
    @note For some reason this editor doesn't handle Type String + Hint PROPERTY_HINT_FILE + Hint string "*.png" (or any other extension) correctly.
          Pop-up file selector window does not use the hint_string to filter files, it shows "All files" unlike a native godot property like:

            @export_custom(PROPERTY_HINT_FILE, "*.png") var file : String

          Same goes for PROPERTY_HINT_RESOURCE_TYPE and it's hint_string
  */
  EditorProperty *editor = EditorInspector::instantiate_property_editor(
    p_object,
    target_dpi->get_type(), 
    p_name, 
    static_cast<PropertyHint>(target_dpi->get_hint()),
    target_dpi->get_hint_string() /** normalize_hint_string(target_dpi->get_hint_string()) */,
    target_dpi->get_usage(),
    p_wide
  );

  creating_native_editor() = false;

  editor->set_object_and_property(p_object, p_name);
  this->add_property_editor(p_name, editor);

  return true;
}

void DynamicPropertyInfoInspectorPlugin::_parse_end(Object *p_object) {
  if (p_object == nullptr) {
    return;
  }

  if (creating_native_editor()) {
    debug_print_rich(vformat("NATIVE PARSE END >>> %s", p_object), false);
    return;
  }

  TypedArray<EditorProperty> root_dpi_inspector_nodes;
  find_inspector_editor_properties(root_dpi_name, root_dpi_inspector_nodes);

  for (int i = 0; i < root_dpi_inspector_nodes.size(); i++) {
    EditorProperty *root_dpi_inspector_node = Object::cast_to<EditorProperty>((Object *)root_dpi_inspector_nodes[i]);
    if (root_dpi_inspector_node == nullptr) {
      continue;
    }

    HBoxContainer *button_target_node = nullptr;
    TypedArray<Node> children = root_dpi_inspector_node->get_children();

    if (children.size() > 1) {
      /** WEAK: This line depends entirely on the correct HBoxContainer being on index 1 of root_dpi_inspector_node's children array, as of Godot 4.7.2 that seems to always be the case */
      Node *likely_target_node = Object::cast_to<Node>((Object*)children[1]);

      if (likely_target_node != nullptr) {
        button_target_node = Object::cast_to<HBoxContainer>(likely_target_node);
      }
    }
    // PROPERTY_HINT_NONE
    if (button_target_node == nullptr) {
      button_target_node = memnew(HBoxContainer);
      root_dpi_inspector_node->add_child(button_target_node);
    }

    Button *submit_button = memnew(Button);
    submit_button->set_text("Apply");
    submit_button->connect("pressed", callable_mp(this, &DynamicPropertyInfoInspectorPlugin::_on_submit));
    button_target_node->add_child(submit_button);
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