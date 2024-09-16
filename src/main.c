#include "mb.h"
#include "lua_mb.h"

#include <modbus/modbus.h>
#include <stdio.h>
#include <gpiod.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex;
modbus_mapping_t *mb_mapping = NULL;
char* ip = NULL;
int port = 1502;
int nb_bits = 10;
int nb_input_bits = 10;
int nb_registers = 10;
int nb_input_registers = 10;

static void clean(void)
{
	exit(EXIT_SUCCESS);
}

int main(void)
{
	atexit(clean);

	pthread_mutex_init(&mutex, NULL);

	lua_mb_init();
	lua_mb_run();

	mb_init();
	mb_run();
	
	for (;;){}
	
	return 0;
}
