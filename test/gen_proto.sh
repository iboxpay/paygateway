#!/bin/bash
SRC_DIR=../proto
DST_DIR=.
#protoc --cpp_out ../ $SRC_DIR/protoc/iboxpay_payment.proto $SRC_DIR/cup_payment.proto
protoc --java_out=$DST_DIR -I=$SRC_DIR $SRC_DIR/iboxpay_payment.proto $SRC_DIR/cup_payment.proto
protoc --python_out=$DST_DIR -I=$SRC_DIR $SRC_DIR/cup_payment.proto
