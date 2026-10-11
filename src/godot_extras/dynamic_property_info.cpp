#include "dynamic_property_info.hpp"

#include "godot_cpp/classes/translation_server.hpp"
#include "godot_cpp/classes/editor_interface.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/resource_uid.hpp"
#include "godot_cpp/classes/input_map.hpp"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/class_db_singleton.hpp"

#include "godot_cpp/variant/callable_method_pointer.hpp"

#include "godot_cpp/templates/hash_map.hpp"
#include "godot_cpp/templates/pair.hpp"
#include "godot_cpp/templates/vector.hpp"

#include "utils/macros.hpp"
#include "utils/error_macros.hpp"
#include "utils/debug.hpp"
#include <cstdint>

using namespace godot;

#pragma region HelperFunctions

static Variant normalize_value(const PropertyInfo &p_property_info, const Object* p_object = nullptr, bool p_prefer_default = false, const Variant &p_inner_value = nullptr);
static String normalize_hint_string(const PropertyInfo &p_property_info, bool is_main_call = true);

/** @note Reviewed */
static PropertyInfo hint_string_to_pi(const String &p_hint_string) {
  PropertyInfo out {};

  int hint_string_separator = p_hint_string.find(":");
  if (hint_string_separator >= 0) {
    String type_as_string = p_hint_string.substr(0, hint_string_separator);
    out.hint_string = p_hint_string.substr(hint_string_separator + 1);

    int hint_slash = p_hint_string.find("/");
    if (hint_slash >= 0) {
      String hint_as_string = type_as_string.substr(hint_slash + 1);
      if (hint_as_string.is_valid_int()) {
        uint32_t hint = hint_as_string.to_int();
        out.hint = hint < PROPERTY_HINT_MAX? hint : out.hint;
      }

      type_as_string = p_hint_string.substr(0, hint_slash);
    } else {
      type_as_string = p_hint_string.substr(0, hint_string_separator);
    }

    uint32_t type = type_as_string.to_int();
    out.type = type < Variant::VARIANT_MAX? Variant::Type(type) : out.type;
  }

  return out;
}

/** @note Reviewed
  @brief
    Normalizes a hint string containing nested Variant type, PropertyHint and hint string definitions, recursively processing Array and Dictionary type specifications.

  @returns
    Original hint string if the property's type is not Array or Dictionary (since these are the only types that use typing in their hint string).
  @returns
    Empty string if the hint string is empty.
  @returns
    Normalized hint string after recursively processing both sides of a semicolon-separated definition.
  @returns
    Original hint string if it does not identify a recognized Variant type and contains no type-hint separator.
  @returns
    Normalized hint string with resolved Variant type, PropertyHint and recursively normalized nested hint string otherwise.

  @note
    Type and PropertyHint names are resolved using their respective name maps. Numeric identifiers are also accepted.
  @note
    Spaces are removed from the resulting hint string. Debug output is printed only for the main call, not recursive calls.
*/
static String normalize_hint_string(const PropertyInfo &p_property_info, bool is_main_call /* = true */) {
  if (p_property_info.type != Variant::ARRAY && p_property_info.type != Variant::DICTIONARY) {
    return p_property_info.hint_string;
  }

  String out {};

  PropertyInfo pi {};
  Variant::Type type {};
  PropertyHint hint {};
  String hint_string {};

  String p_hint_string = p_property_info.hint_string;
  int hint_string_separator = p_hint_string.find(":");
  int key_value_separator = p_hint_string.find(";");
  if (p_hint_string.remove_chars(" ").is_empty()) {
    goto BUILD_STRING;
  }

  if (key_value_separator >= 0) {
    pi = p_property_info;
    pi.hint_string = p_hint_string.substr(0,key_value_separator);
    out += normalize_hint_string(pi, false);
    out += ";";
    pi.hint_string = p_hint_string.substr(key_value_separator + 1);
    out += normalize_hint_string(pi, false);

    goto PRINT_AND_RET;
  }

  if (hint_string_separator >= 0) {
    String type_string = p_hint_string.substr(0, hint_string_separator);
    
    int hint_slash = type_string.find("/");
    if (hint_slash >= 0) {
      String hint_as_string = type_string.substr(hint_slash + 1);

      if (hint_as_string.is_valid_int()) {
        hint = (PropertyHint)hint_as_string.to_int();
      } else {
        hint_as_string = hint_as_string.to_snake_case().to_upper();
        if (property_hint_names_map().has(hint_as_string)) {
          hint = property_hint_names_map().get(hint_as_string);
        }
      }

      type_string = type_string.substr(0, hint_slash);
    }

    if (type_string.is_valid_int()) {
      uint32_t type_as_int = type_string.to_int();
      if (type_as_int < Variant::VARIANT_MAX) {
        type = Variant::Type(type_as_int);
      }
    } else {
      type_string = type_string.to_snake_case().to_upper();
      if (variant_type_names_map().has(type_string)) {
        type = variant_type_names_map().get(type_string);
      }
    }

    pi.type = type;
    pi.hint = hint;
    pi.hint_string = p_hint_string.substr(hint_string_separator + 1);
    hint_string = normalize_hint_string(pi, false);
  } else {
    type = Variant::get_type_by_name(p_hint_string);

    if (type == Variant::VARIANT_MAX) {
      out = p_hint_string;
      goto PRINT_AND_RET;
    }
  }


  BUILD_STRING:
    out += String::num_int64(type) + String("/") + String::num_int64(hint) + String(":") + hint_string;

  PRINT_AND_RET:
    out = out.remove_chars(" ");
    if (is_main_call) {
      debug_print_rich(vformat(COLOR_GREEN("Normalized >>> Hint string from '%s' to '%s'"), p_hint_string, out));
    }

    return out;
}

