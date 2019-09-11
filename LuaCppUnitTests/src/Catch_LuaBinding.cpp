#pragma once

#include <ostream>
#include <LuaVar.h>

// Must be last to include.
#include <catch2/catch.hpp>

TEST_CASE("Initialize", "[LuaCpp][Boolean Operations]")
{
	lpp::LuaState state;

	// Creating a table
	lpp::LuaVar var(&state);
	var.Set<bool>(true);
	var.Print();

	REQUIRE(var.Get<bool>() == true);

	state.PrintStack();

}