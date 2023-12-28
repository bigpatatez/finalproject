#include <stdio.h>
#include "config.h"
#include <stdlib.h>
#include "sbuffer.h"
#include "sensor_db.h"


void * storagemgr_init(void * args)
{
    sensor_data_t * d = malloc(sizeof(sensor_data_t));
    sbuffer_t * b = (sbuffer_t *)args;
    FILE * file = fopen("data.csv","w");
    if(file != NULL)
    {
        write_to_log_process("A new 'data.csv' file has been created");
        while(sbuffer_peek(b,d) != SBUFFER_NO_DATA)
        {
            printf("storage got: %d, %f, %ld",d->id,d->value,d->ts);
            int success = fprintf(file,"%d, %f, %ld\n",d->id,d->value,d->ts);
            fflush(file);
            if(success <0)
            {
                printf("Data insertion failed");
                char string[500];
                snprintf(string,sizeof(string),"Data insertion from sensor %d failed",d->id);
                write_to_log_process(string);
            }
            else
            {
                printf("Data inserted");
                char string[500];
                snprintf(string,sizeof(string),"Data insertion from sensor %d succeeded",d->id);
                write_to_log_process(string);
            }
        }
    }
    if(fclose(file)== 0) write_to_log_process("The 'data.csv' file has been closed");
    free(d);
    return STORAGE_MANAGER_SUCCESS;
}

