#include "bi_map.hpp"

#include "utils/macros.hpp"

namespace godot {
  template <typename T>
  using NamedValue = Pair<T, Vector<String>>;
   
  #define VALUE_NAME_TYPE_ENTRIES() \
    VALUE_NAME_TYPE_ENTRY(VALUE_NAME_FULL, "FULL") \
    VALUE_NAME_TYPE_ENTRY(VALUE_NAME_SHORT, "SHORT") \
    VALUE_NAME_TYPE_ENTRY(VALUE_NAME_PASCAL, "PASCAL")

  enum ValueNameType {
    #define VALUE_NAME_TYPE_ENTRY(m_id, ...) m_id,
      VALUE_NAME_TYPE_ENTRIES()
    #undef VALUE_NAME_TYPE_ENTRY
    VALUE_NAME_MAX
  };

  inline constexpr const char *to_string(ValueNameType p_name_type) {
    switch (p_name_type) {
      #define VALUE_NAME_TYPE_ENTRY(m_id, m_short_name) case m_id: return m_short_name;
        VALUE_NAME_TYPE_ENTRIES()
      #undef VALUE_NAME_TYPE_ENTRY
      default: return "Unknown";
    }
  }


  #define LOCAL_INSERT_NAMED(m_value_type, m_val, m_to, m_name, ...) \
    m_to.push_back(NamedValue<m_value_type>({m_val, {m_name __VA_OPT__(,) __VA_ARGS__}}))

  #pragma region Variant

  inline Variant get_variant_default_value(Variant::Type p_type) {
    switch (p_type) {
      case Variant::NIL:
        return Variant();
      case Variant::BOOL:
        return false;
      case Variant::INT:
        return 0;
      case Variant::FLOAT:
        return 0.0f;
      case Variant::STRING:
        return String();
      case Variant::VECTOR2:
        return Vector2();
      case Variant::VECTOR2I:
        return Vector2i();
      case Variant::RECT2:
        return Rect2();
      case Variant::RECT2I:
        return Rect2i();
      case Variant::VECTOR3:
        return Vector3();
      case Variant::VECTOR3I:
        return Vector3i();
      case Variant::TRANSFORM2D:
        return Transform2D();
      case Variant::VECTOR4:
        return Vector4();
      case Variant::VECTOR4I:
        return Vector4i();
      case Variant::PLANE:
        return Plane();
      case Variant::QUATERNION:
        return Quaternion();
      case Variant::AABB:
        return AABB();
      case Variant::BASIS:
        return Basis();
      case Variant::TRANSFORM3D:
        return Transform3D();
      case Variant::PROJECTION:
        return Projection();
      case Variant::COLOR:
        return Color();
      case Variant::STRING_NAME:
        return StringName();
      case Variant::NODE_PATH:
        return NodePath();
      case Variant::RID:
        return RID();
      case Variant::OBJECT:
        return Variant();
      case Variant::CALLABLE:
        return Callable();
      case Variant::SIGNAL:
        return Signal();
      case Variant::DICTIONARY:
        return Dictionary();
      case Variant::ARRAY:
        return Array();
      case Variant::PACKED_BYTE_ARRAY:
        return PackedByteArray();
      case Variant::PACKED_INT32_ARRAY:
        return PackedInt32Array();
      case Variant::PACKED_INT64_ARRAY:
        return PackedInt64Array();
      case Variant::PACKED_FLOAT32_ARRAY:
        return PackedFloat32Array();
      case Variant::PACKED_FLOAT64_ARRAY:
        return PackedFloat64Array();
      case Variant::PACKED_STRING_ARRAY:
        return PackedStringArray();
      case Variant::PACKED_VECTOR2_ARRAY:
        return PackedVector2Array();
      case Variant::PACKED_VECTOR3_ARRAY:
        return PackedVector3Array();
      case Variant::PACKED_COLOR_ARRAY:
        return PackedColorArray();
      case Variant::PACKED_VECTOR4_ARRAY:
        return PackedVector4Array();
      default:
        return Variant();
    }
  }

