#!/bin/bash
# Download and prepare JavaScript libraries for local bundling

set -e

RESOURCES_DIR="resources/js"
CSS_DIR="resources/css"

mkdir -p "$RESOURCES_DIR"
mkdir -p "$CSS_DIR"

echo "Downloading KaTeX..."
curl -L "https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.js" -o "$RESOURCES_DIR/katex.min.js"
curl -L "https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/contrib/auto-render.min.js" -o "$RESOURCES_DIR/katex-auto-render.min.js"
curl -L "https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.css" -o "$CSS_DIR/katex.min.css"

echo "Downloading Highlight.js..."
curl -L "https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js" -o "$RESOURCES_DIR/highlight.min.js"
curl -L "https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/github.min.css" -o "$CSS_DIR/highlight-github.min.css"
curl -L "https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/github-dark.min.css" -o "$CSS_DIR/highlight-github-dark.min.css"

echo "Downloading Mermaid..."
curl -L "https://cdn.jsdelivr.net/npm/mermaid@11/dist/mermaid.min.js" -o "$RESOURCES_DIR/mermaid.min.js"

echo "All libraries downloaded successfully!"
echo ""
echo "Files created:"
ls -lh "$RESOURCES_DIR"
ls -lh "$CSS_DIR"
