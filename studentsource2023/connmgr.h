#ifndef _CONMGR_H_
#define _CONMGR_H_

#ifndef TIMEOUT
#define TIMEOUT 5
#endif

#include "config.h"
#include "sbuffer.h"

#define CONMGR_SUCCESS 0
/**
 * Connection manager routine
 * Manages the incoming tcp connections
 * Shuts down when the number of connections is equal to MAX_CONN
 * inserts a dummy node to the buffer when all clients disconnect to signal the end of transmission
 * @param args a pointer to mainargs struct which contains the port number, the number of maximum connections and a pointer to the shared buffer
 * @return CONMGR_SUCCESS
 */
void* conmgr_init(void* args) ;

/**
 * Routine for client tcp connections
 * processes the incoming data from the client tcp connections, inserts the data into the shared data structure and send messages to the log process
 * closes the tcp connection when the client disconnects
 * @param param a pointer to tcpsock_t struct -> the tcp socket for the client
 * @return CONMGR_SUCCESS
 */
void * conmgr_routine(void * param);


#endif  //_CONMGR_H_