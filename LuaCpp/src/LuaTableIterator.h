#pragma once

#include <lua.hpp>
#include <unordered_map>
#include <LuaKeyValuePair.h>

namespace lpp
{
	class LuaState;

	class LuaTableIterator
	{
		int m_index;
		int m_luaRef; //Reference number of the table on the registry.
		LuaState* m_pState;

		LuaKeyValuePair m_pair;

	public:
		//Also serves as the End iterator.
		LuaTableIterator()
			: m_index(-1)
			, m_luaRef(LUA_NOREF)
			, m_pState(nullptr)
		{}

		LuaTableIterator(int ref, LuaState* pState);

		LuaTableIterator(const LuaTableIterator&) = default;
		LuaTableIterator& operator=(const LuaTableIterator&) = default;
		LuaTableIterator(LuaTableIterator&&) = default;
		LuaTableIterator& operator=(LuaTableIterator&&) = default;

		~LuaTableIterator() = default;

		LuaTableIterator& operator++();
		LuaTableIterator operator++(int);

		bool operator==(const LuaTableIterator& other);
		bool operator!=(const LuaTableIterator& other) { return !(*this == other); }

		LuaKeyValuePair* operator->() { return &m_pair; }
		LuaKeyValuePair& operator*() { return m_pair; }

	private:
		void ConstructKeyValuePair();
		bool PushReference();
	};

}