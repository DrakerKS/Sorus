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
#include <charconv>
#include <cstddef>
#include <cstdint>

using namespace godot;

#pragma region HelperFunctions

static Variant normalize_value(const PropertyInfo &p_property_info, const Variant &p_value = nullptr, const Node *p_node_context = nullptr, bool p_type_strict = false);
static String normalize_hint_string(const PropertyInfo &p_property_info, bool p_verbose = true);

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
static String normalize_hint_string(const PropertyInfo &p_property_info, bool p_verbose /* = true */) {
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
    if (p_verbose) {
      debug_print_rich(vformat(COLOR_GREEN("Normalized >>> Hint string from '%s' to '%s'"), p_hint_string, out));
    }

    return out;
}

/** @note Reviewed++ 
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

  /** @todo Maybe do something to take into account the step, since a value of 5.5 in a Float/Range:0,10,1 should not be possible */
  #define NORMALIZE_TYPE(m_type) do { \
    if (not hint_string_entries[0].JOIN(is_valid,m_type)() || not hint_string_entries[1].JOIN(is_valid,m_type)()) { \
      return {}; \
    } \
    if (p_value.get_type() == Variant::NIL) { \
      return hint_string_entries[0].JOIN(to,m_type)(); \
    } \
    m_type min = (hint_string_entries.has("or_less"))? (m_type)p_value : hint_string_entries[0].JOIN(to,m_type)(); \
    m_type max = (hint_string_entries.has("or_greater"))? (m_type)p_value : hint_string_entries[1].JOIN(to,m_type)(); \
    m_type clamped = CLAMP((m_type)p_value, min, max); \
    return clamped; \
   } while(false) 

  /** @todo Check weird floating point cases */
  if (type == Variant::INT) {
    NORMALIZE_TYPE(int);
  } else {
    NORMALIZE_TYPE(float);
  }

  #undef NORMALIZE_TYPE
}

/** @note Reviewed++ 
  @brief 
    From a hint string "Op1,Op2,Op3:Op3Val,Op4,..." ensures p_value is equal to one of these values. Follows Godot's rules for Enum:hint_string formatting.

  @returns 
    Null variant if hint string is empty.  
  @returns 
    Same value if suitable.
  @returns
    Default value first entry's value if not suitable, or if default is preferred as int/Enum.

  @note Parameter 'p_prefer_default' only matters if int typed.
*/
static Variant normalize_enum(const PropertyInfo &p_property_info, const Variant &p_value) {
  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {};
  }

  String first_entry = hint_string_entries[0];
  if (p_property_info.type != Variant::INT) {
    if (hint_string_entries.has(p_value)) {
      return {p_value};
    }

    return (p_property_info.type == Variant::STRING_NAME)? Variant(StringName(first_entry)) : Variant(first_entry);
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
      first_visit = false;
    }

    if ((int64_t)p_value == last_visited) {
      return {(int64_t)p_value};
    }
  }

  return {first_entry_numeric};
}

/** @note Reviewed++ 
  @brief
    From a hint string optionally containing "positive_only" ensures p_value is not negative.

  @returns
    Default value 0.0 if "positive_only" is present and p_value is negative.
  @returns
    Same value otherwise.
*/
static Variant normalize_exp_easing(const PropertyInfo &p_property_info, const Variant &p_value) {
  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",",false);
  if ((hint_string_entries.has("positive_only") && (float)p_value < 0)) {
    return {0.0f};
  }

  return {(float)p_value};
}

/** @note Reviewed++ 
  @brief
    From a hint string "Bit0,Bit1,Bit2:Bit2Val,Bit3,..." ensures p_value only contains known bits. Follows Godot's rules for Flags:hint_string formatting.

  @returns
    Null variant if the hint string is empty.
  @returns
    Same value with unknown bits masked out.

  @note
    If Bit32 is reached in the hint string, this and following entries must specify a value (BitN:val), otherwise they will be ignored.
*/
static Variant normalize_flags(const PropertyInfo &p_property_info, const Variant &p_value) {
  /** For flags hint string's entry position is absolute truth if no specific value is provided "a:16,b,c" a = 16, b = 2, c = 4 */
  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {};
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

/** @note Reviewed++ 
  @brief
    From a hint string containing file name filters such as "*.png,*.jpg" ensures p_value contains a valid file name matching one of the filters. If p_global is true, also ensures the path is absolute.

  @returns
    Same value if the file name is valid and the hint string is empty, or if the file name matches one of the hint string filters.
  @returns
    Null variant if p_global is true and the path is not absolute, if the file name is invalid, or if the file name does not match any hint string filter.
*/
static Variant normalize_file(const PropertyInfo &p_property_info, const Variant &p_value, bool p_global = false) {
  String path = (String)p_value.duplicate();
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
    return {(String)p_value};
  }

  for (String &entry : hint_string_entries) {
    if (filename.match(entry)) {
      return {(String)p_value};
    }
  }

  return {};
}

/** @note Reviewed++ 
  @brief
    Ensures p_value is an absolute path.

  @returns
    Same value if p_value is an absolute path.
  @returns
    Null variant if p_value is not an absolute path.
*/
static Variant normalize_global_dir(const PropertyInfo &p_property_info, const Variant &p_value) {
  if (String(p_value).is_absolute_path()) {
    return {(String)p_value};
  }

  return {};
}

