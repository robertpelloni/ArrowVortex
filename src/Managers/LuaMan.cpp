#include <Managers/LuaMan.h>

#include <lua.hpp>

#include <System/Debug.h>
#include <System/File.h>

namespace Vortex {

// ================================================================================================
// LuaManImpl :: member data.

struct LuaManImpl : public LuaMan {

lua_State* L;

// ================================================================================================
// LuaManImpl :: constructor and destructor.

LuaManImpl()
	: L(nullptr)
{
}

~LuaManImpl()
{
	shutdown();
}

// ================================================================================================
// LuaManImpl :: init and shutdown.

static int lua_print(lua_State* L)
{
	int n = lua_gettop(L);
	String msg;
	for(int i = 1; i <= n; ++i)
	{
		if(i > 1) msg += "\t";
		if(lua_isstring(L, i))
		{
			msg += lua_tostring(L, i);
		}
		else
		{
			msg += "???";
		}
	}
	HudNote("%s", msg.str());
	return 0;
}

void init()
{
	if(L) return;

	L = luaL_newstate();
	luaL_openlibs(L);

	lua_register(L, "print", lua_print);

	HudNote("Lua initialized.");
}

void shutdown()
{
	if(L)
	{
		lua_close(L);
		L = nullptr;
	}
}

// ================================================================================================
// LuaManImpl :: run script.

bool runScript(StringRef path)
{
	if(!L) init();

	if(luaL_dofile(L, path.c_str()) != LUA_OK)
	{
		const char* err = lua_tostring(L, -1);
		HudError("Lua Error: %s", err);
		lua_pop(L, 1);
		return false;
	}
	return true;
}

bool runString(StringRef script)
{
	if(!L) init();

	if(luaL_dostring(L, script.c_str()) != LUA_OK)
	{
		const char* err = lua_tostring(L, -1);
		HudError("Lua Error: %s", err);
		lua_pop(L, 1);
		return false;
	}
	return true;
}

lua_State* getState() const
{
	return L;
}

}; // LuaManImpl

// ================================================================================================
// LuaMan :: create and destroy.

LuaMan* gLua = nullptr;

void LuaMan::create()
{
	gLua = new LuaManImpl;
}

void LuaMan::destroy()
{
	delete gLua;
	gLua = nullptr;
}

}; // namespace Vortex