  static const Vector<NamedValue<Variant::Type>> &variant_type_named_values() {
    static Vector<NamedValue<Variant::Type>> entries;

    #pragma region
    #define LOCAL_ELEMENTS() \
      LOCAL_ELEMENT(NIL) \
      LOCAL_ELEMENT(BOOL) \
		  LOCAL_ELEMENT(INT) \
		  LOCAL_ELEMENT(FLOAT) \
		  LOCAL_ELEMENT(STRING) \
		  LOCAL_ELEMENT(VECTOR2) \
		  LOCAL_ELEMENT(VECTOR2I) \
		  LOCAL_ELEMENT(RECT2) \
		  LOCAL_ELEMENT(RECT2I) \
		  LOCAL_ELEMENT(VECTOR3) \
		  LOCAL_ELEMENT(VECTOR3I) \
		  LOCAL_ELEMENT(TRANSFORM2D) \
		  LOCAL_ELEMENT(VECTOR4) \
		  LOCAL_ELEMENT(VECTOR4I) \
		  LOCAL_ELEMENT(PLANE) \
		  LOCAL_ELEMENT(QUATERNION) \
		  LOCAL_ELEMENT(AABB) \
		  LOCAL_ELEMENT(BASIS) \
		  LOCAL_ELEMENT(TRANSFORM3D) \
		  LOCAL_ELEMENT(PROJECTION) \
		  LOCAL_ELEMENT(COLOR) \
		  LOCAL_ELEMENT(STRING_NAME) \
		  LOCAL_ELEMENT(NODE_PATH) \
		  LOCAL_ELEMENT(RID) \
		  LOCAL_ELEMENT(OBJECT) \
		  LOCAL_ELEMENT(CALLABLE) \
		  LOCAL_ELEMENT(SIGNAL) \
		  LOCAL_ELEMENT(DICTIONARY) \
		  LOCAL_ELEMENT(ARRAY) \
		  LOCAL_ELEMENT(PACKED_BYTE_ARRAY) \
		  LOCAL_ELEMENT(PACKED_INT32_ARRAY) \
		  LOCAL_ELEMENT(PACKED_INT64_ARRAY) \
		  LOCAL_ELEMENT(PACKED_FLOAT32_ARRAY) \
		  LOCAL_ELEMENT(PACKED_FLOAT64_ARRAY) \
		  LOCAL_ELEMENT(PACKED_STRING_ARRAY) \
		  LOCAL_ELEMENT(PACKED_VECTOR2_ARRAY) \
		  LOCAL_ELEMENT(PACKED_VECTOR3_ARRAY) \
		  LOCAL_ELEMENT(PACKED_COLOR_ARRAY) \
		  LOCAL_ELEMENT(PACKED_VECTOR4_ARRAY) 
    #pragma endregion

    if (entries.is_empty()) {
      #define LOCAL_ELEMENT(m_val) LOCAL_INSERT_NAMED(Variant::Type, Variant::m_val, entries, STR(JOIN(TYPE,m_val)), STR(m_val), String(STR(m_val)).to_pascal_case());
        LOCAL_ELEMENTS();
    }

    #undef LOCAL_ELEMENT
    #undef LOCAL_ELEMENTS

    return entries;
  }

  static BiMultimap<Variant::Type, String> &variant_type_names_map() {
    static BiMultimap<Variant::Type, String> lookup;

    if (lookup.is_empty()) {
      for (const NamedValue<Variant::Type> &entry : variant_type_named_values()) {
        lookup.insert(entry.first, entry.second);
      }
    }

    return lookup;
  }

  #pragma endregion

  #pragma region PropertyHint

