#!/bin/sh

if [ -z $1 ]; then
    echo "require un fichier en entrée" > stderr
    exit 1
fi

cat $1 | tr "\015" "\n"
