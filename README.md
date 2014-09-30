paygateway
======================

paygateway is proxy for web frontend, communicate with third-party bank channel.

Current Features
------------

* Supports HTTP, HTTPS protocols
* Use Google protobuf protocols
* Use vhtp,libevent library

TODO
* ISO8583 message pack and unpack
* Add more proto file
* Add queue.

Architecture
------------

                          protobuf
       +----------------+   http     +---------------+ iso8583 +----------------+
       |                +----------> |               | socket  |                |
       |    router      |            |  paygateway   +-------> |3party channel  |
       |                | <----------+               <-------+-+                |
       +----------------+            +---------------+         +----------------+


Install in Linux/Mac OS
------------
Make sure your cmake version >= 2.6

   $ cmake .
   $ make


Usage
------------
    ./paygateway -help
    PayGateway server.

    -l <ip_addr> interface to listen on, default is 0.0.0.0
    -p <num>     port number to listen on, default is 8080
    -d           run as a deamon
    -t <second>  timeout for a http request, default is 120 seconds
    -h           print this help and exit

