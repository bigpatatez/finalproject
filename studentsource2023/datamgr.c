
#include <stdlib.h>
#include <stdio.h>
#include "datamgr.h"
#include "config.h"
#include <assert.h>
#include "lib/dplist.h"
#include "sbuffer.h"

#ifndef DATAMGR_H_
#define DATAMGR_H_
#endif

#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5
#endif


#ifndef SET_MAX_TEMP
#error SET_MAX_TEMP not set
#endif

#ifndef SET_MIN_TEMP
#error SET_MIN_TEMP not set
#endif

/*
 * Use ERROR_HANDLER() for handling memory allocation problems, invalid sensor IDs, non-existing files, etc.
 */
#define ERROR_HANDLER(condition, ...)    do {                       \
                      if (condition) {                              \
                        printf("\nError: in %s - function %s at line %d: %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
                        exit(EXIT_FAILURE);                         \
                      }                                             \
                    } while(0)



typedef struct {
    uint16_t sensor_id;
    uint16_t room_id;
    double running_avg;
    time_t last_modified;
    double values[RUN_AVG_LENGTH];
} sensor_element_t;

dplist_t * data;

void running_average_calculator(sensor_element_t *element, double temp, time_t timestamp);

void print_sensor_element_info(sensor_element_t *element);

void checkAverage(sensor_element_t * element);

void * element_copy(void * element) {
    //TODO
    sensor_element_t * copy = malloc(sizeof (sensor_element_t ));
    assert(copy != NULL);
    copy->sensor_id = ((sensor_element_t *)element)->sensor_id;
    copy->room_id = ((sensor_element_t *)element)->room_id;
    copy->running_avg = ((sensor_element_t *)element)->running_avg;
    copy->last_modified = ((sensor_element_t*)element)->last_modified;
    for(int i = 0 ; i<RUN_AVG_LENGTH;i++)
    {
        copy->values[i] = ((sensor_element_t *)element)->values[i];
    }
    return (void *) copy;
}

void element_free(void ** element) {
    //TODO
    free(*element);
    *element = NULL;
}

int element_compare(void * x, void * y) {
    //TODO
    return ((((sensor_element_t *)x)-> room_id <  ((sensor_element_t *)y)->room_id) ? -1 : (((sensor_element_t*)x)->room_id == ((sensor_element_t*)y)->room_id) ? 0 : 1);
}


void* datamgr_init(void* args)
{
    sbuffer_t * buffer = (sbuffer_t*) args;
    FILE * map = fopen("room_sensor.map","r");
    datamgr_parse_sensor_files(map,buffer);
    datamgr_free();
    printf("datamgr exiting\n");
    return NULL;
}

/**
 * Puts the sensor id and room id inside the shared linked list
 */

void populate_sensor_list(dplist_t* list, FILE* fp_sensor_map)
{
    // first read the data from the file
    // initialize the element for the node in the list
    // then insert the node at the end of the list

    if(fp_sensor_map != NULL)
    {
        uint16_t room_id  = 0;
        uint16_t sensor_id = 0;
        printf("file successfully opened\n");
        fflush(stdout);
        while(fscanf(fp_sensor_map,"%hd %hd",&room_id,&sensor_id) == 2)
        {
            sensor_element_t * el = malloc(sizeof(sensor_element_t));
            el->room_id = room_id;
            el->sensor_id = sensor_id;
            el->last_modified = (time_t) NULL;
            el->running_avg = 0;
            for(int i = 0 ; i<RUN_AVG_LENGTH;i++)
            {
                el->values[i] = 999;
            }
            dpl_insert_at_index(list,el,0,false);
            printf("room_id : %d\n",room_id);
            printf("sensor_id: %d\n",sensor_id);
        }
    }
}
/**
 *  This method holds the core functionality of your datamgr. It takes in 2 file pointers to the sensor files and parses them.
 *  When the method finishes all data should be in the internal pointer list and all log messages should be printed to stderr.
 *  \param fp_sensor_map file pointer to the map file
 *  \param fp_sensor_data file pointer to the binary data file
 */
void datamgr_parse_sensor_files(FILE *fp_sensor_map, sbuffer_t * buffer)
{
    data = dpl_create(element_copy,element_free,element_compare);
    populate_sensor_list(data,fp_sensor_map);
    //printf("room_id inserted in list : %hd\n", ((sensor_element_t*)dpl_get_element_at_index(data,0))->room_id);
    uint16_t sensor_id;
    double temp;
    time_t timestamp;
    sensor_data_t * sensorData = malloc(sizeof(sensor_data_t));

    while(sbuffer_remove(buffer,sensorData)!= SBUFFER_NO_DATA)
    {
        sensor_id = sensorData->id;
        temp = sensorData->value;
        timestamp = sensorData->ts;
        for(int i =0;i< dpl_size(data);i++)
        {
            sensor_element_t * element =(sensor_element_t *)dpl_get_element_at_index(data,i);
            //printf("looking for the sensor_id\n");
            if(element->sensor_id == sensor_id)
            {
                printf("from data manager: found the sensor\n");
                //printf("old info:\n");
                //fflush(stdout);
                //print_sensor_element_info(element);
                printf("new info:\n");
                fflush(stdout);
                // this function modifies the data inside the linked list and calculates the new running average
                running_average_calculator(element,temp,timestamp);
                print_sensor_element_info(element);
            }
        }
    }
    printf("no more data: datamanager\n");
    free(sensorData);
}


