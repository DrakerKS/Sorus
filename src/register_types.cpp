#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/editor_plugin_registration.hpp>

// #include "core/settings/categories/mouse_keyboard_settings.hpp"
// #include "core/settings/settings_data.hpp"
// #include "core/settings/categories/controller_settings.hpp"
// #include "core/settings/categories/game_settings.hpp"
// #include "core/settings/categories/audio_settings.hpp"
// #include "core/settings/categories/video_settings.hpp"
// #include "core/settings/categories/graphics_settings.hpp"
// #include "core/settings/categories/accessibility_settings.hpp"

// #include "core/settings/settings_manager.hpp"
#include "reactive/reactive.hpp"
#include "reactive/autobinder.hpp"

#include "core/settings/settings_data.hpp"

#include "utils/debug.hpp"
#include "DEMO/demo.hpp"

using namespace godot;

void initialize_gdextension_types(ModuleInitializationLevel p_level)
{
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(Debug);
		GDREGISTER_CLASS(DemoContainer);
		GDREGISTER_CLASS(DynamicPropertyInfo);
		GDREGISTER_INTERNAL_CLASS(__CLASS__SECTION__END__);

		GDREGISTER_INTERNAL_CLASS(Reactive);
		GDREGISTER_CLASS(ReactiveVariant);
		GDREGISTER_CLASS(Autobinder);
		
		GDREGISTER_CLASS(AccessibilitySettings);
		GDREGISTER_CLASS(SettingsData);
	} 
	else if (p_level == godot::MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_CLASS(DynamicPropertyInfoInspectorPlugin);
		GDREGISTER_CLASS(DynamicPropertyInfoEditorPlugin);
		GDREGISTER_CLASS(DynamicEditorProperty);

		EditorPlugins::add_by_type<DynamicPropertyInfoEditorPlugin>();
	}

	// GDREGISTER_CLASS(GameSettings);
	// GDREGISTER_CLASS(AudioSettings);
	// GDREGISTER_CLASS(VideoSettings);
	// GDREGISTER_CLASS(GraphicsSettings);
	// GDREGISTER_CLASS(AccessibilitySettings);
	// GDREGISTER_CLASS(MouseKeyboardSettings);
	// GDREGISTER_CLASS(MouseKeyboardSettingsPreset);
	// GDREGISTER_CLASS(ControllerSettings);
	// GDREGISTER_CLASS(ControllerSettingsPreset);

	// GDREGISTER_CLASS(SettingsData);
	// GDREGISTER_CLASS(SettingsManager);
}

void uninitialize_gdextension_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C"
{
	// Initialization
	GDExtensionBool GDE_EXPORT sorus_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
	{
		GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
		init_obj.register_initializer(initialize_gdextension_types);
		init_obj.register_terminator(uninitialize_gdextension_types);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}