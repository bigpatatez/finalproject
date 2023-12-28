/**
 * \author {AUTHOR}
 */

#ifndef _SBUFFER_H_
#define _SBUFFER_H_
#include <unistd.h>
#include "config.h"

#define SBUFFER_FAILURE -1
#define SBUFFER_SUCCESS 0
#define SBUFFER_NO_DATA 1

typedef struct sbuffer sbuffer_t;
typedef struct sbuffer_node sbuffer_node_t;
/**
 * Allocates and initializes a new shared buffer
 * \param buffer a double pointer to the buffer that needs to be initialized
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_init(sbuffer_t **buffer);

/**
 * All allocated resources are freed and cleaned up
 * \param buffer a double pointer to the buffer that needs to be freed
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_free(sbuffer_t **buffer);

/**
 * Removes the first sensor data in 'buffer' (at the 'head') and returns this sensor data as '*data'
 * If 'buffer' is empty, the function blocks until new sensor data becomes available
 * returns SBUFFER_NO_DATA when the end of the transmission is detected
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to pre-allocated sensor_data_t space, the data will be copied into this structure. No new memory is allocated for 'data' in this function.
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data);

/**
 * Inserts the sensor data in 'data' at the end of 'buffer' (at the 'tail')
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to sensor_data_t data, that will be copied into the buffer
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
*/
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data);
/**
 * Peeks at the sensor data at the 'head' of 'buffer' and signals the remove and peek to indicate that it has read the data
 * when a certain node has been peeked, the r_data flag in the sbuffer_node struct is set
 * has a blocking wait if there is no data available or if the data has already been read
 * \param data a pointer to pre-allocated sensor_data_t space, the data will be copied into this structure. No new memory is allocated for 'data' in this function.
 * \param buffer a pointer to the buffer that is used
 * \return SBUFFER_SUCCESS when succesfully reads and SBUFFER_NO_DATA when end of transmission is detected
 * */
int sbuffer_peek(sbuffer_t *buffer, sensor_data_t* data);

/**
 * Peeks and removes the sensor data at the 'head' of 'buffer'
 * waits until new data becomes available or for the peek function to set the r_data flag then proceeds
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to pre-allocated sensor_data_t space, the data will be copied into this structure. No new memory is allocated for 'data' in this function.
 * \return SBUFFER_FAILURE when there is an error, SBUFFER_NO_DATA when the end of transmission is detected and SBUFFER_SUCCESS when the operation is successful
 */
int sbuffer_remove_and_peek(sbuffer_t* buffer, sensor_data_t* data);

#endif  //_SBUFFER_H_
