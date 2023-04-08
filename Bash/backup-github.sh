#!/bin/bash -e

###
### git clone recursively all my personal public projects for backup.
### Private repos are not downloaded.
### Script not working if repos are already cloned.
###

function backup
{
    GITHUB_USER=$1
    FOLDER=$2

    mkdir -p $FOLDER
    (cd $FOLDER
      curl "https://api.github.com/users/$GITHUB_USER/repos?per_page=1000" | grep -o 'git@[^"]*' > repos
      for REPO in `cat repos`; do
        git clone $REPO --recurse;
      done
    )
}

FOLDER="$1/backup-$(date +"%d-%m-%Y")"

backup "Lecrapouille" $FOLDER
backup "stigmee" "$FOLDER/Stigmee"

(cd $FOLDER
 git clone git@github.com:Lecrapouille/CV.git --recurse
 git clone git@github.com:Lecrapouille/Highway.git --recurse
)
