#pragma once

#include <ostream>

#if __has_include(<vld.h>)
#include <vld.h>
#endif

#include <LuaVar.h>

class MyClass
{
	int m_value;
	std::string m_data;

public:
	MyClass(int val, std::string&& data) : m_value(val), m_data(data) {}

	int MyIntFunc(int a) { return a + m_value; }
	std::string MyStrFunc() { return m_data; }
	void MyVoidFunc() { printf("MyClass::MyVoidFunc, Says : Hello"); }
};

// Must be last to include.
#include <catch2/catch.hpp>

TEST_CASE("Class Binding", "[LuaCpp][Class Binding]")
{
	lpp::LuaState state;
	lpp::LuaVar var(&state);

	SECTION("Meta Table Binding")
	{
		var.CreateMetaTable("MyClass_Meta");
		var.BindMemberFunction<MyClass>("myIntFunc", &MyClass::MyIntFunc);
		var.BindMemberFunction<MyClass>("myStrFunc", &MyClass::MyStrFunc);
		var.BindMemberFunction<MyClass>("myVoidFunc", &MyClass::MyVoidFunc);
		
		MyClass cppInstance(10, "Hello");

		lpp::LuaVar instanceTable(&state);
		instanceTable.CreateTable();
		instanceTable.SetField("__this", &cppInstance);
		instanceTable.SetMetaTable("MyClass_Meta");

		instanceTable.Call("myVoidFunc");
		
		lpp::LuaVar intResult = instanceTable.Call("myIntFunc", 10);
		REQUIRE(intResult.Get<int>() == 20);

		lpp::LuaVar strResult = instanceTable.Call("myStrFunc");
		REQUIRE(strResult.Get<std::string>() == "Hello");
	}

	SECTION("Table Binding")
	{

	}
}