/** @note Reviewed+ 
  @brief 
    From a hint string "min,max,step" and optionally "...,or_greater" and/or "...,or_less" (other possible entries don't matter for this operation)
    takes min and max value and ensures p_value is within this range. The "or_less" and "or_greater" entries respectively remove the min and max restrictions.

  @returns 
    Null variant if hint string is invalid.
  @returns 
    Default value 'max' if p_value is null.
  @returns 
    Same value if in range.
  @returns 
    Normalized value clamped to min or max (keeping in mind "or_greater" and "or_less" liberties).

  @note 
    Works with int, float or Array. If an Array, redirects into normalize_value() as Array/TypeString to later treat each value individually.
*/
static Variant normalize_range(const PropertyInfo &p_property_info, const Variant &p_value) {
  Variant::Type type = p_property_info.type;

  if (type == Variant::ARRAY) {
    PropertyInfo larper {p_property_info};
    larper.hint = PROPERTY_HINT_TYPE_STRING;
    return normalize_value(larper, p_value);
  }

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
    m_type min = (hint_string_entries.has("or_less"))? (m_type)p_value : hint_string_entries[0].JOIN(to,m_type)(); \
    m_type max = (hint_string_entries.has("or_greater"))? (m_type)p_value : hint_string_entries[1].JOIN(to,m_type)(); \
    return CLAMP((m_type)p_value, min, max); \
   } while(false)

  if (type == Variant::INT) {
    NORMALIZE_TYPE(int);
  } else {
    NORMALIZE_TYPE(float);
  }

  #undef NORMALIZE_TYPE
}

/** @note Reviewed+ 
  @brief 
    From a hint string "Op1,Op2,Op3:Op3Val,Op4,..." ensures p_value is equal to one of these values. Follows Godot's rules for Enum:hint_string formatting.

  @returns 
    Null variant if hint string is invalid.  
  @returns 
    Same value if suitable.
  @returns
    Default value first entry's value if not suitable, or if default is preferred as int/Enum.

  @note Parameter 'p_prefer_default' only matters if int typed.
*/
static Variant normalize_enum(const PropertyInfo &p_property_info, const Variant &p_value, bool p_prefer_default) {
  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {};
  }

  String first_entry = hint_string_entries[0];
  if (p_property_info.type != Variant::INT) {
    if (hint_string_entries.has(p_value)) {
      return {p_value};
    }

    return first_entry;
  }

  int64_t last_visited;
  bool first_visit = true;
  int64_t first_entry_numeric;

  for (String &entry : hint_string_entries) {
    PackedStringArray split_entry = entry.split(":",false); // i.e. "a:3" -> 3, "b:3:44" -> 3, "c:::4:2" -> 4
    String entry_numeric_as_string = (split_entry.size() > 1)? split_entry[1] : "";

    if (entry_numeric_as_string.is_valid_int()) {
      last_visited = entry_numeric_as_string.to_int();
    } else {
      last_visited = first_visit ? 0 : last_visited + 1;
    }

    if (first_visit) {
      first_entry_numeric = last_visited;
      if (p_prefer_default) {
        return {first_entry_numeric};
      }
      first_visit = false;
    }

    if ((int64_t)p_value == last_visited) {
      return {p_value};
    }
  }

  return {first_entry_numeric};
}

/** @note Reviewed 
  @brief
    From a hint string optionally containing "positive_only" ensures p_value is not negative.

  @returns
    Same value if not a float.
  @returns
    Default value 0.0 if "positive_only" is present and p_value is negative.
  @returns
    Same value otherwise.
*/
static Variant normalize_exp_easing(const PropertyInfo &p_property_info, const Variant &p_value) {
  if (p_property_info.type != Variant::FLOAT) { 
    return {p_value};
  }

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",",false);
  if (hint_string_entries.has("positive_only") && (float)p_value < 0) {
    return {0.0};
  }

  return {(float)p_value};
}

/** @note Reviewed 
  @brief
    From a hint string "Bit0,Bit1,Bit2:Bit2Val,Bit3,..." ensures p_value only contains known bits. Follows Godot's rules for Flags:hint_string formatting.

  @returns
    Same value if not an int or if the hint string is empty.
  @returns
    Same value with unknown bits masked out.

  @note
    If Bit32 is reached in the hint string, this and following entries must specify a value (BitN:val), otherwise they will be ignored.
*/
static Variant normalize_flags(const PropertyInfo &p_property_info, const Variant &p_value) {
  if (p_property_info.type != Variant::INT) {
    return {p_value};
  }

  /** For flags hint strings entry position is absolute truth if no specific value is provided "A:16,B,C" @A = 16, B = 2, C = 4 */
  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {p_value};
  }

  uint32_t current_bit = 0;
  uint32_t allowed_bits = 0;
  for (String& entry : hint_string_entries) {
    String bit_value_str = entry.get_slice(":", 1);
    uint32_t bit_value;

    if (bit_value_str == entry || bit_value_str.is_empty()) {
      bit_value = (current_bit < 32)? 1u << current_bit : 0;
    } else {
      bit_value = bit_value_str.to_int(); // No need to check if power of two, since values can also be a combination of bits
    }

    allowed_bits |= bit_value;
    current_bit++;
  }

  return {(uint32_t)p_value & allowed_bits};
}

