#ifndef _CONMGR_H_
#define _CONMGR_H_

#ifndef TIMEOUT
#define TIMEOUT 5
#endif

#include "config.h"
#include "sbuffer.h"


int conmgr_init(int argc, char *argv[], sbuffer_t* buffer) ;
void * conmgr_routine(void * param);


#endif  //_CONMGR_H_