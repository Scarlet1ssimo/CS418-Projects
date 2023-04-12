#!/bin/zsh

for allll in test/mpray_*.png; do
    st1=$(echo "${allll##*_}" | cut -d '.' -f1)
    flag=0
    for file in result/look_at_this_*.png; do
        st2=$(echo "${file##*_}" | cut -d '.' -f1)
        if [[ "$st1" == "$st2" ]]; then
            flag=1
            break
        fi
    done
    if [[ $flag -eq 0 ]]; then
        echo "${allll##*_}" | cut -d '.' -f1
    fi
done
