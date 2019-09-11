#pragma once

#include <lua.hpp>
#include <string>
#include <type_traits>

#include <RefCounter.h>
#include <LuaState.h>
#include <LuaStack.h>

namespace lpp
{
	class LuaTableIterator;

	/// \class LuaVar
	/// \brief LuaVar is the glue between C++ and Lua, It allows you to virtually do anything with a simple interface.
	/// LuaVar is a reference counted object be aware that a copy of the LuaVar isn't an actual copy of the data and its just another LuaVar pointing to the same reference.
	/// Therefore the life cycle of the referenced lua object is dependent on the last LuaVar with the reference.
	///
	/// \devnote Even though the "Setter" functions could be marked `const` their intention is to "Set" the Lua object, This can obfuscate the intention of the function. DO NOT MARK AS CONST.
	class LuaVar
	{
	private:
		LuaState* m_pState;
		int m_luaRef;
		RefCounter* m_pRefCount;

	public:

#pragma region Constructors / Initialization

		/// Creates an empty LuaVar. This LuaVar has no LuaState
		LuaVar()
			: m_pState(nullptr)
			, m_luaRef(LUA_NOREF)
			, m_pRefCount(new RefCounter(1))
		{}

		/// Creates an empty LuaVar.
		/// <param name="pState"></param>
		LuaVar(LuaState* pState)
			: m_pState(pState)
			, m_luaRef(LUA_NOREF)
			, m_pRefCount(new RefCounter(1))
		{}

		LuaVar(const LuaVar&);
		LuaVar(LuaVar&&) noexcept;
		LuaVar& operator=(const LuaVar&);
		LuaVar& operator=(LuaVar&&) noexcept;

		/// Creates a LuaVar from the stack.
		/// <param name="pState"></param>
		/// <param name="index"></param>
		LuaVar(LuaState* pState, int index);

		/// Creates a LuaVar from a global variable.
		/// <param name="pState"></param>
		/// <param name="globalName"></param>
		LuaVar(LuaState* pState, const char* globalName);

		/// Unregisters the variable from the registry.
		~LuaVar();

#pragma endregion

		/// <summary>
		/// Puts the global into this lua var
		/// </summary>
		void GetGlobal(const char* globalName);

		/// <summary>
		/// Creates a global var of this LuaVar
		/// </summary>
		void SetGlobal(const char* globalName);

		///Getter for any templated value.
		template<typename Type>
		Type GetValue(const Type& defaultVal = Type()) const;

		///Setter for any templated value.
		template<typename Type>
		void SetValue(const Type& value);

#pragma region Getters / Setters, Native Types

		template<typename Type>
		void Set(Type&& val)
		{
			LuaStack::Push(m_pState->GetState(), std::forward<Type>(val));
			ReferenceTop();
		}

		template<typename Type>
		void Set(const Type& val)
		{
			LuaStack::Push(m_pState->GetState(), val);
			ReferenceTop();
		}

		template<typename Type>
		Type Get()
		{
			if (!PushToStack())
				return Type();

			Type val = LuaStack::Get<Type>(m_pState->GetState(), -1);
			lua_pop(m_pState->GetState(), 1);
			return val;
		}

		template<typename Type>
		Type Get(Type&& defaultVal)
		{
			if (!PushToStack())
				return defaultVal;

			Type val = defaultVal;

			if (Is(m_pState->GetState(), -1))
				val = LuaStack::Get<Type>(m_pState->GetState(), -1);
			else
				val = defaultVal;

			lua_pop(m_pState->GetState(), 1);
			return val;
		}

		template<typename Type>
		bool Is()
		{
			if (!PushToStack())
			{
				if constexpr (std::is_null_pointer_v<Type>)
					return true;
				else
					return false;
			}
			else
			{
				return LuaStack::Is<Type>(m_pState->GetState(), -1);
			}
		}

#pragma endregion

#pragma region Table Functions

		/// <summary>
		/// Check if the LuaVar reference represents a table value.
		/// </summary>
		bool IsTable() const;

		/// <summary>
		/// Override the current value with an empty table.
		/// </summary>
		void CreateTable();

		/// <summary>
		/// Get a field from the LuaVar table.
		/// </summary>
		LuaVar GetField(const char* fieldName);

		/// <summary>
		/// Get a field from the LuaVar table.
		/// </summary>
		const LuaVar GetField(const char* fieldName) const;

		/// <summary>
		///	Set a field on the Lua table.
		/// </summary>
		template<typename Type>
		void SetField(const char* fieldName, const Type& val);

		/// <summary>
		/// Try to get a field from the LuaVar table.
		/// </summary>
		/// <param name="field"></param>
		/// <returns></returns>
		LuaVar operator[](const char* field);

