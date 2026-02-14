# Seeing Your Words Come Alive

Markdown is plain text, but it's meant to be beautiful. TreeMk's preview panel transforms your text into formatted documents, complete with styled headings, colorful code blocks, rendered equations, and interactive diagrams. It's not just a feature—it's your second screen, updating instantly as you write.

Let's explore what the preview can do.

**Deep Dives Available:**
- **[Math Formulas](formulas.md)** - LaTeX equations and scientific notation
- **[Code Blocks](code-blocks.md)** - Syntax highlighting for every language  
- **[Mermaid Diagrams](diagrams.md)** - Flowcharts, sequences, and more

## Live Preview

### Real-Time Rendering

As you type in the editor, the preview updates automatically:
- Configurable refresh rate (100-2000ms)
- Smooth scrolling synchronized with editor
- Theme-aware styling

### Toggle Preview

- **Ctrl+R**: Toggle preview panel on/off
- **Ctrl+\\**: Cycle through view modes (editor only, preview only, both)

## Markdown Rendering

### Basic Formatting

All standard Markdown is supported:

- **Bold** with `**text**` or `__text__`
- *Italic* with `*text*` or `_text_`
- `Code` with backticks
- ~~Strikethrough~~ with `~~text~~`

### Headings

```markdown
# Heading 1
## Heading 2
### Heading 3
#### Heading 4
##### Heading 5
###### Heading 6
```

### Lists

```markdown
- Bullet list
  - Nested item
  - Another nested item

1. Numbered list
2. Second item
   1. Nested numbered
   2. Another nested

- [ ] Task list item
- [x] Completed task
```

### Links and Images

```markdown
[External link](https://example.com)
[[Internal wiki link]]
![Image alt text](path/to/image.png)
```

## Mathematical Formulas

### LaTeX Support

TreeMk renders LaTeX formulas using KaTeX or MathJax.

#### Inline Formulas

Use single dollar signs for inline math:

```markdown
The formula $E = mc^2$ is Einstein's famous equation.
```

Result: The formula $E = mc^2$ is Einstein's famous equation.

#### Block Formulas

Use double dollar signs for display math:

```markdown
$$
\int_{-\infty}^{\infty} e^{-x^2} dx = \sqrt{\pi}
$$
```

Result:

$$
\int_{-\infty}^{\infty} e^{-x^2} dx = \sqrt{\pi}
$$

### Common Formula Examples

**Quadratic Formula:**

```markdown
$$x = \frac{-b \pm \sqrt{b^2 - 4ac}}{2a}$$
```

**Matrix:**

```markdown
$$
\begin{bmatrix}
a & b \\
c & d
\end{bmatrix}
$$
```

**Sum Notation:**

```markdown
$$\sum_{i=1}^{n} i = \frac{n(n+1)}{2}$$
```

## Code Blocks

### Syntax Highlighting

Code blocks support syntax highlighting for many languages:

\`\`\`python
def hello_world():
    print("Hello, World!")
    return True
\`\`\`

\`\`\`javascript
function greet(name) {
    console.log(`Hello, ${name}!`);
}
\`\`\`

\`\`\`cpp
#include <iostream>
int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
\`\`\`

## Tables

### Markdown Tables

Create tables with pipes and hyphens:

```markdown
| Header 1 | Header 2 | Header 3 |
|----------|----------|----------|
| Cell 1   | Cell 2   | Cell 3   |
| Cell 4   | Cell 5   | Cell 6   |
```

### Alignment

```markdown
| Left | Center | Right |
|:-----|:------:|------:|
| L1   | C1     | R1    |
| L2   | C2     | R2    |
```

## Blockquotes

### Quote Formatting

```markdown
> This is a blockquote.
> It can span multiple lines.
>
> > Nested blockquotes are also supported.
```

### Callouts (with custom CSS)

Use blockquotes with special formatting for callouts:

```markdown
> **Note:** This is an informational callout.

> **Warning:** This is a warning message.

> **Tip:** This is a helpful tip.
```

## File Inclusion

### Embedding Content

Include content from other files:

```markdown
[[!filename]]
```

When the preview renders, it will embed the content of that file inline.

### Use Cases

- Reusable snippets
- Template inclusion
- Modular documentation
- Shared content blocks

## Custom Styling

### Custom CSS

Load a custom CSS file to style your preview:

1. Go to **Edit → Preferences**
2. Navigate to **Preview** section
3. Browse for your custom CSS file
4. CSS applies immediately

### CSS File Location

Place custom CSS anywhere, but consider:

- Project-specific: in your workspace folder
- Global: in a dedicated styles folder
- Version control: include with your notes

## Preview Settings

### Configurable Options

Access via **Edit → Preferences**:

**Refresh Rate:**

- 100-2000 milliseconds
- Lower = more responsive
- Higher = less CPU usage

**Font Size:**

- 8-72 points
- Independent from editor font
- Zoom with Ctrl+Plus/Minus

**Custom CSS:**

- Optional styling override
- Applies on top of theme
- Full CSS3 support

## Tips for Better Previews

1. **Use Semantic Headings**: Structure improves readability
2. **Add Alt Text to Images**: Better accessibility
3. **Format Code Blocks**: Specify language for highlighting
4. **Test Formulas**: Preview LaTeX before finalizing
5. **Leverage Tables**: Organize data clearly
6. **Use Callouts**: Highlight important information

## Troubleshooting

### Formula Not Rendering

- Check for balanced delimiters ($..$ or $$...$$)
- Verify LaTeX syntax is correct
- Ensure formulas are enabled in settings

### Image Not Showing

- Check file path is correct
- Use relative paths from document location
- Verify image file exists

### Slow Preview

- Increase refresh rate in settings
- Disable features you don't need
- Consider document size

## Export Features

TreeMk can export your notes to multiple formats using [Pandoc](https://pandoc.org/).

### Supported Formats

**HTML Export**

- Self-contained HTML file
- Includes CSS styling
- Preserves syntax highlighting
- Mermaid diagrams (with mermaid-filter)

**PDF Export**

- Professional document layout
- Code syntax highlighting
- LaTeX formula rendering
- Requires Pandoc with PDF support

**DOCX Export**

- Microsoft Word format
- Preserves formatting
- Tables and lists
- Code blocks

**Plain Text**

- Simple text export
- No formatting
- Quick and lightweight

### How to Export

1. Open the document you want to export
2. Go to **File → Export** or press **Ctrl+E**
3. Choose your format (HTML, PDF, DOCX, or Text)
4. Select destination and filename
5. Click Save

### Requirements

**Pandoc** is required for HTML, PDF, and DOCX exports:

```bash
# Ubuntu/Debian
sudo apt install pandoc

# macOS
brew install pandoc

# Fedora
sudo dnf install pandoc
```

**For Mermaid Diagrams in Exports:**

Install mermaid-filter via npm:

```bash
npm install -g mermaid-filter
```

If `mermaid-filter` is in your PATH, TreeMk will automatically use it during export to render Mermaid diagrams. Otherwise, they'll export as code blocks.

### Export Options

Access **Edit → Preferences → Export** to configure:

- Default export format
- PDF engine (pdflatex, xelatex, etc.)
- Custom Pandoc options
- Template selection

### Tips for Better Exports

1. **Use Semantic Headings**: Structure with H1, H2, H3 for better formatting
2. **Test Before Exporting**: Preview how it will look
3. **Relative Image Paths**: Use relative paths for images
4. **Check Formulas**: Ensure LaTeX renders correctly in preview
5. **Code Language**: Specify language in code blocks for syntax highlighting