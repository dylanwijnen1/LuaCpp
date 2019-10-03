# LuaCpp
Lua binding framework for C++ 17

# Features
* Binding to Lua variables
* Ability to Set/Get/GetDefault/Is operations for any native type.
* Lua Table iteration using C++ iterators.
* Binding to functions
  * Bind any function to a lua variable.
  * Bind any lua function to a C++ variable. Allows for any amount of parameters and any amount of return values.
  
  
# Upcoming Features
* C++ Class binding, Easily bind an entire class to lua, with the ability to set memory ownership to Lua or C++. This would mean if ownership is in C++ the garbage collector will not clean up the created instances.
* Lua `nil` type implementation for C++.
* `operator` support for `LuaVar`, Multiply two bindings and get the result.
