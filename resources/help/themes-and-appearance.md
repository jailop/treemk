# Making TreeMk Look Like Home

Your workspace should feel comfortable. TreeMk adapts to your preferences with themes, custom styling, and font choices. Whether you're working in bright daylight or late at night, TreeMk can match your environment—and your taste.

## Choosing Your Theme

TreeMk offers three theme modes that affect the entire application interface:

**System** (default) - Follows your operating system's theme setting. Switch to dark mode on your OS, and TreeMk switches too. It's automatic, seamless.

**Light** - Always uses the light theme regardless of system settings. Clean, bright, classic. Perfect for well-lit rooms and traditional aesthetics.

**Dark** - Always uses the dark theme. Easy on the eyes in low light. Many people prefer it for long writing sessions.

To change your theme:
1. Open **Edit → Preferences**
2. Go to the **Appearance** tab
3. Under **Application Theme**, select your preference
4. Changes apply immediately—no restart needed

## Preview Color Schemes

The preview panel can use a different color scheme than the editor. Maybe you prefer dark mode for writing but want to see how your document looks in light mode. TreeMk handles this.

Preview schemes available:
- **Auto** - Matches the application theme
- **Light** - White background, black text, like printed paper
- **Dark** - Dark background, light text, easier on eyes at night

Set your preview scheme:
1. **Edit → Preferences → Preview**
2. Under **Color Scheme**, choose your option
3. The preview updates instantly

This is especially useful when you're preparing documents for others who might view them in light mode while you're writing in dark mode.

## Customizing Fonts

Different fonts work better for different people. Some prefer the clean lines of Sans Serif. Others need the distinction of Serif fonts. Coders often want monospace everywhere.

### Editor Font

Your editor font affects what you see while typing:

1. **Edit → Preferences → Editor**
2. **Font Family** - Click to browse installed fonts
3. **Font Size** - Adjust from 8 to 72 points
4. Changes preview immediately in the sample text

Popular choices:
- **Monospace fonts**: JetBrains Mono, Fira Code, Source Code Pro (great for technical writing)
- **Sans serif**: Inter, Roboto, Open Sans (clean and modern)
- **Serif**: Georgia, Merriweather (traditional, book-like)

Try a few. The right font makes writing feel effortless.

### Preview Font

The preview panel can use a different font:

1. **Edit → Preferences → Preview**
2. **Font Size** - Set the base size for rendered text
3. The preview respects your custom CSS (more on that below)

Many people use monospace in the editor but serif in the preview—writing in code style, reading in book style.

## Line Numbers and Visual Guides

Some writers love line numbers. Others find them distracting. TreeMk lets you choose:

1. **Edit → Preferences → Editor**
2. **Show Line Numbers** - Toggle on or off
3. **Current Line Highlight** - Subtle highlight on the line you're editing

Line numbers are useful when:
- Working with longer documents
- Collaborating and discussing specific lines
- Debugging structured content like code or lists

Turn them off when:
- Writing prose and want minimal distraction
- Using a small screen where every pixel counts

## Custom CSS: Making the Preview Yours

The preview panel renders your Markdown with HTML and CSS. Want to change how it looks? Add your own stylesheet.

### Creating a Custom CSS File

Create a file called `treemk-preview.css` (or any name you like) somewhere accessible. Here's a starting template:

```css
/* Make all headings blue */
h1, h2, h3, h4, h5, h6 {
    color: #2563eb;
}

/* Wider line spacing for readability */
p {
    line-height: 1.8;
}

/* Styled blockquotes */
blockquote {
    border-left: 4px solid #3b82f6;
    padding-left: 1rem;
    font-style: italic;
    background-color: #f1f5f9;
}

/* Highlight code blocks differently */
pre {
    background-color: #1e293b;
    border-radius: 8px;
    padding: 1rem;
}

/* Custom link colors */
a {
    color: #7c3aed;
    text-decoration: none;
}

a:hover {
    text-decoration: underline;
}
```

### Loading Your CSS

1. **Edit → Preferences → Preview**
2. **Custom CSS** - Click **Browse**
3. Select your CSS file
4. The preview updates immediately with your styles

### What You Can Style

The preview HTML includes these elements and classes:

**Standard Elements:**
- `h1`, `h2`, `h3`, `h4`, `h5`, `h6` - Headings
- `p` - Paragraphs
- `a` - Links
- `code` - Inline code
- `pre` - Code blocks
- `blockquote` - Blockquotes
- `ul`, `ol`, `li` - Lists
- `table`, `tr`, `th`, `td` - Tables
- `img` - Images

**Special Classes:**
- `.wiki-link` - Wiki-style links like `[[note]]`
- `.markdown-link` - Regular markdown links
- `.included-content` - Content from `[[!file]]` inclusions
- `.inclusion-title` - Title for included content
- `.language-*` - Code blocks (e.g., `.language-python`)

