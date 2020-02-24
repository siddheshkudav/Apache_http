This project was done in two parts(N1,N2) over the course of semester where we were asked a web server like APACHE:

Project APACHE: N1 (Apache.c)
Your term project is to build a web server like Apache which you will do in stages. 
In this first stage you will develop DNS functions. Your Apache will listen on a
user-specified socket for DNS lookup requests and return a list of IP addresses with a preferred address.


Project APACHE: N2 (Apache2.c)

You will extend Apache to field HTTP/1.1 web requests, one at a time, per the below requirements. You will receive a HTTP request, connect to the specified host and retrieve the HTTP object/s.

Log / server output are the same as N1. RESP: should indicate the number of bytes transferred for that request.
GET requests work and images and arbitrary length binary files are transferred correctly.
Apache should properly handle Full-Requests (RFC 1945 section 4.1) up to 65535 bytes. You may close the connection if a Full-Request is larger than that.
You must support URLs with a numerical IP address instead of the server name (e.g. http://128.181.0.31/).
You are not allowed to use fork() or threads/multiprocesses of any kind.
You may not allocate or use more than 20MB of memory or have more than 10 file descriptors / Socket objects. (We will test this using VALGRIND (or equivalent) or Jikes/RVM for Java heap monitoring, FYI.)
Apache should correctly service each request if possible. If errors occur it should close the connection and then proceed to the next request. Errors and exceptions should be handled in the spirit of the protocol/s, and in the spirit of "Apache" a crash or hang should never happen.
Apache will serve HTTP objects from local directory.
A request like http://LOCALFILE/xyz.txt (Links to an external site.) means serve up file xyz.txt which is in the working directory where Apache is running.
SPEED REQUIREMENT Apache was designed to be fast and accurate, so your Apache should fetch and deliver files in 1 second or less (of Apache time.)
You may NOT use any predefined libraries (e.g. libwww, libhttpX) or parsers or copy others’ code or code from the web
