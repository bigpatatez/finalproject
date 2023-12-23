#include "logger.h"
#include <stdio.h>
#include "config.h"
#include <stdbool.h>
#include <unistd.h>
#include <wait.h>
#include "sbuffer.h"

int pid ;

void * storagemgr_init(void * args)
{
    sensor_data_t * d = malloc(sizeof(sensor_data_t));
    sbuffer_t * b = (sbuffer_t *)args;
    FILE * file = fopen("data.csv","w");
    while(sbuffer_remove(b,d,1) != SBUFFER_NO_DATA)
    {
        printf("storage got: %d, %f, %ld",d->id,d->value,d->ts);
        int success = fprintf(file,"%d, %f, %ld\n",d->id,d->value,d->ts);
        fflush(file);
        if(success <0)
        {
            printf("Data insertion failed");
        }
        else
        {
            printf("Data inserted");
        }
    }
    fclose(file);
    free(d);
    return NULL;
}

FILE * open_db(char * filename, bool append)
{

    /*// first create a log process
    int s = create_log_process();
    if(s==0) // pipe success and the log file is open
    {
        pid = fork();
        if(pid<0)
        {
            printf("Forking failed\n");
            return NULL;
        }
        else if(pid>0)
        {
            FILE *file = fopen(filename, "r");
            if (file == NULL) {
                printf("the file does not exist, a new file with this name will be created\n");
                file = fopen(filename, "w");
                if (file != NULL)
                {
                    write_to_log_process("Data file created and opened");
                    return file;
                }

            }
            else {

                if (append == true) {
                    fclose(file);
                    file = fopen(filename, "a");
                    //wait(NULL);
                    if (file != NULL)
                    {
                        write_to_log_process("Data file opened");
                        return file;
                    }

                } else {
                    fclose(file);
                    file = fopen(filename, "w");
                    if (file != NULL)
                    {
                        write_to_log_process("Data file opened");
                        return file;
                    }
                }
            }
        }
        else
        {
            write_to_log_process("child");
        }
    }*/
    return NULL;
}
int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts)
{
    if(pid >0)
    {
        int success = fprintf(f,"%d, %f, %ld\n",id,value,ts);
        if(success <0)
        {
            write_to_log_process("Data insertion failed");
        }
        else
        {
            write_to_log_process("Data inserted");
        }
        return  success;
    }
    else
    {
        write_to_log_process("child");
    }
    return 0;
}
int close_db(FILE * f)
{
    if(pid >0)
    {
        int a = fclose(f);
        if(a == 0)
        {
            write_to_log_process("Data file is closed");
            wait(NULL);
            return 0;
        }
        else
        {
            printf("%d",a);
            fflush(stdout);
            write_to_log_process("Data file couldn't be closed correctly");
            wait(NULL);
            return -1;
        }

    }
    else
    {
        write_to_log_process("child");
        end_log_process();
    }

    return 0;
}