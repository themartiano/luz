#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Usage: tools/zip_current_dir_to_ssh.sh [options] TARGET

Zip the current working directory, excluding .git and compiled/build binaries,
copy the archive to TARGET with scp, then remove the local temporary archive.

TARGET is any scp destination, for example:
  user@example.com:/tmp/
  user@example.com:/tmp/luz.zip
  user@example.com

If TARGET has no colon, the archive is copied to the remote user's home
directory, matching normal ssh-style targets.

Options:
  -i, --identity-file FILE
                   SSH private key to pass to scp, same as ssh -i FILE.
                   If FILE is inside the current directory, it is excluded from
                   the archive.
  --name NAME.zip   Use NAME.zip for the temporary archive name.
  --keep           Keep the local temporary archive after copying.
  --dry-run        Build the archive and print the scp action without copying.
  -h, --help       Show this help.
USAGE
}

die() {
  printf 'error: %s\n' "$*" >&2
  exit 1
}

require_command() {
  command -v "$1" >/dev/null 2>&1 || die "missing required command: $1"
}

is_compiled_binary() {
  local path="$1"
  local kind

  kind=$(file -b "$path" 2>/dev/null || true)

  case "$kind" in
    *Mach-O* | \
    *ELF\ * | \
    *PE32* | \
    *COFF\ object* | \
    *relocatable* | \
    *current\ ar\ archive* | \
    *shared\ object* | \
    *dynamically\ linked\ shared\ library*)
      return 0
      ;;
  esac

  return 1
}

should_skip_file() {
  local path="$1"

  if [[ -n "${identity_file_abs:-}" ]]; then
    local abs_path="$source_dir/${path#./}"

    if [[ "$abs_path" == "$identity_file_abs" ]]; then
      return 0
    fi
  fi

  case "$path" in
    ./.DS_Store | \
    */.DS_Store | \
    *.o | \
    *.a | \
    *.so | \
    *.dylib | \
    *.dll | \
    *.exe | \
    *.out | \
    *.class | \
    *.pyc | \
    *.pyo)
      return 0
      ;;
  esac

  is_compiled_binary "$path"
}

build_manifest() {
  local manifest="$1"

  : >"$manifest"

  find . \
    -path './.git' -prune -o \
    -path './build' -prune -o \
    -path './objs' -prune -o \
    -path './.cache' -prune -o \
    -path './.pytest_cache' -prune -o \
    -name '*.dSYM' -prune -o \
    -path '*/__pycache__' -prune -o \
    -type f -print |
    while IFS= read -r path; do
      if should_skip_file "$path"; then
        continue
      fi

      printf '%s\n' "$path" >>"$manifest"
    done
}

normalize_target() {
  local raw_target="$1"

  case "$raw_target" in
    *:*) printf '%s\n' "$raw_target" ;;
    *) printf '%s:\n' "$raw_target" ;;
  esac
}

print_command() {
  local arg

  for arg in "$@"; do
    printf '%q ' "$arg" >&2
  done
  printf '\n' >&2
}

archive_name=''
dry_run=0
identity_file=''
keep=0
target=''

while [[ $# -gt 0 ]]; do
  case "$1" in
    -i | --identity-file)
      [[ $# -ge 2 ]] || die "$1 requires a value"
      identity_file="$2"
      shift 2
      ;;
    --name)
      [[ $# -ge 2 ]] || die '--name requires a value'
      archive_name="$2"
      shift 2
      ;;
    --dry-run)
      dry_run=1
      shift
      ;;
    --keep)
      keep=1
      shift
      ;;
    -h | --help)
      usage
      exit 0
      ;;
    -*)
      die "unknown option: $1"
      ;;
    *)
      target="$1"
      shift
      break
      ;;
  esac
done

[[ -n "$target" ]] || {
  usage >&2
  exit 1
}
[[ $# -eq 0 ]] || die "unexpected extra arguments: $*"

target=$(normalize_target "$target")

require_command find
require_command file
require_command mktemp
require_command zip
if [[ "$dry_run" -eq 0 ]]; then
  require_command scp
fi

source_dir=$(pwd -P)
project_name=$(basename "$source_dir")
timestamp=$(date +%Y%m%d-%H%M%S)

scp_args=()
identity_file_abs=''
if [[ -n "$identity_file" ]]; then
  [[ -f "$identity_file" ]] || die "identity file not found: $identity_file"
  identity_dir=$(cd "$(dirname "$identity_file")" && pwd -P)
  identity_file_abs="$identity_dir/$(basename "$identity_file")"
  scp_args+=('-i' "$identity_file_abs")
fi

if [[ -z "$archive_name" ]]; then
  archive_name="${project_name}-${timestamp}.zip"
fi

archive_name=$(basename "$archive_name")
case "$archive_name" in
  *.zip) ;;
  *) archive_name="${archive_name}.zip" ;;
esac

tmpdir=$(mktemp -d)
archive="$tmpdir/$archive_name"
manifest="$tmpdir/files.txt"

cleanup() {
  if [[ "$keep" -eq 1 && -f "$archive" ]]; then
    printf 'Kept local archive: %s\n' "$archive" >&2
    return
  fi

  rm -rf "$tmpdir"
}
trap cleanup EXIT

printf 'Building manifest from %s\n' "$source_dir" >&2
build_manifest "$manifest"

file_count=$(wc -l <"$manifest" | tr -d ' ')
[[ "$file_count" -gt 0 ]] || die 'manifest is empty; nothing to archive'

printf 'Zipping %s files into %s\n' "$file_count" "$archive" >&2
zip -q "$archive" -@ <"$manifest"

archive_size=$(du -h "$archive" | awk '{print $1}')
printf 'Archive ready: %s (%s)\n' "$archive" "$archive_size" >&2

if [[ "$dry_run" -eq 1 ]]; then
  printf 'Dry run: would run: ' >&2
  print_command scp "${scp_args[@]}" "$archive" "$target"
  exit 0
fi

printf 'Copying archive to %s\n' "$target" >&2
scp "${scp_args[@]}" "$archive" "$target"
printf 'Copied archive to %s\n' "$target" >&2