  static const Vector<NamedValue<PropertyHint>> &property_hint_named_values() {
    static Vector<NamedValue<PropertyHint>> entries;
    entries.reserve_exact(PROPERTY_HINT_MAX - 2); // Not counting the obsolete 'PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE' and 'PROPERTY_HINT_INT_IS_OBJECTID' hints

    #pragma region
    #define LOCAL_ELEMENTS() \
      LOCAL_ELEMENT(PROPERTY_HINT_NONE, "NONE") \
      LOCAL_ELEMENT(PROPERTY_HINT_RANGE, "RANGE") \
      LOCAL_ELEMENT(PROPERTY_HINT_ENUM, "ENUM") \
      LOCAL_ELEMENT(PROPERTY_HINT_ENUM_SUGGESTION, "ENUM_SUGGESTION") \
      LOCAL_ELEMENT(PROPERTY_HINT_EXP_EASING, "EXP_EASING") \
      LOCAL_ELEMENT(PROPERTY_HINT_LINK, "LINK") \
      LOCAL_ELEMENT(PROPERTY_HINT_FLAGS, "FLAGS") \
      LOCAL_ELEMENT(PROPERTY_HINT_LAYERS_2D_RENDER, "LAYERS_2D_RENDER") \
      LOCAL_ELEMENT(PROPERTY_HINT_LAYERS_2D_PHYSICS, "LAYERS_2D_PHYSICS") \
      LOCAL_ELEMENT(PROPERTY_HINT_LAYERS_2D_NAVIGATION, "LAYERS_2D_NAVIGATION") \
      LOCAL_ELEMENT(PROPERTY_HINT_LAYERS_3D_RENDER, "LAYERS_3D_RENDER") \
      LOCAL_ELEMENT(PROPERTY_HINT_LAYERS_3D_PHYSICS, "LAYERS_3D_PHYSICS") \
      LOCAL_ELEMENT(PROPERTY_HINT_LAYERS_3D_NAVIGATION, "LAYERS_3D_NAVIGATION") \
      LOCAL_ELEMENT(PROPERTY_HINT_LAYERS_AVOIDANCE, "LAYERS_AVOIDANCE") \
      LOCAL_ELEMENT(PROPERTY_HINT_FILE, "FILE") \
      LOCAL_ELEMENT(PROPERTY_HINT_DIR, "DIR") \
      LOCAL_ELEMENT(PROPERTY_HINT_GLOBAL_FILE, "GLOBAL_FILE") \
      LOCAL_ELEMENT(PROPERTY_HINT_GLOBAL_DIR, "GLOBAL_DIR") \
      LOCAL_ELEMENT(PROPERTY_HINT_RESOURCE_TYPE, "RESOURCE_TYPE") \
      LOCAL_ELEMENT(PROPERTY_HINT_MULTILINE_TEXT, "MULTILINE_TEXT") \
      LOCAL_ELEMENT(PROPERTY_HINT_EXPRESSION, "EXPRESSION") \
      LOCAL_ELEMENT(PROPERTY_HINT_PLACEHOLDER_TEXT, "PLACEHOLDER_TEXT") \
      LOCAL_ELEMENT(PROPERTY_HINT_COLOR_NO_ALPHA, "COLOR_NO_ALPHA") \
      LOCAL_ELEMENT(PROPERTY_HINT_OBJECT_ID, "OBJECT_ID") \
      LOCAL_ELEMENT(PROPERTY_HINT_TYPE_STRING, "TYPE_STRING") \
      LOCAL_ELEMENT(PROPERTY_HINT_OBJECT_TOO_BIG, "OBJECT_TOO_BIG") \
      LOCAL_ELEMENT(PROPERTY_HINT_NODE_PATH_VALID_TYPES, "NODE_PATH_VALID_TYPES") \
      LOCAL_ELEMENT(PROPERTY_HINT_SAVE_FILE, "SAVE_FILE") \
      LOCAL_ELEMENT(PROPERTY_HINT_GLOBAL_SAVE_FILE, "GLOBAL_SAVE_FILE") \
      LOCAL_ELEMENT(PROPERTY_HINT_INT_IS_POINTER, "INT_IS_POINTER") \
      LOCAL_ELEMENT(PROPERTY_HINT_ARRAY_TYPE, "ARRAY_TYPE") \
      LOCAL_ELEMENT(PROPERTY_HINT_DICTIONARY_TYPE, "DICTIONARY_TYPE") \
      LOCAL_ELEMENT(PROPERTY_HINT_LOCALE_ID, "LOCALE_ID") \
      LOCAL_ELEMENT(PROPERTY_HINT_LOCALIZABLE_STRING, "LOCALIZABLE_STRING") \
      LOCAL_ELEMENT(PROPERTY_HINT_NODE_TYPE, "NODE_TYPE") \
      LOCAL_ELEMENT(PROPERTY_HINT_HIDE_QUATERNION_EDIT, "HIDE_QUATERNION_EDIT") \
      LOCAL_ELEMENT(PROPERTY_HINT_PASSWORD, "PASSWORD") \
      LOCAL_ELEMENT(PROPERTY_HINT_TOOL_BUTTON, "TOOL_BUTTON") \
      LOCAL_ELEMENT(PROPERTY_HINT_ONESHOT, "ONESHOT") \
      LOCAL_ELEMENT(PROPERTY_HINT_GROUP_ENABLE, "GROUP_ENABLE") \
      LOCAL_ELEMENT(PROPERTY_HINT_INPUT_NAME, "INPUT_NAME") \
      LOCAL_ELEMENT(PROPERTY_HINT_FILE_PATH, "FILE_PATH")
    #pragma endregion

    if (entries.is_empty()) {
      #define LOCAL_ELEMENT(m_value, m_short_name) LOCAL_INSERT_NAMED(PropertyHint, m_value, entries, STR(m_value), m_short_name, String(m_short_name).to_pascal_case());
        LOCAL_ELEMENTS();
    }

    #undef LOCAL_ELEMENT
    #undef LOCAL_ELEMENTS
    

    return entries;
  }

