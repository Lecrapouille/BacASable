#!/bin/bash -e

###
### git clone recursively all my personal public projects for backup.
### Private repos are not downloaded.
### Script not working if repos are already cloned.
###

GITHUB_USER=Lecrapouille
FOLDER="/media/data/MyGitHubBackups/backup-$(date +"%d-%m-%Y")"

mkdir -p $FOLDER
(cd $FOLDER
  curl "https://api.github.com/users/$GITHUB_USER/repos?per_page=100" | grep -o 'git@[^"]*' > repos
  for REPO in `cat repos`; do
    git clone $REPO --recurse;
  done
)
