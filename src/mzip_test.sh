#!/bin/sh

make mzip
make munzip

cat mzip > testfile
cat testfile testfile > testfile.2
cat testfile.2 testfile.2 > testfile
cat testfile testfile > testfile.2
cat testfile.2 testfile.2 > testfile
cat testfile testfile > testfile.2
cat testfile.2 testfile.2 > testfile

cp testfile testfile.bak

for i in 0 -1 -2 -3 -4 -5 -6 -7 -8 -9
do
    echo Testing compression level $i

    mzip $i testfile | sed 's/^/    /g'
    echo

    munzip testfile.mz | sed 's/^/    /g'
    echo

    diff testfile testfile.bak
    if [ $? -ne 0 ]
    then
        echo files differ
        exit 1
    fi
done

for i in 0 -1 -2 -3 -4 -5 -6 -7 -8 -9
do
    echo $i
    (time (mzip $i testfile)) 2>&1 | grep real | sed 's/^/    /g'
    (time (munzip testfile.mz)) 2>&1 |grep real | sed 's/^/    /g'
    diff testfile testfile.bak
    if [ $? -ne 0 ]
    then
        echo files differ
        exit 1
    fi
done
