#pragma once

#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/core/property_info.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "utils/error_macros.hpp"

// +-----------------------------------------------------------------------------------------+
// |==================================== TOKEN_UTILITIES ====================================|
// +-----------------------------------------------------------------------------------------+

#define STR_RAW(x) #x
#define STR(x) STR_RAW(x)
#define CAT_RAW(a, b) a##b
#define CAT(a, b) CAT_RAW(a, b)
#define JOIN(a, b) CAT(a, CAT_RAW(_,b))

// +----------------------------------------------------------------------------------------+
// |================================== NAMING_CONVENTIONS ==================================|
// +----------------------------------------------------------------------------------------+

#define SORUS_SETTER_TOKEN(m_member) JOIN(set,m_member)
#define SORUS_GETTER_TOKEN(m_member) JOIN(get,m_member)
#define P_TOKEN(m_param) JOIN(p, m_param)

// +----------------------------------------------------------------------------------------+
// |===================================== PLACEHOLDERS =====================================|
// +----------------------------------------------------------------------------------------+

#define BLANK
#define NO_MODIFIER BLANK

// >>> PLACEHOLDERS: godot::PropertyInfo variant

#define NO_HINT godot::PROPERTY_HINT_NONE
#define NO_HINT_STR ""
#define DEFAULT_USAGE godot::PROPERTY_USAGE_DEFAULT

// +----------------------------------------------------------------------------------------+
// |===================================== COMMON_NAMES =====================================|
// +----------------------------------------------------------------------------------------+

#define N_TYPE "type"
#define N_NAME "name"
#define N_HINT "hint"
#define N_HINT_STRING "hint_string"
#define N_USAGE "usage"

// +----------------------------------------------------------------------------------------+
// |========================================= MISC =========================================|
// +----------------------------------------------------------------------------------------+

#define NO_INLAY(x) ((void)nullptr, x)
#define END_MACRO() static_assert(true)

#define SIGNAL(m_instance, m_signal) ( \
  [&]() -> StringName { \
    constexpr const char *name = STR(m_signal); \
    if (not m_instance->has_signal(name)) { \
      WARN_PRINT(vformat( \
        "The '%s' object has no '%s' signal", \
        m_instance, name) \
      ); \
      return {};\
    } \
    return StringName(name); \
    }())
  
#define METHOD(m_instance, m_method) ( \
  [&]() -> StringName { \
    constexpr const char *name = STR(m_method); \
    if (not m_instance->has_method(name)) { \
      WARN_PRINT(vformat( \
        "The '%s' object has no '%s' method", \
        m_instance, name) \
      ); \
      return {};\
    } \
    return StringName(name); \
    }())

// +----------------------------------------------------------------------------------------+
// |============================= MEMBER_COMPILE_TIME_CHECKING =============================|
// +----------------------------------------------------------------------------------------+

/**
 * @brief Standard way of declaring class properties. Given a type and member name (optionally an init value),
 *        Members are declared private. A 'static constexpr const char[]' with the member name and setter/getter
 *        signatures for the member property are declared public. Setters use 'const T&' for the input parameter.
*/
#define SORUS_DECL(m_type, m_member, ...) \
    private: \
      m_type m_member {__VA_ARGS__}; \
    public: \
      static constexpr const char JOIN(member,m_member)[] = STR(m_member); \
      void SORUS_SETTER_TOKEN(m_member)(m_type P_TOKEN(m_member)); \
      m_type SORUS_GETTER_TOKEN(m_member)() const;

#define SORUS_MEMBER_CHECK(m_member) \
  static_cast<void>(sizeof(((self_type*)nullptr)->m_member))

#define SORUS_MEMBER_TOKEN(m_member) \
  (SORUS_MEMBER_CHECK(m_member), m_member)

#define SORUS_MEMBER_NAME(m_member) \
  (SORUS_MEMBER_CHECK(m_member), STR(m_member))

#define SORUS_MEMBER_WRAPPED_NAME(m_wrap_scope, m_member) \
  (SORUS_MEMBER_CHECK(m_wrap_scope.m_member), STR(m_member))  

// >>> MEMBER_COMPILE_TIME_CHECKING: External variant

#define SORUS_EXT_MEMBER_CHECK(m_scope, m_member) \
  static_cast<void>(sizeof(((m_scope*)nullptr)->m_member))

#define SORUS_EXT_MEMBER_TOKEN(m_scope, m_member) \
  (SORUS_EXT_MEMBER_CHECK(m_scope, m_member), m_member)

