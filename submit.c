/*
 * Queue a list of files to be sent via UDT. The "sender" program must be
 *	running on the localhost; the "receiver" program must be running on
 *	the receiving host.
 * Usage submit named-pipe-to-talk-to-sender receiving-host file1...
 */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/param.h>
#include <string.h>
#include <unistd.h>

#include "copy.h"

int
main(int argc, const char **argv)
{
	int fifo;
	const char *remote;

	if(argc < 4) {
		fputs("Arg count\n", stderr);
		return 1;
	}

	fifo = open(*++argv, O_WRONLY);

	if(fifo < 0) {
		perror(*argv);
		return errno;
	}
	--argc;

	remote = *++argv;
	--argc;

	while(--argc) {
		struct queue q;

		strcpy(q.q_filename, *++argv);
		strcpy(q.q_hostname, remote);

		if(write(fifo, &q, sizeof(struct queue)) != sizeof(struct queue))
			perror("write");
	}

	return close(fifo);
}
