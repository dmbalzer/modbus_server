#include "lua_mb_bits.h"
#include <lua5.1/lua.h>
#include <lua5.1/lualib.h>
#include <lua5.1/lauxlib.h>
#include <modbus/modbus.h>

extern modbus_mapping_t* mb_mapping; /* main.c */
extern int nb_bits; /* main.c */
extern lua_State* L; /* lua_mb.c */

static int _get(lua_State* L)
{
	uint8_t* ud = (uint8_t*)luaL_checkudata(L, 1, "mt_bits");
	int idx = luaL_checkint(L, 2);
	luaL_argcheck(L, ud != NULL, 1, "bits expected\n");
	luaL_argcheck(L, idx > 0 && idx <= nb_bits, 2, "index out of range.\n");
	lua_pushboolean(L, (int)ud[idx - 1]);
	return 1;	
}

static int _set(lua_State* L)
{
	uint8_t* ud = (uint8_t*)luaL_checkudata(L, 1, "mt_bits");
	int idx = luaL_checkint(L, 2);
	int v = lua_toboolean(L, 3);
	luaL_argcheck(L, ud != NULL, 1, "bits expected\n");
	luaL_argcheck(L, idx > 0 && idx <= nb_bits, 2, "index out of range.\n");
	luaL_argcheck(L, lua_isboolean(L, 3), 3, "boolean expected.\n");
	ud[idx - 1] = (uint8_t)v;
	return 0;
}

static int _len(lua_State* L)
{
	lua_pushinteger(L, nb_bits);
	return 1;
}

static const luaL_Reg _reg[] = {
	{"__index", _get},
	{"__newindex", _set},
	{"__len", _len},
	{NULL, NULL},
};

void lua_mb_bits_init(void)
{
	mb_mapping->tab_bits = (uint8_t*)lua_newuserdata(L, nb_bits * sizeof(uint8_t));
	for ( int i = 0; i < nb_bits; i++ ) mb_mapping->tab_bits[i] = 0;
	mb_mapping->nb_bits = nb_bits;
	mb_mapping->start_bits = 0;

	luaL_newmetatable(L, "mt_bits");
	luaL_register(L, NULL, _reg);
	lua_setmetatable(L, -2);
	
	lua_setglobal(L, "bits");
	lua_settop(L, 0);
}
