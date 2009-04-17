#! /usr/bin/env bash

set -e -o pipefail

print() { echo "$1" >&2; }
inform() { print "$0: $1"; }
warn() { print "Warning: $1"; }
error() { inform "Error: $1"; }
fail() { error "$1"; exit 1; }
status() { echo -n "$1" >&2; }

relpath() {
	PATH_FROM=$(readlink -f "$1" | sed -r 's,/+$,,')
	PATH_TO=$(readlink -f "$2" | sed -r 's,/+$,,')
	
	while [ "$PATH_FROM" ] && [ "$PATH_TO" ] && [ "$(echo "$PATH_FROM" | grep -oE '^/[^/]+' | head -n 1)" == "$(echo "$PATH_TO" | grep -oE '^/[^/]+' | head -n 1)" ]; do
		PATH_FROM=$(echo "$PATH_FROM" | sed -r 's,^/[^/]+,,')
		PATH_TO=$(echo "$PATH_TO" | sed -r 's,^/[^/]+,,')
	done
	
	while [ "$PATH_FROM" ]; do
		PATH_TO="/..$PATH_TO"
		PATH_FROM=$(echo "$PATH_FROM" | sed -r 's,/[^/]+,,')
	done
	
	[ "$PATH_TO" ] || PATH_TO='.'
	echo "$PATH_TO" | sed -r 's,^/+,,'
}

[ "$1" ] && MODULE_NAME=$1 || fail "Module name not given!"
[ "$2" ] && MODULE_PATH=$2

if [ "$MODULE_PATH" ]; then
	if ! [ -e "$MODULE_NAME" ] || [ -h "$MODULE_NAME" ]; then
		echo ln -fsn "$MODULE_PATH" "$MODULE_NAME"
		ln -fsn "$MODULE_PATH" "$MODULE_NAME"
	else
		warn "The symlink to the lgx module could not be created as the file ./lgx already exists and is something other than a symlink. Pleas remove it and run 'make reconfigure' to create the symlink."
	fi
fi

if ! [ -e "$MODULE_NAME/oscar" ] || [ -h "$MODULE_NAME/oscar" ]; then
	ln -fsn $(relpath "$MODULE_NAME" '.') "$MODULE_NAME/oscar"
else
	warn "The lgx module already contains a file 'oscar' that is not a symlink. Please remove that file and run 'make reconfigure' to create a symlink back to the Oscar Framework there."
fi
