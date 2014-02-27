#!/usr/bin/env ruby

require 'socket'               # Get sockets from stdlib

server = TCPServer.open(8000)  # Socket to listen on port 8000
loop {                         # Servers run forever
  client = server.accept       # Wait for a client to connect
  client.puts(Time.now.ctime)  # Send the time to the client
  client.puts "Closing the connection. Bye!"
  client.close                 # Disconnect from the client
}
