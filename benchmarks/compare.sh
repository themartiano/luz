#!/bin/bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
	echo "Usage: $0 BEFORE.csv AFTER.csv" >&2
	exit 1
fi

before_file=$1
after_file=$2

if [[ ! -f "$before_file" ]]; then
	echo "Before CSV not found: $before_file" >&2
	exit 1
fi

if [[ ! -f "$after_file" ]]; then
	echo "After CSV not found: $after_file" >&2
	exit 1
fi

stats_for_case()
{
	local file=$1
	local benchmark_case=$2

	awk -F, -v benchmark_case="$benchmark_case" '
		NR > 1 && $1 == benchmark_case && $3 != "" {
			print $3
		}
	' "$file" | sort -n | awk '
		{
			values[++count] = $1
			sum += $1
		}
		END {
			if (count == 0) {
				exit 2
			}
			if (count % 2 == 1) {
				median = values[int(count / 2) + 1]
			} else {
				median = (values[count / 2] + values[count / 2 + 1]) / 2.0
			}
			mean = sum / count
			for (i = 1; i <= count; i++) {
				diff = values[i] - mean
				variance += diff * diff
			}
			stddev = sqrt(variance / count)
			printf "%d,%.6f,%.6f,%.6f,%.6f,%.6f", count, values[1], median, mean, values[count], stddev
		}
	'
}

config_for_case()
{
	local file=$1
	local benchmark_case=$2

	awk -F, -v benchmark_case="$benchmark_case" '
		NR > 1 && $1 == benchmark_case {
			printf "%s,%s,%s,%s,%s,%s", $5, $6, $7, $8, $9, $10
			exit
		}
	' "$file"
}

cases=$(
	{
		awk -F, 'NR > 1 && $1 != "" { print $1 }' "$before_file"
		awk -F, 'NR > 1 && $1 != "" { print $1 }' "$after_file"
	} | sort -u
)

echo "case,width,height,samples,bounces,threads,seed,before_runs,after_runs,before_median_ms,after_median_ms,speedup,improvement_percent,before_mean_ms,after_mean_ms,before_stddev_ms,after_stddev_ms,before_median_samples_per_second,after_median_samples_per_second"

for benchmark_case in $cases; do
	before_stats=$(stats_for_case "$before_file" "$benchmark_case" || true)
	after_stats=$(stats_for_case "$after_file" "$benchmark_case" || true)

	if [[ -z "$before_stats" || -z "$after_stats" ]]; then
		echo "Skipping $benchmark_case because it is missing from one CSV." >&2
		continue
	fi

	before_config=$(config_for_case "$before_file" "$benchmark_case")
	after_config=$(config_for_case "$after_file" "$benchmark_case")
	if [[ "$before_config" != "$after_config" ]]; then
		echo "Warning: $benchmark_case benchmark config differs between before and after CSVs." >&2
	fi

	IFS=',' read -r before_runs _ before_median before_mean _ before_stddev <<< "$before_stats"
	IFS=',' read -r after_runs _ after_median after_mean _ after_stddev <<< "$after_stats"
	IFS=',' read -r width height samples bounces threads seed <<< "$before_config"

	awk -v benchmark_case="$benchmark_case" \
		-v width="$width" \
		-v height="$height" \
		-v samples="$samples" \
		-v bounces="$bounces" \
		-v threads="$threads" \
		-v seed="$seed" \
		-v before_runs="$before_runs" \
		-v after_runs="$after_runs" \
		-v before_median="$before_median" \
		-v after_median="$after_median" \
		-v before_mean="$before_mean" \
		-v after_mean="$after_mean" \
		-v before_stddev="$before_stddev" \
		-v after_stddev="$after_stddev" '
		BEGIN {
			paths = width * height * samples
			speedup = before_median / after_median
			improvement = ((before_median - after_median) / before_median) * 100.0
			before_sps = paths / (before_median / 1000.0)
			after_sps = paths / (after_median / 1000.0)
			printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n", benchmark_case, width, height, samples, bounces, threads, seed, before_runs, after_runs, before_median, after_median, speedup, improvement, before_mean, after_mean, before_stddev, after_stddev, before_sps, after_sps
		}
	'
done
