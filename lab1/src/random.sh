#!/bin/bash

for i in $(seq 1 150); do
  echo $((RANDOM % 1000)) >> numbers.txt
done
