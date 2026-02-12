#!/bin/bash
# Bundle CSS files

OUTPUT_DIR="resources/css"
mkdir -p "$OUTPUT_DIR"

# Copy KaTeX CSS (already minified)
cp node_modules/katex/dist/katex.min.css "$OUTPUT_DIR/katex.min.css"

# Bundle highlight.js themes
cat node_modules/highlight.js/styles/github.min.css > "$OUTPUT_DIR/highlight-github.min.css"
cat node_modules/highlight.js/styles/github-dark.min.css > "$OUTPUT_DIR/highlight-github-dark.min.css"

echo "CSS files bundled successfully!"
ls -lh "$OUTPUT_DIR"