#define SORUS_EXT_MEMBER_NAME(m_scope, m_member) \
  (SORUS_EXT_MEMBER_CHECK(m_scope, m_member), STR(m_member))

// >>> MEMBER_COMPILE_TIME_CHECKING: Static variant

#define SORUS_STATIC_MEMBER_CHECK(m_member) \
  static_cast<void>(sizeof(self_type::m_member))

#define SORUS_STATIC_MEMBER(m_member) \
  (SORUS_STATIC_MEMBER_CHECK(m_member), m_member)

#define SORUS_STATIC_MEMBER_NAME(m_member) \
  (SORUS_STATIC_MEMBER_CHECK(m_member), STR(m_member))

// >>> MEMBER_COMPILE_TIME_CHECKING: Static external variant

#define SORUS_STATIC_EXT_MEMBER_CHECK(m_scope, m_member) \
  static_cast<void>(sizeof(m_scope::m_member))

#define SORUS_STATIC_EXT_MEMBER_TOKEN(m_scope, m_member) \
  (SORUS_STATIC_EXT_MEMBER_CHECK(m_scope, m_member), m_member)

#define SORUS_STATIC_EXT_MEMBER_NAME(m_scope, m_member) \
  (SORUS_STATIC_EXT_MEMBER_CHECK(m_scope, m_member), STR(m_member))

// +----------------------------------------------------------------------------------------+
// |============================= METHOD_COMPILE_TIME_CHECKING =============================|
// +----------------------------------------------------------------------------------------+

#define SORUS_METHOD_CHECK(m_method) \
  static_cast<void>(&self_type::m_method)

#define SORUS_METHOD_PTR(m_method) \
  (SORUS_METHOD_CHECK(m_method), &self_type::m_method)

#define SORUS_METHOD_TOKEN(m_method) \
  (SORUS_METHOD_CHECK(m_method), m_method)

#define SORUS_METHOD_NAME(m_method) \
  (SORUS_METHOD_CHECK(m_method), STR(m_method))

// >>> METHOD_COMPILE_TIME_CHECKING: External variant

#define SORUS_EXT_METHOD_CHECK(m_scope, m_method) \
  static_cast<void>(&m_scope::m_method)

#define SORUS_EXT_METHOD_PTR(m_scope, m_method) \
  (SORUS_EXT_METHOD_CHECK(m_scope, m_method), &m_scope::m_method)

#define SORUS_EXT_METHOD_TOKEN(m_scope, m_method) \
  (SORUS_EXT_METHOD_CHECK(m_scope, m_method), m_method)

#define SORUS_EXT_METHOD_NAME(m_scope, m_method) \
  (SORUS_EXT_METHOD_CHECK(m_scope, m_method), STR(m_method))

// +-----------------------------------------------------------------------------------------+
// |================================ BASIC_SETTERS_&_GETTERS ================================|
// +-----------------------------------------------------------------------------------------+

#define SORUS_MEMBER_SETTER(m_datatype, m_member) \
  void SORUS_SETTER_TOKEN(m_member)(m_datatype P_TOKEN(m_member)) { \
    m_member = P_TOKEN(m_member); \
  }

#define SORUS_MEMBER_GETTER(m_datatype, m_member) \
  m_datatype SORUS_GETTER_TOKEN(m_member)() const { \
    return m_member; \
  }

// +----------------------------------------------------------------------------------------+
// |==================================== METHOD_BINDING ====================================|
// +----------------------------------------------------------------------------------------+

#define SORUS_BIND_METHOD(m_method, ...) \
  godot::ClassDB::bind_method( \
    godot::D_METHOD( \
      SORUS_METHOD_NAME(m_method) \
      __VA_OPT__(,) __VA_ARGS__ \
    ), \
    SORUS_METHOD_PTR(m_method) \
  )

#define SORUS_BIND_SETTER(m_member) \
  SORUS_BIND_METHOD( \
    SORUS_SETTER_TOKEN(m_member), \
    STR(m_member) \
  )
  
#define SORUS_BIND_GETTER(m_member) \
  SORUS_BIND_METHOD( \
    SORUS_GETTER_TOKEN(m_member) \
  )

#define SORUS_BIND_SETTER_GETTER(m_member) \
  SORUS_BIND_SETTER(m_member); \
  SORUS_BIND_GETTER(m_member)

// >>> METHOD_BINDING: Static variant

