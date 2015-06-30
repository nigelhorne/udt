Simple queue system to transfer files using UDT.
Download the library for UDT from http://udt.sourceforge.net/.

There are 3 programs:
	sender and receiver are daemons to run on the source and target machine;
	submit is a program to submit a file to be transferred.

Run "build" to compile, and move the executables to a suitable place.
Add "udt-copy" to /etc/services, using any port number (above 1024) that is not
in use on your system.

Receiving machine:

	mv receiver /usr/local/etc
Add to /etc/rc.local. I have, for example:
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib /usr/local/etc/receiver 2> /var/log/udt&

Sending machines:

	mv submit /usr/local/bin
	mv sender /usr/local/etc
Add this to /etc/rc.local:
	mkfifo /var/run/udt && chmod 666 /var/run/udt
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib /usr/local/etc/sender /var/run/udt 2> /var/log/udt&

Then to copy a file from a sending machine to the receiver named "server1"
	submit /var/run/udt server1 /tmp/foo