/** @note Reviewed++ 
  @brief
    From a hint string containing resource class names such as "Texture2D,ShaderMaterial" ensures p_value is a Resource whose class matches one of the specified types.

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
  Ref<Resource> out {p_value};
  if (out.is_null()) {
    return {};
  }

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {(Object*)p_value};
  }

  for (String &entry : hint_string_entries) {
    if (out->is_class(entry)) {
      return {(Object*)p_value};
    }
  }

  return {};
}

/** @note Reviewed++ 
  @brief
    Normalizes a value according to a PROPERTY_HINT_TYPE_STRING definition.

  @returns
    For String: Original value if it matches a registered ClassDB class in the hint string, null otherwise.
  @returns
    For Array: Normalized Array with invalid elements removed and salvageable elements normalized.
  @returns
    For Dictionary: Normalized Dictionary with invalid keys or values removed. Unsuitable keys are removed, values normalized.

  @param r_removed_indexes
    Optional output Array containing the original indexes of removed elements from an Array.
*/
static Variant normalize_type_string(const PropertyInfo &p_property_info, const Variant &p_value, const Node *p_node_context, Array *r_removed_indexes = nullptr, bool p_remove_if_normalized = false) {
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
      return {(String)p_value};
    }

    for (const String &entry : hint_string_entries) {
      if (String(p_value) == entry && ClassDBSingleton::get_singleton()->is_class(entry)) {
        return {(String)p_value};
      }
    }

    return {};
  }

  if (type == Variant::ARRAY) {
    PropertyInfo pi {hint_string_to_pi(p_property_info.hint_string)};
    Array array = p_value.duplicate(true);

    int deleted = 0;
    int size = array.size();
    for (int i = 0; i < size; i++) {
      pi.name = p_property_info.name + vformat("[%s]", i);
      Variant curr_normalized = normalize_value(pi, array[i - deleted], p_node_context, true);
      
      // if ((curr_normalized.get_type() == Variant::NIL && pi.type != Variant::OBJECT) || (p_remove_if_normalized && curr_normalized != array[i - deleted])) {
      // if ((curr_normalized.get_type() == Variant::NIL && pi.type != Variant::OBJECT)) {
      //   array.remove_at(i - deleted++);
      //   if (r_removed_indexes) {
      //     r_removed_indexes->push_back(i);
      //   }
      // } else {
        array.set(i - deleted, curr_normalized);
      // }
    }

    return {array};
  }
  
  if (type == Variant::DICTIONARY) {
    Dictionary dict = p_value.duplicate(true);
    int key_val_separator = p_property_info.hint_string.find(";");
    if (key_val_separator < 0) {
      return {p_value};
    }

    Array keys = dict.keys();
    Array values {};
    Array removed_indexes {};

    /** Key side */
    PropertyInfo pi_keys {
      Variant::ARRAY, 
      p_property_info.name + String(":keys"),
      PROPERTY_HINT_TYPE_STRING,
      p_property_info.hint_string.substr(0, key_val_separator)
    };

    normalize_type_string(pi_keys, keys, p_node_context, &removed_indexes, true);

    for (int idx : removed_indexes) {
      dict.erase(keys[idx]);
    }

    /** Value side */
    keys = dict.keys();
    for (const Variant &key : keys) values.push_back(dict[key]);
    int normalized_idx = 0;
    removed_indexes.clear();
    
    PropertyInfo pi_values {
      Variant::ARRAY, 
      p_property_info.name + String(":key_filtered_values"),
      PROPERTY_HINT_TYPE_STRING,
      p_property_info.hint_string.substr(key_val_separator + 1)
    };
    
    values = normalize_type_string(pi_values, values, p_node_context, &removed_indexes);
    
    for (int i = 0; i < keys.size(); i++) {
      if (removed_indexes.has(i)) {
        dict.erase(keys[i]);
      } else {
        dict[keys[i]] = values[normalized_idx++];
      }
    }
    
    return {dict};
  }

  return {p_value};
}

/** @note Reviewed++ 
  @brief
    From a hint string containing node class names such as "Node2D,Sprite2D" ensures p_value points to an existing Node whose class matches one of the specified types.

  @returns
    Same value if no node context is provided, i.e: p_value is a Resource's property, therefore, lacks context to validate such value.
  @returns
    Null variant if the NodePath does not point to an existing Node.
  @returns
    Same value if the hint string is empty.
  @returns
    Same value if the target Node matches one of the hint string node types.
  @returns
    Null variant if the target Node does not match any hint string node type.

  @note
    Validation is performed relative to p_object, which must be a Node containing the target path.
*/
static Variant normalize_node_path_valid_types(const PropertyInfo &p_property_info, const Variant &p_value, const Node *p_node_context) {
  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (p_node_context == nullptr) {
    return {(NodePath)p_value};
  }

  const Node *target = p_node_context->get_node_or_null((NodePath)p_value);
  if (target == nullptr) {
    return {};
  }

  if (hint_string_entries.is_empty()) {
    return {(NodePath)p_value};
  }

  for (String &entry : hint_string_entries) {
    if (target->is_class(entry)) {
      return {(NodePath)p_value};
    }
  }

  return {};
}

