#!/bin/bash
# Build preview bundle with Bun

set -e

echo "Building JavaScript bundle with Bun..."
bun build bundle.ts --outfile=resources/js/preview-bundle.min.js --minify --target=browser

echo "Copying CSS files..."
mkdir -p resources/css
cp node_modules/katex/dist/katex.min.css resources/css/katex.min.css
cp node_modules/highlight.js/styles/github.min.css resources/css/highlight-github.min.css
cp node_modules/highlight.js/styles/github-dark.min.css resources/css/highlight-github-dark.min.css

echo "Bundle created successfully!"
echo ""
echo "Bundle size:"
ls -lh resources/js/preview-bundle.min.js
echo ""
echo "Total size of embedded libraries:"
du -sh resources/js resources/css
