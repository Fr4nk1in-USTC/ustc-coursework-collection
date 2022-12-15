#!/usr/bin/env bash

src_dir=$(cd "$(dirname BASH_SOURCE[0])" && pwd)
latex_dir="$src_dir/../latex/"

# Clean LaTeX directory
mkdir -p "$latex_dir"
rm -f "$latex_dir/*"

# Compile Source Code
cd "$src_dir" || exit
make clean
make debug

printf "Generate New Inputs? [y/N]"
read -r generate_new_inputs
if [[ "$generate_new_inputs" == "y" ]]; then
	echo "Generating New Inputs..."
	./gen-input
else
	echo "Using Existing Inputs..."
fi
printf "Running Program..."
./main
echo "Done!"
printf "Generating LaTeX Output..."
cd "$latex_dir" || exit
pdflatex -interaction=nonstopmode -halt-on-error -file-line-error tree-original.tex >/dev/null
pdflatex -interaction=nonstopmode -halt-on-error -file-line-error tree-removed.tex >/dev/null
echo "Done!"
