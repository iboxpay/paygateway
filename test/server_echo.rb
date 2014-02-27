#!/usr/bin/env ruby

require 'socket'

gs = TCPServer.open(8000)
addr = gs.addr
addr.shift
puts "server is on #{addr.join(':')}"

while true
  Thread.start(gs.accept) do |s|
    puts "#{s} is accepted"
    while s.gets
      s.write($_)
    end
    puts "#{s} is gone"
    s.close
  end
end

