#pragma once

//#include <Dragon/Logic/Scripts/LuaVar.h>

#include <lua.hpp>

namespace lpp
{

	/// <summary>
	/// \class LuaState
	/// 
	/// Encapsulates the Lua State memory and adds script loading and debug functions.
	/// 
	/// \b Example:
	/// ~~~~~
	/// LuaState* pState = LuaState::Create().get();
	/// if(!pState->LoadScript("test.lua"))
	///		//DEBUG_LOG("Warning", "Failed to load Lua file: test.lua");
	///
	/// LuaVar foo = pState->GetGlobal("myGlobalVar");
	/// float fooFloat = foo.GetFloat();
	/// ~~~~~
	/// </summary>
	class LuaState
	{
		lua_State* m_pState;


	public:
		LuaState() : LuaState(luaL_newstate()) {}
		LuaState(lua_State* pState) : m_pState(pState) {}

		/// <summary>
		/// Clean up the underlying lua state memory.
		/// </summary>
		~LuaState();

		/// <summary>
		/// Initializes the lua state and general purpose libraries.
		/// </summary>
		/// <returns></returns>
		bool Init();

		/// <summary>
		/// Get the actual lua state.
		/// </summary>
		/// <returns>\ret The underlying lua state memory.</returns>
		lua_State* GetState() { return m_pState; }

		/// <summary>
		/// Loads a script and returns if the file was loaded.
		/// </summary>
		/// <param name="fileName">\param fileName The path to the file</param>
		/// <returns>\ret Wether the file was loaded or not</returns>
		bool LoadScript(const char* fileName);

		/// <summary>
		/// Prints the current stack of the state to the console.
		/// TODO: Make it fancy by using the //DEBUG_LOG and coloring for different types.
		/// </summary>
		void PrintStack();
	};

}