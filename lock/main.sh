#!/bin/bash
killprocess(){
  findby='.\..\..'
  ignore='grep|yrssf|lwan'
  while [ "1"="1" ]
  do
    pid=`ps -ef|grep $findby|grep -v $ignore|wc -l`
    if [ 0 -ne $pid ];
    then
      #echo "ps -ef|grep $findby|grep -v $ignore|awk '{print $2}'|xargs kill -9"
      #echo "kill $pid process"
      #ps -ef|grep $findby|grep -v $ignore
      (ps -ef|grep $findby|grep -v $ignore|awk '{print $2}'|xargs kill -9) > /dev/null
    fi
    sleep 2
  done
}
killprocess