#define SORUS_BIND_STATIC_METHOD(m_method, ...) \
  godot::ClassDB::bind_static_method( \
    get_class_static(), \
    godot::D_METHOD( \
      SORUS_METHOD_NAME(m_method) \
      __VA_OPT__(,) __VA_ARGS__ \
    ), \
    SORUS_METHOD_PTR(m_method) \
  )

#define SORUS_BIND_STATIC_SETTER(m_member) \
  SORUS_BIND_STATIC_METHOD( \
    SORUS_SETTER_TOKEN(m_member), \
    STR(m_member) \
  )

#define SORUS_BIND_STATIC_GETTER(m_member) \
  SORUS_BIND_STATIC_METHOD( \
    SORUS_GETTER_TOKEN(m_member) \
  )

#define SORUS_BIND_STATIC_SETTER_GETTER(m_member) \
  SORUS_BIND_STATIC_SETTER(m_member); \
  SORUS_BIND_STATIC_GETTER(m_member)

// +----------------------------------------------------------------------------------------+
// |=================================== PROPERTY_BINDING ===================================|
// +----------------------------------------------------------------------------------------+

/**
  Using the PropertyInfo constructor:
  
  PropertyInfo(
    Variant::Type p_type,
    const StringName &p_name,
    PropertyHint p_hint = PROPERTY_HINT_NONE,
    const String &p_hint_string = "",
    uint32_t p_usage = PROPERTY_USAGE_DEFAULT,
    const StringName &p_class_name = ""
  )
 */
#define SORUS_ADD_PROPERTY_RAW(m_variant, m_property, ...) \
  godot::ClassDB::add_property( \
    get_class_static(), \
    godot::PropertyInfo( \
      NO_INLAY(m_variant), \
      STR(m_property) \
      __VA_OPT__(,) __VA_ARGS__ \
    ), \
    SORUS_METHOD_NAME(SORUS_SETTER_TOKEN(m_property)), \
    SORUS_METHOD_NAME(SORUS_GETTER_TOKEN(m_property)) \
  )

#define SORUS_BIND_PROPERTY(m_variant, m_property, ...) \
  do{ \
    SORUS_BIND_SETTER_GETTER(m_property); \
    SORUS_ADD_PROPERTY_RAW(m_variant, m_property, __VA_ARGS__); \
  } while(false)

#define SORUS_BIND_STATIC_PROPERTY(m_variant, m_property, ...) \
  do{ \
    SORUS_BIND_STATIC_SETTER_GETTER(m_property); \
    SORUS_ADD_PROPERTY_RAW(m_variant, m_property, __VA_ARGS__); \
  } while(false)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_BOOL_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::BOOL, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  ) 

