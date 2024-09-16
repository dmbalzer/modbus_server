#include "lua_mb_input_registers.h"
#include <lua5.1/lua.h>
#include <lua5.1/lualib.h>
#include <lua5.1/lauxlib.h>
#include <modbus/modbus.h>

extern modbus_mapping_t* mb_mapping; /* main.c */
extern int nb_input_registers; /* main.c */
extern lua_State* L; /* lua_mb.c */

static int _get(lua_State* L)
{
	uint16_t* ud = (uint16_t*)luaL_checkudata(L, 1, "mt_input_registers");
	int idx = luaL_checkint(L, 2);
	luaL_argcheck(L, ud != NULL, 1, "input registers expected\n");
	luaL_argcheck(L, idx > 0 && idx <= nb_input_registers, 2, "index out of range.\n");
	lua_pushinteger(L, (int16_t)ud[idx - 1]);
	return 1;	
}

static int _set(lua_State* L)
{
	uint16_t* ud = (uint16_t*)luaL_checkudata(L, 1, "mt_input_registers");
	int idx = luaL_checkint(L, 2);
	int v = luaL_checkint(L, 3);
	luaL_argcheck(L, ud != NULL, 1, "input registers expected\n");
	luaL_argcheck(L, idx > 0 && idx <= nb_input_registers, 2, "index out of range.\n");
	ud[idx - 1] = (uint16_t)v;
	return 0;
}

static int _len(lua_State* L)
{
	lua_pushinteger(L, nb_input_registers);
	return 1;
}

static const luaL_Reg _reg[] = {
	{"__index", _get},
	{"__newindex", _set},
	{"__len", _len},
	{NULL, NULL},
};

void lua_mb_input_registers_init(void)
{
	mb_mapping->tab_input_registers = (uint16_t*)lua_newuserdata(L, nb_input_registers * sizeof(uint16_t));
	for ( int i = 0; i < nb_input_registers; i++ ) mb_mapping->tab_input_registers[i] = 0;
	mb_mapping->nb_input_registers = nb_input_registers;
	mb_mapping->start_input_registers = 0;
	
	luaL_newmetatable(L, "mt_input_registers");
	luaL_register(L, NULL, _reg);
	lua_setmetatable(L, -2);

	lua_setglobal(L, "input_registers");
	lua_settop(L, 0);
}