/** @note Reviewed++ 
  @brief
    Ensures p_value contains a locale identifier with a recognized language and, if present, a recognized script or country as its second component.

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

  return {(String)p_value};
}

/** @note Reviewed++ 
  @brief
    From a hint string containing node class names such as "Node,Node3D" ensures p_value is a Node whose class matches or inherits one of the specified types.
    Also ensures that the Node exists, is inside the SceneTree, belongs to the same SceneTree as the context node, and is accessible from it.

  @returns
    Null variant if p_value cannot be cast to a Node or if the node is not inside a scene tree, if it is in a different tree than p_node_context 
    or is not accessible through p_node_context.
  @returns
    Same value if the hint string is empty.
  @returns
    Same value if the Node matches one of the hint string node types.
  @returns
    Null variant if the Node does not match any hint string node type.
*/
static Variant normalize_node_type(const PropertyInfo &p_property_info, const Variant &p_value, const Node* p_node_context) {
  Node *node = Object::cast_to<Node>((Object*) p_value);
  if (node == nullptr) {
    return {};
  }

  if (p_node_context == nullptr) {
    return {};
  }

  if (not node->is_inside_tree() || not p_node_context->is_inside_tree() || node->get_tree() != p_node_context->get_tree()) {
    return {};
  }

  NodePath path = p_node_context->get_path_to(node);
  if (p_node_context->get_node_or_null(path) != node) {
    return {};
  }

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",", false);
  if (hint_string_entries.is_empty()) {
    return {node};
  }

  for (String &entry : hint_string_entries) {
    if (node->is_class(entry)) {
      return {node};
    }
  }

  return {};
}