/** @note Reviewed 
  @brief
    From a hint string containing file name filters such as "*.png,*.jpg" ensures p_value contains a valid file name matching one of the filters. If p_global is true, also ensures the path is absolute.

  @returns
    Same value if not a String, if the file name is valid and the hint string is empty, or if the file name matches one of the hint string filters.
  @returns
    Null variant if p_global is true and the path is not absolute, if the file name is invalid, or if the file name does not match any hint string filter.
*/
static Variant normalize_file(const PropertyInfo &p_property_info, const Variant &p_value, bool p_global = false) {
  if (p_property_info.type != Variant::STRING) {
    return {p_value};
  }

  String path = (String)p_value;
  if (path.begins_with("uid://")) {
    path = ResourceUID::uid_to_path(path);
  }

  if (p_global && not path.begins_with("/")) {
    return {};
  }

  String filename = path.get_file();
  if (not filename.is_valid_filename()) {
    return {};
  }

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {p_value};
  }

  for (String &entry : hint_string_entries) {
    if (filename.match(entry)) {
      return {p_value};
    }
  }

  return {};
}

/** @note Reviewed 
  @brief
    Ensures p_value is an absolute path.

  @returns
    Same value if not a String.
  @returns
    Same value if p_value is an absolute path.
  @returns
    Null variant if p_value is not an absolute path.
*/
static Variant normalize_global_dir(const PropertyInfo &p_property_info, const Variant &p_value) {
  if (p_property_info.type != Variant::STRING) {
    return {p_value};
  }

  if (String(p_value).is_absolute_path()) {
    return {p_value};
  }

  return {};
}

