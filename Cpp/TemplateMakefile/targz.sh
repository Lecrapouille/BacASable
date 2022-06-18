#!/bin/bash

# This script, called by Makefile, makes a backup of the code source as
# an unique tarball. Tarballs name are unique $PROJECT$DATE-v$NTH.tar.gz
# where $DATE is the date of today and $NTH a counter to make unique if
# a tarball already exists. Some informations are not backuped like .git
# generated doc, builds ...

# $1 is the content of pwd where to execut this script.
# $2 is the tarball name

#set -x

HERE=${1##*/}
PROJECT=$2
DATE='-'`date +%Y-%m-%d`
VERSION='-'`cat VERSION`
NTH=0
TARGET_TGZ="${PROJECT}${DATE}-v${NTH}.tar.gz"

### Iterate for finding an unique name
while [ -e "$TARGET_TGZ" ];
do
    NTH=$(( NTH + 1 ))
    TARGET_TGZ="${PROJECT}${DATE}-v${NTH}.tar.gz"
done

### Display informations with the same look than Makefile
echo -e "\033[35m*** Tarball:\033[00m \033[36m""${TARGET_TGZ}""\033[00m <= \033[33m$1\033[00m"

### Compress ../$PROJECT in /tmp, append the version number to the name and move the
### created tarball from /tmp into the project root directory.
(cd .. && tar --transform s/${PROJECT}/${PROJECT}${VERSION}/ \
              --exclude='.git' --exclude="${PROJECT}-*.tar.gz" --exclude="doc/html" \
              --exclude "tests/doc" --exclude "*/build" --exclude "*~"  --exclude ".*~" -czvf /tmp/$TARGET_TGZ $HERE \
              > /dev/null && mv /tmp/$TARGET_TGZ $HERE)
