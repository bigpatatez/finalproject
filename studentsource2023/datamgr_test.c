#include <unistd.h>
#include "datamgr.h"
#include "pthread.h"
#include "sbuffer.h"
sbuffer_t * buffer;

void * dummy_connection_manager(void * args)
{

    double value = 15;
    sensor_data_t * data = malloc(sizeof(sensor_data_t));
    for(int i= 0; i<10;i++)
    {
        data->id = 15;
        time(&data->ts);
        value += 0.1;
        data->value = value;
        sbuffer_insert(buffer,data);
        sleep(1);
    }
    data->id = 0;
    sbuffer_insert(buffer,data);
    free(data);
    return NULL;
}
int main()
{
    sbuffer_init(&buffer);
    pthread_t dummy_conn;
    pthread_t datamgr;
    pthread_create(&dummy_conn,NULL,&dummy_connection_manager,NULL);
    pthread_create(&datamgr,NULL,&datamgr_init,(void*)buffer);

    pthread_join(dummy_conn,NULL);
    pthread_join(datamgr,NULL);
    sbuffer_free(&buffer);
    return 0;
}
