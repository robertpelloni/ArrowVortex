#include <Managers/LuaMan.h>
#include <Managers/SimfileMan.h>
#include <Managers/StyleMan.h>
#include <Simfile/Simfile.h>
#include <Simfile/Chart.h>

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

static int lua_getSongTitle(lua_State* L)
{
	if (gSimfile && gSimfile->get())
		lua_pushstring(L, gSimfile->get()->title.str());
	else
		lua_pushstring(L, "");
	return 1;
}

static int lua_getSongArtist(lua_State* L)
{
	if (gSimfile && gSimfile->get())
		lua_pushstring(L, gSimfile->get()->artist.str());
	else
		lua_pushstring(L, "");
	return 1;
}

static int lua_getSongDir(lua_State* L)
{
	if (gSimfile && gSimfile->get())
		lua_pushstring(L, gSimfile->get()->dir.str());
	else
		lua_pushstring(L, "");
	return 1;
}

static int lua_getChartDifficulty(lua_State* L)
{
	if (gSimfile)
	{
		const Chart* chart = gSimfile->getChart(gSimfile->getActiveChart());
		if (chart)
		{
			lua_pushstring(L, GetDifficultyName(chart->difficulty));
			return 1;
		}
	}
	lua_pushstring(L, "");
	return 1;
}

static int lua_getChartMeter(lua_State* L)
{
	if (gSimfile)
	{
		const Chart* chart = gSimfile->getChart(gSimfile->getActiveChart());
		if (chart)
		{
			lua_pushinteger(L, chart->meter);
			return 1;
		}
	}
	lua_pushinteger(L, 0);
	return 1;
}

void init()
{
	if(L) return;

	L = luaL_newstate();
	luaL_openlibs(L);

	lua_register(L, "print", lua_print);

	// Create Vortex table
	lua_newtable(L);
	
	lua_pushcfunction(L, lua_print);
	lua_setfield(L, -2, "log");

	lua_pushcfunction(L, lua_getSongTitle);
	lua_setfield(L, -2, "getSongTitle");

	lua_pushcfunction(L, lua_getSongArtist);
	lua_setfield(L, -2, "getSongArtist");

	lua_pushcfunction(L, lua_getSongDir);
	lua_setfield(L, -2, "getSongDir");

	lua_pushcfunction(L, lua_getChartDifficulty);
	lua_setfield(L, -2, "getChartDifficulty");

	lua_pushcfunction(L, lua_getChartMeter);
	lua_setfield(L, -2, "getChartMeter");

	lua_setglobal(L, "Vortex");

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