/**
 * Function printing the information contained in a sensor node
 * @param element
 */
void print_sensor_element_info(sensor_element_t *element)
{
    printf("sensor_id: %d\t",element->sensor_id);
    printf("room_id:%d \t",element->room_id);
    printf("running average:%f \t",element->running_avg);
    printf("last modified: %s",asctime(gmtime(&(element->last_modified))));
    printf("\t");

    printf("last %d temperature values:\n",RUN_AVG_LENGTH);
    for(int i = 0 ; i<RUN_AVG_LENGTH;i++)
    {
        printf("value %d: %f\t",i,element->values[i]);
    }
    printf("\n");
    fflush(stdout);
}

/**
 * This function calculates the new running average and updates the timestamp based on the received values from the sensor node
 * @param element element from the list that stores the information about the sensor node
 * @param temp new temperature data
 * @param timestamp timestamp linked to the new temperature data
 */

void running_average_calculator(sensor_element_t *element, double temp, time_t timestamp)
{
    double sum = 0;
    int zero_counter = 0;
    for(int i=RUN_AVG_LENGTH-1;i>0 ;i--)
    {
        element->values[i] = element->values[i-1];
        if(element-> values[i] == 999)
        {
            zero_counter ++;
        }
        else
        {
            sum += element->values[i];
        }

    }
    element->values[0] = temp;
    sum += temp;
    element->last_modified = timestamp;
    if(zero_counter>0){element->running_avg = sum/(RUN_AVG_LENGTH-zero_counter);}
    else{element->running_avg = sum/RUN_AVG_LENGTH;}
    //checkAverage(element);
}
/**
 * checks the running average and displays an error message if its higher than SET_MAX_TEMP or lower than SET_MIN_TEMP
 * @param element
 */
void checkAverage(sensor_element_t * element)
{
    // this function needs to pass a message to the logger --> so we need to pass the pipe to the thread when initializing
    if(element->running_avg >SET_MAX_TEMP)
    {
        fprintf(stderr,"Room %d is too hot\n",element->room_id);
    }
     if(element->running_avg < SET_MIN_TEMP)
    {
        fprintf(stderr,"Room %d is too cold\n",element->room_id);
    }
}


/**
 * This method should be called to clean up the datamgr, and to free all used memory.
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors will not return a valid result
 */
void datamgr_free()
{
    dpl_free(&data,true);
}

/**
 * Gets the room ID for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the corresponding room id
 */
uint16_t datamgr_get_room_id(sensor_id_t sensor_id)
{
    for(int i = 0; i< dpl_size(data);i++)
    {
        sensor_element_t * el = dpl_get_element_at_index(data,i);
        if((el->sensor_id) == sensor_id)
        {
            return el->room_id;
        }
    }
    fprintf(stderr, "Given sensor id is invalid");
    exit(EXIT_FAILURE);
}

/**
 * Gets the running AVG of a certain senor ID (if less then RUN_AVG_LENGTH measurements are recorded the avg is 0)
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the running AVG of the given sensor
 */
sensor_value_t datamgr_get_avg(sensor_id_t sensor_id)
{
    for(int i = 0; i< dpl_size(data);i++)
    {
        sensor_element_t * el = dpl_get_element_at_index(data,i);
        if((el->sensor_id) == sensor_id)
        {
            return el->running_avg;
        }
    }
    fprintf(stderr, "Given sensor id is invalid");
    exit(EXIT_FAILURE);
}

/**
 * Returns the time of the last reading for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the last modified timestamp for the given sensor
 */
time_t datamgr_get_last_modified(sensor_id_t sensor_id)
{
    for(int i = 0; i< dpl_size(data);i++)
    {
        sensor_element_t * el = dpl_get_element_at_index(data,i);
        if((el->sensor_id) == sensor_id)
        {
            return el->last_modified;
        }
    }
    fprintf(stderr, "Given sensor id is invalid");
    exit(EXIT_FAILURE);
}

/**
 *  Return the total amount of unique sensor ID's recorded by the datamgr
 *  \return the total amount of sensors
 */
int datamgr_get_total_sensors()
{
    int count = 0;
    for(int i =0;i< dpl_size(data);i++)
    {
        int same_id = 0;
        uint16_t sensor_id = ((sensor_element_t*)dpl_get_element_at_index(data,i))->sensor_id;
        for(int j = i+1; j< dpl_size(data);j++)
        {
            if(((sensor_element_t*) dpl_get_element_at_index(data,j))->sensor_id == sensor_id)
            {
                same_id ++;
            }
        }
        if(same_id ==0 )
        {
            count++;
        }
    }
    return count;
}
