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

stats_for_column()
{
	local results_file=$1
	local benchmark_case=$2
	local column_name=$3

	awk -F, -v benchmark_case="$benchmark_case" -v column_name="$column_name" '
		NR == 1 {
			for (i = 1; i <= NF; i++) {
				if ($i == column_name) {
					column = i
				}
			}
			if (column == 0) {
				exit 3
			}
			next
		}
		NR > 1 && $1 == benchmark_case && $column != "" {
			print $column
		}
	' "$results_file" | sort -n | awk '
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

stats_count()
{
	local stats=$1

	if [[ -z "$stats" ]]; then
		return
	fi
	IFS=',' read -r count _ _ _ _ _ <<< "$stats"
	printf '%s' "$count"
}

stats_median()
{
	local stats=$1

	if [[ -z "$stats" ]]; then
		return
	fi
	IFS=',' read -r _ _ median _ _ _ <<< "$stats"
	printf '%s' "$median"
}

config_for_case()
{
	local results_file=$1
	local benchmark_case=$2

	awk -F, -v benchmark_case="$benchmark_case" '
		NR == 1 {
			for (i = 1; i <= NF; i++) {
				columns[$i] = i
			}
			next
		}
		NR > 1 && $1 == benchmark_case {
			printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", field("width"), field("height"), field("samples"), field("bounces"), field("threads"), field("seed"), field("view_transform"), field("bloom"), field("adaptive"), field("denoise")
			exit
		}
		function field(name, field_index) {
			field_index = columns[name]
			if (field_index > 0) {
				return $field_index
			}
			return ""
		}
	' "$results_file"
}

score_fields_from_elapsed()
{
	local width=$1
	local height=$2
	local samples=$3
	local median_ms=$4

	awk -v width="$width" \
		-v height="$height" \
		-v samples="$samples" \
		-v median_ms="$median_ms" \
		-v score_sample_unit="$score_sample_unit" '
		BEGIN {
			if (width <= 0 || height <= 0 || samples <= 0 || median_ms <= 0) {
				printf ","
				exit
			}
			paths = width * height * samples
			samples_per_second = paths / (median_ms / 1000.0)
			score = samples_per_second * 60.0 / score_sample_unit
			printf "%.6f,%.2f", samples_per_second, score
		}
	'
}

score_from_samples_per_second()
{
	local samples_per_second=$1

	awk -v samples_per_second="$samples_per_second" \
		-v score_sample_unit="$score_sample_unit" '
		BEGIN {
			if (samples_per_second <= 0) {
				exit
			}
			printf "%.2f", samples_per_second * 60.0 / score_sample_unit
		}
	'
}

render_score_fields()
{
	local rendered_samples=$1
	local render_ms=$2
	local width=$3
	local height=$4
	local samples=$5

	awk -v rendered_samples="$rendered_samples" \
		-v render_ms="$render_ms" \
		-v width="$width" \
		-v height="$height" \
		-v samples="$samples" \
		-v score_sample_unit="$score_sample_unit" '
		BEGIN {
			if (rendered_samples <= 0 && width > 0 && height > 0 && samples > 0) {
				rendered_samples = width * height * samples
			}
			if (rendered_samples <= 0 || render_ms <= 0) {
				printf ","
				exit
			}
			samples_per_second = rendered_samples / (render_ms / 1000.0)
			score = samples_per_second * 60.0 / score_sample_unit
			printf "%.6f,%.2f", samples_per_second, score
		}
	'
}

render_time_fields()
{
	local elapsed_ms=$1
	local render_ms=$2

	awk -v elapsed_ms="$elapsed_ms" -v render_ms="$render_ms" '
		BEGIN {
			if (elapsed_ms <= 0 || render_ms <= 0) {
				printf ","
				exit
			}
			printf "%.6f,%.6f", elapsed_ms - render_ms, (render_ms / elapsed_ms) * 100.0
		}
	'
}

ratio_percent()
{
	local numerator=$1
	local denominator=$2

	awk -v numerator="$numerator" -v denominator="$denominator" '
		BEGIN {
			if (numerator <= 0 || denominator <= 0) {
				exit
			}
			printf "%.6f", (numerator / denominator) * 100.0
		}
	'
}

geomean()
{
	printf '%s' "$1" | awk '
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
			printf "%.2f", exp(sum_log / count)
		}
	'
}

