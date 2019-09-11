#include "LuaState.h"

#include <LuaVar.h>


namespace lpp
{
	LuaState::~LuaState()
	{
		//Cleanup the state.
		lua_close(m_pState);
	}

	bool LuaState::Init()
	{
		m_pState = luaL_newstate();
		luaL_openlibs(m_pState);

		return true;
	}

	bool LuaState::LoadScript(const char* fileName)
	{
		int result = luaL_loadfile(m_pState, fileName);
		
		if (result == LUA_ERRFILE)
		{
			//DEBUG_LOG("Could not load file: %s", fileName);
			lua_pop(m_pState, 1);
			return false;
		}
		else if (result != LUA_OK)
		{
			//DEBUG_LOG("%s : %s", fileName, lua_tostring(m_pState, -1));
			lua_pop(m_pState, 1);
			return false;
		}

		if (lua_pcall(m_pState, 0, 0, 0) != LUA_OK)
		{
			//DEBUG_LOG("%s : %s", fileName, lua_tostring(m_pState, -1));
			lua_pop(m_pState, 1);
			return false;
		}

		return true;
	}

	void LuaState::PrintStack()
	{
		if(m_pState)
			LuaStack::PrintStack(m_pState);
	}

}