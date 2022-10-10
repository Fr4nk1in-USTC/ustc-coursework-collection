#!/usr/bin/env bash

cache_dir="../output/std/"
input_file="../input/input.txt"
output_dir="../output/"

# Compile source code
echo "Compiling source code..."
make clean
make all

# Generate input and run executables
echo ""
echo "Generating inputs and running sort executables..."
./gen-input >/dev/null
for m in "heap" "quick" "merge" "counting"; do
	"./$m-sort" >/dev/null
done

# Generate standard results
echo ""
echo "Generating standard results..."
mkdir $cache_dir
for i in {3..18..3}; do
	len=$((1 << i))
	head --lines=$len $input_file |
		sort --numeric-sort >"$cache_dir/result_$i.txt"
done

# Test output correctness
echo ""
echo "Testing output correctness..."
for m in "heap" "quick" "merge" "counting"; do
	flag=1
	echo -n "    Testing $m-sort: "
	# Compare each result file with generated file
	for i in {3..18..3}; do
		if ! cmp -s "$cache_dir/result_$i.txt" \
			"$output_dir/$m""_sort/result_$i.txt"; then
			echo -n "$i "
			flag=0
		fi
	done
	if [ $flag -eq 1 ]; then
		echo "PASSED"
	else
		echo "FAILED"
	fi
done

# Remove standard results
echo ""
echo "Removing standard results..."
rm -rf $cache_dir
