source /home/ytian/.bashrc

BENCHMARKS="xalan"

NUMBERS="1
2
3
4
5
6
7
8
9
10"

DUR=/home/ytian/hotspot/tests/durations.csv
#echo "" > $DUR

for BM in $BENCHMARKS
do
    echo $BM >> $DUR

    for NUM in $NUMBERS
    do
	hs-debug -XX:+CacheOptimalGC -jar ~/DaCapo/dacapo-9.12-bach.jar -s small $BM  &> /home/ytian/hotspot/tests/"$BM"_"$NUM".log
	grep -o "PASSED in [0-9]*" /home/ytian/hotspot/tests/"$BM"_"$NUM".log | grep -o "[0-9]*" >> $DUR
    done
    
    echo "" >> $DUR
    echo "=======================" >> $DUR
done
