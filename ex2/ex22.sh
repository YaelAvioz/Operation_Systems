#!/bin/bash

print_lines()
{
	cur_dir=$1
	cd ${cur_dir}
	new_dirs=$(ls -d */ 2> /dev/null)

	# run on files in current directory
        find_result=($(find . -maxdepth 1 -name "*.${ext}" | sort -r))
        for file in ${find_result[@]}
        do
                cat ${file} | grep -wi ${word} | awk -v len=$len '{if(NF >= len) print}';
        done


	# recursive step into every sub directory
	for d in ${new_dirs[@]}
	do
		print_lines $d
	done

        cd ..	
}


if [ $# -lt 4 ]; then
        echo "Not enough parameters"
else
	dir=$1
	ext=$2
	word=$3
	len=$4
	print_lines ${dir}
fi
