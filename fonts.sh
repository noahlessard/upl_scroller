#!/bin/bash

mkdir -p fonts

# Generate A-Z with TRANSPARENT background
for letter in {A..Z}; do
    convert -size 200x200 xc:none -font pix.ttf -pointsize 150 \
            -fill white -gravity center -annotate +0+0 "$letter" \
            "fonts/letter_$letter.png"
    echo "Generated letter_$letter.png"
done

# Generate a-z
for letter in {a..z}; do
    convert -size 200x200 xc:none -font pix.ttf -pointsize 150 \
            -fill white -gravity center -annotate +0+0 "$letter" \
            "fonts/letter_$letter.png"
    echo "Generated letter_$letter.png"
done

# Generate 0-9
for num in {0..9}; do
    convert -size 200x200 xc:none -font pix.ttf -pointsize 150 \
            -fill white -gravity center -annotate +0+0 "$num" \
            "fonts/letter_$num.png"
    echo "Generated letter_$num.png"
done

echo "Done! All characters generated in ./fonts/"