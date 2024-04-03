# Common flags for both commands
common_flags="-t -P 1000 -p 1000 -g 10 -d "data/italy.tsp""

# Execute ./bin/tspg_seq
echo "Executing ./bin/tspg_seq $common_flags"
./bin/tspg_seq $common_flags

# Iterate over the values of $i for ./bin/tspg_par_native
for ((i = 2; i <= 64; i *= 2)); do 
    # Concatenate common flags with the changing flag
    flags="$common_flags -w $i"
    
    # Execute the command with the concatenated flags
    echo "Executing ./bin/tspg_par_native $flags"
    ./bin/tspg_par_native $flags
done