File transfer using UDT on Linux.  Download the library for UDT from
http://udt.sourceforge.net/.

There are 3 programs:
	sender and receiver are daemons to run on the source and target machine;
	submit is a program to submit a file to be transferred.

Run 'build' to compile, and move the executables to a suitable place.
Add "udt" to /etc/services, using any port number (above 1024) that is not in
use on your system.
