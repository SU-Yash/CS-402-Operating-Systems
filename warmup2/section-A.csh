#!/bin/tcsh -f

echo "(A) Basic running of the code ..."

    /bin/rm -f f?.out f?.ana
    echo "===> test_0"
    ./warmup2 -n 4 -r 7 -B 7 -P 5 -lambda 3.333 -mu 8.5 > f0.out
    echo "===> test_1"
    ./warmup2 -mu 0.85 -r 0.5 -P 5 -lambda 33.33 -B 7 -n 4 > f1.out
    echo "===> test_2"
    ./warmup2 -B 3 -t warmup2data/f0.txt > f2.out
    echo "===> test_3"
    ./warmup2 -t warmup2data/f1.txt -B 2 > f3.out
    echo "===> test_4"
    ./warmup2 -B 1 -t warmup2data/f2.txt > f4.out
    echo "===> test_5"
    ./warmup2 -t warmup2data/f3.txt > f5.out
    echo "===> test_6"
    ./warmup2 -r 5 -B 2 -t warmup2data/f4.txt > f6.out
    echo "===> test_7"
    ./warmup2 -B 2 -t warmup2data/f5.txt -r 15 > f7.out
    echo "===> test_8"
    ./warmup2 -t warmup2data/f6.txt -r 25 -B 2 > f8.out
    echo "===> test_9"
    ./warmup2 -t warmup2data/f7.txt -B 1 -r 5 > f9.out

    ./analyze-trace.txt -n 4 -r 7 -B 7 -P 5 -lambda 3.333 -mu 8.5 f0.out > f0.ana
    ./analyze-trace.txt -mu 0.85 -r 0.5 -P 5 -lambda 33.33 -B 7 -n 4 f1.out > f1.ana
    ./analyze-trace.txt -B 3 -t warmup2data/f0.txt f2.out > f2.ana
    ./analyze-trace.txt -t warmup2data/f1.txt -B 2 f3.out > f3.ana
    ./analyze-trace.txt -B 1 -t warmup2data/f2.txt f4.out > f4.ana
    ./analyze-trace.txt -t warmup2data/f3.txt f5.out > f5.ana
    ./analyze-trace.txt -r 5 -B 2 -t warmup2data/f4.txt f6.out > f6.ana
    ./analyze-trace.txt -B 2 -t warmup2data/f5.txt -r 15 f7.out > f7.ana
    ./analyze-trace.txt -t warmup2data/f6.txt -r 25 -B 2 f8.out > f8.ana
    ./analyze-trace.txt -t warmup2data/f7.txt -B 1 -r 5 f9.out > f9.ana