/** @note Reviewed++ 
  @brief
    Ensures p_value conforms to the input action restrictions specified by the hint string.

  @returns
    Same value if p_value is empty.
  @returns
    Null variant if "show_builtin" is present, "loose_mode" is absent, and p_value does not match an existing InputMap action.
  @returns
    Same value otherwise.
*/
static Variant normalize_input_name(const PropertyInfo &p_property_info, const Variant &p_value) {
  String input = String(p_value);
  if (input.is_empty()) {
    return {input};
  }

  const TypedArray<StringName> &actions = InputMap::get_singleton()->get_actions();

  PackedStringArray hint_string_entries = p_property_info.hint_string.split(",");
  if (hint_string_entries.has("show_builtin") && not hint_string_entries.has("loose_mode")) {
    if (not actions.has(input)) {
      return {};
    }
  }

  return {input};
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
static Variant normalize_value(const PropertyInfo &p_property_info,  const Variant &p_value,  const Node *p_node_context /* = nullptr*/, bool p_type_strict /* = false */) {
  Variant out {};

  PropertyInfo pi {p_property_info};
  if (pi.type == Variant::ARRAY || pi.type == Variant::DICTIONARY) {
    if (pi.hint == PROPERTY_HINT_NONE) {
      pi.hint = PROPERTY_HINT_TYPE_STRING;
    }
  }
  
  /**
    In this case Variant::NIL could mean "any type" in containers, so it doesn't make sense to be strict here.
  */
  if (p_type_strict && pi.type != Variant::NIL) {
    if (p_value.get_type() != pi.type && p_value.get_type() != Variant::NIL) {
      return {};
    }
  }

  /** 
    When reading case annotations assume Type/Hint combination is compatible. Don't worry when this is not true as any incompatible Type/Hint 
    combination behaves exactly the same as Type/HintNone, even inside normalize_*() functions.
  */
  switch ((PropertyHint) pi.hint) {
    #define PASS_OR_NULL p_type_strict? Variant{} : p_value
    case PROPERTY_HINT_NONE:
      out = p_value; /** No restrictions, no need to normalize. */
      break;
    case PROPERTY_HINT_RANGE:
      if (pi.type != Variant::INT && pi.type != Variant::FLOAT && pi.type != Variant::ARRAY) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_range(pi, p_value);
      }
      break;
    case PROPERTY_HINT_ENUM:
      if (pi.type != Variant::INT && pi.type != Variant::STRING && pi.type != Variant::STRING_NAME) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_enum(pi, p_value);
      }
      break;
    case PROPERTY_HINT_ENUM_SUGGESTION:
      if (pi.type != Variant::STRING && pi.type != Variant::STRING_NAME) {
        out = PASS_OR_NULL;
      } else {
        out = p_value; /** Any value is valid, no need to normalize. */
      }
      break;
    case PROPERTY_HINT_EXP_EASING:
      if (pi.type != Variant::FLOAT) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_exp_easing(pi, p_value);
      }
      break;
    case PROPERTY_HINT_LINK:
      /** Hint string is a visual suffix only. No need to normalize. */
      if (not variant_type_names_map().get(pi.type)[VALUE_NAME_SHORT].begins_with("VECTOR")) {
        out = PASS_OR_NULL;
      } else {
        out = p_value;
      }
      break;
    case PROPERTY_HINT_FLAGS:
      if (pi.type != Variant::INT) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_flags(pi, p_value);
      }
      break;
    case PROPERTY_HINT_LAYERS_2D_RENDER:
    case PROPERTY_HINT_LAYERS_2D_PHYSICS:
    case PROPERTY_HINT_LAYERS_2D_NAVIGATION:
    case PROPERTY_HINT_LAYERS_3D_RENDER:
    case PROPERTY_HINT_LAYERS_3D_PHYSICS:
    case PROPERTY_HINT_LAYERS_3D_NAVIGATION:
    case PROPERTY_HINT_LAYERS_AVOIDANCE:
      /** These cases don't use the hint string. These are bitmasks that use layers already defined in project settings, thus, there's no need to normalize. Any p_value could be valid, but not every type can use it correctly. */
      if (pi.type != Variant::INT) {
        out = PASS_OR_NULL;
      } else {
        out = p_value;
      }
      break;
    case PROPERTY_HINT_FILE:
      if (pi.type != Variant::STRING) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_file(pi, p_value);
      }
      break;
    case PROPERTY_HINT_DIR:
      /** Any value is valid, no need to normalize. */
      if (pi.type != Variant::STRING) {
        out = PASS_OR_NULL;
      } else {
        out = p_value;
      }
      break;
    case PROPERTY_HINT_GLOBAL_FILE:
      if (pi.type != Variant::STRING) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_file(pi, p_value, true);
      }
      break;
    case PROPERTY_HINT_GLOBAL_DIR:
      if (pi.type != Variant::STRING) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_global_dir(pi, p_value);
      }
      break;
    case PROPERTY_HINT_RESOURCE_TYPE:
      if (pi.type != Variant::OBJECT) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_resource_type(pi, p_value);
      }
      break;
    case PROPERTY_HINT_MULTILINE_TEXT:
      /** Hint string is visual only, no need to normalize */
      if (pi.type != Variant::STRING) {
        out = PASS_OR_NULL;
      } else {
        out = p_value;
      }
      break;
    case PROPERTY_HINT_EXPRESSION:
      /** Doesn't use the hint string, no need to normalize */
      if (pi.type != Variant::STRING) {
        out = PASS_OR_NULL;
      } else {
        out = p_value;
      }
      break;
    case PROPERTY_HINT_PLACEHOLDER_TEXT:
      /** Hint string is visual only, no need to normalize */
      if (pi.type != Variant::STRING) {
        out = PASS_OR_NULL;
      } else {
        out = p_value;
      }
      break;
    case PROPERTY_HINT_COLOR_NO_ALPHA:
      /** Doesn't use the hint string, no need to normalize */
      if (pi.type != Variant::COLOR) {
        out = PASS_OR_NULL;
      } else {
        out = p_value;
      }
      break;
    case PROPERTY_HINT_OBJECT_ID:
      /** Can't understand this, but has something to do with the debugger so, no need to normalize? */
      out = p_value;
      break;
    case PROPERTY_HINT_TYPE_STRING:
      if (pi.type != Variant::STRING && pi.type != Variant::ARRAY && pi.type != Variant::DICTIONARY) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_type_string(pi, p_value, p_node_context);
      }
      break;
    case PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE:
      /** @deprecated Not used by godot anymore, no need to normalize */
      out = p_value;
      break;
    case PROPERTY_HINT_OBJECT_TOO_BIG:
      /** Internal info for godot only, no need to normalize */
      out = p_value;
      break;
    case PROPERTY_HINT_NODE_PATH_VALID_TYPES:
      if (pi.type != Variant::NODE_PATH) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_node_path_valid_types(pi, p_value, p_node_context);
      }
      break;
    case PROPERTY_HINT_SAVE_FILE:
      if (pi.type != Variant::STRING) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_file(pi, p_value);
      }
      break;
    case PROPERTY_HINT_GLOBAL_SAVE_FILE:
      if (pi.type != Variant::STRING) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_file(pi, p_value, true);
      }
      break;
    case PROPERTY_HINT_INT_IS_OBJECTID:
      /** @deprecated Not used by godot anymore, no need to normalize */
      out = p_value;
      break;
    case PROPERTY_HINT_INT_IS_POINTER:
      /** Internal info for godot only, no need to normalize */
      if (pi.type != Variant::INT) {
        out = PASS_OR_NULL;
      } else {
        out = p_value;
      }
      break;
    case PROPERTY_HINT_ARRAY_TYPE:
      if (pi.type != Variant::ARRAY) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_type_string(pi, p_value, p_node_context);
      }
      break;
    case PROPERTY_HINT_DICTIONARY_TYPE:
      if (pi.type != Variant::DICTIONARY) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_type_string(pi, p_value, p_node_context);
      }
      break;
    case PROPERTY_HINT_LOCALE_ID:
      if (pi.type != Variant::STRING) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_locale_id(pi, p_value);
      }
      break;
    case PROPERTY_HINT_LOCALIZABLE_STRING:
      if (pi.type != Variant::DICTIONARY) {
        out = PASS_OR_NULL;
      } else {
        pi.hint_string = vformat("%s/%s:;%s/0:", 
          String::num_int64(Variant::STRING), 
          String::num_int64(PROPERTY_HINT_LOCALE_ID),
          String::num_int64(Variant::STRING)
        );
        out = normalize_type_string(pi, p_value, p_node_context);
      }
      break;
    case PROPERTY_HINT_NODE_TYPE:
      if (pi.type != Variant::OBJECT) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_node_type(pi, p_value, p_node_context);
      }
      break;
    case PROPERTY_HINT_HIDE_QUATERNION_EDIT:
      /** Doesn't use the hint string, no need to normalize */
      if (pi.type != Variant::QUATERNION) {
        out = PASS_OR_NULL;
      } else {
        out = p_value;
      }
      break;
    case PROPERTY_HINT_PASSWORD:
      /** Visual only, no need to normalize */
      out = p_value;
      break;
    case PROPERTY_HINT_TOOL_BUTTON:
      /** Hint string is visual only, no need to normalize */
      if (pi.type != Variant::CALLABLE) {
        out = PASS_OR_NULL;
      } else {
        out = p_value;
      }
      break;
    case PROPERTY_HINT_ONESHOT: 
    /** Can't understand this, but seems to not use hint string, so, no need to normalize? */
      out = p_value;
      break;
    case PROPERTY_HINT_GROUP_ENABLE:
      /** Hint string is visual only, no need to normalize */
      if (pi.type != Variant::BOOL) {
        out = PASS_OR_NULL;
      } else {
        out = p_value;
      }
      break;
    case PROPERTY_HINT_INPUT_NAME:
      if (pi.type != Variant::STRING && pi.type != Variant::STRING_NAME) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_input_name(pi, p_value);
      }
      break;
    case PROPERTY_HINT_FILE_PATH:
      if (pi.type != Variant::STRING) {
        out = PASS_OR_NULL;
      } else {
        out = normalize_file(pi, p_value);
      }
      break;
    case PROPERTY_HINT_MAX:
      WARN_PRINT("Attempted to normalize a value using \"PROPERTY_HINT_MAX\" hint. Input 'p_value' is returned.");
      out = p_value;
    #undef PASS_OR_NULL
  }

  if (out.get_type() == Variant::NIL) {
    if (p_type_strict) {
      return {};
    }

    out = get_variant_default_value(pi.type);
  }

  if (out == p_value) {
    return out;
  }

  if (p_value.get_type() == Variant::NIL) {
    debug_print_rich(vformat(
      COLOR_GREEN("Normalized >>> Property '%s' value to '%s'"), 
      pi.name, out
    ), true);
  } else {
    debug_print_rich(vformat(
      COLOR_GREEN("Normalized >>> Property '%s' value from '%s' to '%s' since previous value was no longer suitable"), 
      pi.name, p_value, out
    ), true);
  }

  return out;
}

