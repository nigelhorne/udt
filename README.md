Simple queue system to transfer files using UDT.
Download the library for UDT from http://udt.sourceforge.net/ or

	apt install libudt-dev
	yum install udt-devel

There are 3 programs:
	sender and receiver are daemons to run on the source and target machine;
	submit is a program to submit a file to be transferred.

Run "build" to compile, and move the executables to a suitable place.
Add "udt-copy" to /etc/services, using any port number (above 1024) that is not
in use on your system.

To install:

	mv receiver /usr/local/etc
	mv submit /usr/local/bin
	mv sender /usr/local/etc

If you are using systemd, copy and paste this content into
/etc/systemd/system/udt.service

	[Unit]
	Description=Send and receive files using UDT
	After=network.target

	[Service]
	ExecStart=/bin/dash -c "/bin/rm -f /var/run/udt && /usr/bin/mkfifo -m 666 /var/run/udt && /usr/local/etc/receiver && /usr/local/etc/sender /var/run/udt"
	KillMode=process
	Restart=on-failure

	[Install]
	WantedBy=multi-user.target

Then run

	systemctl daemon-reload
	systemctl enable udt.service
	systemctl start udt.service

If you are not using systemd, put these into /etc/rc.local:

	mkfifo /var/run/udt && chmod 666 /var/run/udt
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib /usr/local/etc/sender /var/run/udt 2>&1 >> /var/log/udt&
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib /usr/local/etc/receiver 2> /var/log/udt&

Then to copy a file from a sending machine to the receiver named "server1"

	submit /var/run/udt server1 /tmp/foo
