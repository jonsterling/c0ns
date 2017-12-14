#!/bin/bash
for ii in `cat /usr/share/dict/words`; do
    echo www.$ii.com A
    echo www.$ii.net A
    echo www.$ii.org A
done
