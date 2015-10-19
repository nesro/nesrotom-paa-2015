#!/bin/bash
 set -xv
#for m in "h 1" "h 2" "h 3" b; do
for m in "h 3" b; do
	echo -n "m=$m, "
	for i in tests/*.inst.dat; do
		echo -n "t=$i, "
		fail=0
		for j in $(paste -d" " \
		    <(./main -$m <$i | awk '{ print $3 }') \
		    <(awk '{ print $3 }' $(echo $i | sed 's/inst/sol/')) \
		    | awk '
		    { print $1 "_" $2 }'); do
			cost_heur=$(echo $j | cut -d_ -f1)
			cost_brut=$(echo $j | cut -d_ -f2)
			echo $cost_heur ... $cost_brut
			
			if (( $cost_heur != $cost_brut )); then
				((fail++))
			fi
		done

		if (( $fail == 0 )); then
			echo " [OK]"
		else
			echo " [FAIL] $fail lines has failed"
		fi

		exit 1
	done
done

