/*
 * Crashes pmcd on IRIX. Linux seems to be OK. PV 935490.
 */
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <pcp/pmapi.h>
#include <pcp/impl.h>

static __pmPDUHdr hdr;
static char *target;

void
try(int len)
{
    int fd;
    int sts;
    static int first = 1;
    static struct sockaddr_in  myAddr;
    static struct hostent*     servInfo;

    if (first) {
	first = 0;
	if ((servInfo = gethostbyname(target)) == NULL) {
	    fprintf(stderr, "host \"%s\" unknown\n", target);
	    exit(1);
	}
	memset(&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	memcpy(&myAddr.sin_addr, servInfo->h_addr, servInfo->h_length);
	myAddr.sin_port = htons(SERVER_PORT);
    }

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	fprintf(stderr, "socket failed: %s\n", pmErrStr(errno));
	return;
    }

    if ((sts = connect(fd, (struct sockaddr*) &myAddr, sizeof(myAddr))) < 0) {
	fprintf(stderr, "connect failed: %s\n", pmErrStr(sts));
	close(fd);
	return;
    }

    if ((sts = write(fd, &hdr, len)) != len) {
	fprintf(stderr, "write failed: %s\n", pmErrStr(sts));
	close(fd);
	return;
    }
    close(fd);
}


main(int argc, char *argv[])
{
    int j;
    int k;

    target = argc == 2 ? argv[1] : "localhost";

    hdr.from = htonl(12345);

    for (k = -1; k <= 12; k++) {
	hdr.len = htonl(k);
	hdr.type = htonl(0x55aa0000);
	for (j = 0; j <= 12; j++) {
	    try(j);
	}
    }

    for (k = 0; k <= 12; k++) {
	hdr.len = htonl(k<<24);
	hdr.type = htonl(0x000055aa);
	for (j = 0; j <= 12; j++) {
	    try(j);
	}
    }


    exit(0);
}
