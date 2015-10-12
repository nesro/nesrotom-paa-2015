#!/bin/bash
# Tomas Nesrovnal
# nesro@nesro.cz
# https://github.com/nesro/nesrotom-paa-2015

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

gpfile=./time.plot
for m in h b; do
	echo "m=$m"
	[[ $m == h ]] && r=50000;
	[[ $m == b ]] && r=1;
	echo "r=$r"
	echo "time instance_size" >$gpfile
	for i in tests/*.inst.dat; do
		echo "   i=$i"
		echo "$(./main -$m -t -r $r <$i) $(echo $i | sed 's/[^0-9]*//g')" >>$gpfile
	done
	cat $gpfile
	gnuplot_wrapper $gpfile ./time_$m.png instance_size time 2
done
#rm $gpfile