		/// <summary>
		/// Try to get a field from the LuaVar table.
		/// </summary>
		/// <param name="index"></param>
		/// <returns></returns>
		LuaVar operator[](int index);

		/// <summary>
		/// Parses the current LuaVar reference as a table.
		/// </summary>
		void PrintTable();

		/// <summary>
		/// Returns the first field in the LuaVar table.
		/// </summary>
		LuaTableIterator Begin();

		/// <summary>
		/// Returns an ending iterator for the LuaVar table.
		/// </summary>
		LuaTableIterator End();

		/// Creates a metatable and references it.
		void CreateMetaTable(const char* metatableName);

		/// Sets the metatable of a table from the registry and modifies the __index for that table. 
		bool SetMetaTable(const char* metatableName);

		/// Sets the metatable from a LuaVar
		//void SetMetaTable(const LuaVar& metaTable);

#pragma endregion

#pragma region Functions and Binding

		///Calls a function on the LuaVar passing the table as reference.
		LuaVar Call(const char* functionName);

		/// Calls a function on the LuaVar passing the table as reference.
		template<typename... Args>
		LuaVar Call(const char* functionName, Args... args);

		/// Check wether the current LuaVar reference is a function.
		bool IsFunction();

		/// Calls the current LuaVar reference as a function.
		LuaVar operator()();

		/// Calls the current function with arguments.
		template<typename... Args>
		LuaVar operator()(Args... args);

		template<typename Object, typename Function>
		void BindMemberFunction(const char* funcName, Function&& func);

#pragma endregion

		/// Pushes the LuaVar reference to the stack.
		/// \return If the LuaVar is a nil reference it will return false.
		bool PushToStack() const;

		/// Prints the LuaVar as best to its ability.
		void Print() const;

	private:

		/// <summary>
		/// Creates a reference from the value on top of the stack.
		/// </summary>
		bool ReferenceTop();

		/// <summary>
		/// Sets the current reference to the value on top of the stack.
		/// </summary>
		void SetReferenceTop();

		/// <summary>
		/// Returns wether the LuaVar has a reference.
		/// </summary>
		bool HasReference() const { return m_luaRef != LUA_NOREF; }

		/// <summary>
		/// Print the LuaVar table with a specified amount of spacing.
		/// \param spacing Amount of spaces before the line to be printed.
		/// </summary>
		void PrintTable(int spacing) const;

		/// <summary> 
		/// Calls the bound member function 
		/// </summary>
		/// <devnote>
		///	Grabs the object from the table from the field `__this`
		///	Then we extract the function pointer we bound from the upvalue and call the function.
		/// </devnote>
		template<typename Object, typename Function>
		static int CallBoundMemberFunction(lua_State* pState);

		/// We first grab the amount of arguments we require to call the function.
		/// After building the argument pack we call the function passing the arguments through.
		/// \returns If the function has a return type we push the value to the stack using LuaStack::Push() and return 1
		template<typename Object, typename ReturnType, typename... Args>
		static int StdCall(lua_State* pState, Object* pObj, ReturnType(Object::* pFunc)(Args...));

		
		/// <summary>
		/// Build a tuple from the given Variadic Template, Calling LuaStack::Get() for each argument.
		/// </summary>
		template<typename... Args>
		static std::tuple<Args...> BuildArguments(lua_State* pState);

		/// <summary>
		/// Builds a return table for the function that was called.
		/// </summary>
		/// <param name="stackTop">The lua stack index for where the function is called.</param>
		/// <param name="stackNew">The lua stack index that is the new top after the function was called.</param>
		LuaVar BuildReturnValue(int stackTop, int stackNew);

		/// <summary>
		/// Formats a message according to the error code returned by calling the lua function. Then pops the error message from the stack.
		/// </summary>
		/// <devnote>
		/// LUA_ERRRUN: A runtime error occured calling the function, Error : %s
		/// LUA_ERRMEM: Memory allocation error, Error : %s
		/// LUA_ERRERR: An error occured whilst generating the error, Error : %s
		/// LUA_ERRGCMM: An error occured running the __gc method, Error : %s
		/// </devnote>
		void FormatCallError(int errorCode, const char* msg);
	};
	
#pragma region Template Definitions

	template<typename Type>
	inline Type LuaVar::GetValue(const Type& defaultVal) const
	{
		if (!PushToStack())
			return defaultVal;

														// [?]

		lua_State* L = m_pState->GetState();

		Type val = LuaStack::Get(L, -1, defaultVal);

		lua_pop(L, 1);									// []

		return val;
	}

