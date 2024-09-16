#include "lua_mb.h"
#include "lua_mb_bits.h"
#include "lua_mb_input_bits.h"
#include "lua_mb_registers.h"
#include "lua_mb_input_registers.h"
#include <lua5.1/lua.h>
#include <lua5.1/lualib.h>
#include <lua5.1/lauxlib.h>
#include <modbus/modbus.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

extern pthread_mutex_t mutex;
extern modbus_mapping_t* mb_mapping; /* main.c */
extern char* ip; /* main.c */
extern int port; /* main.c */
extern int nb_bits; /* main.c */
extern int nb_input_bits; /* main.c */
extern int nb_registers; /* main.c */
extern int nb_input_registers; /* main.c */

lua_State* L;

static void _init_mapping_config(void)
{
	L = luaL_newstate();
	luaL_openlibs(L);
	
	if ( luaL_dofile(L, "./scripts/config.lua") != 0 )
	{
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		return;
	}
	
	lua_getglobal(L, "ip");
	lua_getglobal(L, "port");
	lua_getglobal(L, "nb_bits");
	lua_getglobal(L, "nb_input_bits");
	lua_getglobal(L, "nb_registers");
	lua_getglobal(L, "nb_input_registers");
	
	if ( !lua_isnil(L, 1) && lua_isstring(L, 1) )
	{
		ip = (char*)calloc(80, sizeof(char));
		strncpy(ip, lua_tostring(L, 1), 80);
		fprintf(stderr, "Server IP address set to: %s\n", ip);
	}

	if ( !lua_isnil(L, 2) && lua_isnumber(L, 2) )
	{
		port = lua_tointeger(L, 2);
		fprintf(stderr, "Port set to: %d\n", port);
	}

	if ( !lua_isnil(L, 3) && lua_isnumber(L, 3) )
	{
		nb_bits = lua_tointeger(L, 3);
		fprintf(stderr, "# of Bits set to: %d\n", nb_bits);
	}

	if ( !lua_isnil(L, 4) && lua_isnumber(L, 4) )
	{
		nb_input_bits = lua_tointeger(L, 4);
		fprintf(stderr, "# of Input Bits set to: %d\n", nb_input_bits);
	}

	if ( !lua_isnil(L, 5) && lua_isnumber(L, 5) )
	{
		nb_registers = lua_tointeger(L, 5);
		fprintf(stderr, "# of Registers set to: %d\n", nb_registers);
	}

	if ( !lua_isnil(L, 6) && lua_isnumber(L, 6) )
	{
		nb_input_registers = lua_tointeger(L, 6);
		fprintf(stderr, "# of Input Registers set to: %d\n", nb_input_registers);
	}

	lua_close(L);
}

static void _init_mapping(void)
{
	L = luaL_newstate();
	luaL_openlibs(L);

	mb_mapping = (modbus_mapping_t*)lua_newuserdata(L, sizeof(modbus_mapping_t));	
	lua_settop(L, 0);

	lua_mb_bits_init();
	lua_mb_input_bits_init();
	lua_mb_registers_init();
	lua_mb_input_registers_init();
}

static void clean(void)
{
	if ( ip != NULL ) free(ip);
	lua_close(L);
}

int lua_mb_init(void)
{
	atexit(clean);

	_init_mapping_config();
	_init_mapping();
	
	return 0;
}

static void* _lua_mb_run(void* arg)
{
	for (;;)
	{
		pthread_mutex_lock(&mutex);
		if ( luaL_dofile(L, "./scripts/script.lua") != 0 )
		{
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
		}
		pthread_mutex_unlock(&mutex);

		lua_settop(L, 0);
		sleep(5);
	}
	return NULL;
}

int lua_mb_run(void)
{
	pthread_t t;
	pthread_create(&t, NULL, _lua_mb_run, NULL);
	return 0;	
}
