#!/bin/bash -e

if [ $# -ne 5 ]
then
  echo "usage: ./reqreview <revToDiffFrom> <summary> <description> <reviewer> <bugNumber>"
  exit 0
fi

hg diff -r $1 > diff.txt
/usr/local/bin/post-review --server=http://reviews/ --diff-file=diff.txt --summary="$2" --description="$3" --target-people=$4 -d --repository-url=/home/mr-dev/source --bugs-closed=$5 -p
rm diff.txt
