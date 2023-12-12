#!/bin/sh
#
# Copyright (c) Microsoft Corporation. All rights reserved.
#
# A hook script to verify what is about to be committed.
# Called by "git commit" with no arguments.  The hook should
# exit with non-zero status after issuing an appropriate message if
# it wants to stop the commit.
# To avoid having this script called, use "git commit -n"

# Get files that need to be checked against clang-format into an array

# To understand the regex format in bash, take a look at https://tldp.org/LDP/abs/html/x17129.html
# For example, this is how the regex ^client/.*(\.cpp|\.h)$ matches the example file client\include\SFSClient.h:
#   ^: Matches beginning of string
#   client/: direct string match
#   .*: Matches any number of characters (. means any character except newline, and * means match any number of the character before, including 0)
#   (\.cpp|\.h): Match either .cpp or .h. The . character is escaped because it has a special meaning in regex.
#   $: Match end of string
pattern_to_include_in_clang_format='^client/.*(\.cpp|\.h)$'
pattern_to_include_in_cmake_format='CMakeLists.txt'

# Get the names of staged files (--cached) that are not deleted (--diff-filter=d)
all_staged_files=$(git diff --cached --name-only --diff-filter=d)
mapfile -t filtered_files <<<"$all_staged_files"

files_with_unstaged_changes=()
clang_format_files=()
cmake_format_files=()

# Only those that match a pattern will be analyzed
max_index=${#filtered_files[@]}
for ((i = 0; i < max_index; i++)); do
	if [[ "${filtered_files[$i]}" =~ $pattern_to_include_in_clang_format || "${filtered_files[$i]}" =~ $pattern_to_include_in_cmake_format ]]; then
        if [[ "${filtered_files[$i]}" =~ $pattern_to_include_in_clang_format ]]; then
            clang_format_files+=("${filtered_files[$i]}")
        elif [[ "${filtered_files[$i]}" =~ $pattern_to_include_in_cmake_format ]]; then
            cmake_format_files+=("${filtered_files[$i]}")
        fi

		# Collect files that will be analyzed but have unstaged changes, as we don't want to overwrite these changes
		# `git diff` is empty if a file has no unstaged changes
		if [[ -n "$(git diff "${filtered_files[$i]}")" ]]; then
			files_with_unstaged_changes+=("${filtered_files[$i]}")
		fi
	fi
done

# Early stop if no files to check
if [[ ${clang_format_files[*]} == "" && ${cmake_format_files[*]} == "" ]]; then
	exit 0
fi

RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No color

IFS=$'\n' sorted_files_with_unstaged_changes=($(sort <<<"${files_with_unstaged_changes[*]}"))
unset IFS

# Early stop if files have unstaged changes
if [[ ${sorted_files_with_unstaged_changes[*]} != "" ]]; then
	echo -e "${RED}There are files that need to be checked by the formatters but have unstaged changes. Either stage the files or discard the unstaged changes:"
	for file in "${sorted_files_with_unstaged_changes[@]}"; do
		echo -e "M\t$file"
	done
	echo -e "${NC}"
	exit 1
fi

#
# clang-format
#

# Check that clang-format is installed
python_dir="$(python -c 'import os,sysconfig;print(sysconfig.get_path("scripts",f"{os.name}_user"))')"
clang_format="$python_dir/clang-format.exe"

if [[ ! -x "$(command -v "$clang_format")" ]]; then
	echo -e "${RED}There are staged changes that need to be checked by the formatter."
	echo -e "${RED}clang-format toolset was not found. Re-run scripts\Setup.ps1 to install it.${NC}"
	exit 1
fi

formatted_files=()
for file in "${clang_format_files[@]}"; do
	# Check if file will be modified so we can properly inform the user of modified files
	# Use `clang-format -n` which does a dry run and outputs a non-empty response if there's something to format
	if [[ -n $("$clang_format" "$file" -n 2>&1) ]]; then
		formatted_files+=("$file")
		# Now actually format in-place
		"$clang_format" -i "$file"
	fi
done

# Report modified files at end so we can run both formatters and report all changes at once

#
# cmake-format
#
cmake_format="$python_dir/cmake-format.exe"

if [[ ! -x "$(command -v "$cmake_format")" ]]; then
	echo -e "${RED}There are staged changes that need to be checked by the formatter."
	echo -e "${RED}cmake-format toolset was not found. Re-run scripts\Setup.ps1 to install it.${NC}"
	exit 1
fi

for file in "${cmake_format_files[@]}"; do
	# Check if file will be modified so we can properly inform the user of modified files
	# Use `cmake-format --check` which does a dry run and returns 1 if there's something to format
    "$cmake_format" --check "$file" > /dev/null 2>&1
	if [[ $? -ne 0 ]]; then
		formatted_files+=("$file")
		# Now actually format in-place
		"$cmake_format" -i "$file"
	fi
done

IFS=$'\n' sorted_formatted_files=($(sort <<<"${formatted_files[*]}"))
unset IFS

# If there were formatted files, we report the changes
if [[ ${sorted_formatted_files[*]} != "" ]]; then
	echo -e "${YELLOW}There are files that have been automatically formatted. Review and stage the changes before committing:"
	for file in "${sorted_formatted_files[@]}"; do
		echo -e "M\t$file"
	done
	echo -e "${NC}"
	exit 1
fi

# If no modifications to staged files, we can exit the script
