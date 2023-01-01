# dts
A Data Transfer System.

This a simple way to transfer data between systems using an ascii syntax.

The server side uses a trietree to store the data in memory, so the data is lost each time the server
program is restarted.  To some this may seem like a bad thing but I designed this system as a way to
store other programs data, like stats and other data to be sharted across systems.

This uses UDP so that it is as fast as possible.  To help with any data lose do to missed UDP packets, I
send each packet to the server N number of times.  But if it loses a packet it really does not matter cause
a packet being lost does not effect the system.

Read the Readme.txt file for message syntax.

The dtsengine.c is a server program and the dtsclient.c is a client programi example.

You will notice that I use the same port number for both server and client programs, this means you can
not run the server and client on the same system with a single network interface.

If you have the following interfaces on two different systems 192.168.0.15 and 192.168.0.24

The server would call the easyUdp function with the following.

	easUdp("192.168.0.15", "192.168.0.24", seqNumStart, &callback);

end the client would call the easyUdp with the following.

	easyUdp("192.168.0.24", "192.168.0.15", seqNumStart, &callback);

The first IP address is the IP address of the interface you wish to listen and the second IP address
is the servers.  The server is listen on interface 192.168.0.15 and sending data to the 192.168.0.24 address.
The client is listening on interface 192.168.0.24 and sending data to the 192.168.0.15 address.

This system usess the easyudp source code found on my github account.