#pragma endregion HelperFunctions

#pragma region DynamicPropertyInfo

String DynamicPropertyInfo::get_variant_type_hint_string(ValueNameType p_name_type) {
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

String DynamicPropertyInfo::get_property_hint_hint_string(ValueNameType p_name_type, bool p_is_owner_a_resource) {
  String hint_string {};
  
  for (const auto &entry : property_hint_names_map().get_entries()) {
    if (p_is_owner_a_resource && entry.first == PROPERTY_HINT_NODE_TYPE) {
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

String DynamicPropertyInfo::get_property_usage_flags_hint_string(ValueNameType p_name_type) {
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

const String& DynamicPropertyInfo::get_values_name_type_hint_string() {
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

void DynamicPropertyInfo::_bind_methods() {
  SORUS_BIND_PROPERTY_ENUM_STRING(property_select, PLACEHOLDER_PROPERTY_SELECT);
  SORUS_BIND_PROPERTY_ENUM(type, get_variant_type_hint_string(DEFAULT_NAME_TYPE));
  SORUS_BIND_PROPERTY_ENUM(hint,get_property_hint_hint_string(DEFAULT_NAME_TYPE, true)); /** @note Assuming a resource here prevents NodeType being available for Resources in case of any weird scenario where the property hasn't been validated */
  SORUS_BIND_PROPERTY_MULTILINE_TEXT(hint_string);

  ClassDB::add_property_group(DynamicPropertyInfo::get_class_static(), "Usage bitfield", "");
  SORUS_BIND_PROPERTY_FLAGS(usage, get_property_usage_flags_hint_string(DEFAULT_NAME_TYPE));

  ClassDB::add_property_group(DynamicPropertyInfo::get_class_static(), "Properties", "");
  SORUS_BIND_PROPERTY_DICTIONARY(properties_dict,
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
  SORUS_BIND_PROPERTY_ENUM(values_name_type, get_values_name_type_hint_string());
}

void DynamicPropertyInfo::_validate_property(PropertyInfo &p_property) const {
  StringName name = p_property.name;

  /** Config hint strings for any DPI */
  if (name == StringName{member_type}) {
    p_property.hint_string = get_variant_type_hint_string((ValueNameType)values_name_type);
  } else if (name == StringName{member_hint}) {
    p_property.hint_string = get_property_hint_hint_string((ValueNameType)values_name_type, is_owner_a_resource);
  } else if (name == StringName{member_usage}) {
    p_property.hint_string = get_property_usage_flags_hint_string((ValueNameType)values_name_type);
  }

  /** Exclude dictionary and name type selector for dictionary's DPIs only, make everything else read only */
  if (not is_root) {
    if (name == StringName{member_properties_dict} || name == StringName{member_values_name_type}) {
      p_property.usage = PROPERTY_USAGE_NONE;
    } else {
      p_property.usage |= PROPERTY_USAGE_READ_ONLY;
    }
    return;
  }

  /** In root DPI make everything but property selector read only if no current selection */
  if (name != StringName(member_property_select)) {
    if (this->property_select == StringName{PLACEHOLDER_PROPERTY_SELECT}) {
      p_property.usage = p_property.usage | PROPERTY_USAGE_READ_ONLY;
    }
    return;
  }

  /** If root DPI fill the property selector with the detected dynamic properties */
  p_property.hint_string = get_property_names_hint_string();
}

String DynamicPropertyInfo::_to_string() const {
  return vformat("%s<%s#%s>", 
    this->get_class_static(), 
    this->get_parent_class_static(), 
    String::num_uint64(get_instance_id())
  );
}

PropertyInfo DynamicPropertyInfo::get_property_info(bool p_canonical_hint_string, bool p_verbose) {
  PropertyInfo property_info {
    type,
    property_select,
    static_cast<PropertyHint>(hint),
    hint_string,
    usage
  };

  if (p_canonical_hint_string) {
    property_info.hint_string = normalize_hint_string(property_info, p_verbose);
  }

  return property_info;
}
  
  #pragma region ::Setters & Getters
    #define DPI_SETTER(m_type, m_member) \
      void DynamicPropertyInfo::SORUS_SETTER_TOKEN(m_member)(m_type P_TOKEN(m_member)) { \
        m_member = P_TOKEN(m_member); \
      } END_MACRO()

    #define DPI_GETTER(m_type, m_member) \
      m_type DynamicPropertyInfo::SORUS_GETTER_TOKEN(m_member)() const { \
        return static_cast<m_type>(m_member); \
      } END_MACRO()

      void DynamicPropertyInfo::set_property_select(StringName p_property_select) {
        if (property_select == p_property_select) {
          return;
        }
        property_select = p_property_select;

        if (p_property_select == StringName(DynamicPropertyInfo::PLACEHOLDER_PROPERTY_SELECT)) {
          type = Variant::NIL;
          hint = PROPERTY_HINT_NONE;
          hint_string = "";
          usage = PROPERTY_USAGE_DEFAULT;
        } else {
          Ref<DynamicPropertyInfo> serialized_dpi = 
          properties_dict.get(
            p_property_select, 
            nullptr
          );

          if (serialized_dpi.is_valid()) {
            type = serialized_dpi->type;
            hint = serialized_dpi->hint;
            hint_string = serialized_dpi->hint_string;
            usage = serialized_dpi->usage;
          }
        }

        if (is_root) {
          this->emit_changed();
        }
      }

      StringName DynamicPropertyInfo::get_property_select() const {
        return property_select;
      }

      DPI_SETTER(Variant::Type, type);
      DPI_GETTER(Variant::Type, type);

      DPI_SETTER(uint32_t, hint);
      DPI_GETTER(uint32_t, hint);

      DPI_SETTER(String, hint_string);
      DPI_GETTER(String, hint_string);

      DPI_SETTER(uint32_t, usage);
      DPI_GETTER(uint32_t, usage);

      DPI_SETTER(Dictionary, properties_dict);
      DPI_GETTER(Dictionary, properties_dict);

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

bool DynamicPropertyInfoInspectorPlugin::find_root_dpi(const Object *p_object, Ref<DynamicPropertyInfo> *r_dpi /** = nullptr */, StringName *r_dpi_name /** = nullptr */) {
  if (p_object == nullptr) {
    return false;
  }

  if (r_dpi && r_dpi->is_valid() && r_dpi_name && Ref<DynamicPropertyInfo>{p_object->get(*r_dpi_name)} == *r_dpi) {
    return true;
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

void DynamicPropertyInfoInspectorPlugin::find_inspector_property_editors(HashMap<ObjectID, ObjectParseContext> &p_object_parse_context_map) {
  size_t found = 0;

  auto find = [&](auto&& self, Node* current_node) -> void {
    if (found == p_object_parse_context_map.size()) {
      return;
    }

    if (current_node == nullptr) {
      current_node = EditorInterface::get_singleton()->get_inspector();
    }

    EditorProperty* likely_target = Object::cast_to<EditorProperty>(current_node);
    if (likely_target != nullptr) {
      Object* edited = likely_target->get_edited_object();
      if (edited != nullptr) {
        ObjectID id {edited->get_instance_id()};
        if (p_object_parse_context_map.has(id) && p_object_parse_context_map[id].root_dpi_name == likely_target->get_edited_property()) {
          p_object_parse_context_map[id].editor_property = likely_target;
          found++;
        }
      }
    }

    TypedArray<Node> children = current_node->get_children();

    for (int i = 0; i < children.size(); i++) {
      Node* child = Object::cast_to<Node>((Object*)children[i]);
      if (child != nullptr) {
        self(self, child);
      }
    }
  };

  find(find, nullptr);
}

void DynamicPropertyInfoInspectorPlugin::normalize_property(Object *p_object, const StringName &p_property_name) {
  if (p_object == nullptr || p_property_name.is_empty()) {
    return;
  }

  Ref<DynamicPropertyInfo> root_dpi = nullptr;
  if (ObjectID id{p_object->get_instance_id()}; object_parse_context_map.has(id)) {
    root_dpi = object_parse_context_map.get(id).root_dpi;
  }

  Ref<DynamicPropertyInfo> target_dpi {root_dpi}; /** Assuming 'p_name' is currently selected */
  Ref<DynamicPropertyInfo> previous = root_dpi->properties_dict.get(p_property_name, nullptr);
  if (root_dpi->property_select != p_property_name) {
    if (previous.is_null()) {
      previous = Ref<DynamicPropertyInfo>(memnew(DynamicPropertyInfo({Variant::NIL, p_property_name})));
    }
    target_dpi = previous;
  }
  
  Variant normalized, value = p_object->get(p_property_name);
  if (target_dpi != previous && target_dpi->type != previous->type) {
    normalized = normalize_value(root_dpi->get_property_info(true, true));
  } else {
    normalized = normalize_value(target_dpi->get_property_info(true, false), value, Object::cast_to<Node>(p_object));
  }

  print_line("Normalized >>> ", normalized);
  if (normalized != value) {
    p_object->set(p_property_name, normalized);
  }
}

void DynamicPropertyInfoInspectorPlugin::on_dynamic_property_info_changed(Object *p_object, const Ref<DynamicPropertyInfo> &root_dpi) {
  if (p_object == nullptr) {
    return;
  }

  p_object->notify_property_list_changed();
}

void DynamicPropertyInfoInspectorPlugin::on_submit(const Ref<DynamicPropertyInfo> p_root_dpi) {
  if (p_root_dpi.is_valid() && p_root_dpi->is_root) {
    p_root_dpi->emit_changed();
  }
}

void DynamicPropertyInfoInspectorPlugin::on_property_changed(StringName p_property_name, Variant p_value, StringName p_field, bool p_changing, Object *p_object) {
  if (p_object == nullptr || p_property_name.is_empty()) {
    return;
  }

  callable_mp(this, &DynamicPropertyInfoInspectorPlugin::normalize_property).bind(p_object, p_property_name).call_deferred();
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
    debug_print_rich(vformat(
      "Can handle >>> %s: %s%s", 
      p_object, can_handle, (due_to_guard ? " due to loop guard while instantiating property editor" : "")), 
      true
    );
    return can_handle;
}

void DynamicPropertyInfoInspectorPlugin::_parse_begin(Object *p_object) {
  if (p_object == nullptr) {
    return;
  }

  parse_first_responder = (parse_first_responder == nullptr)? p_object : parse_first_responder;
  if (p_object == parse_first_responder) {
    object_parse_context_map.clear();
  }

  ObjectID id {p_object->get_instance_id()};
  object_parse_context_map[id] = ObjectParseContext{};


  ObjectParseContext &context = object_parse_context_map[id];
  Ref<DynamicPropertyInfo> &root_dpi = context.root_dpi;
  StringName &root_dpi_name = context.root_dpi_name;

  if (not find_root_dpi(p_object, &root_dpi, &root_dpi_name)) {
    object_parse_context_map.erase(id);
    return;
  }

  if (root_dpi.is_null()) {
    root_dpi.instantiate();
    p_object->set(root_dpi_name, root_dpi);
  }

  if (p_object->is_class(Resource::get_class_static())) {
    root_dpi->is_owner_a_resource = true;
  }

  debug_print_rich(vformat("Parse begin >>> %s - %s", p_object, root_dpi), true);

  Callable bound_callable = callable_mp_static(&DynamicPropertyInfoInspectorPlugin::on_dynamic_property_info_changed).bind(p_object, root_dpi);
  if (not root_dpi->is_connected(SIGNAL(root_dpi, changed), bound_callable)) {
    root_dpi->connect(SIGNAL(root_dpi, changed), bound_callable);
  }

  root_dpi->is_root = true;
}

bool DynamicPropertyInfoInspectorPlugin::_parse_property(
  Object *p_object, 
  Variant::Type p_type, 
  const String &p_name, 
  PropertyHint p_hint, 
  const String &p_hint_string, 
  BitField<PropertyUsageFlags> p_usage, 
  bool p_wide
) { 
  if (not p_name.begins_with(STD_DYNAMIC_PROP_PREFIX)) {
    return false;
  }

  if (p_object == nullptr) {
    return false;
  }

  if (p_type == Variant::OBJECT && p_hint_string == DynamicPropertyInfo::get_class_static()) { 
    return false;
  }

  /** Obtain this object's parsing context 
    A single inspector can have multiple embedded DPIs, the 'object_parse_context_map' avoids collisions and ensures every DPI instance works independently
    for each object by linking every handleable object's id with their respective parsing context, including root_dpi, root_dpi property name
    and the names of the currently parsed properties.
  */
  ObjectID id {p_object->get_instance_id()}; 
  if (not object_parse_context_map.has(id)) return false;
  ObjectParseContext &context = object_parse_context_map[id];
  Ref<DynamicPropertyInfo> &root_dpi = context.root_dpi;
  context.current_parsed_properties.insert(p_name);

  /** Obtain the target_dpi.
    Target dpi is the one that focuses on the same property as the current parse 'p_name', this can be the root dpi directly or one serialized in properties_dict.
    If the root_dpi is not target and 'p_name' hasn't been serialized, target dpi holds a new instance and is used to serialize, this ensures that there's
    property information available for properties that haven't even been edited yet, so when changing the selection empty property information is used.
  */
  Ref<DynamicPropertyInfo> target_dpi {root_dpi}; /** Assuming 'p_name' is currently selected */
  if (p_name != root_dpi->property_select) {
    target_dpi = root_dpi->properties_dict.get(p_name, nullptr);
    if (target_dpi.is_null()) {
      target_dpi = Ref<DynamicPropertyInfo>(memnew(DynamicPropertyInfo({Variant::NIL, p_name})));
    }
  }

  /** Normalize potentially stale value of this property. 
    Apply a normalzation for types that hold values that might not be valid anymore and are not necesarilly selected:
      I.e. NodePath pointing to a no longer existent node.
  */
  normalize_property(p_object, p_name);

  /** Instantiate the EditorProperty for this property with loop guard. 

    Internally, when instantiating a property editor Godot checks every plugin's can handle, if true parses property again.
    To avoid an infinite loop a guard is used (this plugin's 'can_handle()' function returns false is creating_native_editor() == true)
    this way another plugins handles the EditorProperty creation (likely Godot's EditorInspectorDefaultPlugin).

    @see https://github.com/godotengine/godot/blob/65e8d16951d6963cb3984c090e45f40d1ba5f704/editor/inspector/editor_inspector.cpp#L4006
  */
  creating_native_editor() = true;
    PropertyInfo target_pi = target_dpi->get_property_info(true);
    EditorProperty *editor = EditorInspector::instantiate_property_editor(
      p_object,
      target_pi.type, 
      p_name, 
      static_cast<PropertyHint>(target_pi.hint),
      target_pi.hint_string,
      target_pi.usage,
      p_wide
    );

    editor->set_object_and_property(p_object, p_name);
    if (target_pi.type == Variant::ARRAY) {
      /** If the property is an array, normalize on each change. 
        If the editor holds an Array, normalizing on each change ensures it is impossible to ever have an invalid value as an element.
        This would usually happen when adding new elements to the array, since <null> is always added, even if the "Type/Hint:HintString"
        combination doesn't allow <null> as an element.
      */
      editor->connect(
        SIGNAL(editor, property_changed), 
        callable_mp(this, &DynamicPropertyInfoInspectorPlugin::on_property_changed).bind(p_object)
      );
    }
    this->add_property_editor(p_name, editor);
  creating_native_editor() = false;

  /** Serialize 'target_dpi' */
  Ref<DynamicPropertyInfo> serialized_dpi = target_dpi->duplicate(true);
  serialized_dpi->is_root = false;
  serialized_dpi->properties_dict.clear();
  root_dpi->properties_dict[p_name] = serialized_dpi;

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

  if (p_object == parse_first_responder) {
    parse_first_responder = nullptr;
  }

  ObjectID id {p_object->get_instance_id()}; if (not object_parse_context_map.has(id)) return;
  ObjectParseContext &context = object_parse_context_map[id];
  Ref<DynamicPropertyInfo> &root_dpi = context.root_dpi;
  StringName &root_dpi_name = context.root_dpi_name;
  HashSet<StringName> &current_parsed_properties = context.current_parsed_properties;

  /** Find the right node to add the "Apply" button to and then add it */
  if (context.editor_property == nullptr) {
    find_inspector_property_editors(object_parse_context_map);  
  }
  EditorProperty *root_dpi_inspector_node = context.editor_property;

  if (root_dpi_inspector_node != nullptr) {
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
    submit_button->connect("pressed", callable_mp(this, &DynamicPropertyInfoInspectorPlugin::on_submit).bind(root_dpi));
    button_target_node->add_child(submit_button);
  }

  /** Clear properties no longer available from root dpi */
  bool req_refresh = false;
  if (root_dpi.is_valid()) {
    Array keys = root_dpi->properties_dict.keys();
    for (int i = 0; i < keys.size() ; i++) {
      if (not current_parsed_properties.has(StringName(keys[i]))) {
        root_dpi->properties_dict.erase(keys[i]);
        req_refresh = true;
      }
    }

    if (not current_parsed_properties.has(root_dpi->property_select) && root_dpi->property_select != StringName(DynamicPropertyInfo::PLACEHOLDER_PROPERTY_SELECT)) {
      Array properties = root_dpi->properties_dict.keys();
      if (properties.size() > 0) {
        root_dpi->set_property_select(properties[0]);
      } else {
        root_dpi->set_property_select(DynamicPropertyInfo::PLACEHOLDER_PROPERTY_SELECT);
      }
      req_refresh = true;
    }
  }

  if (req_refresh) {
    p_object->call_deferred("notify_property_list_changed");
  } else {
    debug_print_rich(vformat(COLOR_GREEN("Inspector updated >>> %s"), p_object));
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