**Task Lists:**
- `.task-list-item` - Task list items
- `.task-list-item input[type="checkbox"]` - The checkboxes

### Example: Sepia Theme for Reading

Want a book-like reading experience?

```css
body {
    background-color: #f4ecd8;
    color: #5c4d3c;
}

a {
    color: #8b6914;
}

code {
    background-color: #e8dcc4;
    color: #5c4d3c;
}

pre {
    background-color: #e8dcc4;
    border: 1px solid #d4c5a9;
}
```

### Example: Minimal Monospace

Prefer everything in monospace?

```css
body {
    font-family: 'JetBrains Mono', 'Fira Code', monospace;
    font-size: 14px;
    line-height: 1.6;
}

h1, h2, h3, h4, h5, h6 {
    font-weight: bold;
}
```

### Finding Inspiration

Look at existing themes:
- GitHub's CSS for Markdown
- Dev.to article styling
- Bear notes appearance
- Notion's clean design

Copy what works. Tweak colors and spacing. Make it yours.

## Layout and View Modes

Control how much screen space each panel gets:

**View Modes** (Cycle with **Ctrl+\\**):
- **Both** - Editor and preview side by side (default)
- **Editor Only** - Full screen for writing, no preview
- **Preview Only** - Full screen for reading rendered output

**Toggle Preview**: **Ctrl+R** shows/hides the preview panel entirely.

**Resize Panels**: Drag the divider between editor and preview to adjust widths. TreeMk remembers your preference.

## Zoom In and Out

Sometimes you need bigger text. Other times, you want to see more content.

**Preview Zoom:**
- **Ctrl++** (Ctrl and Plus) - Zoom in
- **Ctrl+-** (Ctrl and Minus) - Zoom out  
- **Ctrl+0** (Ctrl and Zero) - Reset to default

This only affects the preview panel, not the editor.

## Dark Mode Best Practices

Using dark mode? A few tips:

1. **Syntax Highlighting**: TreeMk automatically switches to dark-mode-friendly code highlighting in the preview
2. **Custom CSS**: If you use custom CSS, add dark mode variants using media queries:

```css
/* Light mode styles */
body {
    background-color: white;
    color: black;
}

/* Dark mode overrides */
@media (prefers-color-scheme: dark) {
    body {
        background-color: #1e1e1e;
        color: #e0e0e0;
    }
}
```

3. **Screen Brightness**: Pair dark mode with reduced screen brightness for the most comfortable experience

## Making It Consistent

Want your workspace to feel unified? Here's a coherent setup:

**For Dark Mode Users:**
1. Application Theme: Dark
2. Preview Scheme: Dark
3. Editor Font: JetBrains Mono (monospace, designed for dark backgrounds)
4. No custom CSS needed—the defaults look great

**For Light Mode Users:**
1. Application Theme: Light
2. Preview Scheme: Light
3. Editor Font: Inter or Roboto (clean sans-serif)
4. Optional custom CSS for serif preview:

```css
body {
    font-family: Georgia, serif;
    font-size: 16px;
}
```

**For System Following:**
1. Application Theme: System
2. Preview Scheme: Auto
3. Font: Whatever you prefer
4. Custom CSS with dark mode support (see above)

## Troubleshooting

**Custom CSS not applying?**
- Check file path is correct
- Ensure CSS syntax is valid (one typo breaks everything)
- Try removing it and adding it again

**Preview looks broken?**
- Reset Custom CSS to none
- Restart TreeMk
- Your CSS might have conflicting rules

**Fonts not showing up?**
- Ensure font is installed system-wide
- Try closing and reopening Preferences
- Check font name spelling (case-sensitive on some systems)

**Theme not changing?**
- Some themes require app restart (rare)
- Check system theme settings if using "Auto"

## Going Further

Your TreeMk setup is personal. Experiment. Try different combinations. You might discover that:

- Sepia backgrounds reduce eye strain
- Larger fonts make writing flow better
- Monospace everywhere helps you focus
- Custom CSS makes reviewing documents enjoyable

The best setup is the one where you forget about the interface and just write.

## Quick Reference

| Setting | Location | Effect |
|---------|----------|--------|
| Application Theme | Edit → Preferences → Appearance | Changes entire UI |
| Preview Scheme | Edit → Preferences → Preview | Changes preview colors |
| Editor Font | Edit → Preferences → Editor | Changes what you type in |
| Preview Font Size | Edit → Preferences → Preview | Changes rendered text size |
| Custom CSS | Edit → Preferences → Preview | Adds your styling |
| Line Numbers | Edit → Preferences → Editor | Shows/hides line numbers |
| View Mode | Ctrl+\\ or View menu | Editor/Preview/Both |
| Preview Zoom | Ctrl++ / Ctrl+- / Ctrl+0 | Zoom preview panel |

Make TreeMk yours. Your workspace, your rules.
