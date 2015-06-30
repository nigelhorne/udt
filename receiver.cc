/*
 * This end is the receiver
 *
 * Accepts connections from the sender,
 *	For each connection
 *		Validate given password, send OK, receive and store the file
 */
#include <iostream>
#ifdef	_MSC_VER
#include <winsock.h>
#include <direct.h>
#else
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <endian.h>
#endif
#include <errno.h>
#include <udt.h>

#include "copy.h"

using namespace std;
using namespace UDT;

#ifdef	_MSC_VER
typedef	unsigned short	in_port_t;
#endif

#ifndef	htonll
extern "C" {
static	uint64_t	htonll(uint64_t n);
static	uint64_t	ntohll(uint64_t n);
}
#endif

static	int64_t	do_recv(UDTSOCKET s, ofstream& out_stream, int64_t& offset, int64_t nbytes);

int
main(void)
{
	const struct protoent *proto = getprotobyname("tcp");
	UDTSOCKET sock;

	if((sock = UDT::socket(PF_INET, SOCK_STREAM, proto ? proto->p_proto : 0)) == UDT::ERROR) {
		perror("socket");
		return errno;
	}

	struct sockaddr_in sin;
	memset(&sin, '\0', sizeof(struct sockaddr_in));
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	const struct servent *service = getservbyname("udt-copy", "tcp");
	sin.sin_port = (in_port_t)htons(service ? service->s_port : PORT);
	sin.sin_family = AF_INET;

	if(UDT::bind(sock, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) == UDT::ERROR) {
		perror("bind");
		return errno;
	}

	if(UDT::listen(sock, 10) == UDT::ERROR) {
		perror("listen");
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

#ifdef	_MSC_VER
	if(getenv("TEMP")) {
		const char *tmp = getenv("TEMP");
		if(_chdir(tmp) < 0)
			perror(tmp);
	}
#else
	endprotoent();
	endservent();

	if(getenv("TMPDIR")) {
		const char *tmp = getenv("TMPDIR");
		if(chdir(tmp) < 0)
			perror(tmp);
		else if(chroot(tmp) < 0)
			perror(tmp);
	}

	if(getuid() == 0) {
		setgid(99);
		setuid(99);
	}
#endif

	for(;;) {
		int len = sizeof(struct sockaddr_in);

		UDTSOCKET s = UDT::accept(sock, (struct sockaddr *)&sin, &len);

		if(s == UDT::ERROR) {
			perror("accept");
			return errno;
		}

		struct request request;
#ifdef	_MSC_VER
		size_t nbytes = UDT::recv(s, (char *)&request, sizeof(struct request), 0);
#else
		ssize_t nbytes = UDT::recv(s, (char *)&request, sizeof(struct request), 0);
#endif	

		if(nbytes != sizeof(struct request)) {
			if(nbytes == UDT::ERROR)
				perror("recv");
			else {
				fputs("Bad request record received\n", stderr);
				UDT::send(s, "Bad request", 12, 0);
			}
			UDT::close(s);
			continue;
		}

		/* FIXME: authenticate the password */

		if(UDT::send(s, "OK", 3, 0) != 3) {
			perror("send");
			UDT::close(s);
			continue;
		}

#ifdef	SHUT_WR
		shutdown(s, SHUT_WR);
#endif		

		request.r_len = ntohll(request.r_len);
		/*request.r_rlen = ntohll(request.r_rlen);*/

		printf("receiving %s, %llu bytes\n",
			request.r_filename, request.r_len);

		if(request.r_len) {
			ofstream ofs(request.r_filename, ofstream::out|ofstream::binary);

			if((ofs == NULL) || ofs.bad()) {
				perror(request.r_filename);
				UDT::close(s);
				continue;
			}
			// if(UDT::recvfile(s, ofs, 0L, request.r_len) == UDT::ERROR) {
			int64_t zero = 0;
			if(do_recv(s, ofs, zero, request.r_len) == UDT::ERROR) {
				cerr << request.r_filename << ": " <<
					UDT::getlasterror().getErrorMessage() <<
					'\n';
				UDT::close(s);
				continue;
			}
		}
		UDT::close(s);

		/* Also get resource fork if needed */
		/*
			QFileInfo fiRsrc(fi->absFilePath() + ".rsrc");
			if (fiRsrc.exists()) {
				addFileEntry(fiRsrc.absFilePath(), strBasePath + "/" + dir.dirName());
			}
		 */
	}
	return 0;
}

static int64_t
do_recv(UDTSOCKET s, ofstream& out_stream, int64_t& offset, int64_t nbytes)
{
	return UDT::recvfile(s, (fstream &)out_stream, offset, nbytes);
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

static uint64_t
ntohll(uint64_t n)
{
#if __BYTE_ORDER == __BIG_ENDIAN
	return n; 
#else
	return (((uint64_t)ntohl(n)) << 32) + ntohl(n >> 32);
#endif
}
}
#endif
