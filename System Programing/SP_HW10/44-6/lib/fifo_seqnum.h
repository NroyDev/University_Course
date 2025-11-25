#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"
#define SERVER_FIFO "./tmp/seqnum_sv"
/* Well-known name for server's FIFO */
#define CLIENT_FIFO_TEMPLATE "./tmp/seqnum_cl.%ld"
/* Template for building client FIFO name */
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)
/* Space required for client FIFO pathname
(+20 as a generous allowance for the PID) */
struct request {
    pid_t pid;
    int seqLen;
};
/* Request (client --> server) */
/* PID of client */
/* Length of desired sequence */
struct response {
    int seqNum;
};
