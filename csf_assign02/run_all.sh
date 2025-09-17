#! /usr/bin/env bash

# Run c_imgproc or asm_imgproc on test input and check whether
# the correct output images are produced.

error_count="0"

run_test() {
  local exe="./run_test.rb"
  echo -n "Running '${exe} $@'..."
  local out_stem=$(echo "$@" | tr ' ./' '___')
  local out_file="actual/${out_stem}.out"
  local err_file="actual/${out_stem}.err"
  ${exe} "$@" > ${out_file} 2> ${err_file}
  if [[ $? -ne 0 ]]; then
    echo "FAILED"
    error_count=$((${error_count} + 1))
  else
    echo "passed"
  fi
}

if [[ $# != 1 ]]; then
  >&2 echo "Usage: ./run_all.sh <exe version>"
  >&2 echo "  <exe version> is 'c' or 'asm'"
  exit 1
fi

exe_version="$1"

mkdir -p actual

run_test ${exe_version} ingo complement
run_test ${exe_version} ingo transpose
run_test ${exe_version} ingo ellipse
run_test ${exe_version} ingo emboss

run_test ${exe_version} kittens complement
run_test ${exe_version} kittens ellipse
run_test ${exe_version} kittens emboss

run_test ${exe_version} dice complement
run_test ${exe_version} dice ellipse
run_test ${exe_version} dice emboss

if [[ ${error_count} -eq 0 ]]; then
  echo "All tests passed!"
  exit 0
else
  echo "${error_count} test(s) failed\n"
  exit 1
fi
