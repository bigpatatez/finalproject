#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <wait.h>


int fd[2];
char send[30];
char received[30];
int seq;
FILE * f;
int write_to_log_process(char *msg)
{
    if(strcmp(msg,"child") == 0)
    {
        close(fd[1]);
        read(fd[0],received,sizeof(received));
        char * timestamp;

        time_t ltime;
        ltime = time(NULL);
        timestamp = asctime(localtime(&ltime));
        timestamp[strlen(timestamp)-1] = '\0';

        char buffer[100];
        snprintf(buffer,sizeof(buffer),"%d - %s - %s\n",seq,timestamp,received);
        printf("\nwriting to file: %s",buffer);
        fflush(stdout);
        fprintf(f, "%s",buffer);
        printf("wrote to file\n");
        fflush(stdout);
        seq++;
        return 2;
    }
    else
    {
        close(fd[0]);
        strncpy(send,msg,sizeof(send));
        printf("sending message\n");
        write(fd[1],send,sizeof(send));
        printf("sent message\n");
        if(strcmp(msg,"Data file is closed") == 0)
        {
            wait(NULL);
            close(fd[1]);
        }
        return 1;
    }
}

int create_log_process()
{
    int p = pipe(fd);
    if(p<0)
    {
        printf("Pipe failed\n");
        return -1;
    }
    else
    {
        printf("Pipe creation succeeded\n");
        f = fopen("gateway.log","a");
        if(f != NULL)
        {
            printf("log file is opened\n");
            return 0;
        }
        else {return -2;}

    }
}
int end_log_process()
{
    printf("ending log process\n");
    close(fd[0]);
    printf("closed pipe\n");
    fclose(f);
    printf("closed the log file\n");
    return -1;
}