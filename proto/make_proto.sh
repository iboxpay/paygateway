#!/bin/bash
protoc --cpp_out ../ iboxpay_payment.proto cup_payment.proto
protoc --java_out=. iboxpay_payment.proto cup_payment.proto
protoc --python_out=. cup_payment.proto

