#pragma once

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
#include <cstdint>
#include <unordered_map>

namespace godot {

// +-----------------------------------------------------------------------------------------+
// |================================= DYNAMIC_PROPERTY_INFO =================================|
// +-----------------------------------------------------------------------------------------+-

  class DynamicPropertyInfo : public Resource {
    GDCLASS(DynamicPropertyInfo, Resource);

    friend class DynamicPropertyInfoInspectorPlugin;
    friend class DynamicEditorProperty;

    private:
      SORUS_DECL(Variant::Type, type, Variant::NIL);
      SORUS_DECL(StringName, property_select);
      SORUS_DECL(uint32_t, hint, PROPERTY_HINT_NONE);
      SORUS_DECL(String, hint_string)
      SORUS_DECL(uint32_t, usage, PROPERTY_USAGE_DEFAULT);
      SORUS_DECL(Dictionary, property_info_dict); /*<[StringName, Ref<DynamicPropertyInfo>]>*/

      bool is_root = false;
      static constexpr const char * PLACEHOLDER_PROPERTY_SELECT = "Select";

      String get_property_names_hint_string () const;

    protected:
      static void _bind_methods();
      void _validate_property(PropertyInfo &p_property) const;
      String _to_string() const;

    public:
      /** @deprecated property_info is no longer a property, use get_property_info instead */
      static constexpr const char *MEMBER_PROPERTY_INFO = "property_info";

      PropertyInfo get_property_info(){
        PropertyInfo property_info {
          type,
          property_select,
          static_cast<PropertyHint>(hint),
          hint_string,
          usage
        };

        return property_info;
      }

      DynamicPropertyInfo() {
        property_select = PLACEHOLDER_PROPERTY_SELECT;
      }

      DynamicPropertyInfo(
        PropertyInfo p_property_info
      ) : type {p_property_info.type}, 
          property_select {p_property_info.name}, 
          hint {p_property_info.hint}, 
          hint_string {p_property_info.hint_string}, 
          usage {p_property_info.usage} {
            /** nop */
          }

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
      Ref<DynamicPropertyInfo> root_dpi {nullptr};
      StringName root_dpi_name {};

      static constexpr const char STD_DYNAMIC_PROP_PREFIX[] = "d_";

      static bool &creating_native_editor() {
        static thread_local bool value = false;
        return value;
      }

      static void on_dynamic_property_info_changed(Object *p_object, const Ref<DynamicPropertyInfo> &root_dpi);
      static bool find_root_dpi(
        const Object *p_object, 
        Ref<DynamicPropertyInfo> *r_dpi = nullptr, 
        StringName *r_dpi_name = nullptr
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

      void _on_submit();

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
