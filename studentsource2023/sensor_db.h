/**
 * \author Bert Lagaisse
 */

#ifndef _SENSOR_DB_H_
#define _SENSOR_DB_H_

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <stdbool.h>

#define STORAGE_MANAGER_SUCCESS NULL

/**
 * Thread routine for the storage manager
 * reads data from the sbuffer and logs the data into 'data.csv' as comma separated values
 * Sends log messages to the logger process in case of successful file opening,data insertion and the closing of the file
 * ends when the end of transmission is detected from sbuffer
 * closes the csv file and cleans up its resources
 * \param args a pointer to the shared buffer
 * \return STORAGE_MANAGER_SUCCESS on successful operation
 */
void * storagemgr_init(void * args);



#endif /* _SENSOR_DB_H_ */