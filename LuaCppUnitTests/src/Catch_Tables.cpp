#pragma once

#include <ostream>
#include <LuaVar.h>

// Must be last to include.
#include <catch2/catch.hpp>

TEST_CASE("Lua Tables", "[LuaCpp][Table Operations]")
{
	lpp::LuaState state;

	// Creating a table
	lpp::LuaVar var(&state);
	var.CreateTable();

	var.SetField("myInt", 100);
	var.SetField("myString", "Hello");
	

	REQUIRE(var.GetField("myInt").Get<int>() == 100);
	REQUIRE(var.GetField("myString").Get<std::string>() == "Hello");
}