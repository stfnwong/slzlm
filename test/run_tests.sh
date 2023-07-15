#!/bin/bash
# Run unit tests


for t in bin/*; do
    echo $t && ./$t || rc=$?
    if [[ rc -ne 0 ]] ; then
        exit $rc
    fi
done
