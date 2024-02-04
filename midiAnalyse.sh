#!/bin/bash
# Midi File Analyzer for future Options of MihDi

declare -a N=("C" "C#" "D" "D#" "E" "F" "F#" "G" "G#" "A" "A#" "B")

if [ -z "$1" ]; then
 F1="bach_850_format0.mid"
else
 F1="$1"
fi

F2="$F1.ly"


if [ "v1" == true ]; then
  midi2ly "$F1" -o "$F2"
  cat "$F2" | grep tempo | awk '{print $4}' | sort -n | uniq -c | sort -n
else
  M=$(midi2abc "$F1" -midigram)
  if [ -n "$2" ]; then
    echo "$M"
  fi
  echo "$M" | awk '{print $5}' | sort -n | uniq -c | sort -n
  echo "-----------"
  echo "$M" | awk '{print $5%12}' | sort -n | uniq -c | sort -n
fi

echo "$N[3]"
