#!/bin/bash
source ../../../env.sh
source local-config-button.mak

echo "+ make clean"
make -f Makefile_button clean

if [[ $1 != "DONT_WAIT" ]]; then
  vim $PROJECT_NAME.c
  echo "+ build $PROJECT_NAME"
  echo "+ press <ENTER> to go on"
  read r
fi

echo "+ make <Makefile>"
make -f Makefile_button

if [[ $DEVKIT = "native" ]]; then
  echo "+ execute $PROJECT_NAME"
  echo "+ press <ENTER> to go on"
  read r
  ./$PROJECT_NAME.out
fi
