#!/bin/bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
	echo "Usage: $0 BEFORE.csv AFTER.csv" >&2
	exit 1
fi

before_file=$1
after_file=$2
score_sample_unit=${BENCH_SCORE_SAMPLE_UNIT:-1000}

if [[ ! -f "$before_file" ]]; then
	echo "Before CSV not found: $before_file" >&2
	exit 1
fi

if [[ ! -f "$after_file" ]]; then
	echo "After CSV not found: $after_file" >&2
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
	local file=$1
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

stats_mean()
{
	local stats=$1

	if [[ -z "$stats" ]]; then
		return
	fi
	IFS=',' read -r _ _ _ mean _ _ <<< "$stats"
	printf '%s' "$mean"
}

stats_stddev()
{
	local stats=$1

	if [[ -z "$stats" ]]; then
		return
	fi
	IFS=',' read -r _ _ _ _ _ stddev <<< "$stats"
	printf '%s' "$stddev"
}

config_for_case()
{
	local file=$1
	local benchmark_case=$2

	awk -F, -v benchmark_case="$benchmark_case" '
		NR == 1 {
			for (i = 1; i <= NF; i++) {
				columns[$i] = i
			}
			next
		}
		NR > 1 && $1 == benchmark_case {
			printf "%s,%s,%s,%s,%s,%s,%s,%s", field("width"), field("height"), field("samples"), field("bounces"), field("threads"), field("seed"), field("adaptive"), field("denoise")
			exit
		}
		function field(name, field_index) {
			field_index = columns[name]
			if (field_index > 0) {
				return $field_index
			}
			return ""
		}
	' "$file"
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

case_metrics()
{
	local file=$1
	local benchmark_case=$2
	local width=$3
	local height=$4
	local samples=$5

	local elapsed_stats render_stats average_spp_stats actual_sps_stats rendered_samples_stats
	local runs elapsed_median elapsed_mean elapsed_stddev render_median average_spp
	local score_fields nominal_sps score actual_sps actual_score rendered_samples render_score_values render_score

	elapsed_stats=$(stats_for_column "$file" "$benchmark_case" "elapsed_ms" || true)
	if [[ -z "$elapsed_stats" ]]; then
		return 2
	fi
	render_stats=$(stats_for_column "$file" "$benchmark_case" "render_ms" || true)
	average_spp_stats=$(stats_for_column "$file" "$benchmark_case" "average_spp" || true)
	actual_sps_stats=$(stats_for_column "$file" "$benchmark_case" "actual_samples_per_second" || true)
	rendered_samples_stats=$(stats_for_column "$file" "$benchmark_case" "rendered_samples" || true)

	runs=$(stats_count "$elapsed_stats")
	elapsed_median=$(stats_median "$elapsed_stats")
	elapsed_mean=$(stats_mean "$elapsed_stats")
	elapsed_stddev=$(stats_stddev "$elapsed_stats")
	render_median=$(stats_median "$render_stats")
	average_spp=$(stats_median "$average_spp_stats")
	actual_sps=$(stats_median "$actual_sps_stats")
	rendered_samples=$(stats_median "$rendered_samples_stats")
	if [[ -z "$average_spp" ]]; then
		average_spp=$samples
	fi

	score_fields=$(score_fields_from_elapsed "$width" "$height" "$samples" "$elapsed_median")
	IFS=',' read -r nominal_sps score <<< "$score_fields"
	if [[ -z "$actual_sps" ]]; then
		actual_sps=$nominal_sps
	fi
	actual_score=$(score_from_samples_per_second "$actual_sps")
	render_score_values=$(render_score_fields "$rendered_samples" "$render_median" "$width" "$height" "$samples")
	IFS=',' read -r _ render_score <<< "$render_score_values"

	printf "%s,%s,%s,%s,%s,%s,%s,%s,%s" "$runs" "$elapsed_median" "$elapsed_mean" "$elapsed_stddev" "$render_median" "$average_spp" "$score" "$actual_score" "$render_score"
}

lower_is_better_fields()
{
	local before=$1
	local after=$2

	awk -v before="$before" -v after="$after" '
		BEGIN {
			if (before <= 0 || after <= 0) {
				printf ","
				exit
			}
			speedup = before / after
			improvement = ((before - after) / before) * 100.0
			printf "%.6f,%.6f", speedup, improvement
		}
	'
}

higher_is_better_fields()
{
	local before=$1
	local after=$2

	awk -v before="$before" -v after="$after" '
		BEGIN {
			if (before <= 0 || after <= 0) {
				printf ","
				exit
			}
			speedup = after / before
			improvement = ((after - before) / before) * 100.0
			printf "%.6f,%.6f", speedup, improvement
		}
	'
}

render_time_share_percent()
{
	local render_ms=$1
	local elapsed_ms=$2

	awk -v render_ms="$render_ms" -v elapsed_ms="$elapsed_ms" '
		BEGIN {
			if (render_ms <= 0 || elapsed_ms <= 0) {
				exit
			}
			printf "%.6f", (render_ms / elapsed_ms) * 100.0
		}
	'
}

relative_delta_percent()
{
	local reference=$1
	local value=$2

	awk -v reference="$reference" -v value="$value" '
		BEGIN {
			if (reference <= 0 || value <= 0) {
				exit
			}
			printf "%.6f", ((value / reference) - 1.0) * 100.0
		}
	'
}

point_delta()
{
	local before=$1
	local after=$2

	awk -v before="$before" -v after="$after" '
		BEGIN {
			if (before == "" || after == "") {
				exit
			}
			printf "%.6f", after - before
		}
	'
}

cases=$(
	{
		awk -F, 'NR > 1 && $1 != "" { print $1 }' "$before_file"
		awk -F, 'NR > 1 && $1 != "" { print $1 }' "$after_file"
	} | sort -u
)

echo "case,width,height,samples,bounces,threads,seed,adaptive,denoise,before_runs,after_runs,before_elapsed_ms,after_elapsed_ms,elapsed_speedup,elapsed_improvement_percent,before_render_ms,after_render_ms,render_time_speedup,render_time_improvement_percent,before_average_spp,after_average_spp,before_score,after_score,score_speedup,score_improvement_percent,before_actual_score,after_actual_score,actual_score_speedup,actual_score_improvement_percent,before_render_score,after_render_score,render_score_speedup,render_score_improvement_percent,score_vs_render_time_speedup_delta_percent,actual_score_vs_render_time_speedup_delta_percent,render_score_vs_render_time_speedup_delta_percent,before_render_time_share_percent,after_render_time_share_percent,render_time_share_delta_points,before_elapsed_mean_ms,after_elapsed_mean_ms,before_elapsed_stddev_ms,after_elapsed_stddev_ms"

for benchmark_case in $cases; do
	before_config=$(config_for_case "$before_file" "$benchmark_case")
	after_config=$(config_for_case "$after_file" "$benchmark_case")

	if [[ -z "$before_config" || -z "$after_config" ]]; then
		echo "Skipping $benchmark_case because it is missing from one CSV." >&2
		continue
	fi
	if [[ "$before_config" != "$after_config" ]]; then
		echo "Warning: $benchmark_case benchmark config differs between before and after CSVs." >&2
	fi

	IFS=',' read -r before_width before_height before_samples before_bounces before_threads before_seed before_adaptive before_denoise <<< "$before_config"
	IFS=',' read -r after_width after_height after_samples after_bounces after_threads after_seed after_adaptive after_denoise <<< "$after_config"
	width=${before_width:-$after_width}
	height=${before_height:-$after_height}
	samples=${before_samples:-$after_samples}
	bounces=${before_bounces:-$after_bounces}
	threads=${before_threads:-$after_threads}
	seed=${before_seed:-$after_seed}
	adaptive=${before_adaptive:-$after_adaptive}
	denoise=${before_denoise:-$after_denoise}

	before_metrics=$(case_metrics "$before_file" "$benchmark_case" "$width" "$height" "$samples" || true)
	after_metrics=$(case_metrics "$after_file" "$benchmark_case" "$width" "$height" "$samples" || true)
	if [[ -z "$before_metrics" || -z "$after_metrics" ]]; then
		echo "Skipping $benchmark_case because it has no timing data in one CSV." >&2
		continue
	fi
	IFS=',' read -r before_runs before_elapsed before_elapsed_mean before_elapsed_stddev before_render before_average_spp before_score before_actual_score before_render_score <<< "$before_metrics"
	IFS=',' read -r after_runs after_elapsed after_elapsed_mean after_elapsed_stddev after_render after_average_spp after_score after_actual_score after_render_score <<< "$after_metrics"

	elapsed_comparison=$(lower_is_better_fields "$before_elapsed" "$after_elapsed")
	render_comparison=$(lower_is_better_fields "$before_render" "$after_render")
	score_comparison=$(higher_is_better_fields "$before_score" "$after_score")
	actual_score_comparison=$(higher_is_better_fields "$before_actual_score" "$after_actual_score")
	render_score_comparison=$(higher_is_better_fields "$before_render_score" "$after_render_score")
	IFS=',' read -r elapsed_speedup elapsed_improvement <<< "$elapsed_comparison"
	IFS=',' read -r render_speedup render_improvement <<< "$render_comparison"
	IFS=',' read -r score_speedup score_improvement <<< "$score_comparison"
	IFS=',' read -r actual_score_speedup actual_score_improvement <<< "$actual_score_comparison"
	IFS=',' read -r render_score_speedup render_score_improvement <<< "$render_score_comparison"
	score_vs_render_time_speedup_delta=$(relative_delta_percent "$render_speedup" "$score_speedup")
	actual_score_vs_render_time_speedup_delta=$(relative_delta_percent "$render_speedup" "$actual_score_speedup")
	render_score_vs_render_time_speedup_delta=$(relative_delta_percent "$render_speedup" "$render_score_speedup")
	before_render_time_share=$(render_time_share_percent "$before_render" "$before_elapsed")
	after_render_time_share=$(render_time_share_percent "$after_render" "$after_elapsed")
	render_time_share_delta=$(point_delta "$before_render_time_share" "$after_render_time_share")

	printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n" \
		"$benchmark_case" "$width" "$height" "$samples" "$bounces" "$threads" "$seed" "$adaptive" "$denoise" \
		"$before_runs" "$after_runs" "$before_elapsed" "$after_elapsed" "$elapsed_speedup" "$elapsed_improvement" \
		"$before_render" "$after_render" "$render_speedup" "$render_improvement" \
		"$before_average_spp" "$after_average_spp" \
		"$before_score" "$after_score" "$score_speedup" "$score_improvement" \
		"$before_actual_score" "$after_actual_score" "$actual_score_speedup" "$actual_score_improvement" \
		"$before_render_score" "$after_render_score" "$render_score_speedup" "$render_score_improvement" \
		"$score_vs_render_time_speedup_delta" "$actual_score_vs_render_time_speedup_delta" "$render_score_vs_render_time_speedup_delta" \
		"$before_render_time_share" "$after_render_time_share" "$render_time_share_delta" \
		"$before_elapsed_mean" "$after_elapsed_mean" "$before_elapsed_stddev" "$after_elapsed_stddev"
done
