#pragma once

#include <Core/Core.h>

struct lua_State;

namespace Vortex {

/// Manages the Lua scripting environment.
struct LuaMan
{
	static void create();
	static void destroy();

	/// Initializes the Lua state.
	virtual void init() = 0;

	/// Shuts down the Lua state.
	virtual void shutdown() = 0;

	/// Loads and executes a Lua script from a file.
	virtual bool runScript(StringRef path) = 0;

	/// Executes a Lua string.
	virtual bool runString(StringRef script) = 0;

	/// Returns the Lua state.
	virtual lua_State* getState() const = 0;
};

extern LuaMan* gLua;

}; // namespace Vortex
