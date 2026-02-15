#!/bin/bash

# Loop through Digits
for upper in {A..Z}; do
    
    # Construct the filename based on your requested format
    FILE="pngs/ets_font/upper_${upper}.png"

    # Check if the file actually exists before running the tool
    if [ -f "$FILE" ]; then
        # echo "Processing: $char"
        ./sprite_tool_m "$FILE" upper_"$upper"
    else
        echo "Warning: File '$FILE' not found, skipping..."
    fi

done

# Loop through Digits, Lowercase, and Uppercase
for lower in {a..z}; do
    
    # Construct the filename based on your requested format
    FILE="pngs/ets_font/lower_${lower}.png"

    # Check if the file actually exists before running the tool
    if [ -f "$FILE" ]; then
        # echo "Processing: $lower"
        ./sprite_tool_m "$FILE" lower_"$lower"
    else
        echo "Warning: File '$FILE' not found, skipping..."
    fi

done