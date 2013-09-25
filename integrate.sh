#!/bin/sh

echo Branch:$1 
echo User:$2

hg pull -b $1 $2 
hg up $1
hg merge default
hg ci -m merge
hg up default
hg merge $1
hg st
hg diff

echo -n "Do you want to continue and merge? (y/N): "
read answer
if [ X$answer != "Xy" ]; then
	echo "Aborting"
        exit 1
fi
echo -n "Enter commit message:"
read answer

hg ci -v -m  "$1: $answer"
hg up $1
hg ci -v --close-branch -m closed
hg up default
hg push -b default -b $1 --new-branch
