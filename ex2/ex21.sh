#!/bin/bash

if [ $# -lt 3 ]; then
	echo "Not enough parameters"
else
	ls_result=($(ls *.$2))
	for file in ${ls_result}
	do
		grep -wi $3 $file
	done
fi
