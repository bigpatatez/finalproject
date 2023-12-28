/**
 * \author {AUTHOR}
 */


#ifndef DATAMGR_H_
#define DATAMGR_H_
//



#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "sbuffer.h"

#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5
#endif

#ifndef SET_MAX_TEMP
#error SET_MAX_TEMP not set
#endif



#ifndef SET_MIN_TEMP
#error SET_MIN_TEMP not set
#endif


typedef struct sensor_node sensor_node_t;

/*
 * Use ERROR_HANDLER() for handling memory allocation problems, invalid sensor IDs, non-existing files, etc.
 */
#define ERROR_HANDLER(condition, ...)    do {                       \
                      if (condition) {                              \
                        printf("\nError: in %s - function %s at line %d: %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
                        exit(EXIT_FAILURE);                         \
                      }                                             \
                    } while(0)
/**
 * Thread routine for Data Manager
 * Initializes the datamanager by opening the room_sensor.map file and calling the datamgr_parse_sensor_files
 * closes the mapping file and calls datamgr_free() at the end of the program
 * @param args a pointer to the shared buffer
 * @return NULL
 */
void* datamgr_init(void* args);

/**
 *  This method holds the core functionality of your datamgr. It takes in 1 file pointer to the sensor-room mapping and a reference to the shared buffer
 *  When the method finishes all data should be in the internal pointer list and all log messages should be sent to the logger
 *  \param fp_sensor_map file pointer to the map file
 *  \param buffer pointer to the shared data structure
 */
void datamgr_parse_sensor_files(FILE *fp_sensor_map, sbuffer_t* buffer);

/**
 * This method should be called to clean up the datamgr, and to free all used memory. 
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors will not return a valid result
 */
void datamgr_free();


/**
 * Gets the room ID for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the corresponding room id
 */
uint16_t datamgr_get_room_id(sensor_id_t sensor_id);

/**
 * Gets the running AVG of a certain senor ID (if less then RUN_AVG_LENGTH measurements are recorded the avg is 0)
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the running AVG of the given sensor
 */
sensor_value_t datamgr_get_avg(sensor_id_t sensor_id);

/**
 * Returns the time of the last reading for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the last modified timestamp for the given sensor
 */
time_t datamgr_get_last_modified(sensor_id_t sensor_id);

/**
 *  Return the total amount of unique sensor ID's recorded by the datamgr
 *  \return the total amount of sensors
 */
int datamgr_get_total_sensors();

#endif  //DATAMGR_H_
