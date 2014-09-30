#!/usr/bin/python

import os
import httplib, urllib
import sys
import base64
import gzip
import StringIO
import cup_payment_pb2
from google.protobuf import text_format

sale_req = cup_payment_pb2.SaleRequest()
sale_req.cmdType		= 1
sale_req.signType		= 2
sale_req.terminalNo 		= "03434324234" 
sale_req.merchantNo		= "4342432434234"
sale_req.terminalNum		= "2432443"
sale_req.trackBathNo		= "243243"
sale_req.pinBathNo		= "433434"
sale_req.isPinExist		= True
sale_req.track			= "000000000000000000"
sale_req.pin			= "4234322545"
sale_req.transAmount		= "10000"
sale_req.currency		= "155"
sale_req.orderSerial		= "3423423423423"
#sale_req.clearMerchantNo	= "fdsfdsfsf"
sale_req.sysRefNo		= "fsdfd"

requestData = base64.urlsafe_b64encode(sale_req.SerializeToString())
payment_headers = {
        "User-Agent": "Android-payment/2 (sapphire PLAT-RC33); gzip",
        "Content-type": "application/x-www-form-urlencoded",
        "Accept-Charset":"ISO-8859-1,utf-8;q=0.7,*;q=0.7",
        "Connection": "keep-alive"
}

payment_data = "version=2&request=" + requestData

conn = httplib.HTTPConnection("127.0.0.1:8080")
conn.request("POST", "/paygateway/api/ApiRequest", payment_data, payment_headers)

response = conn.getresponse()

if response.status == 200 :
   raw_data = response.read()
   data = raw_data
   #stream = StringIO.StringIO(raw_data)
   #decompressor = gzip.GzipFile(fileobj=stream)
   #data = decompressor.read()
else :
  data = ""
conn.close

print data

