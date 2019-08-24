#!/bin/bash

cd pages

for file in *; do
    cat ../template/begin.html "$file" ../template/end.html > ../"$file"
done
