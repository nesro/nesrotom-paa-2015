#!/bin/bash
# Tomas Nesrovnal
# nesro@nesro.cz
# https://github.com/nesro/nesrotom-paa-2015

if false; then
	make
	valgrind="valgrind --leak-check=full --track-origins=yes -q"
else
	make fast
	valgrind=""
fi

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

		set logscale y

		set key top left reverse Left
		set key autotitle columnhead
		set key title "Legend"
		set key box right width 1 height 1

		set ylabel "$ylabel"
		set xlabel "$xlabel"

		$plot
__EOF__
}

for m in f0 f25 f50 f75 f100 d1 d2 b0 b1 h1 h2 h3; do
	gpfile_err=./err_$m.plot
	gpfile_time=./time_$m.plot
	echo "> m=$m"
	echo "err_max err_rel instance_size" >$gpfile_err
	echo "time instance_size" >$gpfile_time
	for i in tests/*.inst.dat; do
		problem_size=$(echo $i | sed 's/[^0-9]*//g')
		if [[ $m =~ b[01] ]] && (( $problem_size >= 25 )); then
			continue
		fi
		if [[ $m == d2 ]] && (( $problem_size >= 22 )); then
			continue
		fi

		echo "> > problem_size=$problem_size"

		if [[ $m =~ b[01] ]]; then
			r=5
		else
			r=50
		fi

		r=$(paste -d ' ' <(cat $(echo $i | sed 's/inst/sol/') | cut -d ' ' -f 3) $i | $valgrind ./main -p -$m -r $r | grep '_')
		err_rel=$(echo $r | cut -d_ -f1)
		err_max=$(echo $r | cut -d_ -f2)

		# tady jsem si zjistil, ze to mam dobre
		if [[ $m =~ b[01] ]] || [[ $m = d[12] ]] || [[ $m =~ f* ]]; then
			echo "err_max=$err_max"
		fi

		prg_time=$(echo $r | cut -d_ -f3)
		echo "$err_max $err_rel $problem_size" >>$gpfile_err
		echo "$prg_time $problem_size" >>$gpfile_time
	done

	if [[ ! $m =~ b[01] ]] && [[ ! $m =~ d[12] ]] && [[ ! $m == f0 ]]; then
		gnuplot_wrapper $gpfile_err ./err_$m.pdf "instance size" "relative error" 3
	fi

	gnuplot_wrapper $gpfile_time ./time_$m.pdf "instance size" "time[s]" 2
done

