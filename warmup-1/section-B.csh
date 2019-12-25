#!/bin/tcsh -f

echo "(B) Sort..."

                /bin/rm -f f?.sort f??.sort
                foreach f (0 1 2 3 4 5 6 7 8 9 10 11 12 13 14)
                    echo "===> warmup1data/f$f"
                    ./warmup1 sort warmup1data/f$f > f$f.sort
                    diff warmup1data/f$f.sort f$f.sort
                end

                /bin/rm -f f?.sort f??.sort
                foreach f (15 16 17 18 19 20 21 22 23 24 25 26 27 28 29)
                    echo "===> warmup1data/f$f"
                    cat warmup1data/f$f | ./warmup1 sort > f$f.sort
                    diff warmup1data/f$f.sort f$f.sort
                end
                /bin/rm -f f?.sort f??.sort
