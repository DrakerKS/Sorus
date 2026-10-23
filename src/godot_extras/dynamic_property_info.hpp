#pragma once

#include "godot_extras/globals.hpp"

#include "godot_cpp/core/property_info.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/wrapped.hpp"

#include "godot_cpp/classes/editor_property.hpp"
#include "godot_cpp/classes/editor_inspector_plugin.hpp"
#include "godot_cpp/classes/editor_plugin.hpp"
#include "godot_cpp/classes/editor_inspector.hpp"

#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/string_name.hpp"

#include "godot_cpp/templates/hash_set.hpp"
#include "godot_cpp/templates/hash_map.hpp"

#include "utils/macros.hpp"
#include <cstddef>

namespace godot {

// +-----------------------------------------------------------------------------------------+
// |================================= DYNAMIC_PROPERTY_INFO =================================|
// +-----------------------------------------------------------------------------------------+-

  class DynamicPropertyInfo : public Resource {
    GDCLASS(DynamicPropertyInfo, Resource);

    friend class DynamicPropertyInfoInspectorPlugin;

    private:
      SORUS_DECL(Variant::Type, type, Variant::NIL);
      SORUS_DECL(StringName, property_select, PLACEHOLDER_PROPERTY_SELECT);
      SORUS_DECL(uint32_t, hint, PROPERTY_HINT_NONE);
      SORUS_DECL(String, hint_string);
      SORUS_DECL(uint32_t, usage, PROPERTY_USAGE_DEFAULT);
      SORUS_DECL(Dictionary, properties_dict); /*<[StringName, Ref<DynamicPropertyInfo>]>*/
      
      SORUS_DECL(int, values_name_type, DEFAULT_NAME_TYPE);

      String get_property_names_hint_string () const;

    protected:
      static void _bind_methods();
      void _validate_property(PropertyInfo &p_property) const;
      String _to_string() const;

    public:
      static constexpr const ValueNameType DEFAULT_NAME_TYPE = VALUE_NAME_PASCAL;
      static constexpr const char *MEMBER_PROPERTY_INFO = "property_info"; /** @deprecated property_info is no longer a property, use get_property_info instead */
      static constexpr const char * PLACEHOLDER_PROPERTY_SELECT = "Select";

      bool is_root = false;

      PropertyInfo get_property_info(bool p_canonical_hint_string, bool p_verbose = false);

      DynamicPropertyInfo(PropertyInfo p_property_info): 
        type {p_property_info.type}, 
        property_select {p_property_info.name}, 
        hint {p_property_info.hint}, 
        hint_string {p_property_info.hint_string}, 
        usage {p_property_info.usage} {
          
        }

      DynamicPropertyInfo() = default;
      ~DynamicPropertyInfo() = default;
  };

// +----------------------------------------------------------------------------------------+
// |======================== DYNAMIC_PROPERTY_INFO_INSPECTOR_PLUGIN ========================|
// +----------------------------------------------------------------------------------------+

  class DynamicPropertyInfoInspectorPlugin : public EditorInspectorPlugin {
    GDCLASS(DynamicPropertyInfoInspectorPlugin, EditorInspectorPlugin);

    protected:
      static void _bind_methods() { }

    private:
      static constexpr const char STD_DYNAMIC_PROP_PREFIX[] = "d_";

      struct ObjectParseContext {
        Ref<DynamicPropertyInfo> root_dpi {};
        StringName root_dpi_name {};
        HashSet<StringName> current_parsed_properties {};
        EditorProperty *editor_property {};
      };

      /** 
        @note
          This map is cleared once every parse chain begin, meaning that it is
          required to check if the map has certain entry before trying to use it.
          This also means it is only guaranteed that every inner object has been 
          parsed until parse_first_responder's parse_end() execution. Any other
          access or modification to the map before this should only be directed
          towards the currently parsed object and no other else.
      */
      HashMap<ObjectID, ObjectParseContext> object_parse_context_map {};
      /** 
        @note
          First object that handles the parsing is considered as 'parse_first_responder' 
          during the first 'parse_begin()' and then is cleaned at 'parse_end()'. Any 
          code that should be or may be better to execute just once is better inside a
          conditional block 'p_object == parse_first_responder'
      */
      Object *parse_first_responder = nullptr;

      static bool &creating_native_editor() {
        static thread_local bool value = false;
        return value;
      }

      static bool find_root_dpi(
        const Object *p_object, 
        Ref<DynamicPropertyInfo> *r_dpi = nullptr, 
        StringName *r_dpi_name = nullptr
      );

      static void find_inspector_property_editors(
        HashMap<ObjectID, ObjectParseContext> &p_object_parse_context_map
      );

      void normalize_property(
        Object *p_object,
        const Ref<DynamicPropertyInfo> &p_dpi
      );

      static void on_dynamic_property_info_changed(
        Object *p_object, 
        const Ref<DynamicPropertyInfo> &root_dpi
      );

      void on_submit(
        const Ref<DynamicPropertyInfo> p_root_dpi
      );

      void on_property_changed(
        StringName p_property_name, 
        Variant p_value, 
        StringName p_field, 
        bool changing, 
        Object *p_object,
        const Ref<DynamicPropertyInfo> &p_dpi
      );

    public:
      bool _can_handle(Object *p_object) const override;
      void _parse_begin(Object *p_object) override;
      bool _parse_property(
        Object *p_object,
        Variant::Type p_type,
        const String &p_name,
        PropertyHint p_hint,
        const String &p_hint_string,
        BitField<PropertyUsageFlags> p_usage,
        bool p_wide
      ) override;
      void _parse_end(Object *p_object) override;

      DynamicPropertyInfoInspectorPlugin() = default;
      ~DynamicPropertyInfoInspectorPlugin() = default;
  };

// +-----------------------------------------------------------------------------------------+
// |========================== DYNAMIC_PROPERTY_INFO_EDITOR_PLUGIN ==========================|
// +-----------------------------------------------------------------------------------------+

  class DynamicPropertyInfoEditorPlugin : public EditorPlugin {
    GDCLASS(DynamicPropertyInfoEditorPlugin, EditorPlugin);

    protected:
      static void _bind_methods() { }

    private:
      Ref<DynamicPropertyInfoInspectorPlugin> inspector_plugin;

    public:
      void _enter_tree() override;
      void _exit_tree() override;

      DynamicPropertyInfoEditorPlugin() = default;
      ~DynamicPropertyInfoEditorPlugin() = default;
  };
}
