#!/bin/bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
	echo "Usage: $0 RESULTS.csv" >&2
	exit 1
fi

results_file=$1
score_sample_unit=${BENCH_SCORE_SAMPLE_UNIT:-1000}

if [[ ! -f "$results_file" ]]; then
	echo "Results CSV not found: $results_file" >&2
	exit 1
fi

awk -v score_sample_unit="$score_sample_unit" '
	BEGIN {
		if (score_sample_unit !~ /^[0-9]+(\.[0-9]+)?$/ || score_sample_unit <= 0) {
			exit 1
		}
	}
' || {
	echo "BENCH_SCORE_SAMPLE_UNIT must be positive." >&2
	exit 1
}

stats_for_case()
{
	local benchmark_case=$1

	awk -F, -v benchmark_case="$benchmark_case" '
		NR > 1 && $1 == benchmark_case && $3 != "" {
			print $3
		}
	' "$results_file" | sort -n | awk '
		{
			values[++count] = $1
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
			printf "%d,%.6f", count, median
		}
	'
}

config_for_case()
{
	local benchmark_case=$1

	awk -F, -v benchmark_case="$benchmark_case" '
		NR > 1 && $1 == benchmark_case {
			printf "%s,%s,%s,%s,%s,%s,%s,%s,%s", $5, $6, $7, $8, $9, $10, $11, $12, $13
			exit
		}
	' "$results_file"
}

cases=$(awk -F, 'NR > 1 && $1 != "" { print $1 }' "$results_file" | sort -u)

echo "case,width,height,samples,bounces,threads,seed,gamma,tonemapping,bloom,runs,median_ms,median_samples_per_second,score"

scores=""
for benchmark_case in $cases; do
	stats=$(stats_for_case "$benchmark_case" || true)
	if [[ -z "$stats" ]]; then
		echo "Skipping $benchmark_case because it has no timing data." >&2
		continue
	fi

	config=$(config_for_case "$benchmark_case")
	IFS=',' read -r runs median_ms <<< "$stats"
	IFS=',' read -r width height samples bounces threads seed gamma tonemapping bloom <<< "$config"

	score_fields=$(awk -v benchmark_case="$benchmark_case" \
		-v width="$width" \
		-v height="$height" \
		-v samples="$samples" \
		-v median_ms="$median_ms" \
		-v score_sample_unit="$score_sample_unit" '
		BEGIN {
			if (width <= 0 || height <= 0 || samples <= 0 || median_ms <= 0) {
				printf "Invalid benchmark data for %s.\n", benchmark_case > "/dev/stderr"
				exit 1
			}
			paths = width * height * samples
			samples_per_second = paths / (median_ms / 1000.0)
			score = samples_per_second * 60.0 / score_sample_unit
			printf "%.6f,%.2f", samples_per_second, score
		}
	')
	IFS=',' read -r samples_per_second score <<< "$score_fields"

	printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%.6f,%s,%s\n" "$benchmark_case" "$width" "$height" "$samples" "$bounces" "$threads" "$seed" "$gamma" "$tonemapping" "$bloom" "$runs" "$median_ms" "$samples_per_second" "$score"
	scores+="${score}"$'\n'
done

if [[ -z "$scores" ]]; then
	echo "No benchmark cases to score." >&2
	exit 1
fi

printf '%s' "$scores" | awk '
	{
		if ($1 <= 0) {
			exit 1
		}
		sum_log += log($1)
		count++
	}
	END {
		if (count == 0) {
			exit 1
		}
		score = exp(sum_log / count)
		printf "overall,,,,,,,,,,,,,%.2f\n", score
	}
'