#define SORUS_BIND_PROPERTY_BOOL(m_property, ...) \
  SORUS_BIND_PROPERTY_BOOL_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_BOOL(m_property, ...) \
  SORUS_BIND_PROPERTY_BOOL_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_INT_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::INT, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_INT(m_property, ...) \
  SORUS_BIND_PROPERTY_INT_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_INT(m_property, ...) \
  SORUS_BIND_PROPERTY_INT_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_FLOAT_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::FLOAT, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_FLOAT(m_property, ...) \
  SORUS_BIND_PROPERTY_FLOAT_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_FLOAT(m_property, ...) \
  SORUS_BIND_PROPERTY_FLOAT_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_STRING_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::STRING, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_STRING(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_STRING(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)


/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_VECTOR2_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::VECTOR2, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_VECTOR2(m_property, ...) \
  SORUS_BIND_PROPERTY_VECTOR2_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_VECTOR2(m_property, ...) \
  SORUS_BIND_PROPERTY_VECTOR2_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_VECTOR2I_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::VECTOR2I, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_VECTOR2I(m_property, ...) \
  SORUS_BIND_PROPERTY_VECTOR2I_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_VECTOR2I(m_property, ...) \
  SORUS_BIND_PROPERTY_VECTOR2I_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_RECT2_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::RECT2, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_RECT2(m_property, ...) \
  SORUS_BIND_PROPERTY_RECT2_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_RECT2(m_property, ...) \
  SORUS_BIND_PROPERTY_RECT2_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_RECT2I_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::RECT2I, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_RECT2I(m_property, ...) \
  SORUS_BIND_PROPERTY_RECT2I_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_RECT2I(m_property, ...) \
  SORUS_BIND_PROPERTY_RECT2I_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_VECTOR3_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::VECTOR3, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_VECTOR3(m_property, ...) \
  SORUS_BIND_PROPERTY_VECTOR3_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_VECTOR3(m_property, ...) \
  SORUS_BIND_PROPERTY_VECTOR3_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_VECTOR3I_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::VECTOR3I, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_VECTOR3I(m_property, ...) \
  SORUS_BIND_PROPERTY_VECTOR3I_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_VECTOR3I(m_property, ...) \
  SORUS_BIND_PROPERTY_VECTOR3I_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_TRANSFORM2D_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::TRANSFORM2D, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_TRANSFORM2D(m_property, ...) \
  SORUS_BIND_PROPERTY_TRANSFORM2D_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_TRANSFORM2D(m_property, ...) \
  SORUS_BIND_PROPERTY_TRANSFORM2D_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_VECTOR4_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::VECTOR4, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_VECTOR4(m_property, ...) \
  SORUS_BIND_PROPERTY_VECTOR4_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_VECTOR4(m_property, ...) \
  SORUS_BIND_PROPERTY_VECTOR4_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_VECTOR4I_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::VECTOR4I, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_VECTOR4I(m_property, ...) \
  SORUS_BIND_PROPERTY_VECTOR4I_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_VECTOR4I(m_property, ...) \
  SORUS_BIND_PROPERTY_VECTOR4I_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_PLANE_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::PLANE, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_PLANE(m_property, ...) \
  SORUS_BIND_PROPERTY_PLANE_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PLANE(m_property, ...) \
  SORUS_BIND_PROPERTY_PLANE_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_QUATERNION_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::QUATERNION, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_QUATERNION(m_property, ...) \
  SORUS_BIND_PROPERTY_QUATERNION_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_QUATERNION(m_property, ...) \
  SORUS_BIND_PROPERTY_QUATERNION_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_AABB_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::AABB, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_AABB(m_property, ...) \
  SORUS_BIND_PROPERTY_AABB_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_AABB(m_property, ...) \
  SORUS_BIND_PROPERTY_AABB_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_BASIS_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::BASIS, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_BASIS(m_property, ...) \
  SORUS_BIND_PROPERTY_BASIS_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_BASIS(m_property, ...) \
  SORUS_BIND_PROPERTY_BASIS_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_TRANSFORM3D_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::TRANSFORM3D, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_TRANSFORM3D(m_property, ...) \
  SORUS_BIND_PROPERTY_TRANSFORM3D_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_TRANSFORM3D(m_property, ...) \
  SORUS_BIND_PROPERTY_TRANSFORM3D_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_PROJECTION_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::PROJECTION, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_PROJECTION(m_property, ...) \
  SORUS_BIND_PROPERTY_PROJECTION_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PROJECTION(m_property, ...) \
  SORUS_BIND_PROPERTY_PROJECTION_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)


