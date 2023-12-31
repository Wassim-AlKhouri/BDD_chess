#!/bin/bash

input_file="test9.pgn"

# Use awk to separate games and store them in an array
mapfile -t games < <(awk -v RS= '{gsub(/\n/," ")}1' "$input_file")

# Iterate through the array of games
for game in "${games[@]}"; do
    if [[ $game == "1."* ]]; then
        echo "INSERT INTO tcg VALUES ('$game');" >> insert8.sql
    fi
done
