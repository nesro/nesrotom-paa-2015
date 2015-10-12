#!/bin/bash
# Tomas Nesrovnal
# nesro@nesro.cz
# https://github.com/nesro/nesrotom-paa-2015

c() { bc <<< "scale=10;$1" | sed 's/^\./0./'; }

gnuplot_wrapper() {
	input_filename=$1
	output_filename=$2
	xlabel=$3
	ylabel=$4
	nlines=$5
	
	plot="plot \"$input_filename\" using $nlines:1"
	for (( i=2; i<nlines; i++ )); do
		plot+=", \"\" using $nlines:$i"
	done  
	
	gnuplot << __EOF__
		set term pdf
		set output "$output_filename"
		set pointsize 2
		set style data point

		set key top left reverse Left
		set key autotitle columnhead
		set key title "Legend"
		set key box width 1 height 1

		set ylabel "$ylabel"
		set xlabel "$xlabel"

		$plot
__EOF__
}

#set -xv
gpfile=./time.plot
for m in h; do # add b to test the error for the bruteforce
	echo "relative_error instance_size" >$gpfile
	for i in tests/*.inst.dat; do
		max_err=0
		sum_cost_heur=0
		sum_cost_brut=0
		sum_err_rel=0
		sum_err_abs=0

		for j in $(paste -d" " \
		    <(./main -$m <$i | awk '{ print $3 }') \
		    <(awk '{ print $3 }' $(echo $i | sed 's/inst/sol/')) \
		    | awk '
		    { print $1 "_" $2 "_" ($2-$1) "_" ($2-$1)/$1  }'); do
		
			cost_heur=$(echo $j | cut -d_ -f1)
			cost_brut=$(echo $j | cut -d_ -f2)
			err_abs=$(echo $j | cut -d_ -f3)
			err_rel=$(echo $j | cut -d_ -f4)

			sum_cost_brut=$(c "$sum_cost_brut+$cost_brut")
			sum_cost_heur=$(c "$sum_cost_heur+$cost_heur")
			sum_err_abs=$(c "$sum_err_abs+$err_abs")
			sum_err_rel=$(c "$sum_err_rel+$err_rel")
		done

		sum_err_rel=$(c "$sum_err_rel/$(wc -l < $(echo $i | sed 's/inst/sol/'))")
		glob_err=$(c "($sum_cost_brut-$sum_cost_heur)/$sum_cost_brut")

		echo "method=$m, file=$i sum_err_rel=$sum_err_rel gl=$glob_err"
		echo -e "$sum_err_rel $(echo $i | sed 's/[^0-9]*//g')" >>$gpfile
	done

	cat $gpfile
	gnuplot_wrapper $gpfile ./err_$m.png instance_size error 2
done

