// #ifndef EXAMPLE_REGISTER_TYPES_H
// #define EXAMPLE_REGISTER_TYPES_H

// void initialize_gdextension_types();
// void uninitialize_gdextension_types();

// #endif // EXAMPLE_REGISTER_TYPES_H

#ifndef SORUS_REGISTER_TYPES_H
#define SORUS_REGISTER_TYPES_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void initialize_gdextension_types(ModuleInitializationLevel p_level);
void uninitialize_gdextension_types(ModuleInitializationLevel p_level);

#endif