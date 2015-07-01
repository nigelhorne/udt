/*
 * This end is the sender. It opens a named pipe from which it receives
 *	full paths of files to be sent. The named pipe is given as an argument
 * to this program.
 */
#include <iostream>
#include <string>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <memory.h>
#include <netdb.h>
#include <udt/udt.h>
#include <endian.h>

using namespace std;

#include "copy.h"

#ifndef	htonll
extern "C" {
static	uint64_t	htonll(uint64_t n);
// static	uint64_t	ntohll(uint64_t n);
}
#endif

int
main(int argc, char **argv)
{
	if(argc != 2) {
		fputs("Arg count\n", stderr);
		return 1;
	}

	/*
	 * Open the named pipe which receives requests
	 */
	int fin = open(argv[1], O_RDONLY);

	if(fin < 0) {
		perror(argv[1]);
		return errno;
	}

#if	0
	switch(fork()) {
		case -1:
			perror("fork");
			return errno;
		case 0:
			break;
		default:
			return 0;
	}
#endif

	struct queue q;

	const struct protoent *proto = getprotobyname("tcp");
	const struct servent *service = getservbyname("udt-copy", "tcp");

	int nbytes;

	while((nbytes = read(fin, &q, sizeof(struct queue))) >= 0) {
		if(nbytes == 0) {
			close(fin);
			fin = open(argv[1], O_RDONLY);
			if(fin < 0) {
				perror(argv[1]);
				return errno;
			}
			continue;
		}

		char *ptr = strrchr(q.q_filename, '/');

		if(ptr == NULL) {
			cerr << argv[0] << ": filenames must be absolute " <<
				q.q_filename << endl;
			continue;
		}

		struct stat statb;

		if(stat(q.q_filename, &statb) < 0) {
			perror(q.q_filename);
			continue;
		}

		/*
		 * security risk, race condition:
		 * The file could now be replaced, but since
		 * UDT needs C++, and ifstream doesn't allow us to get to the
		 * file descriptor, the use of fstat() is not possible :-(
		 */
		fstream ifs(q.q_filename, ios::in|ifstream::binary);

		if((ifs == NULL) || ifs.fail()) {
			perror(q.q_filename);
			continue;
		}

		sockaddr_in sin;
		memset(&sin, '\0', sizeof(struct sockaddr_in));
		sin.sin_family = AF_INET;
		sin.sin_port = (in_port_t)htons(service ? service->s_port : PORT);
		in_addr_t ip = inet_addr(q.q_hostname);
		if(ip == INADDR_NONE) {
			const struct hostent *h = gethostbyname(q.q_hostname);

			if(h == NULL) {
				cerr << "Unknown host " << q.q_hostname << endl;
				continue;
			}

			memcpy((char *)&ip, h->h_addr, sizeof(ip));
		}
		sin.sin_addr.s_addr = ip;

		UDTSOCKET s = UDT::socket(AF_INET, SOCK_STREAM, proto ? proto->p_proto : 0);

		if(s < 0) {
			perror("UDT::socket");
			break;
		}

		if(UDT::connect(s, (sockaddr *)&sin, sizeof(struct sockaddr)) == UDT::ERROR) {
			perror(q.q_hostname);
			UDT::close(s);
			continue;
		}

		struct request request;

		strcpy(request.r_password, "ignored");
		strcpy(request.r_filename, ++ptr);

		int64_t rlen = (int64_t)statb.st_size;
		request.r_len = htonll(rlen);

		if(UDT::send(s, (const char *)&request, sizeof(request), 0) == UDT::ERROR) {
			cerr << "send" << ": " <<
				UDT::getlasterror().getErrorMessage() << '\n';
			UDT::close(s);
			continue;
		}

		char status[24];
		if(UDT::recv(s, status, sizeof(status) - 1, 0) != 3) {
			cerr << q.q_hostname << " rejected the request " <<
				status << endl;
			UDT::close(s);
			continue;
		}
		shutdown(s, SHUT_RD);

		printf("sending %s, %lld bytes\n", request.r_filename, (long long int)rlen);

		int64_t offset = 0;
		if(UDT::sendfile(s, ifs, offset, rlen) == UDT::ERROR)
			cerr << request.r_filename << ": " <<
				UDT::getlasterror().getErrorMessage() << '\n';
		UDT::close(s);
	}

	return close(fin);
}

#ifndef	htonll
extern "C" {
static uint64_t
htonll(uint64_t n)
{
#if __BYTE_ORDER == __BIG_ENDIAN
	return n; 
#else
	return (((uint64_t)htonl(n)) << 32) + htonl(n >> 32);
#endif
}

// static uint64_t
// ntohll(uint64_t n)
// {
// #if __BYTE_ORDER == __BIG_ENDIAN
	// return n; 
// #else
	// return (((uint64_t)ntohl(n)) << 32) + ntohl(n >> 32);
// #endif
// }
}
#endif
