#!/bin/sh
make
make
rm out*
#generate dofile
node simGen.js $1 $2

echo "#########################"
echo "###########REF###########"
echo "#########################"

./ref/fraig -f sim_refdo > sim_out.ref
# ./ref/fraig -f sim_refdo > sim_out.ref 2>&1
# ./ref/fraig -f refdo 2>&1 |tee out.ref
for i in `seq $1 $2`
do
   if [ $i -lt 10 ]
   then
      cp "out0"$i".log" "out0"$i".ref.log"
   else
      cp "out"$i".log" "out"$i".ref.log"
   fi
done

echo "                         "
echo "                         "
echo "#########################"
echo "###########XXX###########"
echo "#########################"

./fraig -f sim_mydo > sim_out
# ./fraig -f sim_mydo > sim_out 2>&1 
# ./fraig -f mydo 2>&1 |tee out

for i in `seq $1 $2`
do
   if [ $i -lt 10 ]
   then
      colordiff "out0"$i".log" "out0"$i".ref.log"
   else
      colordiff "out"$i".log" "out"$i".ref.log"
   fi
done

diff sim_out.ref sim_out > sim_out.diff
colordiff sim_out.ref sim_out
rm out*.log
