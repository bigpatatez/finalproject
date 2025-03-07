/**
 * \author {AUTHOR}
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <time.h>

typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;// UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine



typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;
} sensor_data_t;

/**
 * Sends a log message to the log process through the pipe set up in the main function
 * Appends a sequence number and a timestamp to the message to be sent
 * @param msg the message to be sent to the logger
 * @return 1
 */
int write_to_log_process(char *msg);


#endif /* _CONFIG_H_ */
