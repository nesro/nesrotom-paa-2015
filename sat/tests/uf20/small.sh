#!/usr/bin/env bash

c() { bc <<< "scale=10;$1" | sed 's/^\./0./'; }

r=5
it=0
for i in *.cnf; do
  echo  "f= $i "
  
  for lim in 90 70 50; do
  echo -n "lim= $lim "
  bf=$(grep -v "^c" $i | ../../main -l $lim -b -S 2>&1 | awk '{ print $2 }' )
  echo -n "bf= $bf "
  
  c1_all=$(grep -v "^c" $i | ../../main -l $lim -c 1 -r $r 2>&1 )
  c1_avg=$(echo $c1_all | awk '{ print $2 }' )
  c1_bst=$(echo $c1_all | awk '{ print $4 }' ) 
  #c1_tim=$(echo $c1_all | awk '{ print $6 }' )
  c1_erv=$(c "(( $bf - $c1_avg ) / $bf ) * 100" ) # error from average
  c1_erb=$(c "(( $bf - $c1_bst ) / $bf ) * 100" ) # error from best
  echo -n "c1avg= $c1_avg c1bst= $c1_bst c1erv= $c1_erv c1erb= $c1_erb "

  c2_all=$(grep -v "^c" $i | ../../main -l $lim -c 2 -r $r 2>&1 )
  c2_avg=$(echo $c2_all | awk '{ print $2 }' )
  c2_bst=$(echo $c2_all | awk '{ print $4 }' ) 
  #c1_tim=$(echo $c1_all | awk '{ print $6 }' )
  c2_erv=$(c "(( $bf - $c2_avg ) / $bf ) * 100" ) # error from average
  c2_erb=$(c "(( $bf - $c2_bst ) / $bf ) * 100" ) # error from best
  echo -n "c2avg= $c2_avg c2bst= $c2_bst c2erv= $c2_erv c2erb= $c2_erb "

  c3_all=$(grep -v "^c" $i | ../../main -l $lim -c 3 -r $r 2>&1 )
  c3_avg=$(echo $c3_all | awk '{ print $2 }' )
  c3_bst=$(echo $c3_all | awk '{ print $4 }' ) 
  #c1_tim=$(echo $c1_all | awk '{ print $6 }' )
  c3_erv=$(c "(( $bf - $c3_avg ) / $bf ) * 100" ) # error from average
  c3_erb=$(c "(( $bf - $c3_bst ) / $bf ) * 100" ) # error from best
  echo -n "c3avg= $c3_avg c3bst= $c3_bst c3erv= $c3_erv c3erb= $c3_erb "

 

  echo ""
  done
  
  (( it = it + 1 ))
done



# time cat ./tests/uf/uf200-0100.cnf| ./main -c 3 -r 1 1>/dev/null
