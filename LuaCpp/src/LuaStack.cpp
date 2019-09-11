#include "LuaStack.h"

namespace lpp
{

	bool LuaStack::SafePop(lua_State* pState, int amount)
	{
		if (lua_gettop(pState) >= amount)
		{
			lua_pop(pState, amount);
			return true;
		}
		else
		{
			return false;
		}
	}

	void LuaStack::Print(lua_State* pState, int index)
	{
		int t = lua_type(pState, index);
		switch (t)
		{
		case LUA_TSTRING:
			printf("'%s'\n", lua_tostring(pState, index));
			break;
		case LUA_TBOOLEAN:
			printf("%s\n", lua_toboolean(pState, index) ? "true" : "false");
			break;
		case LUA_TNUMBER:
			printf("%g\n", lua_tonumber(pState, index));
			break;
		default:
			printf("%s\n", lua_typename(pState, t));
			break;
		}
	}

	void LuaStack::PrintStack(lua_State* pState)
	{
		if (!pState)
			return;

		int i = lua_gettop(pState);
		printf(" ----------------  Stack Dump ----------------\n");
		while (i)
		{
			int t = lua_type(pState, i);
			switch (t)
			{
			case LUA_TSTRING:
				printf("%i:'%s'\n", i, lua_tostring(pState, i));
				break;
			case LUA_TBOOLEAN:
				printf("%i: %s\n", i, lua_toboolean(pState, i) ? "true" : "false");
				break;
			case LUA_TNUMBER:
				printf("%i: %g\n", i, lua_tonumber(pState, i));
				break;
			default:
				printf("%i: %s\n", i, lua_typename(pState, t));
				break;
			}
			i--;
		}
		printf("--------------- Stack Dump Finished ---------------\n");
	}
}
