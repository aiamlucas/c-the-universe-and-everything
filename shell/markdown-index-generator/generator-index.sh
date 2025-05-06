#!/bin/bash

# Change to the root of the repository
cd "$(dirname "$0")/../.." || exit 1

START_MARKER="<!-- INDEX-START -->"
END_MARKER="<!-- INDEX-END -->"

generate_index() {
	echo "$START_MARKER"
	echo ""

	# Loop through top-level directories (c, shell, etc.)
	for dir in c shell; do
		if [ -d "$dir" ]; then
			header=$(echo "$dir" | sed 's/\b\(.\)/\u\1/g') # Capitalize
			echo "### $header"
			echo ""

			find "$dir" -name "*.md" | while read -r file; do
				title=$(basename "$file" .md | sed 's/-/ /g' | sed 's/\b\(.\)/\u\1/g')
				echo "- [$title]($file)"
			done

			echo ""
		fi
	done

	echo "$END_MARKER"
}

update_readme() {
	new_index=$(generate_index)

	if grep -q "$START_MARKER" README.md && grep -q "$END_MARKER" README.md; then
		awk -v new="$new_index" -v start="$START_MARKER" -v end="$END_MARKER" '
        BEGIN {in_block=0}
        {
            if ($0 ~ start) {print new; in_block=1; next}
            if ($0 ~ end) {in_block=0; next}
            if (!in_block) print $0
        }' README.md >README.tmp && mv README.tmp README.md
		echo "README.md updated with grouped index."
	else
		echo -e "\n$new_index" >>README.md
		echo "Markers not found. Appended grouped index at the end of README.md."
	fi
}

update_readme
