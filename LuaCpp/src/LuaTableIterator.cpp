#include "LuaTableIterator.h"

#include <LuaState.h>

#define L m_pState->GetState()

namespace lpp
{
	LuaTableIterator::LuaTableIterator(int ref, LuaState * pState)
		: m_index(0)
		, m_luaRef(ref)
		, m_pState(pState)
	{
		if (m_pState)
			++(*this);
	}

	LuaTableIterator& LuaTableIterator::operator++()
	{
		if (!PushReference())
		{
			m_luaRef = LUA_NOREF;
			m_index = -1;
			return *this;
		}

										//Stack: [1] table
		if (!lua_istable(L, -1))
		{
			//DEBUG_LOG("Attempted to iterate over a non table value.");
			lua_pop(L, 1);
			return *this;
		}

		if (m_index == 0)
		{
			lua_pushnil(L);					//Stack: [1] table, [2] nil
			if(lua_next(L, -2) != 0)
				ConstructKeyValuePair();	//Stack: [1] table
			else
			{
				//Nothing in the table
				lua_pop(L, 1);				//Stack: [1] table
				m_index = -1;
				m_luaRef = LUA_NOREF;
				return *this;
			}
		}
		else
		{
			m_pair.key.PushToStack();		//Stack: [1] table, [2] key
			if (lua_next(L, -2) != 0)
				ConstructKeyValuePair();	//Stack: [1] table
			else
			{
				//We are done !
				lua_pop(L, 1);				//Stack: [1] table
				m_index = -1;
				m_luaRef = LUA_NOREF;
				return *this;
			}
		}

		//Pop the table.
		lua_pop(L, 1);						//Stack: 

		++m_index;
		return *this;
	}

	LuaTableIterator LuaTableIterator::operator++(int)
	{
		return ++LuaTableIterator(*this);
	}

	bool LuaTableIterator::operator==(const LuaTableIterator & other)
	{
		return (m_luaRef == other.m_luaRef) && (m_index == other.m_index);
	}

	void LuaTableIterator::ConstructKeyValuePair()
	{
		m_pair.value = LuaVar(m_pState, -1);
		m_pair.key = LuaVar(m_pState, -2);
		lua_pop(L, 2);
	}

	bool LuaTableIterator::PushReference()
	{
		if (m_luaRef != LUA_NOREF)
		{
			lua_rawgeti(m_pState->GetState(), LUA_REGISTRYINDEX, m_luaRef);
			return true;
		}
		else
		{
			//DEBUG_LOG("Trying to iterate a LUA_NOREF");
			return false;
		}
	}

}
