#!/usr/bin/env bash

set -euo pipefail

usage() {
    cat <<'USAGE'
Usage:
  scripts/release.sh v1.2.3
  scripts/release.sh 1.2.3
  scripts/release.sh --major | --minor | --patch

Creates an annotated vX.Y.Z tag on the current commit and pushes the tag to origin.
USAGE
}

die() {
    echo "Error: $*" >&2
    exit 1
}

if [[ $# -ne 1 ]]; then
    usage
    exit 1
fi

arg="$1"
tag=""

latest_version_tag() {
    git tag --list 'v[0-9]*.[0-9]*.[0-9]*' --sort=-v:refname | head -n 1
}

bump_version() {
    local bump_type="$1"
    local current
    local version
    local major
    local minor
    local patch

    current="$(latest_version_tag)"
    if [[ -z "$current" ]]; then
        current="v0.0.0"
    fi

    version="${current#v}"
    IFS='.' read -r major minor patch <<< "$version"

    [[ "$major" =~ ^[0-9]+$ ]] || die "latest tag is not semver-like: $current"
    [[ "$minor" =~ ^[0-9]+$ ]] || die "latest tag is not semver-like: $current"
    [[ "$patch" =~ ^[0-9]+$ ]] || die "latest tag is not semver-like: $current"

    case "$bump_type" in
        major) major=$((major + 1)); minor=0; patch=0 ;;
        minor) minor=$((minor + 1)); patch=0 ;;
        patch) patch=$((patch + 1)) ;;
        *) die "unknown bump type: $bump_type" ;;
    esac

    printf 'v%s.%s.%s\n' "$major" "$minor" "$patch"
}

case "$arg" in
    --major) tag="$(bump_version major)" ;;
    --minor) tag="$(bump_version minor)" ;;
    --patch) tag="$(bump_version patch)" ;;
    v[0-9]*.[0-9]*.[0-9]*) tag="$arg" ;;
    [0-9]*.[0-9]*.[0-9]*) tag="v$arg" ;;
    -h|--help)
        usage
        exit 0
        ;;
    *)
        usage
        die "unknown version/tag argument: $arg"
        ;;
esac

[[ "$tag" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]] || die "tag must look like vX.Y.Z: $tag"

git rev-parse --is-inside-work-tree >/dev/null 2>&1 || die "not inside a git repository"

current_branch="$(git branch --show-current)"
[[ -n "$current_branch" ]] || die "cannot release from a detached HEAD"

if [[ -n "$(git status --short)" ]]; then
    die "working tree is not clean; commit or stash changes before tagging"
fi

if git rev-parse -q --verify "refs/tags/$tag" >/dev/null; then
    die "local tag already exists: $tag"
fi

echo ""
echo "  Branch:      $current_branch"
echo "  Commit:      $(git rev-parse --short HEAD)"
echo "  Tag:         $tag"
echo ""
echo "This will:"
echo "  1. Create annotated tag: $tag"
echo "  2. Push tag to origin"
echo "  3. Start the GitHub Release workflow"
echo ""

read -r -p "Push $tag? [y/N] " confirm
if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
    echo "Aborted."
    exit 0
fi

git tag -a "$tag" -m "Release $tag"
git push origin "$tag"

echo ""
echo "Pushed $tag. Release workflow will start shortly."
