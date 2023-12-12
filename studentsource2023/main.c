#include "sbuffer.h"
#include "connmgr.h"
sbuffer_t * buffer;
int main(int argc, char *argv[])
{
    sbuffer_init(&buffer);
    conmgr_init(argc,argv,buffer);
    sbuffer_free(&buffer);
}
