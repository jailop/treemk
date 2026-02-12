# Getting Started with TreeMk

Welcome! This guide will help you start using TreeMk effectively.

## First Steps

### 1. Open a Folder
TreeMk works with folders containing your Markdown notes.

- Go to **File → Open Folder** (or press **Ctrl+O**)
- Select a folder on your computer
- TreeMk will display all files in the folder tree on the left

### 2. Create Your First Note

- Right-click in the file tree
- Select **New File**
- Give it a name (e.g., `my-first-note.md`)
- Start typing in the editor!

### 3. Link Notes Together

Use wiki-style links to connect your notes:

```markdown
This is a link to [[another-note]]
```

When you **Ctrl+Click** on the link, TreeMk opens that note. If the file doesn't exist, TreeMk will ask if you want to create it.

## Understanding the Interface

### Left Panel (Files & Outline)
- **Files Tab**: Browse your folder structure
- **Outline Tab**: Navigate headings in the current document
- **Backlinks Tab**: See which notes link to the current one

### Center Panel (Editor)
The main editor supports:
- Multiple tabs for different notes
- Line numbers
- Syntax highlighting
- Word prediction

### Right Panel (Preview)
Live preview of your formatted Markdown, including:
- Rendered text with proper formatting
- Code blocks with syntax highlighting
- Mathematical formulas
- Diagrams

Toggle the preview with **Ctrl+R** or from the View menu.

## Basic Markdown Syntax

```markdown
# Heading 1
## Heading 2
### Heading 3

**Bold text**
*Italic text*

- Bullet list item
- Another item

1. Numbered list
2. Second item

[External link](https://example.com)
[[Internal wiki link]]

`inline code`

\`\`\`python
# Code block
print("Hello, World!")
\`\`\`
```

## Next Steps

- Explore **[Editor Features](editor.md)** to learn about advanced editing
- Learn about **[Navigation](navigation.md)** features
- Check out **[Keyboard Shortcuts](keyboard-shortcuts.md)** for productivity tips

## Tips

- Use **Ctrl+P** for quick file opening with fuzzy search
- Use **Ctrl+Shift+F** to search across all files
- Press **F1** to see all keyboard shortcuts
- Paste images directly from clipboard (**Ctrl+V**)
