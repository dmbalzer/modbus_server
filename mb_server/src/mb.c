#include "mb.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#if defined(_WIN32)
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#endif

#include <modbus/modbus.h>

#define NB_CONNECTION 5

extern pthread_mutex_t mutex;
extern modbus_mapping_t* mb_mapping; /* main.c */
extern char* ip; /* main.c */
extern int port; /* main.c */
extern int nb_bits; /* main.c */
extern int nb_input_bits; /* main.c */
extern int nb_registers; /* main.c */
extern int nb_input_registers; /* main.c */

static modbus_t* ctx = NULL;
static int server_socket = -1;
static uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
static int master_socket;
static int rc;
static fd_set refset;
static fd_set rdset;
static int fdmax;

static void close_sigint(int dummy)
{
    if (server_socket != -1) {
        close(server_socket);
    }
    modbus_free(ctx);

    exit(dummy);
}

int mb_init(void)
{
    ctx = modbus_new_tcp(ip, port);

    server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);
    if (server_socket == -1) {
        fprintf(stderr, "Unable to listen TCP connection\n");
        modbus_free(ctx);
        return -1;
    }

    signal(SIGINT, close_sigint);

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = server_socket;
	return 0;
}

static void* _mb_run(void* arg)
{
    for (;;) {
        rdset = refset;
        if (select(fdmax + 1, &rdset, NULL, NULL, NULL) == -1) {
            perror("Server select() failure.");
            close_sigint(1);
        }

        /* Run through the existing connections looking for data to be
         * read */
        for (master_socket = 0; master_socket <= fdmax; master_socket++) {

            if (!FD_ISSET(master_socket, &rdset)) {
                continue;
            }

            if (master_socket == server_socket) {
                /* A client is asking a new connection */
                socklen_t addrlen;
                struct sockaddr_in clientaddr;
                int newfd;

                /* Handle new connections */
                addrlen = sizeof(clientaddr);
                memset(&clientaddr, 0, sizeof(clientaddr));
                newfd = accept(server_socket, (struct sockaddr *) &clientaddr, &addrlen);
                if (newfd == -1) {
                    perror("Server accept() error");
                } else {
                    FD_SET(newfd, &refset);

                    if (newfd > fdmax) {
                        /* Keep track of the maximum */
                        fdmax = newfd;
                    }
                    printf("New connection from %s:%d on socket %d\n",
                           inet_ntoa(clientaddr.sin_addr),
                           clientaddr.sin_port,
                           newfd);
                }
            } else {
                modbus_set_socket(ctx, master_socket);
                rc = modbus_receive(ctx, query);
                if (rc > 0) {
					pthread_mutex_lock(&mutex);
                    modbus_reply(ctx, query, rc, mb_mapping);
					pthread_mutex_unlock(&mutex);
                } else if (rc == -1) {
                    /* This example server in ended on connection closing or
                     * any errors. */
                    printf("Connection closed on socket %d\n", master_socket);
                    close(master_socket);

                    /* Remove from reference set */
                    FD_CLR(master_socket, &refset);

                    if (master_socket == fdmax) {
                        fdmax--;
                    }
                }
            }
        }
    }

    return NULL;
}

int mb_run(void)
{
	pthread_t t;
	pthread_create(&t, NULL, _mb_run, NULL);
	return 0;
}
