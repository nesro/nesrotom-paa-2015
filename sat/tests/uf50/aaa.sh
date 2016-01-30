#!/usr/bin/env bash

echo "diky! :)"
sleep 1

c() { bc <<< "scale=10;$1" | sed 's/^\./0./'; }

r=5
it=1
solved1=0
solved2=0
solved3=0
for i in *.cnf; do
  echo -n	 "f= $i "

  c1=$(grep -v "^c" $i | ../../main -c 1 -r $r -s 2>&1 )
  if (( c1 > 0 )); then
    (( solved1 = solved1 + 1 ))
  fi
  per1=$( c "$solved1 / $it" )
  
  c2=$(grep -v "^c" $i | ../../main -c 2 -r $r -s 2>&1 )
  if (( c2 > 0 )); then
    (( solved2 = solved2 + 1 ))
  fi
  per2=$( c "$solved2 / $it" )
  
  c3=$(grep -v "^c" $i | ../../main -c 3 -r $r -s 2>&1 )
  if (( c3 > 0 )); then
    (( solved3 = solved3 + 1 ))
  fi
  per3=$( c "$solved3 / $it" )
  
  
  echo "it= $it s1= $solved1 p1= $per1 s2= $solved2 p2= $per2 s3= $solved3 p3= $per3"

  (( it = it + 1 ))  
  
  if (( it >= 3 )); then
    echo "konec"
    break;
  fi
done



