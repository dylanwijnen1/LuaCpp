#include "LuaVar.h"

#include <LuaState.h>
#include <LuaTableIterator.h>
#include <assert.h>

#define L m_pState->GetState()

namespace lpp
{

#pragma region Initialization / Construction

	LuaVar::LuaVar(const LuaVar& other)
		: m_pState(other.m_pState)
		, m_luaRef(other.m_luaRef)
		, m_pRefCount(other.m_pRefCount)
	{
		assert(m_pRefCount != nullptr);
		other.m_pRefCount->Increment();
	}

	LuaVar::LuaVar(LuaVar&& other) noexcept
		: m_pState(std::move(other.m_pState))
		, m_luaRef(std::exchange(other.m_luaRef, LUA_NOREF))
		, m_pRefCount(std::move(other.m_pRefCount))
	{
	}

	LuaVar& LuaVar::operator=(const LuaVar& other)
	{
		if (m_pRefCount)
		{
			if (m_pRefCount->Decrement() == 0)
				delete m_pRefCount;
		}

		m_pState = other.m_pState;
		m_luaRef = other.m_luaRef;
		m_pRefCount = other.m_pRefCount;

		assert(m_pRefCount != nullptr);
		m_pRefCount->Increment();

		return *this;
	}

	LuaVar& LuaVar::operator=(LuaVar&& other) noexcept
	{
		if (m_pRefCount)
		{
			if (m_pRefCount->Decrement() == 0)
				delete m_pRefCount;
		}

		m_pState = std::move(other.m_pState);
		m_luaRef = std::exchange(other.m_luaRef, LUA_NOREF);
		m_pRefCount = std::move(other.m_pRefCount);

		return *this;
	}

