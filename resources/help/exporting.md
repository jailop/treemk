# Sharing Your Work with the World

You've written your notes in Markdown. Now you need to share them—send them to colleagues, publish them online, or print them as PDF. TreeMk exports your work to multiple formats, preserving your formatting, code blocks, equations, and diagrams.

## Export Formats

TreeMk can export to four formats:

**HTML** - Self-contained web pages with embedded CSS. Open in any browser, host on any web server, or send via email. Works offline.

**PDF** - Professional documents ready to print or distribute. Preserves layout, fonts, and formatting. Universal format everyone can open.

**DOCX** - Microsoft Word format. Edit further in Word, Google Docs, or LibreOffice. Great for collaborative editing.

**Plain Text** - Strips all formatting, leaving just the text. Useful for copying content or working with systems that don't understand markup.

## Quick Export

The fastest way to export:

1. Open the document you want to export
2. Press **Ctrl+E** or choose **File → Export**
3. Select your format from the dropdown
4. Choose where to save
5. Click **Save**

TreeMk handles the conversion and tells you when it's done.

## Export Requirements

TreeMk uses Pandoc for HTML, PDF, and DOCX exports. Pandoc is a powerful document converter that TreeMk calls behind the scenes.

### Installing Pandoc

**Ubuntu/Debian:**
```bash
sudo apt install pandoc
```

**Fedora/RHEL:**
```bash
sudo dnf install pandoc
```

**macOS (with Homebrew):**
```bash
brew install pandoc
```

