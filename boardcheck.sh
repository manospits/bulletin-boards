#!/bin/bash
spid="_spid"
server="board_server"
NEWLINE='\n'
dirName=${1-`pwd `}
dir="/"
active=0
nonactive=0
for  file in $( find  $dirName  -type d )
    do
        check="$file/_spid"
        selfr="$file/self_r"
        selfw="$file/self_w"
        other="$file/others_r"
        othew="$file/others_w"
        var=0
        if [ -f $check ]; then
            if [ -p $selfr ]; then
                var=`expr $var + 1`
            fi
            if [ -p $selfw ]; then
               var=`expr $var + 1`
            fi
            if [ -p $other ]; then
                var=`expr $var + 1`
            fi
            if [ -p $othew ]; then
                var=`expr $var + 1`
            fi
            if [ $var == 4 ]; then
               pid=$(cat "$file/$spid")
               name=$( ps -p $pid -o comm= )
               #name=$( ps -p $pid |grep  $pid |tr -s " "|cut -d " " -f5)
               if [ "$name" == "$server" ];then
                    actboards="$actboards$file \n"
                    active=`expr $active + 1`
                else
                    nonactive=`expr $nonactive + 1`
                    nactsboards="$nactsboards$file \n"
                fi
            else
                nonactive=`expr $nonactive + 1`
                nactsboards="$nactsboards$file \n"
            fi
        fi
    done
echo $active Boards active :
printf "$actboards \n"
echo  $nonactive Inactive Boards :
printf "$nactsboards \n"

