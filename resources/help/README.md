# TreeMk Help Documentation

This directory contains TreeMk's documentation, which serves two purposes:

1. **In-App Help System** - Embedded in the application as Qt resources
2. **Website Documentation** - Published to GitHub Pages via MkDocs

## Structure

```
resources/help/
├── *.md              # Markdown documentation files
├── images/           # Screenshots and diagrams
├── javascripts/      # Custom JS (MkDocs only, not embedded in app)
└── stylesheets/      # Custom CSS (MkDocs only, not embedded in app)
```

## Single Source Documentation

The same Markdown files are used for both:
- The help dialog in TreeMk (via `:/help/` Qt resources)
- The documentation website at https://datainquiry.github.io/treemk

## Files

### Core Documentation (embedded in app)
- `index.md` - Home page
- `getting-started.md` - Quick start guide
- `editor.md` - Editor features
- `navigation.md` - Navigation and wiki-links
- `preview.md` - Preview features
- `formulas.md` - Math formulas
- `code-blocks.md` - Code syntax highlighting
- `diagrams.md` - Mermaid diagrams
- `ai-assistant.md` - AI assistant
- `keyboard-shortcuts.md` - Shortcuts reference

### Web-Only Documentation (not embedded)
- `installation.md` - Installation guide
- `contributing.md` - Contributing guide
- `license.md` - License information
- `javascripts/` - MkDocs JavaScript
- `stylesheets/` - MkDocs CSS

## Editing Documentation

When editing documentation:

1. **Edit once** - Changes automatically apply to both app and website
2. **Test in app** - Help → User Guide
3. **Test website** - Run `./serve-docs.sh` or `mkdocs serve`
4. **Keep it simple** - Use standard Markdown features
5. **Images** - Place in `images/` and use relative paths

## Building Website

The documentation website is built using MkDocs with Material theme:

```bash
# Preview locally
./serve-docs.sh
# or
mkdocs serve

# Build static site
mkdocs build
```

The `docs/` symlink at project root points here for MkDocs compatibility.

## Deployment

GitHub Actions automatically deploys to GitHub Pages when:
- Changes are pushed to `main` branch  
- Files in `resources/help/` are modified
- `mkdocs.yml` is updated

Workflow: `.github/workflows/docs.yml`

## Writing Guidelines

- Use clear, concise language
- Include examples and screenshots
- Test Markdown rendering in both contexts
- Use relative links between pages
- Keep file names lowercase with hyphens
- Avoid web-specific features in core docs
