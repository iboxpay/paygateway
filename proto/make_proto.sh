#!/bin/bash
protoc --cpp_out ../ iboxpay.payment.proto
protoc --python_out=. cup.payment.proto

