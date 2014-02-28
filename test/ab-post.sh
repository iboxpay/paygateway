#!/bin/zsh
#ab -c 4 -n 100 -k -v 3 -T "application/x-www-form-urlencoded"  -p post.txt http://127.0.0.1:8080/paygateway/api/ApiRequest
ab -c 4 -n 1000 -v 3 -H "Connection: close"  -T "application/x-www-form-urlencoded"  -p post.txt http://127.0.0.1:8080/paygateway/api/ApiRequest
