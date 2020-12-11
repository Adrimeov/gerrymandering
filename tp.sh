#!/bin/sh

#./executable "$@

while getopts ":e:c:p" opt; do
  case $opt in
    e) path="$OPTARG"
    ;;
    c) nbDistricts="$OPTARG"
    ;;
    p) print="True"
    ;;
    \?) echo "Invalid option -$OPTARG" >&2
    ;;
  esac
done

python3 ./main.py --path "$path" --nb_districts "$nbDistricts" --print_ "$print"



