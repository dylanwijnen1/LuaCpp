#pragma once

#include <ostream>

#if __has_include(<vld.h>)
	#include <vld.h>
#endif

#include <LuaVar.h>

// Must be last to include.
#include <catch2/catch.hpp>

TEST_CASE("Basic Types", "[LuaCpp][Basic Types]")
{
	lpp::LuaState state;

	// Creating a table
	lpp::LuaVar var(&state);

	SECTION("Boolean")
	{
		var.Set<bool>(true);
		REQUIRE(var.Get<bool>() == true);
		REQUIRE(var.Is<bool>() == true);

		var.Set<bool>(false);

		REQUIRE(var.Get<bool>() == false);
		REQUIRE(var.Is<bool>() == true);
	}

	SECTION("Numbers")
	{
		SECTION("Integers")
		{
			var.Set<int>(10);
			REQUIRE(var.Get<int>() == 10);
			
			REQUIRE(var.Is<int>() == true);
			REQUIRE(var.Is<double>() == true);
			REQUIRE(var.Is<float>() == true);
			REQUIRE(var.Is<size_t>() == true);
		}

		SECTION("Decimals")
		{
			var.Set<float>(10.f);
			
			REQUIRE(var.Get<float>() == 10.f);
			REQUIRE(var.Get<int>() == 10);

			REQUIRE(var.Is<float>() == true);
			REQUIRE(var.Is<double>() == true);
			REQUIRE(var.Is<size_t>() == false);
			REQUIRE(var.Is<int>() == false);
		}
	}

	SECTION("Strings")
	{
		var.Set<std::string>("Hello World");
		REQUIRE(var.Is<std::string>() == true);
		REQUIRE(var.Get<std::string>() == "Hello World");
	}

	SECTION("Nils")
	{
		// TODO: Set<lpp::Nil>()
		var.Set<nullptr_t>(nullptr);

		REQUIRE(var.Is<nullptr_t>() == true);
	}
}