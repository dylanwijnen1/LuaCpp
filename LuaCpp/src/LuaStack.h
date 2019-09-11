#pragma once

#include <type_traits>

#include <lua.hpp>
#include <LuaVar.h>

namespace lpp
{

	template<typename T>
	struct is_c_string
		: public std::disjunction<
		std::is_same<char*, typename std::decay<T>::type>,
		std::is_same<const char*, typename std::decay<T>::type>
		> { };

	template<typename T>
	constexpr bool is_c_string_v = is_c_string<T>::value;

	class LuaStack
	{
	public:
		template<typename Type>
		static Type Get(lua_State* pState, int index);

		template<typename Type>
		static Type Get(lua_State* pState, int index, const Type& defaultVal);

		template<typename Type>
		static bool Is(lua_State* pState, int index);

		template<typename Type>
		static void Push(lua_State* pState, Type&& val);

		static bool SafePop(lua_State* pState, int amount);

		static void Print(lua_State* pState, int index);
		static void PrintStack(lua_State* pState);
	};

	template<typename Type>
	inline static Type LuaStack::Get(lua_State* pState, int index)
	{
		using decayed_t = std::decay_t<Type>;

		if constexpr (std::is_same_v<bool, decayed_t>)
		{
			return (Type)lua_toboolean(pState, index);
		}
		else if constexpr (std::is_integral_v<decayed_t> || std::is_enum_v<decayed_t>)
		{
			return (Type)lua_tointeger(pState, index);
		}
		else if constexpr (std::is_floating_point_v<decayed_t>)
		{
			return (Type)lua_tonumber(pState, index);
		}
		else if constexpr (std::is_pointer_v<decayed_t>)
		{
			return reinterpret_cast<Type>(lua_touserdata(pState, index));
		}
		else if constexpr (std::is_same_v<std::string, decayed_t>)
		{
			return std::string(lua_tostring(pState, index));
		}
		else
		{
			static_assert(false, "Type not implemented, LuaStackHelper::Get<Type> at File " __FILE__ "Line ");
		}
	}

	template<typename Type>
	inline Type LuaStack::Get(lua_State* pState, int index, const Type& defaultVal)
	{
		if (Is(pState, index))
			return Get(pState, index);
		else
			return defaultVal;
	}

	template<typename Type>
	inline bool LuaStack::Is(lua_State* pState, int index)
	{
		using decayed_t = std::decay_t<Type>;

		if constexpr (std::is_same_v<bool, decayed_t>) // TODO: Never hits, Bool is considered integral.
		{
			return lua_isboolean(pState, index);
		}
		else if constexpr (std::is_integral_v<decayed_t> || std::is_enum_v<decayed_t> || std::is_unsigned_v<decayed_t>)
		{
			return lua_isinteger(pState, index);
		}
		else if constexpr (std::is_floating_point_v<decayed_t>)
		{
			return lua_isnumber(pState, index);
		}
		else if constexpr (std::is_same_v<std::string, decayed_t>)
		{
			return lua_isstring(pState, index);
		}
		else if constexpr (std::is_pointer_v<decayed_t>)
		{
			// TODO: Is UserData and DynamicCast ?
			return lua_islightuserdata(pState, index);
		}
		else if constexpr (std::is_null_pointer_v<Type>)
		{
			return lua_isnil(pState, index);
		}
		else
		{
			static_assert(false, "Type not implemented, LuaStackHelper::Is<Type> at File " __FILE__);
		}
	}

	template<typename Type>
	inline void LuaStack::Push(lua_State* pState, Type&& val)
	{
		using decayed_t = std::decay_t<Type>;

		if constexpr (std::is_same_v<bool, decayed_t>)
		{
			lua_pushboolean(pState, val);
		}
		else if constexpr (std::is_integral_v<decayed_t> || std::is_enum_v<decayed_t>)
		{
			lua_pushinteger(pState, val);
		}
		else if constexpr (std::is_floating_point_v<decayed_t>)
		{
			lua_pushnumber(pState, val);
		}
		else if constexpr (is_c_string_v<decayed_t>)
		{
			lua_pushstring(pState, val);
		}
		else if constexpr (std::is_same_v<std::string, decayed_t>)
		{
			lua_pushstring(pState, val.c_str());
		}
		else if constexpr (std::is_pointer_v<decayed_t>)
		{
			lua_pushlightuserdata(pState, reinterpret_cast<void*>(val));
		}
		else if constexpr (std::is_same_v<LuaVar, decayed_t>)
		{
			val.PushToStack();
		}
		else if constexpr (std::is_null_pointer_v<decayed_t>)
		{
			lua_pushnil(pState);
		}
		else
		{
			static_assert(false, "Type not implemented, LuaStack::Push<Type> at File " __FILE__);
		}
	}

}