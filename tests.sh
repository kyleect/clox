#!/bin/bash

bin="./build/clox"           # The application (from command arg)
diff="diff -iad"   # Diff command, or what ever

mkdir -p ./tests/.tmp

# An array, do not have to declare it, but is supposedly faster
declare -a file_base=("add" "bool_false" "bool_true" "nil" "number" "number_negative" "string" "string_concat")

# Loop the array
for file in "${file_base[@]}"; do
    # Padd file_base with suffixes
    file_in="./tests/$file.in.lox"             # The in file
    file_out_val="./tests/$file.out.expected"       # The out file to check against
    file_out_tst="./tests/.tmp/$file.out.actual"   # The outfile from test application

    # Validate infile exists (do the same for out validate file)
    if [ ! -f "$file_in" ]; then
        printf "In file %s is missing\n" "$file_in"
        continue;
    fi
    if [ ! -f "$file_out_val" ]; then
        printf "Validation file %s is missing\n" "$file_out_val"
        continue;
    fi

    printf "Test: %s\n" "$file_in"

    # # Run application, redirect in file to app, and output to out file
    "$bin" "$file_in" > "$file_out_tst"

    # Execute diff
    $diff "$file_out_tst" "$file_out_val"


    # Check exit code from previous command (ie diff)
    # We need to add this to a variable else we can't print it
    # as it will be changed by the if [
    # Iff not 0 then the files differ (at least with diff)
    e_code=$?
    if [ $e_code != 0 ]; then
            printf "TEST FAIL : %d\n" "$e_code"
    else
            printf "TEST OK!\n"
    fi

    printf "\n"
done

# Clean exit with status 0
exit 0