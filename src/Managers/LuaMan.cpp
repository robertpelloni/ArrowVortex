#include <Managers/LuaMan.h>
#include <Managers/SimfileMan.h>
#include <Managers/StyleMan.h>
#include <Managers/TempoMan.h>
#include <Managers/NoteMan.h>
#include <Simfile/Simfile.h>
#include <Simfile/Chart.h>
#include <Simfile/Tempo.h>
#include <Simfile/NoteList.h>

#include <Editor/Selection.h>

#include <lua.hpp>

#include <System/Debug.h>
#include <System/File.h>

namespace Vortex {

// ================================================================================================
// LuaManImpl :: member data.

struct LuaManImpl : public LuaMan {

lua_State* L;
Vector<String> myScripts;

// ================================================================================================
// LuaManImpl :: constructor and destructor.

LuaManImpl()
	: L(nullptr)
{
	refreshScripts();
}

~LuaManImpl()
{
	shutdown();
}

// ================================================================================================
// LuaManImpl :: init and shutdown.

void refreshScripts()
{
	myScripts.clear();
	Vector<Path> files = File::findFiles("scripts/", false);
	for(const auto& file : files)
	{
		if(file.hasExt("lua"))
		{
			myScripts.append(file.filename());
		}
	}
}

int getNumScripts() const
{
	return myScripts.size();
}

StringRef getScriptName(int index) const
{
	if(index >= 0 && index < myScripts.size())
	{
		return myScripts[index];
	}
	return "";
}

StringRef getScriptPath(int index) const
{
	if(index >= 0 && index < myScripts.size())
	{
		return "scripts/" + myScripts[index];
	}
	return "";
}

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

static int lua_getNotes(lua_State* L)
{
	lua_newtable(L);
	if (gSimfile)
	{
		const Chart* chart = gSimfile->getChart(gSimfile->getActiveChart());
		if (chart)
		{
			int i = 1;
			for (const auto& note : chart->notes)
			{
				lua_newtable(L);
				
				lua_pushinteger(L, note.row);
				lua_setfield(L, -2, "row");
				
				lua_pushinteger(L, note.col);
				lua_setfield(L, -2, "col");
				
				lua_pushinteger(L, note.type);
				lua_setfield(L, -2, "type");
				
				lua_pushinteger(L, note.endrow);
				lua_setfield(L, -2, "endrow");
				
				lua_pushinteger(L, note.player);
				lua_setfield(L, -2, "player");

				lua_rawseti(L, -2, i++);
			}
		}
	}
	return 1;
}

static int lua_rowToTime(lua_State* L)
{
	double row = luaL_checknumber(L, 1);
	double time = gTempo->rowToTime(row);
	lua_pushnumber(L, time);
	return 1;
}

static int lua_timeToRow(lua_State* L)
{
	double time = luaL_checknumber(L, 1);
	double row = gTempo->timeToRow(time);
	lua_pushnumber(L, row);
	return 1;
}

static int lua_addNote(lua_State* L)
{
	// Arguments: row, col, type, [endrow], [player]
	int row = (int)luaL_checkinteger(L, 1);
	int col = (int)luaL_checkinteger(L, 2);
	int type = (int)luaL_checkinteger(L, 3);
	int endrow = row;
	if (lua_gettop(L) >= 4) endrow = (int)luaL_checkinteger(L, 4);
	int player = 0;
	if (lua_gettop(L) >= 5) player = (int)luaL_checkinteger(L, 5);

	if (gNotes)
	{
		Note n;
		n.row = row;
		n.col = col;
		n.type = type;
		n.endrow = endrow;
		n.player = player;
		n.quant = 0; // Default quantization

		NoteEdit edit;
		edit.add.append(n);
		gNotes->modify(edit, false);
	}
	return 0;
}

static int lua_deleteNote(lua_State* L)
{
	// Arguments: row, col
	int row = (int)luaL_checkinteger(L, 1);
	int col = (int)luaL_checkinteger(L, 2);

	if (gNotes)
	{
		const ExpandedNote* existing = gNotes->getNoteAt(row, col);
		if (existing)
		{
			NoteEdit edit;
			edit.rem.append(CompressNote(*existing));
			gNotes->modify(edit, false);
		}
	}
	return 0;
}

static int lua_clearChart(lua_State* L)
{
	if (gNotes && !gNotes->empty())
	{
		NoteEdit edit;
		for (const auto* it = gNotes->begin(); it != gNotes->end(); ++it)
		{
			edit.rem.append(CompressNote(*it));
		}
		gNotes->modify(edit, false);
	}
	return 0;
}

static int lua_getSelection(lua_State* L)
{
	if (gSelection)
	{
		SelectionRegion region = gSelection->getSelectedRegion();
		lua_newtable(L);
		lua_pushinteger(L, region.beginRow);
		lua_setfield(L, -2, "beginRow");
		lua_pushinteger(L, region.endRow);
		lua_setfield(L, -2, "endRow");
		return 1;
	}
	return 0;
}

static int lua_setSelection(lua_State* L)
{
	int start = (int)luaL_checkinteger(L, 1);
	int end = (int)luaL_checkinteger(L, 2);
	if (gSelection)
	{
		gSelection->selectRegion(start, end);
	}
	return 0;
}

static int lua_getSelectedNotes(lua_State* L)
{
	lua_newtable(L);
	if (gSelection)
	{
		NoteList list;
		gSelection->getSelectedNotes(list);
		
		int i = 1;
		for (const auto& note : list)
		{
			lua_newtable(L);
			
			lua_pushinteger(L, note.row);
			lua_setfield(L, -2, "row");
			
			lua_pushinteger(L, note.col);
			lua_setfield(L, -2, "col");
			
			lua_pushinteger(L, note.type);
			lua_setfield(L, -2, "type");
			
			lua_pushinteger(L, note.endrow);
			lua_setfield(L, -2, "endrow");
			
			lua_pushinteger(L, note.player);
			lua_setfield(L, -2, "player");

			lua_rawseti(L, -2, i++);
		}
	}
	return 1;
}
static int lua_getBpmAt(lua_State* L)
{
	int row = (int)luaL_checkinteger(L, 1);
	if (gTempo)
	{
		lua_pushnumber(L, gTempo->getBpm(row));
		return 1;
	}
	return 0;
}

static int lua_addBpm(lua_State* L)
{
	int row = (int)luaL_checkinteger(L, 1);
	double bpm = luaL_checknumber(L, 2);
	if (gTempo)
	{
		gTempo->addSegment(BpmChange(row, bpm));
	}
	return 0;
}

static int lua_addStop(lua_State* L)
{
	int row = (int)luaL_checkinteger(L, 1);
	double seconds = luaL_checknumber(L, 2);
	if (gTempo)
	{
		gTempo->addSegment(Stop(row, seconds));
	}
	return 0;
}

static int lua_getOffset(lua_State* L)
{
	if (gTempo)
	{
		lua_pushnumber(L, gTempo->getOffset());
		return 1;
	}
	return 0;
}

static int lua_setOffset(lua_State* L)
{
	double offset = luaL_checknumber(L, 1);
	if (gTempo)
	{
		gTempo->setOffset(offset);
	}
	return 0;
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

	lua_pushcfunction(L, lua_getNotes);
	lua_setfield(L, -2, "getNotes");

	lua_pushcfunction(L, lua_rowToTime);
	lua_setfield(L, -2, "rowToTime");

	lua_pushcfunction(L, lua_timeToRow);
	lua_setfield(L, -2, "timeToRow");

	lua_pushcfunction(L, lua_addNote);
	lua_setfield(L, -2, "addNote");

	lua_pushcfunction(L, lua_deleteNote);
	lua_setfield(L, -2, "deleteNote");

	lua_pushcfunction(L, lua_clearChart);
	lua_setfield(L, -2, "clearChart");

	lua_pushcfunction(L, lua_getSelection);
	lua_setfield(L, -2, "getSelection");

	lua_pushcfunction(L, lua_setSelection);
	lua_setfield(L, -2, "setSelection");

	lua_pushcfunction(L, lua_getSelectedNotes);
	lua_setfield(L, -2, "getSelectedNotes");

	lua_pushcfunction(L, lua_getBpmAt);
	lua_setfield(L, -2, "getBpmAt");

	lua_pushcfunction(L, lua_addBpm);
	lua_setfield(L, -2, "addBpm");

	lua_pushcfunction(L, lua_addStop);
	lua_setfield(L, -2, "addStop");

	lua_pushcfunction(L, lua_getOffset);
	lua_setfield(L, -2, "getOffset");

	lua_pushcfunction(L, lua_setOffset);
	lua_setfield(L, -2, "setOffset");

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