/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_COLOR_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::COLOR, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_COLOR(m_property, ...) \
  SORUS_BIND_PROPERTY_COLOR_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_COLOR(m_property, ...) \
  SORUS_BIND_PROPERTY_COLOR_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_STRING_NAME_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::STRING_NAME, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_STRING_NAME(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_NAME_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_STRING_NAME(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_NAME_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_NODE_PATH_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::NODE_PATH, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_NODE_PATH(m_property, ...) \
  SORUS_BIND_PROPERTY_NODE_PATH_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_NODE_PATH(m_property, ...) \
  SORUS_BIND_PROPERTY_NODE_PATH_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_RID_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::RID, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_RID(m_property, ...) \
  SORUS_BIND_PROPERTY_RID_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_RID(m_property, ...) \
  SORUS_BIND_PROPERTY_RID_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_CALLABLE_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::CALLABLE, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_CALLABLE(m_property, ...) \
  SORUS_BIND_PROPERTY_CALLABLE_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_CALLABLE(m_property, ...) \
  SORUS_BIND_PROPERTY_CALLABLE_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_SIGNAL_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::SIGNAL, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_SIGNAL(m_property, ...) \
  SORUS_BIND_PROPERTY_SIGNAL_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_SIGNAL(m_property, ...) \
  SORUS_BIND_PROPERTY_SIGNAL_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_DICTIONARY_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::DICTIONARY, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_DICTIONARY(m_property, ...) \
  SORUS_BIND_PROPERTY_DICTIONARY_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_DICTIONARY(m_property, ...) \
  SORUS_BIND_PROPERTY_DICTIONARY_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_ARRAY_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::ARRAY, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_ARRAY_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_ARRAY_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)


/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_PACKED_BYTE_ARRAY_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::PACKED_BYTE_ARRAY, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_PACKED_BYTE_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_BYTE_ARRAY_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PACKED_BYTE_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_BYTE_ARRAY_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_PACKED_INT32_ARRAY_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::PACKED_INT32_ARRAY, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_PACKED_INT32_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_INT32_ARRAY_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PACKED_INT32_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_INT32_ARRAY_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_PACKED_INT64_ARRAY_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::PACKED_INT64_ARRAY, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_PACKED_INT64_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_INT64_ARRAY_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PACKED_INT64_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_INT64_ARRAY_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_PACKED_FLOAT32_ARRAY_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::PACKED_FLOAT32_ARRAY, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_PACKED_FLOAT32_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_FLOAT32_ARRAY_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PACKED_FLOAT32_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_FLOAT32_ARRAY_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_PACKED_FLOAT64_ARRAY_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::PACKED_FLOAT64_ARRAY, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_PACKED_FLOAT64_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_FLOAT64_ARRAY_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PACKED_FLOAT64_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_FLOAT64_ARRAY_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_PACKED_STRING_ARRAY_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::PACKED_STRING_ARRAY, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_PACKED_STRING_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_STRING_ARRAY_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PACKED_STRING_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_STRING_ARRAY_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_PACKED_VECTOR2_ARRAY_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::PACKED_VECTOR2_ARRAY, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_PACKED_VECTOR2_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_VECTOR2_ARRAY_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PACKED_VECTOR2_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_VECTOR2_ARRAY_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_PACKED_VECTOR3_ARRAY_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::PACKED_VECTOR3_ARRAY, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_PACKED_VECTOR3_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_VECTOR3_ARRAY_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PACKED_VECTOR3_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_VECTOR3_ARRAY_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_PACKED_COLOR_ARRAY_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::PACKED_COLOR_ARRAY, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_PACKED_COLOR_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_COLOR_ARRAY_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PACKED_COLOR_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_COLOR_ARRAY_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_PACKED_VECTOR4_ARRAY_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::PACKED_VECTOR4_ARRAY, \
    m_property, \
    godot::PROPERTY_HINT_NONE \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_PACKED_VECTOR4_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_VECTOR4_ARRAY_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PACKED_VECTOR4_ARRAY(m_property, ...) \
  SORUS_BIND_PROPERTY_PACKED_VECTOR4_ARRAY_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

// +----------------------------------------------------------------------------------------+
// |============================= SPECIALIZED_PROPERTY_BINDING =============================|
// +----------------------------------------------------------------------------------------+

// >>> SPECIALIZED_PROPERTY_BINDING: Range hints variant

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_RANGE_IMPL(m_base_binder, m_property, m_hint_str, ...) \
  m_base_binder( \
    godot::Variant::FLOAT, \
    m_property, \
    godot::PROPERTY_HINT_RANGE, \
    m_hint_str \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_RANGE(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_RANGE_IMPL(SORUS_BIND_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_RANGE(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_RANGE_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_RANGE_INT_IMPL(m_base_binder, m_property, m_hint_str, ...) \
  m_base_binder( \
    godot::Variant::INT, \
    m_property, \
    godot::PROPERTY_HINT_RANGE, \
    m_hint_str \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_RANGE_INT(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_RANGE_INT_IMPL(SORUS_BIND_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_RANGE_INT(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_RANGE_INT_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

// >>> SPECIALIZED_PROPERTY_BINDING: Enum hints variant

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_ENUM_IMPL(m_base_binder, m_property, m_hint_str, ...) \
  m_base_binder( \
    godot::Variant::INT, \
    m_property, \
    godot::PROPERTY_HINT_ENUM, \
    m_hint_str \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_ENUM(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_ENUM_IMPL(SORUS_BIND_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_ENUM(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_ENUM_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_ENUM_STRING_IMPL(m_base_binder, m_property, m_hint_str, ...) \
  m_base_binder( \
    godot::Variant::STRING, \
    m_property, \
    godot::PROPERTY_HINT_ENUM, \
    m_hint_str \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_ENUM_STRING(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_ENUM_STRING_IMPL(SORUS_BIND_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_ENUM_STRING(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_ENUM_STRING_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_ENUM_SUGGESTION_IMPL(m_base_binder, m_property, m_hint_str, ...) \
  m_base_binder( \
    godot::Variant::STRING, \
    m_property, \
    godot::PROPERTY_HINT_ENUM_SUGGESTION, \
    m_hint_str \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_ENUM_SUGGESTION(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_ENUM_SUGGESTION_IMPL(SORUS_BIND_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_ENUM_SUGGESTION(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_ENUM_SUGGESTION_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

// >>> SPECIALIZED_PROPERTY_BINDING: Layers hints variant

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_LAYERS_IMPL(m_base_binder, m_property, m_hint, ...) \
  m_base_binder( \
    godot::Variant::INT, \
    m_property, \
    m_hint \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_LAYERS_2D_RENDER(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_2D_RENDER, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_LAYERS_2D_RENDER(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_2D_RENDER, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_LAYERS_2D_PHYSICS(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_2D_PHYSICS, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_LAYERS_2D_PHYSICS(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_2D_PHYSICS, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_LAYERS_2D_NAVIGATION(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_2D_NAVIGATION, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_LAYERS_2D_NAVIGATION(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_2D_NAVIGATION, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_LAYERS_3D_RENDER(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_3D_RENDER, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_LAYERS_3D_RENDER(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_3D_RENDER, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_LAYERS_3D_PHYSICS(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_3D_PHYSICS, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_LAYERS_3D_PHYSICS(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_3D_PHYSICS, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_LAYERS_3D_NAVIGATION(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_3D_NAVIGATION, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_LAYERS_3D_NAVIGATION(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_3D_NAVIGATION, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_LAYERS_AVOIDANCE(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_AVOIDANCE, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_LAYERS_AVOIDANCE(m_property, ...) \
  SORUS_BIND_PROPERTY_LAYERS_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_LAYERS_AVOIDANCE, __VA_ARGS__)

// >>> SPECIALIZED_PROPERTY_BINDING: File/Dir hints variant

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(m_base_binder, m_property, m_hint, ...) \
  m_base_binder( \
    godot::Variant::STRING, \
    m_property, \
    m_hint \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_FILE(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_FILE, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_FILE(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_FILE, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_DIR(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_DIR, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_DIR(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_DIR, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_GLOBAL_FILE(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_GLOBAL_FILE, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_GLOBAL_FILE(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_GLOBAL_FILE, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_GLOBAL_DIR(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_GLOBAL_DIR, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_GLOBAL_DIR(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_GLOBAL_DIR, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_SAVE_FILE(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_SAVE_FILE, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_SAVE_FILE(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_SAVE_FILE, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_GLOBAL_SAVE_FILE(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_GLOBAL_SAVE_FILE, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_GLOBAL_SAVE_FILE(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_GLOBAL_SAVE_FILE, __VA_ARGS__)

// >>> SPECIALIZED_PROPERTY_BINDING: Text hints variant

#define SORUS_BIND_PROPERTY_MULTILINE_TEXT(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_MULTILINE_TEXT, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_MULTILINE_TEXT(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_MULTILINE_TEXT, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_EXPRESSION(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_EXPRESSION, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_EXPRESSION(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_EXPRESSION, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_PLACEHOLDER_TEXT(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_PLACEHOLDER_TEXT, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PLACEHOLDER_TEXT(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_PLACEHOLDER_TEXT, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_PASSWORD(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_PASSWORD, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_PASSWORD(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_PASSWORD, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_LOCALE_ID(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_LOCALE_ID, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_LOCALE_ID(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_LOCALE_ID, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_LOCALIZABLE_STRING(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_LOCALIZABLE_STRING, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_LOCALIZABLE_STRING(m_property, ...) \
  SORUS_BIND_PROPERTY_STRING_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_LOCALIZABLE_STRING, __VA_ARGS__)

// >>> SPECIALIZED_PROPERTY_BINDING: Object ID hints variant

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_OBJECT_ID_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::INT, \
    m_property, \
    godot::PROPERTY_HINT_OBJECT_ID \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_OBJECT_ID(m_property, ...) \
  SORUS_BIND_PROPERTY_OBJECT_ID_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_OBJECT_ID(m_property, ...) \
  SORUS_BIND_PROPERTY_OBJECT_ID_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_INT_IS_OBJECTID_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::INT, \
    m_property, \
    godot::PROPERTY_HINT_INT_IS_OBJECTID \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_INT_IS_OBJECTID(m_property, ...) \
  SORUS_BIND_PROPERTY_INT_IS_OBJECTID_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_INT_IS_OBJECTID(m_property, ...) \
  SORUS_BIND_PROPERTY_INT_IS_OBJECTID_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_INT_IS_POINTER_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::INT, \
    m_property, \
    godot::PROPERTY_HINT_INT_IS_POINTER \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_INT_IS_POINTER(m_property, ...) \
  SORUS_BIND_PROPERTY_INT_IS_POINTER_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_INT_IS_POINTER(m_property, ...) \
  SORUS_BIND_PROPERTY_INT_IS_POINTER_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

// >>> SPECIALIZED_PROPERTY_BINDING: NodePath hints variant

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_NODE_PATH_HINTED_IMPL(m_base_binder, m_property, m_hint, m_hint_str, ...) \
  m_base_binder( \
    godot::Variant::NODE_PATH, \
    m_property, \
    m_hint, \
    m_hint_str \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_NODE_PATH_TO_EDITED_NODE(m_property, ...) \
  SORUS_BIND_PROPERTY_NODE_PATH_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE, "", __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_NODE_PATH_TO_EDITED_NODE(m_property, ...) \
  SORUS_BIND_PROPERTY_NODE_PATH_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE, "", __VA_ARGS__)

#define SORUS_BIND_PROPERTY_NODE_PATH_VALID_TYPES(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_NODE_PATH_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_NODE_PATH_VALID_TYPES, m_hint_str, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_NODE_PATH_VALID_TYPES(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_NODE_PATH_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_NODE_PATH_VALID_TYPES, m_hint_str, __VA_ARGS__)

#define SORUS_BIND_PROPERTY_NODE_TYPE(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_NODE_PATH_HINTED_IMPL(SORUS_BIND_PROPERTY, m_property, godot::PROPERTY_HINT_NODE_TYPE, m_hint_str, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_NODE_TYPE(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_NODE_PATH_HINTED_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, godot::PROPERTY_HINT_NODE_TYPE, m_hint_str, __VA_ARGS__)

// >>> SPECIALIZED_PROPERTY_BINDING: Array hints variant

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_TYPED_ARRAY_IMPL(m_base_binder, m_property, m_hint, m_hint_str, ...) \
  m_base_binder( \
    godot::Variant::ARRAY, \
    m_property, \
    m_hint, \
    m_hint_str \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_ARRAY_RESOURCES(m_property, m_hint_str_resource_type, ...) \
  SORUS_BIND_PROPERTY_TYPED_ARRAY_IMPL( \
    SORUS_BIND_PROPERTY, \
    m_property, \
    godot::PROPERTY_HINT_ARRAY_TYPE, \
    vformat( \
      "%d/%d:%s", \
      Variant::OBJECT, \
      godot::PROPERTY_HINT_RESOURCE_TYPE, \
      m_hint_str_resource_type \
    ), \
    __VA_ARGS__ \
  )

#define SORUS_BIND_STATIC_PROPERTY_ARRAY_RESOURCES(m_property, m_hint_str_resource_type, ...) \
  SORUS_BIND_PROPERTY_TYPED_ARRAY_IMPL( \
    SORUS_BIND_STATIC_PROPERTY, \
    m_property, \
    godot::PROPERTY_HINT_ARRAY_TYPE, \
    vformat( \
      "%d/%d:%s", \
      Variant::OBJECT, \
      godot::PROPERTY_HINT_RESOURCE_TYPE, \
      m_hint_str_resource_type \
    ), \
    __VA_ARGS__ \
  )

// >>> SPECIALIZED_PROPERTY_BINDING: Dictionary hints variant

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_TYPE_DICTIONARY_IMPL(m_base_binder, m_property, m_hint_str, ...) \
  m_base_binder( \
    godot::Variant::DICTIONARY, \
    m_property, \
    godot::PROPERTY_HINT_DICTIONARY_TYPE, \
    m_hint_str \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_TYPED_DICTIONARY(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_TYPE_DICTIONARY_IMPL(SORUS_BIND_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_TYPED_DICTIONARY(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_TYPE_DICTIONARY_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

// >>> SPECIALIZED_PROPERTY_BINDING: Misc hints variant

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_RESOURCE_IMPL(m_base_binder, m_property, m_hint_str_resource_type, ...) \
  m_base_binder( \
    godot::Variant::OBJECT, \
    m_property, \
    godot::PROPERTY_HINT_RESOURCE_TYPE, \
    m_hint_str_resource_type \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_RESOURCE(m_property, m_hint_str_resource_type, ...) \
  SORUS_BIND_PROPERTY_RESOURCE_IMPL(SORUS_BIND_PROPERTY, m_property, m_hint_str_resource_type, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_RESOURCE(m_property, m_hint_str_resource_type, ...) \
  SORUS_BIND_PROPERTY_RESOURCE_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, m_hint_str_resource_type, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_TEXT_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::STRING, \
    m_property, \
    godot::PROPERTY_HINT_TYPE_STRING \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_TEXT(m_property, ...) \
  SORUS_BIND_PROPERTY_TEXT_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_TEXT(m_property, ...) \
  SORUS_BIND_PROPERTY_TEXT_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_EXP_EASING_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::FLOAT, \
    m_property, \
    godot::PROPERTY_HINT_EXP_EASING \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_EXP_EASING(m_property, ...) \
  SORUS_BIND_PROPERTY_EXP_EASING_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_EXP_EASING(m_property, ...) \
  SORUS_BIND_PROPERTY_EXP_EASING_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_FLAGS_IMPL(m_base_binder, m_property, m_hint_str, ...) \
  m_base_binder( \
    godot::Variant::INT, \
    m_property, \
    godot::PROPERTY_HINT_FLAGS, \
    m_hint_str \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_FLAGS(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_FLAGS_IMPL(SORUS_BIND_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_FLAGS(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_FLAGS_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_COLOR_NO_ALPHA_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::COLOR, \
    m_property, \
    godot::PROPERTY_HINT_COLOR_NO_ALPHA \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_COLOR_NO_ALPHA(m_property, ...) \
  SORUS_BIND_PROPERTY_COLOR_NO_ALPHA_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_COLOR_NO_ALPHA(m_property, ...) \
  SORUS_BIND_PROPERTY_COLOR_NO_ALPHA_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_HIDE_QUATERNION_EDIT_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::QUATERNION, \
    m_property, \
    godot::PROPERTY_HINT_HIDE_QUATERNION_EDIT \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_HIDE_QUATERNION_EDIT(m_property, ...) \
  SORUS_BIND_PROPERTY_HIDE_QUATERNION_EDIT_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_HIDE_QUATERNION_EDIT(m_property, ...) \
  SORUS_BIND_PROPERTY_HIDE_QUATERNION_EDIT_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_TOOL_BUTTON_IMPL(m_base_binder, m_property, m_hint_str, ...) \
  m_base_binder( \
    godot::Variant::CALLABLE, \
    m_property, \
    godot::PROPERTY_HINT_TOOL_BUTTON, \
    m_hint_str \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_TOOL_BUTTON(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_TOOL_BUTTON_IMPL(SORUS_BIND_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_TOOL_BUTTON(m_property, m_hint_str, ...) \
  SORUS_BIND_PROPERTY_TOOL_BUTTON_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, m_hint_str, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_ONESHOT_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::BOOL, \
    m_property, \
    godot::PROPERTY_HINT_ONESHOT \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_ONESHOT(m_property, ...) \
  SORUS_BIND_PROPERTY_ONESHOT_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_ONESHOT(m_property, ...) \
  SORUS_BIND_PROPERTY_ONESHOT_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

// >>> SPECIALIZED_PROPERTY_BINDING: Link hints variant

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_LINK_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::VECTOR2, \
    m_property, \
    godot::PROPERTY_HINT_LINK \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_LINK(m_property, ...) \
  SORUS_BIND_PROPERTY_LINK_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_LINK(m_property, ...) \
  SORUS_BIND_PROPERTY_LINK_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_LINK_VECTOR3_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::VECTOR3, \
    m_property, \
    godot::PROPERTY_HINT_LINK \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_LINK_VECTOR3(m_property, ...) \
  SORUS_BIND_PROPERTY_LINK_VECTOR3_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_LINK_VECTOR3(m_property, ...) \
  SORUS_BIND_PROPERTY_LINK_VECTOR3_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)

/** @warning Don't use *_IMPL macros directly, instead use their counterparts for readable code */
#define SORUS_BIND_PROPERTY_LINK_VECTOR4_IMPL(m_base_binder, m_property, ...) \
  m_base_binder( \
    godot::Variant::VECTOR4, \
    m_property, \
    godot::PROPERTY_HINT_LINK \
    __VA_OPT__(,) __VA_ARGS__ \
  )

#define SORUS_BIND_PROPERTY_LINK_VECTOR4(m_property, ...) \
  SORUS_BIND_PROPERTY_LINK_VECTOR4_IMPL(SORUS_BIND_PROPERTY, m_property, __VA_ARGS__)

#define SORUS_BIND_STATIC_PROPERTY_LINK_VECTOR4(m_property, ...) \
  SORUS_BIND_PROPERTY_LINK_VECTOR4_IMPL(SORUS_BIND_STATIC_PROPERTY, m_property, __VA_ARGS__)
