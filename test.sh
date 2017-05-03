#!/bin/bash

#pocet cisel bud zadam nebo 10 :)
if [ $# -lt 1 ];then 
    	numbers=10;
else
    	numbers=$1;
fi;

#preklad cpp zdrojaku
mpic++ --prefix /usr/local/share/OpenMPI -o es es.cpp

#vyrobeni souboru s random cisly
dd if=/dev/random bs=1 count=$numbers of=numbers > /dev/null 2>&1

processes=$(($numbers+1));

#spusteni
mpirun --prefix /usr/local/share/OpenMPI -np $processes es

#uklid
rm -f es numbers