	LuaVar::LuaVar(LuaState* pState, int index)
		: m_pState(pState)
		, m_pRefCount(new RefCounter(1))
	{
		//Stack: [-index] {any}
		lua_pushvalue(L, index);
		m_luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	LuaVar::LuaVar(LuaState* pState, const char* globalName)
		: m_pState(pState)
		, m_luaRef(LUA_NOREF)
		, m_pRefCount(new RefCounter(1))
	{
		GetGlobal(globalName);
	}

	LuaVar::~LuaVar()
	{
		if (m_pRefCount)
		{
			// If we were the last RefCounted obj then unreference.
			if (m_pRefCount->Decrement() == 0)
			{
				if(m_luaRef != LUA_NOREF)
					luaL_unref(L, LUA_REGISTRYINDEX, m_luaRef);

				delete m_pRefCount;
			}
		}
	}

	void LuaVar::GetGlobal(const char* globalName)
	{
		lua_getglobal(L, globalName);	// Stack: [*]
		if (HasReference())
		{
			SetReferenceTop();
		}
		else
		{
			ReferenceTop();
		}
	}

	void LuaVar::SetGlobal(const char* globalName)
	{
		if (!PushToStack())
			return;

		lua_setglobal(L, globalName);
	}

#pragma endregion

#pragma region Reference Helpers

	bool LuaVar::PushToStack() const
	{
		//Can't push anything if we have no state to push to.
		if (!m_pState)
			return false;

		if (m_luaRef != LUA_NOREF)
		{
			lua_rawgeti(L, LUA_REGISTRYINDEX, m_luaRef);
			return true;
		}
		else
		{
			//DEBUG_LOG("Trying to access a LUA_NOREF");
			return false;
		}
	}

	bool LuaVar::ReferenceTop()
	{
		// If we have a reference, Overwrite
		if (m_luaRef != LUA_NOREF)
		{
			lua_rawseti(L, LUA_REGISTRYINDEX, m_luaRef);
			return true;
		}

		m_luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
		return m_luaRef != LUA_NOREF;
	}

	void LuaVar::SetReferenceTop()
	{
		lua_rawseti(L, LUA_REGISTRYINDEX, m_luaRef);
	}

#pragma endregion

#pragma region Table Functions

	void LuaVar::PrintTable(int spacing) const
	{
		lua_pushnil(L);
		std::string key = "";
		while (lua_next(L, -2) != 0)	//Stack: [1] table, [2] key, [3] value
		{
			if (lua_isnumber(L, -2))
			{
				lua_pushvalue(L, -2);
				key = lua_tostring(L, -1);
				lua_pop(L, 1);
			}
			else
 				key = lua_tostring(L, -2);

			int t = lua_type(L, -1);

			switch (t)
			{
				case LUA_TSTRING:
				{
					printf("%*s%s:'%s'\n", spacing, "", key.c_str(), lua_tostring(L, -1));
					break;
				}
				case LUA_TBOOLEAN:
				{
					printf("%*s%s: %s\n", spacing, "", key.c_str(), lua_toboolean(L, -1) ? "true" : "false");
					break;
				}
				case LUA_TNUMBER:
				{
					printf("%*s%s: %g\n", spacing, "", key.c_str(), lua_tonumber(L, -1));
					break;
				}
				case LUA_TTABLE:
				{
					printf("%*s%s: table\n", spacing, "", key.c_str());

					// Continue to recurse the "table tree" if they are not equal otherwise we can stack overflow.
					if (!lua_compare(L, -1, -3, LUA_OPEQ))
						PrintTable(spacing + 2);

					break;
				}
				case LUA_TLIGHTUSERDATA:
				case LUA_TUSERDATA:
				{
					printf("%*s%s: %p\n", spacing, "", key.c_str(), lua_touserdata(L, -1));
					break;
				}
				case LUA_TFUNCTION:
				{
					printf("%*s%s: function\n", spacing, "", key.c_str());
					break;
				}
				default:
				{
					printf("%*s%s: %s\n", spacing, "", key.c_str(), lua_typename(L, -1));
					break;
				}
			}

			lua_pop(L, 1);				//Stack: [table, key]
		}
	}

	bool LuaVar::IsTable() const
	{
		if(!PushToStack())
			return false;

		bool result = lua_istable(m_pState->GetState(), -1);	// Stack: [table]
		lua_pop(L, 1);											// Stack: 
		return result;
	}

	void LuaVar::CreateTable()
	{
		lua_newtable(L);									// Stack: [0] table
		if (HasReference())
		{
			SetReferenceTop();
		}
		else
		{
			ReferenceTop();
		}
	}

	LuaVar LuaVar::GetField(const char* fieldName)
	{
		if (!PushToStack())
			return LuaVar(m_pState);	//Return a nil val.
										//Stack: [1] table

		lua_pushstring(L, fieldName);
		lua_rawget(L, -2);				//Stack: [1] table, [2] value

		LuaVar var(m_pState, -1);		//Stack: [1] table, [2] value
		lua_pop(L, 2);					//Stack: 
		return var;
	}

	const LuaVar LuaVar::GetField(const char* fieldName) const
	{
		if (!PushToStack())
			return LuaVar(m_pState);	//Return a nil val.
										//Stack: [1] table

		lua_pushstring(L, fieldName);
		lua_rawget(L, -2);				//Stack: [1] table, [2] value

		LuaVar var(m_pState, -1);		//Stack: [1] table, [2] value
		lua_pop(L, 2);					//Stack: 
		return var;
	}

	LuaVar LuaVar::operator[](const char* field)
	{
		if (!PushToStack())
			return LuaVar(m_pState);	//Return a nil val.
										//Stack: [1] table

		lua_pushstring(L, field);
		lua_rawget(L, -2);				//Stack: [1] table, [2] value

		LuaVar var(m_pState, -1);		//Stack: [1] table, [2] value
		lua_pop(L, 2);					//Stack: 
		return var;
	}

	LuaVar LuaVar::operator[](int index)
	{
		if (!PushToStack())
			return LuaVar(m_pState);	//Return a nil val.
										//Stack: [1] table

		lua_rawgeti(L, -1, index);		//Stack: [1] table, [2] value

		LuaVar var(m_pState, -1);		//Stack: [1] table, [2] value
		lua_pop(L, 2);					//Stack: 
		return var;
	}

	void LuaVar::PrintTable()
	{
		if (!PushToStack())
			return;
										//Stack: [1] table

		// Do not print if we're not a table.
		if (!IsTable())
			return;

		PrintTable(0);

		lua_pop(L, 1);					//Stack: 
	}

	LuaTableIterator LuaVar::Begin()
	{
		return LuaTableIterator(m_luaRef, m_pState);
	}

	LuaTableIterator LuaVar::End()
	{
		return LuaTableIterator();
	}

	void LuaVar::CreateMetaTable(const char* metatableName)
	{
		luaL_newmetatable(L, metatableName);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		ReferenceTop();
	}

	bool LuaVar::SetMetaTable(const char* metatableName)
	{
		if (!PushToStack() && IsTable())
			return false;

												// [varTable]
		lua_newtable(L);						// [varTable, table]
		luaL_getmetatable(L, metatableName);	// [varTable, table, metaTable]
		lua_setfield(L, -2, "__index");			// [varTable, table]
		lua_setmetatable(L, -2);

		lua_pop(L, 1);

		return true;
	}

	//void LuaVar::SetMetaTable(const LuaVar& metaTable)
	//{
	//}

#pragma endregion

#pragma region Function Functions

	bool LuaVar::IsFunction()
	{
		if (!PushToStack())
			return false;

		if (lua_isfunction(L, -1))
		{
			lua_pop(L, -1);
			return true;
		}

		lua_pop(L, -1);
		return false;
	}

	LuaVar LuaVar::Call(const char* functionName)
	{
		if (!PushToStack())
			return LuaVar();

													// [table]

		lua_getfield(L, -1, functionName);			// [table, func]

		if (!lua_isfunction(L, -1))
		{
			lua_pop(L, 2);							// !!! []
			return LuaVar();
		}

		int lastTop = lua_gettop(L);

		lua_pushvalue(L, -2);						// [table, func, table]

		// Call the function
		if (int result = lua_pcall(L, 1, LUA_MULTRET, 0); result != LUA_OK)
		{
			FormatCallError(result, functionName);
			return LuaVar();
		}

		int newTop = lua_gettop(L) + 1;

		LuaVar result = BuildReturnValue(lastTop, newTop);	// [table]

		lua_pop(L, 1);								// []
		return result;
	}

	LuaVar LuaVar::operator()()
	{
		if (!PushToStack())
			return LuaVar();
		
										// [func]

		if (!lua_isfunction(L, -1))
		{
			lua_pop(L, 1);
			return LuaVar();
		}

		int lastTop = lua_gettop(L);

		// Call the function
		if (int result = lua_pcall(L, 0, LUA_MULTRET, 0); result != LUA_OK)
		{
			FormatCallError(result, "Anonymous function call failed.");
			return LuaVar();
		}

		//Detect how many return values we've got.
		int newTop = lua_gettop(L) + 1;

		// Return the result.
		return BuildReturnValue(lastTop, newTop);
	}

	void LuaVar::FormatCallError(int errorCode, const char* msg)
	{
		switch (errorCode)
		{

		case LUA_ERRRUN: 
			//DEBUG_LOG("A runtime error occured: %s", msg);
			break;
		case LUA_ERRMEM: 
			//DEBUG_LOG("Memory allocation error: %s", msg);
			break;
		case LUA_ERRERR: 
			//DEBUG_LOG("Error during Error: %s", msg);
			break;
		}

		// Pop error message from the stack.
		lua_pop(L, 1);
	}

	LuaVar LuaVar::BuildReturnValue(int stackTop, int stackNew)
	{
		LuaVar result(m_pState);
		
		const int count = stackNew - stackTop;

		if (count == 1)
		{
			// Creates a LuaVar reference from the top element.
			result.ReferenceTop();
		}
		else if (count > 1)
		{
			// Create a table to hold the return values.
			result.CreateTable();
			result.PushToStack();

			lua_insert(L, stackTop);

			// Create a table indexed from [1 ... count]
			for (int i = count; i > 0; --i)
			{
				lua_rawseti(L, 1, i);
			}

			lua_pop(L, 1);
		}

		return result;
	}

#pragma endregion

	void LuaVar::Print() const
	{
		if (PushToStack())
		{
			LuaStack::Print(L, -1);
			lua_pop(L, 1);
		}
		else
		{
			printf("LuaVar has no ref.");
		}
	}

}