print_overall_row()
{
	local score=$1
	local actual_score=$2
	local render_score=$3
	local render_score_over_score

	render_score_over_score=$(ratio_percent "$render_score" "$score")

	awk -v score="$score" -v actual_score="$actual_score" -v render_score="$render_score" -v render_score_over_score="$render_score_over_score" '
		BEGIN {
			columns = 26
			fields[1] = "overall"
			fields[19] = score
			fields[21] = actual_score
			fields[23] = render_score
			fields[26] = render_score_over_score
			for (i = 1; i <= columns; i++) {
				if (i > 1) {
					printf ","
				}
				printf "%s", fields[i]
			}
			printf "\n"
		}
	'
}

cases=$(awk -F, 'NR > 1 && $1 != "" { print $1 }' "$results_file" | sort -u)

echo "case,width,height,samples,bounces,threads,seed,view_transform,bloom,adaptive,denoise,runs,median_elapsed_ms,median_render_ms,median_total_ms,median_average_spp,median_samples_per_second,score,median_actual_samples_per_second,actual_score,render_samples_per_second,render_score,median_non_render_ms,render_time_share_percent,render_score_over_score_percent"

scores=""
actual_scores=""
render_scores=""
for benchmark_case in $cases; do
	elapsed_stats=$(stats_for_column "$results_file" "$benchmark_case" "elapsed_ms" || true)
	if [[ -z "$elapsed_stats" ]]; then
		echo "Skipping $benchmark_case because it has no elapsed_ms data." >&2
		continue
	fi

	config=$(config_for_case "$results_file" "$benchmark_case")
	IFS=',' read -r width height samples bounces threads seed view_transform bloom adaptive denoise <<< "$config"

	runs=$(stats_count "$elapsed_stats")
	median_elapsed_ms=$(stats_median "$elapsed_stats")
	render_stats=$(stats_for_column "$results_file" "$benchmark_case" "render_ms" || true)
	total_stats=$(stats_for_column "$results_file" "$benchmark_case" "total_ms" || true)
	average_spp_stats=$(stats_for_column "$results_file" "$benchmark_case" "average_spp" || true)
	actual_sps_stats=$(stats_for_column "$results_file" "$benchmark_case" "actual_samples_per_second" || true)
	rendered_samples_stats=$(stats_for_column "$results_file" "$benchmark_case" "rendered_samples" || true)

	median_render_ms=$(stats_median "$render_stats")
	median_total_ms=$(stats_median "$total_stats")
	median_average_spp=$(stats_median "$average_spp_stats")
	median_actual_samples_per_second=$(stats_median "$actual_sps_stats")
	median_rendered_samples=$(stats_median "$rendered_samples_stats")
	if [[ -z "$median_average_spp" ]]; then
		median_average_spp=$samples
	fi
	if [[ -z "$median_total_ms" ]]; then
		median_total_ms=$median_elapsed_ms
	fi

	score_fields=$(score_fields_from_elapsed "$width" "$height" "$samples" "$median_elapsed_ms")
	IFS=',' read -r median_samples_per_second score <<< "$score_fields"
	if [[ -z "$median_actual_samples_per_second" ]]; then
		median_actual_samples_per_second=$median_samples_per_second
	fi
	actual_score=$(score_from_samples_per_second "$median_actual_samples_per_second")
	render_score_fields=$(render_score_fields "$median_rendered_samples" "$median_render_ms" "$width" "$height" "$samples")
	IFS=',' read -r render_samples_per_second render_score <<< "$render_score_fields"
	render_time_fields=$(render_time_fields "$median_elapsed_ms" "$median_render_ms")
	IFS=',' read -r median_non_render_ms render_time_share_percent <<< "$render_time_fields"
	render_score_over_score_percent=$(ratio_percent "$render_score" "$score")

	printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n" \
		"$benchmark_case" "$width" "$height" "$samples" "$bounces" "$threads" "$seed" "$view_transform" "$bloom" "$adaptive" "$denoise" "$runs" "$median_elapsed_ms" "$median_render_ms" "$median_total_ms" "$median_average_spp" "$median_samples_per_second" "$score" "$median_actual_samples_per_second" "$actual_score" "$render_samples_per_second" "$render_score" "$median_non_render_ms" "$render_time_share_percent" "$render_score_over_score_percent"
	scores+="${score}"$'\n'
	actual_scores+="${actual_score}"$'\n'
	if [[ -n "$render_score" ]]; then
		render_scores+="${render_score}"$'\n'
	fi
done

if [[ -z "$scores" ]]; then
	echo "No benchmark cases to score." >&2
	exit 1
fi

overall_score=$(geomean "$scores")
overall_actual_score=$(geomean "$actual_scores")
overall_render_score=""
if [[ -n "$render_scores" ]]; then
	overall_render_score=$(geomean "$render_scores")
fi
print_overall_row "$overall_score" "$overall_actual_score" "$overall_render_score"
