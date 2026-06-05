#!/bin/bash
set -euo pipefail

cd "${BENCH_ROOT:-/Luz}"

make Luz >&2

repeat=${BENCH_REPEAT:-20}
warmup=${BENCH_WARMUP:-2}
seed=${BENCH_SEED:-424242424}
threads=${BENCH_THREADS:-1}
cases=${BENCH_CASES:-${BENCH_CASE:-default many-objects mesh-bvh diffuse postprocess atmosphere lights emissive-geometry primitives-materials volumes obj-mesh}}
global_width=${BENCH_WIDTH:-}
global_height=${BENCH_HEIGHT:-}
global_samples=${BENCH_SAMPLES:-}
global_bounces=${BENCH_BOUNCES:-}
global_gamma=${BENCH_GAMMA:-}
global_tonemapping=${BENCH_TONEMAPPING:-}
global_bloom=${BENCH_BLOOM:-}
score_sample_unit=${BENCH_SCORE_SAMPLE_UNIT:-1000}
git_commit=${BENCH_GIT_COMMIT:-unknown}
git_status=${BENCH_GIT_STATUS:-unknown}
system=$(uname -srm | tr ',' ';')
compiler=$(clang++ --version | head -n 1 | tr ',' ';')

if (( repeat <= 0 )); then
	echo "BENCH_REPEAT must be positive." >&2
	exit 1
fi

if (( warmup < 0 )); then
	echo "BENCH_WARMUP cannot be negative." >&2
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

case_defaults()
{
	local benchmark_case=$1
	case "$benchmark_case" in
		default)
			echo "${global_width:-200},${global_height:-200},${global_samples:-50},${global_bounces:-8},${global_gamma:-true},${global_tonemapping:-true},${global_bloom:-true}"
			;;
		many-objects)
			echo "${global_width:-160},${global_height:-160},${global_samples:-16},${global_bounces:-5},${global_gamma:-true},${global_tonemapping:-true},${global_bloom:-false}"
			;;
		mesh-bvh)
			echo "${global_width:-160},${global_height:-160},${global_samples:-16},${global_bounces:-5},${global_gamma:-true},${global_tonemapping:-true},${global_bloom:-false}"
			;;
		diffuse)
			echo "${global_width:-180},${global_height:-180},${global_samples:-32},${global_bounces:-7},${global_gamma:-true},${global_tonemapping:-true},${global_bloom:-false}"
			;;
		postprocess)
			echo "${global_width:-360},${global_height:-360},${global_samples:-2},${global_bounces:-1},${global_gamma:-true},${global_tonemapping:-true},${global_bloom:-true}"
			;;
		atmosphere)
			echo "${global_width:-160},${global_height:-160},${global_samples:-6},${global_bounces:-3},${global_gamma:-true},${global_tonemapping:-true},${global_bloom:-false}"
			;;
		lights)
			echo "${global_width:-180},${global_height:-180},${global_samples:-24},${global_bounces:-6},${global_gamma:-true},${global_tonemapping:-true},${global_bloom:-false}"
			;;
		emissive-geometry)
			echo "${global_width:-180},${global_height:-180},${global_samples:-18},${global_bounces:-5},${global_gamma:-true},${global_tonemapping:-true},${global_bloom:-true}"
			;;
		primitives-materials)
			echo "${global_width:-180},${global_height:-180},${global_samples:-24},${global_bounces:-7},${global_gamma:-true},${global_tonemapping:-true},${global_bloom:-false}"
			;;
		volumes)
			echo "${global_width:-160},${global_height:-160},${global_samples:-18},${global_bounces:-8},${global_gamma:-true},${global_tonemapping:-true},${global_bloom:-false}"
			;;
		obj-mesh)
			echo "${global_width:-160},${global_height:-160},${global_samples:-16},${global_bounces:-6},${global_gamma:-true},${global_tonemapping:-true},${global_bloom:-false}"
			;;
		*)
			echo "Unknown benchmark case: $benchmark_case" >&2
			exit 1
			;;
	esac
}

print_case_summary_and_score()
{
	local benchmark_case=$1
	local width=$2
	local height=$3
	local samples=$4
	shift 4
	printf '%s\n' "$@" | sort -n | awk -v benchmark_case="$benchmark_case" \
		-v width="$width" \
		-v height="$height" \
		-v samples="$samples" \
		-v score_sample_unit="$score_sample_unit" '
		{
			values[NR] = $1
			sum += $1
		}
		END {
			if (NR == 0) {
				exit 0
			}
			if (NR % 2 == 1) {
				median = values[int(NR / 2) + 1]
			} else {
				median = (values[NR / 2] + values[NR / 2 + 1]) / 2.0
			}
			mean = sum / NR
			for (i = 1; i <= NR; i++) {
				diff = values[i] - mean
				variance += diff * diff
			}
			stddev = sqrt(variance / NR)
			paths = width * height * samples
			samples_per_second = paths / (median / 1000.0)
			score = samples_per_second * 60.0 / score_sample_unit
			printf "summary case=%s min_ms=%.6f median_ms=%.6f mean_ms=%.6f max_ms=%.6f stddev_ms=%.6f\n", benchmark_case, values[1], median, mean, values[NR], stddev > "/dev/stderr"
			printf "score case=%s median_samples_per_second=%.6f score=%.2f\n", benchmark_case, samples_per_second, score > "/dev/stderr"
			printf "%.12f\n", score
		}
	'
}

print_overall_score()
{
	printf '%s\n' "$@" | awk '
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
			printf "score overall=%.2f\n", score > "/dev/stderr"
		}
	'
}

echo "case,run,elapsed_ms,samples_per_second,width,height,samples,bounces,threads,seed,gamma,tonemapping,bloom,git_commit,git_status,compiler,system"

case_scores=()
for benchmark_case in $cases; do
	IFS=',' read -r width height samples bounces gamma tonemapping bloom <<< "$(case_defaults "$benchmark_case")"

	benchmark_args=(
		--benchmark
		--benchmark-case "$benchmark_case"
		--seed "$seed"
		--threads "$threads"
		--resolution "${width}x${height}"
		--samples "$samples"
		--maxLightBounces "$bounces"
		--gamma "$gamma"
		--tonemapping "$tonemapping"
		--bloom "$bloom"
	)

	echo "benchmark=$benchmark_case seed=$seed threads=$threads resolution=${width}x${height} samples=$samples bounces=$bounces gamma=$gamma tonemapping=$tonemapping bloom=$bloom warmup=$warmup repeat=$repeat" >&2

	for ((n = 0; n < warmup; n++)); do
		./Luz "${benchmark_args[@]}" >/dev/null
	done

	times=()
	total_paths=$((width * height * samples))
	for ((n = 1; n <= repeat; n++)); do
		elapsed_ms=$(./Luz "${benchmark_args[@]}")
		times+=("$elapsed_ms")
		samples_per_second=$(awk -v paths="$total_paths" -v ms="$elapsed_ms" 'BEGIN { printf "%.6f", paths / (ms / 1000.0) }')
		echo "$benchmark_case,$n,$elapsed_ms,$samples_per_second,$width,$height,$samples,$bounces,$threads,$seed,$gamma,$tonemapping,$bloom,$git_commit,$git_status,$compiler,$system"
	done

	case_score=$(print_case_summary_and_score "$benchmark_case" "$width" "$height" "$samples" "${times[@]}")
	case_scores+=("$case_score")
done

print_overall_score "${case_scores[@]}"
