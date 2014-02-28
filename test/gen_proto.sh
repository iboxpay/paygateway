#!/bin/bash
SRC=../proto
#protoc --cpp_out ../ $SRC/protoc/iboxpay_payment.proto $SRC/cup_payment.proto
protoc --java_out=. -I=$SRC $SRC/iboxpay_payment.proto $SRC/cup_payment.proto
protoc --python_out=. -I=$SRC $SRC/cup_payment.proto
