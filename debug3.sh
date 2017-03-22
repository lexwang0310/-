#!/bin/bash
#gcc upload4.c -o upload2 -lfcgi

MYUSER=`whoami`
MYPID1=`ps -u $MYUSER | grep upload_with | awk {'print $1'}`
MYPID2=`ps -u $MYUSER | grep data_cgi | awk {'print $1'}`
MYPID3=`ps -u $MYUSER | grep reg_cgi | awk {'print $1'}`
MYPID4=`ps -u $MYUSER | grep login_cgi | awk {'print $1'}`
#echo $MYPID
kill $MYPID1
kill $MYPID2
kill $MYPID3
kill $MYPID4
spawn-fcgi -a 127.0.0.1 -p 8082 -f ./upload_with
spawn-fcgi -a 127.0.0.1 -p 8083 -f ./data_cgi
spawn-fcgi -a 127.0.0.1 -p 8084 -f ./reg_cgi
spawn-fcgi -a 127.0.0.1 -p 8085 -f ./login_cgi
