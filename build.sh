#!/bin/sh

function make {
    NAME="-DprogName=\"${1}\""
    NUM="-DpageNum=\"${2}\""
    PAGE="-DbootPage=0${3}h"
    mkdir -p $1
    OUT="${1}/G${1}${2}.8xp"
    echo $OUT
    spasm $NAME $NUM $PAGE template.z80 $OUT
}

make 83PBE 1 1F
make 83PSE 1 7F
make 84PBE 1 3F
make 84PBE 2 2F
make 84PSE 1 7F
make 84PSE 2 6F
make 84CSE 1 FF
make 84CSE 2 EF
