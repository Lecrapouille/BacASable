#!/bin/bash

###
### git clone all my personal public projects for backup.
### Private repos are not downloaded.
### Script not working if repos are already cloned.
###

GITHUB_USER=Lecrapouille
curl "https://api.github.com/users/$GITHUB_USER/repos?per_page=100" | grep -o 'git@[^"]*' > repos
for REPO in `cat repos`; do git clone $REPO --recurse; done
