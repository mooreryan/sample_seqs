#!/usr/bin/env bash

src_dir="@PROJECT_SOURCE_DIR@"
test_files="$src_dir"/test_files
expected_outdir="$src_dir"/test_files/expected
actual_outdir="$src_dir"/test_files/actual

# Remove the actual test outdir if it exists.
if [ -d "$actual_outdir" ]; then
  rm -r "$actual_outdir"
fi

exe="@CMAKE_INSTALL_PREFIX@"/bin/sample_seqs

forward_reads="$test_files"/test.1.fq.gz
reverse_reads="$test_files"/test.2.fq.gz
single_reads="$test_files"/test.U.fq.gz

percent_to_sample=10
num_samples=3
seed=1234
outdir="$actual_outdir"
basename=lala

cmd="$exe \
-p $percent_to_sample \
-n $num_samples \
-r $seed \
-o $outdir \
-b $basename \
-1 $forward_reads \
-2 $reverse_reads \
-s $single_reads"

# No quotes here to actually run the command.
$cmd

for f in `ls "$actual_outdir"/*.fq`; do
    base=$(basename -- "$f")

    diff "$f" "$expected_outdir/$base"

    if [ $? -ne 0 ]; then
        echo "diff failed for $f and $expected_outdir/$base"

        exit 111
    fi
done