  static BiMultimap<PropertyHint, String> &property_hint_names_map() {
    static BiMultimap<PropertyHint, String> lookup;

    if (lookup.is_empty()) {
      for (const NamedValue<PropertyHint> &entry : property_hint_named_values()) {
        lookup.insert(entry.first, entry.second);
      }
    }
    
    return lookup;
  }

  #pragma endregion

  #pragma region PropertyUsageFlags
  
  static const Vector<NamedValue<PropertyUsageFlags>> &property_usage_named_values() {
    static Vector<NamedValue<PropertyUsageFlags>> entries;

    #pragma region
    #define LOCAL_ELEMENTS() \
      LOCAL_ELEMENT(PROPERTY_USAGE_NONE, "NONE") \
      LOCAL_ELEMENT(PROPERTY_USAGE_STORAGE, "STORAGE") \
      LOCAL_ELEMENT(PROPERTY_USAGE_EDITOR, "EDITOR") \
      LOCAL_ELEMENT(PROPERTY_USAGE_INTERNAL, "INTERNAL") \
      LOCAL_ELEMENT(PROPERTY_USAGE_CHECKABLE, "CHECKABLE") \
      LOCAL_ELEMENT(PROPERTY_USAGE_CHECKED, "CHECKED") \
      LOCAL_ELEMENT(PROPERTY_USAGE_GROUP, "GROUP") \
      LOCAL_ELEMENT(PROPERTY_USAGE_CATEGORY, "CATEGORY") \
      LOCAL_ELEMENT(PROPERTY_USAGE_SUBGROUP, "SUBGROUP") \
      LOCAL_ELEMENT(PROPERTY_USAGE_CLASS_IS_BITFIELD, "CLASS_IS_BITFIELD") \
      LOCAL_ELEMENT(PROPERTY_USAGE_NO_INSTANCE_STATE, "NO_INSTANCE_STATE") \
      LOCAL_ELEMENT(PROPERTY_USAGE_RESTART_IF_CHANGED, "RESTART_IF_CHANGED") \
      LOCAL_ELEMENT(PROPERTY_USAGE_SCRIPT_VARIABLE, "SCRIPT_VARIABLE") \
      LOCAL_ELEMENT(PROPERTY_USAGE_STORE_IF_NULL, "STORE_IF_NULL") \
      LOCAL_ELEMENT(PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED, "UPDATE_ALL_IF_MODIFIED") \
      LOCAL_ELEMENT(PROPERTY_USAGE_SCRIPT_DEFAULT_VALUE, "SCRIPT_DEFAULT_VALUE") \
      LOCAL_ELEMENT(PROPERTY_USAGE_CLASS_IS_ENUM, "CLASS_IS_ENUM") \
      LOCAL_ELEMENT(PROPERTY_USAGE_NIL_IS_VARIANT, "NIL_IS_VARIANT") \
      LOCAL_ELEMENT(PROPERTY_USAGE_ARRAY, "ARRAY") \
      LOCAL_ELEMENT(PROPERTY_USAGE_ALWAYS_DUPLICATE, "ALWAYS_DUPLICATE") \
      LOCAL_ELEMENT(PROPERTY_USAGE_NEVER_DUPLICATE, "NEVER_DUPLICATE") \
      LOCAL_ELEMENT(PROPERTY_USAGE_HIGH_END_GFX, "HIGH_END_GFX") \
      LOCAL_ELEMENT(PROPERTY_USAGE_NODE_PATH_FROM_SCENE_ROOT, "NODE_PATH_FROM_SCENE_ROOT") \
      LOCAL_ELEMENT(PROPERTY_USAGE_RESOURCE_NOT_PERSISTENT, "RESOURCE_NOT_PERSISTENT") \
      LOCAL_ELEMENT(PROPERTY_USAGE_KEYING_INCREMENTS, "KEYING_INCREMENTS") \
      LOCAL_ELEMENT(PROPERTY_USAGE_DEFERRED_SET_RESOURCE, "DEFERRED_SET_RESOURCE") \
      LOCAL_ELEMENT(PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT, "EDITOR_INSTANTIATE_OBJECT") \
      LOCAL_ELEMENT(PROPERTY_USAGE_EDITOR_BASIC_SETTING, "EDITOR_BASIC_SETTING") \
      LOCAL_ELEMENT(PROPERTY_USAGE_READ_ONLY, "READ_ONLY") \
      LOCAL_ELEMENT(PROPERTY_USAGE_SECRET, "SECRET") \
      LOCAL_ELEMENT(PROPERTY_USAGE_DEFAULT, "DEFAULT") \
      LOCAL_ELEMENT(PROPERTY_USAGE_NO_EDITOR, "NO_EDITOR")

    #pragma endregion

    if (entries.is_empty()) {
      #define LOCAL_ELEMENT(m_val, m_short_name) LOCAL_INSERT_NAMED(PropertyUsageFlags, m_val, entries, STR(m_val), m_short_name, String(m_short_name).to_pascal_case());
        LOCAL_ELEMENTS();
    }

    #undef LOCAL_ELEMENT
    #undef LOCAL_ELEMENTS

    return entries;
  }

  static BiMultimap<PropertyUsageFlags, String> &property_usage_names_map() {
    static BiMultimap<PropertyUsageFlags, String> lookup;

    if (lookup.is_empty()) {
      for (const NamedValue<PropertyUsageFlags> &entry : property_usage_named_values()) {
        lookup.insert(entry.first, entry.second);
      }
    }

    return lookup;
  }

  #pragma endregion

  #undef LOCAL_INSERT_NAMED
}