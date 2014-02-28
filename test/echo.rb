#!/usr/bin/ruby
#
# encoding: UTF-8
# File: echo.rb
# This file use to simulate third-party bank channel server
# Writen by Lytsing Huang 2013-11-19
# $Id$
#

require 'rubygems'
require 'timeout'
require 'webrick'
include WEBrick

$dir = Dir::pwd
$port = 8000

def start_webrick(config = {})
  config.update(:Port => $port, :DocumentRoot => $dir)
  server = HTTPServer.new(config)
  yield server if block_given?
  # Trap signals so as to shutdown cleanly.
  ['INT', 'TERM'].each {|signal|
    trap(signal) {server.shutdown}
  }
  server.start
end

start_webrick {|server|
  server.mount_proc('/') {|req, resp|
    resp.body = req.body
  }
}