/** @note Reviewed 
  @brief
    From a hint string containing resource class names such as "Texture2D,ShaderMaterial" ensures p_value is a Resource whose class matches one of the specified types.

  @returns
    Same value if not an Object.
  @returns
    Null variant if p_value is not a valid Resource.
  @returns
    Same value if the hint string is empty.
  @returns
    Same value if the Resource matches one of the hint string resource types.
  @returns
    Null variant if the Resource does not match any hint string resource type.
*/
static Variant normalize_resource_type(const PropertyInfo &p_property_info, const Variant &p_value) {
  if (p_property_info.type != Variant::OBJECT) {
    return {p_value};
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

  /**
    p_value = [[a, "abc", b, 32.3], [c, d, b, c]]

    PropertyInfo = {Array, name, TypeString/ArrayType, "Array/0:int/Enum:a,b,c,d"}

    inner = [a, "abc", b, 32.3]

    PropertyInfo = {Array, name, TypeString/ArrayType, "int/Enum:a,b,c,d"}

    innerb = a      >>> a
    innerb = "abc"  >>> null
    innerb = b      >>> b
    innerb = 32.3   >>> null

    PropertyInfo = {int, name, Enum, "a,b,c,d"}
  */
static Variant normalize_type_string(const PropertyInfo &p_property_info, const Object* p_object, const Variant &p_value) {
  Variant::Type type = p_property_info.type;
  
  /**
    String/TypeString is a weird case cause it only works if the hint string has exactly one type and nothing else, otherwise using these properties
    leads to a crash. Doesn't seem intentional but that's what it is, thus it's treated in a general way that works with it but also works with how 
    it most likely it's supposed to, which is a bunch of types, separated by commas, if p_value matches one of these then it's returned back, null 
    variant if not, this keeps the code usable even if Godot patches this.
  */
  if (type == Variant::STRING) {
    PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
    if (hint_string_entries.is_empty()) {
      return {p_value};
    }

    for (const String &entry : hint_string_entries) {
      if (String(p_value) == entry && ClassDBSingleton::get_singleton()->is_class(entry)) {
        return {p_value};
      }
    }

    return {};
  }

  PropertyInfo pi {hint_string_to_pi(p_property_info.hint_string)};

  if (type == Variant::ARRAY) {
    Array array {}; array.append_array(p_value);
    int deleted = 0;
    int size = array.size();

    print_line(vformat("FLAG >>> %s/%s:%s - %s", pi.type, pi.hint, pi.hint_string, array));

    for (int i = 0; i < size; i++) {
      pi.name = p_property_info.name + vformat("[%s]", i);
      Variant curr_normalized = normalize_value(pi, p_object, false, array[i - deleted]);
      
      print_line(vformat("FLAG A [%s] >>> %s", i, curr_normalized));
      if (curr_normalized.get_type() == Variant::NIL) {
        array.remove_at(i - deleted++);
        print_line(vformat("FLAG B [%s] >>> %s", i, array));
      } else {
        array.set(i - deleted, curr_normalized);
      }
    }

    return array;
  }

  return {p_value};
}

/** @note Reviewed 
  @brief
    From a hint string containing node class names such as "Node2D,Sprite2D" ensures p_value points to an existing Node whose class matches one of the specified types.

  @returns
    Same value if not a NodePath.
  @returns
    Same value if the hint string is empty.
  @returns
    Same value if p_object cannot be cast to a Node, since the property's path cannot be validated.
  @returns
    Null variant if the NodePath does not point to an existing Node.
  @returns
    Same value if the target Node matches one of the hint string node types.
  @returns
    Null variant if the target Node does not match any hint string node type.

  @note
    Validation is performed relative to p_object, which must be a Node containing the target path.
*/
static Variant normalize_node_path_valid_types(const PropertyInfo &p_property_info, const Object* p_object, const Variant &p_value) {
  if (p_property_info.type != Variant::NODE_PATH) {
    return {p_value};
  }

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {p_value};
  }

  const Node *node = Object::cast_to<Node>(p_object);
  if (node == nullptr) {
    debug_print_rich(vformat(
      COLOR_YELLOW("Normalized >>> Unable to cast 'p_object' to Node, cannot validate property's '%s' path. Same path is returned."),
      p_property_info.name
    ));
    return {p_value};
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

  return {};
}

static Variant normalize_dictionary_type(const PropertyInfo &p_property_info, const Object* p_object, const Variant &p_value) {
  return {};
}

/** @note Reviewed 
  @brief
    Ensures p_value contains a locale identifier with a recognized language and, if present, a recognized script or country as its second component.

  @returns
    Same value if not a String.
  @returns
    Null variant if the language is not recognized.
  @returns
    Null variant if the second component is neither a recognized script nor country.
  @returns
    Same value otherwise.

  @note
    Components after the second are not validated because they may represent locale variants, which are not fully exposed 
    through the public API, thus they're considered valid.
*/
static Variant normalize_locale_id(const PropertyInfo &p_property_info, const Variant &p_value) {
  if (p_property_info.type != Variant::STRING) {
    return {p_value};
  }

  TranslationServer *server = TranslationServer::get_singleton();
  const PackedStringArray &langs = server->get_all_languages();
  const PackedStringArray &countries = server->get_all_countries();
  const PackedStringArray &scripts = server->get_all_scripts();

  String locale_id = p_value;
  locale_id = locale_id.replace("-", "_");
  PackedStringArray split_id = locale_id.split("_", false);

  if (split_id.is_empty() || not langs.has(split_id[0])) {
    return {};
  }

  /** 
    No need to check idx 2 since this could be either country or variant, if invalid country then it has to be considered variant, 
    and any variant is considered valid since they're not in the public API. The only check is for idx 1 to be a valid script or country. 
  */
  if (split_id.size() > 1) {
    const String &second = split_id[1];
    if (not scripts.has(second) && not countries.has(second)) {
      return {};
    }
  }

  return {p_value};
}

static Variant normalize_localizable_string(const PropertyInfo &p_property_info, const Variant &p_value) { 
  if (p_property_info.type != Variant::DICTIONARY) {
    return {p_value};
  }

  return {};
}

/** @note Reviewed
  @brief
    From a hint string containing node class names such as "Node,Node3D" ensures p_value is a Node whose class matches or inherits one of the specified types.

  @returns
    Same value if not an Object.
  @returns
    Null variant if p_value cannot be cast to a Node.
  @returns
    Same value if the hint string is empty.
  @returns
    Same value if the Node matches one of the hint string node types.
  @returns
    Null variant if the Node does not match any hint string node type.
*/
static Variant normalize_node_type(const PropertyInfo &p_property_info, const Variant &p_value) {
  if (p_property_info.type != Variant::OBJECT) {
    return {p_value};
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

/** @note Reviewed
  @brief
    Ensures p_value conforms to the input action restrictions specified by the hint string.

  @returns
    Same value if neither String nor StringName.
  @returns
    Same value if p_value is empty.
  @returns
    Null variant if "show_builtin" is present, "loose_mode" is absent, and p_value does not match an existing InputMap action.
  @returns
    Same value otherwise.
*/
static Variant normalize_input_name(const PropertyInfo &p_property_info, const Variant &p_value) {
    if (p_property_info.type != Variant::STRING && p_property_info.type != Variant::STRING_NAME) {
    return {p_value};
  }

  String input = String(p_value);
  if (input.is_empty()) {
    return {p_value};
  }

  const TypedArray<StringName> &actions = InputMap::get_singleton()->get_actions();

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",");
  if (hint_string_entries.has("show_builtin") && not hint_string_entries.has("loose_mode")) {
    if (not actions.has(input)) {
      return {};
    }
  }

  return {p_value};
}

/**
  @brief 
    Value normalization means making sure a value remains suitable when taking into account both hint and hint string.

  @param p_property_info PropertyInfo describing the target property.
  @param p_object Object containing the current value. Passing just the value directly is not supported since an object is mandatory for some Type/Hint combinations.
  @param p_prefer_default If true, a default value has priority over the current value.

  @returns 
    Type-based default if no object is provided and Type/Hint combination is incompatible.
  @returns 
    Normalized default if no object is provided. The actual value depends on Type/Hint combination, i.e. the first entry of a hint string in the case of an 
    int/Enum property. For more information on how each hint handles each normalization case, check its function documentation.
  @returns 
    Normalized value if p_object is provided but the target property's value is not suitable to remain unchanged due to hint string restrictions, 
    i.e. float/Range:0,100,1 @234.0 normalizes to 100.0. For more information on how each hint handles each normalization case, check its function documentation.
  @returns 
    Same current p_object's target property value if suitable for the Type/Hint:HintString combination or if Type/Hint is incompatible.

  @warning 
    Assumes p_property_info.hint_string is normalized @see normalize_hint_string(), behavior is undefined otherwise in cases that use PropertyInfo syntax on their hint string.
    If the hint string contains spaces the behavior is undefined. Unless it makes sense for the hint string to have spaces like String/Enum where each value could
    be represented by multiple words, in this cases behavior is well defined. Type Array and Dictionary already had spaces removed during normalization.
*/
static Variant normalize_value(
  const PropertyInfo &p_property_info, 
  const Object* p_object /* = nullptr */, 
  bool p_prefer_default /* = false */, 
  const Variant &p_inner_value /* = nullptr */
) {
  Variant out {};

  PropertyInfo pi {p_property_info};
  if (pi.type == Variant::ARRAY || pi.type == Variant::DICTIONARY) {
    if (pi.hint == PROPERTY_HINT_NONE) {
      pi.hint = PROPERTY_HINT_TYPE_STRING;
    }
  }

  Variant value {p_inner_value};
  bool type_strict = false;
  if (value.get_type() == Variant::NIL) {
    value = (p_object)? p_object->get(pi.name) : nullptr;
  } else {
    type_strict = true;
  }

  /** 
    When reading case annotations assume Type/Hint combination is compatible. Don't worry when this is not true as any incompatible Type/Hint 
    combination behaves exactly the same as Type/HintNone, even inside normalize_*() functions.
  */
  switch ((PropertyHint) pi.hint) {
    case PROPERTY_HINT_NONE:
      /** Hint string is a visual suffix only, no need to normalize, any value could be valid. */
      out = value;
      break;
    case PROPERTY_HINT_RANGE:
      if (pi.type != Variant::INT && pi.type != Variant::FLOAT && pi.type != Variant::ARRAY) {
        out = type_strict? Variant{} : value;
      } else {
        out = normalize_range(pi, value);
      }
      break;
    case PROPERTY_HINT_ENUM:
      if (pi.type != Variant::INT && pi.type != Variant::STRING && pi.type != Variant::STRING_NAME) {
        out = type_strict? Variant{} : value;
      } else {
        out = normalize_enum(pi, value, p_prefer_default);
      }
      break;
    case PROPERTY_HINT_ENUM_SUGGESTION:
      /** Any value is valid, no need to normalize. */
      out = value;
      break;
    case PROPERTY_HINT_EXP_EASING:
      out = normalize_exp_easing(pi, value);
      break;
    case PROPERTY_HINT_LINK:
      /** Hint string is a visual suffix only. No need to normalize. */
      out = value;
      break;
    case PROPERTY_HINT_FLAGS:
      out = normalize_flags(pi, value);
      break;
    case PROPERTY_HINT_LAYERS_2D_RENDER:
    case PROPERTY_HINT_LAYERS_2D_PHYSICS:
    case PROPERTY_HINT_LAYERS_2D_NAVIGATION:
    case PROPERTY_HINT_LAYERS_3D_RENDER:
    case PROPERTY_HINT_LAYERS_3D_PHYSICS:
    case PROPERTY_HINT_LAYERS_3D_NAVIGATION:
    case PROPERTY_HINT_LAYERS_AVOIDANCE:
      /** These cases don't use the hint string. These are bitmasks that use layers already defined in project settings, thus, there's no need to normalize. Any value could be valid, but not every type can use it correctly. */
      out = value;
      break;
    case PROPERTY_HINT_FILE:
      out = normalize_file(pi, value);
      break;
    case PROPERTY_HINT_DIR:
      /** Any value is valid, no need to normalize. */
      out = value;
      break;
    case PROPERTY_HINT_GLOBAL_FILE:
      out = normalize_file(pi, value, true);
      break;
    case PROPERTY_HINT_GLOBAL_DIR:
      out = normalize_global_dir(pi, value);
      break;
    case PROPERTY_HINT_RESOURCE_TYPE:
      out = normalize_resource_type(pi, value);
      break;
    case PROPERTY_HINT_MULTILINE_TEXT:
      /** Hint string is visual only, no need to normalize */
      out = value;
      break;
    case PROPERTY_HINT_EXPRESSION:
      /** Doesn't use the hint string, no need to normalize */
      out = value;
      break;
    case PROPERTY_HINT_PLACEHOLDER_TEXT:
      /** Hint string is visual only, no need to normalize */
      out = value;
      break;
    case PROPERTY_HINT_COLOR_NO_ALPHA:
      /** Doesn't use the hint string, no need to normalize */
      out = value;
      break;
    case PROPERTY_HINT_OBJECT_ID:
      /** Can't understand this, but has something to do with the debugger so, no need to normalize? */
      out = value;
      break;
    case PROPERTY_HINT_TYPE_STRING: /** @todo Check */
      if (pi.type != Variant::STRING && pi.type != Variant::ARRAY && pi.type != Variant::DICTIONARY) {
        return type_strict? Variant{} : value;
      }
      out = normalize_type_string(pi, p_object, value);
      break;
    case PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE:
      /** @deprecated Not used by godot anymore, no need to normalize */
      out = value;
      break;
    case PROPERTY_HINT_OBJECT_TOO_BIG:
      /** Internal info for godot only, no need to normalize */
      out = value;
      break;
    case PROPERTY_HINT_NODE_PATH_VALID_TYPES:
      out = normalize_node_path_valid_types(pi, p_object, value);
      break;
    case PROPERTY_HINT_SAVE_FILE:
      out = normalize_file(pi, value);
      break;
    case PROPERTY_HINT_GLOBAL_SAVE_FILE:
      out = normalize_file(pi, value, true);
      break;
    case PROPERTY_HINT_INT_IS_OBJECTID:
      /** @deprecated Not used by godot anymore, no need to normalize */
      out = value;
      break;
    case PROPERTY_HINT_INT_IS_POINTER:
      /** Internal info for godot only, no need to normalize */
      out = value;
      break;
    case PROPERTY_HINT_ARRAY_TYPE:
      /** @todo handle */
      out = value;
      break;
    case PROPERTY_HINT_DICTIONARY_TYPE:
      /** @todo handle */
      break;
    case PROPERTY_HINT_LOCALE_ID:
      out = normalize_locale_id(pi, value);
      break;
    case PROPERTY_HINT_LOCALIZABLE_STRING:
      /** @todo handle */
      break;
    case PROPERTY_HINT_NODE_TYPE:
      out = normalize_node_type(pi, value);
      break;
    case PROPERTY_HINT_HIDE_QUATERNION_EDIT:
      /** Doesn't use the hint string, no need to normalize */
      out = value;
      break;
    case PROPERTY_HINT_PASSWORD:
      /** Visual only, no need to normalize */
      out = value;
      break;
    case PROPERTY_HINT_TOOL_BUTTON:
      /** Hint string is visual only, no need to normalize */
      out = value;
      break;
    case PROPERTY_HINT_ONESHOT:
      /** Can't understand this, but seems to not use hint string, so, no need to normalize? */
      out = value;
      break;
    case PROPERTY_HINT_GROUP_ENABLE:
      /** Hint string is visual only, no need to normalize */
      out = value;
      break;
    case PROPERTY_HINT_INPUT_NAME:
      out = normalize_input_name(pi, value);
      break;
    case PROPERTY_HINT_FILE_PATH:
      out = normalize_file(pi, value);
      break;
    case PROPERTY_HINT_MAX:
      WARN_PRINT("Attempted to normalize a value using \"PROPERTY_HINT_MAX\" hint. Input value is returned.");
      out = value;
  }

  if (out.get_type() == Variant::NIL) {
    if (type_strict) {
      return {};
    }

    out = get_variant_default_value(pi.type);
  }

  if (type_strict && out.get_type() != pi.type) {
    return {};
  }

  if (out == value) {
    return out;
  }

  if (p_object == nullptr) {
    debug_print_rich(vformat(
      COLOR_GREEN("Normalized >>> Property's '%s' value to '%s'"), 
      pi.name, out
    ));
  } else if (p_prefer_default) {
    debug_print_rich(vformat(
      COLOR_GREEN("Normalized >>> Property's '%s' value from '%s' to '%s'"), 
      pi.name, value, out
    ));
  } else {
    debug_print_rich(vformat(
      COLOR_GREEN("Normalized >>> Property's '%s' value from '%s' to '%s' since previous value was no longer valid"), 
      pi.name, value, out
    ));
  }

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

#pragma endregion HelperFunctions

#pragma region DynamicPropertyInfo

static String get_variant_type_hint_string(ValueNameType p_name_type) {
  String hint_string {};

  for (const auto &entry : variant_type_names_map().get_entries()) {
    if (not hint_string.is_empty()) {
      hint_string += ",";
    }

    hint_string += entry.second[p_name_type];
    hint_string += ":";
    hint_string += String::num_int64(entry.first);
  }

  return hint_string;
}

static String get_property_hint_hint_string(ValueNameType p_name_type) {
  String hint_string {};
  
  for (const auto &entry : property_hint_names_map().get_entries()) {
    if (not hint_string.is_empty()) {
      hint_string += ",";
    }

    hint_string += entry.second[p_name_type];
    hint_string += ":";
    hint_string += String::num_int64(entry.first);
  }

  return hint_string;
}

static String get_property_usage_flags_hint_string(ValueNameType p_name_type) {
  String hint_string {};

  for (const auto &entry : property_usage_names_map().get_entries()) {
    if (property_usage_names_map().get(entry.second[p_name_type]) == PROPERTY_USAGE_DEFAULT) {
      continue;
    }

    if (not hint_string.is_empty()) {
      hint_string += ",";
    }

    hint_string += entry.second[p_name_type];
    hint_string += ":";
    hint_string += String::num_int64(entry.first);
  }

  return hint_string;
}

static const String& get_values_name_type_hint_string() {
  static String hint_string {};

  if (not hint_string.is_empty()) {
    return hint_string;
  }

  for (int type = 0; type < VALUE_NAME_MAX ; type++) {
    if (not hint_string.is_empty()) {
      hint_string += ",";
    }

    hint_string += String(::to_string((ValueNameType)type)).to_pascal_case();
  }

  return hint_string;
}

void DynamicPropertyInfo::_bind_methods() {
  SORUS_BIND_PROPERTY_ENUM_STRING(property_select, PLACEHOLDER_PROPERTY_SELECT);
  SORUS_BIND_PROPERTY_ENUM(type, get_variant_type_hint_string(DEFAULT_NAME_TYPE));
  SORUS_BIND_PROPERTY_ENUM(hint,get_property_hint_hint_string(DEFAULT_NAME_TYPE));
  SORUS_BIND_PROPERTY_MULTILINE_TEXT(hint_string);

  ClassDB::add_property_group(DynamicPropertyInfo::get_class_static(), "Usage bitfield", "");
  SORUS_BIND_PROPERTY_FLAGS(usage, get_property_usage_flags_hint_string(DEFAULT_NAME_TYPE));

  ClassDB::add_property_group(DynamicPropertyInfo::get_class_static(), "Properties", "");
  SORUS_BIND_PROPERTY_DICTIONARY(
    properties_dict,
    vformat(
      "%d:;%d/%d:%s", 
      Variant::Type::STRING_NAME,
      Variant::Type::OBJECT,
      PROPERTY_HINT_RESOURCE_TYPE,
      DynamicPropertyInfo::get_class_static()
    ),
    PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY
  );

  ClassDB::add_property_group(DynamicPropertyInfo::get_class_static(), "Settings", "");
  SORUS_BIND_PROPERTY_ENUM(
    values_name_type,
    get_values_name_type_hint_string()
  );
}

void DynamicPropertyInfo::_validate_property(PropertyInfo &p_property) const {
  StringName name = p_property.name;

  static HashSet<StringName> to_validate = {
    member_type, 
    member_property_select, 
    member_hint, 
    member_hint_string,
    member_usage,
    member_properties_dict,
    member_values_name_type
  };

  if (not to_validate.has(name)) {
    return;
  }

  // if (is_root) {
  //   debug_print_rich(vformat("VALIDATE >>> %s[%s]", this, p_property.name), true);
  // }

  if (name == StringName{member_type}) {
    p_property.hint_string = get_variant_type_hint_string((ValueNameType)values_name_type);
  } else if (name == StringName{member_hint}) {
    p_property.hint_string = get_property_hint_hint_string((ValueNameType)values_name_type);
  } else if (name == StringName{member_usage}) {
    p_property.hint_string = get_property_usage_flags_hint_string((ValueNameType)values_name_type);
  }

  if (not is_root) {
    if (name == StringName{member_properties_dict} || name == StringName{member_values_name_type}) {
      p_property.usage = PROPERTY_USAGE_NONE;
    } else if (name != StringName{member_values_name_type}) {
      p_property.usage |= PROPERTY_USAGE_READ_ONLY;
    }
    return;
  }

  if (name != StringName(member_property_select)) {
    if (this->get_property_select() == StringName{PLACEHOLDER_PROPERTY_SELECT}) {
      p_property.usage = p_property.usage | PROPERTY_USAGE_READ_ONLY;
    }

    return;
  }

  p_property.hint_string = get_property_names_hint_string();
}

String DynamicPropertyInfo::_to_string() const {
  return vformat("%s<%s#%s>", 
    this->get_class_static(), 
    this->get_parent_class_static(), 
    String::num_uint64(get_instance_id())
  );
}

void DynamicPropertyInfo::set_properties_dict(Dictionary p_properties_dict) {
  properties_dict = p_properties_dict;
}

Dictionary DynamicPropertyInfo::get_properties_dict() const {
  return properties_dict;
}

String DynamicPropertyInfo::get_property_names_hint_string() const {
  Array keys = properties_dict.keys();
  String out {};

  for (int i = 0; i < keys.size(); i++) {
    if (not out.is_empty()) {
      out += ",";
    }

    out += (StringName)keys[i];
  }

  return out;
}
  
  #pragma region ::Setters & Getters
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
          properties_dict.get(
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

      void DynamicPropertyInfo::set_values_name_type(int p_name_type) {
        if (values_name_type == p_name_type) {
          return;
        }

        ERR_FAIL_INDEX(p_name_type, VALUE_NAME_MAX);
        
        values_name_type = p_name_type;
        notify_property_list_changed();

        for (const auto &key : properties_dict.keys()) {
          Ref<DynamicPropertyInfo> inner_dpi = properties_dict[key];

          inner_dpi->values_name_type = values_name_type;
          inner_dpi->notify_property_list_changed();
        }
      }

      int DynamicPropertyInfo::get_values_name_type() const {
        ERR_FAIL_INDEX_V(values_name_type, VALUE_NAME_MAX, values_name_type);
        return values_name_type;
      }

    #undef DPI_SETTER
    #undef DPI_GETTER
  #pragma endregion

#pragma endregion DynamicPropertyInfo

#pragma region DPInfoInspectorPlugin

void DynamicPropertyInfoInspectorPlugin::_on_submit() {
  if (root_dpi.is_valid() && root_dpi->is_root) {
    root_dpi->emit_changed();
  }
}

void DynamicPropertyInfoInspectorPlugin::on_dynamic_property_info_changed(Object *p_object, const Ref<DynamicPropertyInfo> &root_dpi) {
  if (p_object == nullptr) {
    return;
  }

  if (root_dpi.is_valid()) {
    Ref<DynamicPropertyInfo> previous = root_dpi->properties_dict.get(root_dpi->property_select, nullptr);
    if (previous.is_null()) {
      return;
    }

    PropertyInfo root_pi = root_dpi->get_property_info();
    String property = root_dpi->get_property_select();

    if (root_pi.hint_string != previous->hint_string) {
      root_dpi->canonical_hint_string = normalize_hint_string(root_pi);
    }
    root_pi.hint_string = root_dpi->canonical_hint_string;

    if (root_dpi->type != previous->type) {
      Variant new_value = normalize_value(root_pi);
      p_object->set(property, new_value);
    } else if (root_dpi->hint != previous->hint) {
      Variant new_value = normalize_value(root_pi, p_object, true);
      p_object->set(property, new_value);
    }
    else if (root_dpi->hint_string != previous->hint_string) {
      Variant new_value = normalize_value(root_pi, p_object);
      p_object->set(property, new_value);
    }
  }

  p_object->notify_property_list_changed();
}

bool DynamicPropertyInfoInspectorPlugin::_can_handle(Object *p_object) const {
  bool can_handle = false;
  bool due_to_guard = false;

  if (p_object == nullptr) {
    goto RET;
  }

  if (creating_native_editor()) {
    due_to_guard = true;
    goto RET;
  }

  if (p_object->is_class(DynamicPropertyInfo::get_class_static())) {
    goto RET;
  }

  can_handle = find_root_dpi(p_object);

  RET:
    debug_print_rich(vformat("CAN HANDLE >>> %s: %s%s", p_object, can_handle, due_to_guard ? " due to loop guard while instantiating property editor" : ""), true);
    return can_handle;
}

void DynamicPropertyInfoInspectorPlugin::_parse_begin(Object *p_object) {
  if (p_object == nullptr) {
    return;
  }

  debug_print_rich(vformat("PARSE BEGIN >>> %s", p_object), true);

  current_d_properties.clear();

  root_dpi.unref();
  root_dpi_name = StringName{};

  if (not find_root_dpi(p_object, &root_dpi, &root_dpi_name)) {
    return;
  }

  if (root_dpi.is_null()) {
    root_dpi.instantiate();
    p_object->set(root_dpi_name, root_dpi);
  }

  if (root_dpi->canonical_hint_string.is_empty()) {
    root_dpi->canonical_hint_string = normalize_hint_string(root_dpi->get_property_info());
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

  if (p_object == nullptr || root_dpi.is_null()) {
    return false;
  }

  if (p_object->is_class(DynamicPropertyInfo::get_class_static())) {
    return false;
  }

  /** 
    If multiple DPIs are present only one can work with the plugin so olders are disconnected, and the newest is connected. Aside from that DPIs are not parsed.
    @todo maybe change this to check if the current one is still valid, change it only if no longer valid.
  */
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

  /**
    Target dpi is the one that focuses on the same property as the current parse 'p_name', this can be the root dpi directly or one serialized in properties_dict.
    If the root_dpi is not target and 'p_name' hasn't been serialized, target dpi holds a new instance and is used to serialize, this ensures that there's
    property information available for properties that haven't even been edited yet, so when changing the selection empty property information is used.
  */
  Ref<DynamicPropertyInfo> target_dpi {root_dpi};
  if (p_name != root_dpi->get_property_select()) {
    target_dpi = root_dpi->get_properties_dict().get(p_name, nullptr);
    if (target_dpi.is_null()) {
      target_dpi = Ref<DynamicPropertyInfo>(memnew(DynamicPropertyInfo({Variant::NIL, p_name})));
    }
  }

  Ref<DynamicPropertyInfo> serialized_dpi = target_dpi->duplicate(true);
  serialized_dpi->is_root = false;
  serialized_dpi->properties_dict.clear();
  root_dpi->properties_dict[p_name] = serialized_dpi;
  current_d_properties.insert(p_name);

  /** 
    Apply a normalzation for types that hold values that might not be valid anymore and are not necesarilly selected i.e. NodePath pointing to a no longer existent node.
  */
  // Ref<DynamicPropertyInfo> previous = root_dpi->properties_dict.get(p_name, nullptr);
  // if (previous.is_valid()) {
  //   Variant normalized = normalize_value(previous->get_property_info(), p_object);
  //   p_object->set(p_name, normalized);
  // }
  
  /**
    Internally, when instantiating a property editor Godot checks every plugin's can handle, if true parses property again.
    To avoid an infinite loop a guard is used (this plugin's 'can_handle()' function returns false is creating_native_editor() == true)
    this way another plugins handles the EditorProperty creation (likely Godot's EditorInspectorDefaultPlugin).

    @see https://github.com/godotengine/godot/blob/65e8d16951d6963cb3984c090e45f40d1ba5f704/editor/inspector/editor_inspector.cpp#L4006
  */
  creating_native_editor() = true;
  
  EditorProperty *editor = EditorInspector::instantiate_property_editor(
    p_object,
    target_dpi->get_type(), 
    p_name, 
    static_cast<PropertyHint>(target_dpi->get_hint()),
    target_dpi->canonical_hint_string,
    target_dpi->get_usage(),
    p_wide
  );

  creating_native_editor() = false;

  editor->set_object_and_property(p_object, p_name);
  this->add_property_editor(p_name, editor);

  debug_print_rich(vformat(
    COLOR_GREEN("Dynamic property '%s' updated in '%s::%s' instance"), p_name, p_object, root_dpi),
    true
  );

  return true;
}

void DynamicPropertyInfoInspectorPlugin::_parse_end(Object *p_object) {
  if (p_object == nullptr) {
    return;
  }

  /**
    Find the right node to add the "Apply" button to and then add it
  */
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
    
    if (button_target_node == nullptr) {
      button_target_node = memnew(HBoxContainer);
      root_dpi_inspector_node->add_child(button_target_node);
    }

    Button *submit_button = memnew(Button);
    submit_button->set_text("Apply");
    submit_button->connect("pressed", callable_mp(this, &DynamicPropertyInfoInspectorPlugin::_on_submit));
    button_target_node->add_child(submit_button);
  }

  /**
    Clear properties no longer available from root dpi. @todo If a no longer available property is still selected, remove it
  */
  bool props_removed = false;
  if (root_dpi.is_valid()) {
    Array keys = root_dpi->properties_dict.keys();
    for (int i = 0; i < keys.size() ; i++) {
      if (not current_d_properties.has(StringName(keys[i]))) {
        root_dpi->properties_dict.erase(keys[i]);
        props_removed = true;
      }
    }
  }

  if (props_removed) {
    p_object->call_deferred("notify_property_list_changed");
  } else {
    debug_print_rich(vformat(COLOR_GREEN("%s - Inspector updated by %s"), p_object, this->get_class_static()));
  }
}

#pragma endregion DPInfoInspectorPlugin

#pragma region DPInfoEditorPlugin

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

#pragma endregion DPInfoEditorPlugin

