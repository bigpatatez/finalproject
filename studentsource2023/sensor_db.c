#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "sensor_db.h"
#include <stdbool.h>
#include <unistd.h>
#include "logger.h"
#include <string.h>
#include <sys/wait.h>

char send[30];
int *fd;

void set_fd(int pipe[2])
{
    fd = pipe;
}
void sendMessage()
 {
     printf("sending message");
     fflush(stdout);
     //strncpy(send,message,sizeof(send));
     write(fd[1], send, sizeof(send));
     printf("sent message");
     fflush(stdout);
 }

FILE * open_db(char * filename, bool append)
{

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("the file does not exist, a new file with this name will be created");
        strncpy(send, "Data file created and opened", sizeof(send));
        sendMessage();
        return fopen(filename, "w");;
    }
    else {
        strncpy(send, "Data file opened", sizeof(send));
        sendMessage();
        if (append == true) {
            fclose(file);
            return fopen(filename, "a");
        } else {
            fclose(file);
            return fopen(filename, "w");
        }
    }
}
int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts)
{
    int success = fprintf(f,"%d, %f, %ld\n",id,value,ts);
    if(success <0)
    {
        strncpy(send,"Data insertion failed", sizeof(send));
        sendMessage();
    }
    else
    {
        strncpy(send,"Data inserted", sizeof(send));
        sendMessage();
    }
    return  success;
}

int close_db(FILE * f)
{
    strncpy(send,"Data file is closed", sizeof(send));
    sendMessage();
    return fclose(f);
}