**Windows:**
Download the installer from [pandoc.org](https://pandoc.org/installing.html)

TreeMk will tell you if Pandoc is missing when you try to export. Plain text export doesn't need Pandoc—it always works.

### For PDF Export

PDF generation needs extra tools. Pandoc uses LaTeX under the hood:

**Ubuntu/Debian:**
```bash
sudo apt install texlive-latex-base texlive-fonts-recommended
```

**macOS:**
```bash
brew install --cask mactex-no-gui
```

**Windows:**
Install MiKTeX from [miktex.org](https://miktex.org/)

Alternatively, use the smaller BasicTeX for macOS or TinyTeX for any platform.

## Exporting with Mermaid Diagrams

Your notes contain beautiful Mermaid diagrams—flowcharts, sequence diagrams, Gantt charts. To include them in exports, you need one more tool: mermaid-filter.

### Installing mermaid-filter

You'll need Node.js and npm first. Then:

```bash
npm install -g mermaid-filter
```

This installs globally, making it available to TreeMk's Pandoc integration.

**Verify installation:**
```bash
mermaid-filter --version
```

If mermaid-filter is in your PATH, TreeMk automatically uses it during export. Your diagrams render as images in the output.

**Without mermaid-filter:** Diagrams export as code blocks with the Mermaid syntax visible. Still readable, but not rendered.

## Export Features by Format

### HTML Export

**What You Get:**
- Self-contained single file with embedded CSS
- Syntax highlighting in code blocks
- Rendered LaTeX formulas (via MathJax from CDN)
- Mermaid diagrams (if mermaid-filter installed)
- Works offline except for MathJax (loads from CDN)

**Best For:**
- Publishing to websites
- Sharing via email
- Viewing in any browser
- Creating documentation sites

**Tip:** Open the HTML file in a browser, then print to PDF if you need a quick PDF without LaTeX.

### PDF Export

**What You Get:**
- Professional typeset document
- Page numbers and margins
- Rendered code with syntax highlighting
- LaTeX formulas perfectly typeset
- Mermaid diagrams as images (if mermaid-filter installed)

**Best For:**
- Formal documents and reports
- Printing physical copies
- Sharing for reading (not editing)
- Academic papers

**Customization:**
TreeMk uses Pandoc's default LaTeX template. You can customize this in Preferences with template files and Pandoc options (advanced users).

**Note:** PDF export takes longer than other formats because LaTeX does complex typesetting.

### DOCX Export

**What You Get:**
- Microsoft Word format (.docx)
- Editable text with formatting preserved
- Code blocks as formatted text
- Tables and lists structured properly
- Images embedded

**What's Limited:**
- Math formulas export as plain text notation (not rendered equations)
- Mermaid diagrams export as code blocks (not rendered images)
- Some advanced formatting may simplify

**Best For:**
- Collaborative editing
- Converting notes to Word for further work
- Sharing with people who need to edit
- Publishing workflows that require Word

**Tip:** Export to DOCX, then edit further in Word/Google Docs/LibreOffice to add publisher-specific formatting.

### Plain Text Export

**What You Get:**
- Raw text only
- All Markdown formatting removed
- No images, no styling, no structure
- Just words and line breaks

**Best For:**
- Copying text to paste elsewhere
- Word count and text analysis
- Simple backups
- Working with plain-text-only systems

## Export Settings

Configure export behavior in **Edit → Preferences → Export** (if settings exist) or via Pandoc options.

Common Pandoc options you might add:
- `--toc` - Add table of contents
- `--number-sections` - Number headings
- `--standalone` - Create complete document
- `--template=mytemplate.html` - Use custom template

## Export Workflows

### Publishing a Blog Post

1. Write in TreeMk with wiki links to reference notes
2. Export to HTML
3. Extract the body content (inside `<body>` tags)
4. Paste into your CMS or static site generator

### Creating a Report

1. Write sections as separate notes
2. Create a main note that includes them: `[[!section1]]`, `[[!section2]]`, etc.
3. Export the main note to PDF
4. All sections combine into one document

### Sharing Documentation

1. Write with code examples: `` ```python[[!example.py]]``` ``
2. Export to HTML with mermaid-filter installed
3. Host on GitHub Pages, Netlify, or send as file
4. Recipients see rendered code and diagrams

### Collaborative Writing

1. Write initial draft in TreeMk
2. Export to DOCX
3. Share with colleagues for comments
4. Incorporate changes back into Markdown

## Tips for Better Exports

### Before Exporting

**Clean up wiki links:** Export treats `[[note]]` as links. If you want plain text, write it differently.

**Check image paths:** Use relative paths (`./images/diagram.png`) not absolute paths.

**Test formulas:** Ensure LaTeX syntax is correct. Bad syntax breaks PDF export.

**Preview first:** Look at the TreeMk preview. What you see is roughly what exports.

### For Professional PDFs

**Add metadata** at the top of your document:

```yaml
---
title: My Document Title
author: Your Name
date: 2026-02-14
---
```

Pandoc uses this for the title page.

**Use semantic headings:** H1 for title, H2 for sections, H3 for subsections. Pandoc's PDF layout depends on heading hierarchy.

**Page breaks:** Force page breaks with:

```html
<div style="page-break-after: always;"></div>
```

### For HTML Publishing

**Use relative links:** So links work when you move files.

**Optimize images:** Large images make large HTML files. Compress them first.

**Test in browsers:** Firefox, Chrome, Safari—make sure it works everywhere.

## Troubleshooting

### "Pandoc not found"

TreeMk can't find Pandoc. Install it and ensure it's in your system PATH.

**Test from terminal:**
```bash
pandoc --version
```

If that works, restart TreeMk.

### PDF Export Fails

**Check LaTeX installation:**
```bash
pdflatex --version
```

If missing, install TeXLive or MikTeX.

**Check Pandoc's PDF engine:**
Try exporting with `--pdf-engine=xelatex` in Pandoc options (if you can set them).

**Look for error messages:** Pandoc might report what failed.

### Diagrams Don't Render

**Check mermaid-filter:**
```bash
mermaid-filter --version
```

Not found? Install it with `npm install -g mermaid-filter`.

**Check Mermaid syntax:** Invalid syntax causes silent failures. Test diagrams in preview first.

### Formulas Look Wrong in PDF

**LaTeX formula errors:** Check your formula syntax. One typo breaks export.

**Missing packages:** Some LaTeX features need extra packages. The error message tells you which.

### DOCX Looks Plain

DOCX export is functional, not beautiful. It preserves content and basic formatting.

For pretty Word documents:
1. Export to DOCX
2. Apply Word styles and themes
3. Save with your formatting

## Advanced: Custom Templates

Pandoc uses templates to structure output. You can create your own:

1. Get default template: `pandoc -D html > my-template.html`
2. Edit the template (requires Pandoc template syntax knowledge)
3. Use in export: `--template=my-template.html`

This is advanced. Most users won't need it. But if you're publishing regularly, custom templates make consistent output easy.

## Quick Command Reference

| Task | Command |
|------|---------|
| Export menu | Ctrl+E or File → Export |
| Choose format | Dropdown in export dialog |
| Check Pandoc | `pandoc --version` in terminal |
| Check mermaid-filter | `mermaid-filter --version` |
| Install Pandoc (Ubuntu) | `sudo apt install pandoc` |
| Install mermaid-filter | `npm install -g mermaid-filter` |

## The Big Picture

Markdown is your source. It's human-readable and version-controllable. But the world uses many formats.

TreeMk bridges this gap. Write once in Markdown. Export to whatever format your audience needs. Your content stays in Markdown, flexible and future-proof. Your output fits any workflow.

That's the power of plain text.
