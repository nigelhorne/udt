/*
 * FIXME: these are sent unencrypted over the network
 */
#pragma	pack(1)
struct request {
	char	r_password[31];
	char	r_filename[31];	/* not full path, just basename */
	int64_t	r_len;
};
#pragma	pack()

/*
 * These are the items in the FIFO queue
 */
#ifndef	MAXHOSTNAMELEN
#define	MAXHOSTNAMELEN	64
#endif

struct queue {
	char	q_filename[31];
	/*string	q_filename;*/	/* TODO: Use string */
	char	q_hostname[MAXHOSTNAMELEN];
};

#define	PORT	50001	/* NOTE: UDP port to be opened through the firewall */