	template<typename Type>
	inline void LuaVar::SetValue(const Type& value)
	{
		if (!PushToStack())
			return;				// TODO: return false ?

								// [?]

		lua_State* L = m_pState->GetState();

		LuaStack::Push(L, value);
		SetReferenceTop();

		lua_pop(L, 1);			// []
		// return true;
	}

	template<typename Type>
	inline void LuaVar::SetField(const char* fieldName, const Type& val)
	{
		if (!PushToStack())
			return;

		lua_State* L = m_pState->GetState();

		if (!lua_istable(L, -1))
			return;
										// [table]

		LuaStack::Push(L, val);
		lua_setfield(L, -2, fieldName);

		lua_pop(L, 1);					// []
	}

	template<typename... Args>
	inline LuaVar LuaVar::Call(const char* functionName, Args... args)
	{
		if (!PushToStack())
			return LuaVar();

													// [table]

		lua_State* L = m_pState->GetState();

		int lastTop = lua_gettop(L);

		lua_getfield(L, -1, functionName);			// [table, func]

		if (!lua_isfunction(L, -1))
		{
			lua_pop(L, 2);							// !!! []
			return LuaVar();
		}


		lua_pushvalue(L, -2);						// [table, func, table]

		// --
		// Push Arguments

		// C++ 17 Fold Expression on the ',' operator.
		((void)LuaStack::Push<Args>(L, args), ...);

		int argCount = sizeof...(Args) + 1; // arguments + table.
		// --

		// Call the function
		if (int result = lua_pcall(L, argCount, LUA_MULTRET, 0); result != LUA_OK)
		{
			FormatCallError(result, functionName);
			return LuaVar();
		}

		//Detect how many return values we've got.
		int newTop = lua_gettop(L);

		LuaVar result = BuildReturnValue(lastTop, newTop);	// [table]

		lua_pop(L, 1);								// []
		return result;
	}

	template<typename... Args>
	inline LuaVar LuaVar::operator()(Args... args)
	{
		return LuaVar();
	}

	template<typename Object, typename Function>
	inline void LuaVar::BindMemberFunction(const char* funcName, Function&& func)
	{
		PushToStack();																						//  [t]

		// HACK: Literally copying the memory address of the member function storing it as an upvalue.
		void* pBuffer = lua_newuserdata(m_pState->GetState(), sizeof(Function));   //  [t, pMemberFunc]
		std::memcpy(pBuffer, &func, sizeof(Function));

		lua_pushcclosure(m_pState->GetState(), &CallBoundMemberFunction<Object, Function>, 1);				//  [t, closure]
		lua_setfield(m_pState->GetState(), -2, funcName); 													//  [t]
		lua_pop(m_pState->GetState(), 1);
	}

	template<typename Object, typename Function>
	inline int LuaVar::CallBoundMemberFunction(lua_State* pState)
	{
		constexpr const char kInstanceFieldName[] = "__this";

		if (lua_istable(pState, 1))
		{
			// Try to get `__this` from the table.
			lua_getfield(pState, 1, kInstanceFieldName);										// [t, __this]
			Object* pObj = reinterpret_cast<Object*>(lua_touserdata(pState, -1));	// [t]

			// Should pop the object of the stack too
			lua_pop(pState, 1);

			if (pObj != nullptr)
			{
				void* pFuncBuffer = lua_touserdata(pState, lua_upvalueindex(1));		// [t, pMemberFunction]
				Function* pFunc = reinterpret_cast<Function*>(pFuncBuffer);
				return LuaVar::StdCall(pState, pObj, *pFunc);
			}
			else
			{
				//DEBUG_LOG("__this is nil, Cannot call member function on a nil value.");
				return 0;
			}
		}
		else
		{
			//DEBUG_LOG("Attempting to call a member function without object being present.");
			return 0;
		}
	}

	template<typename Object, typename ReturnType, typename ...Args>
	inline int LuaVar::StdCall(lua_State* pState, Object* pObj, ReturnType(Object::* pFunc)(Args...))
	{
		using ObjectTuple = std::tuple<Object*>;
		auto arguments = std::tuple_cat(ObjectTuple(pObj), BuildArguments<Args...>(pState));

		if constexpr (std::is_void_v<ReturnType>)
		{
			// ((*pObj).*(pFunc))(pack...);
			std::apply(pFunc, arguments);
			return 0;
		}
		else
		{
			ReturnType val = std::apply(pFunc, arguments);
			LuaStack::Push(pState, val);
			return 1;
		}
	}

	template<typename... Args>
	inline std::tuple<Args...> LuaVar::BuildArguments(lua_State* pState)
	{
		int i = 1;
		return std::tuple<Args...>(LuaStack::Get<Args>(pState, ++i)...);
	}

#pragma endregion


}