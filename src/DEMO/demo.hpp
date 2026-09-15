#pragma once

#include "godot_cpp/classes/resource.hpp"

class DemoContainer : public godot::Resource {
  GDCLASS(DemoContainer, godot::Resource);

  protected:
    static void _bind_methods() { }
    void _validate_property(godot::PropertyInfo &p_property) const {
      // godot::print_line("DEMO_FLAG: ", p_property.name);
    }
};