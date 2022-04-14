#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

#define MYUDP_PORT 5350
#define DATASIZE 2000
#define BATCHSIZE 4
#define PACKETSIZE DATASIZE * BATCHSIZE
#define BUFSIZE 60000
#define TIMEOUTSEC 1

// data packet structure
struct pack_so
{
    uint32_t num; // the sequence number
    uint32_t len; // the packet length
    char data[PACKETSIZE]; // the packet data
};

struct ack_so
{
    uint8_t num; // the sequence number
    uint32_t len; // the packet length
};