#!/bin/bash
# Script to serve documentation locally

# Check if virtual environment exists
if [ ! -d ".venv" ]; then
    echo "Creating virtual environment..."
    python -m venv .venv
fi

# Activate virtual environment
source .venv/bin/activate

# Check if mkdocs is installed
if ! command -v mkdocs &> /dev/null; then
    echo "Installing MkDocs and plugins..."
    pip install mkdocs-material mkdocs-git-revision-date-localized-plugin
fi

# Serve documentation
echo "Starting MkDocs development server..."
echo "Documentation will be available at: http://127.0.0.1:8000"
echo "Press Ctrl+C to stop"
mkdocs serve
