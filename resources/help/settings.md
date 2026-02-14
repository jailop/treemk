# Settings Reference

TreeMk is customizable. Every user works differently, and TreeMk adapts to your preferences. Here's a complete guide to every setting available.

## Accessing Settings

**Menu:** Edit → Preferences  
**Keyboard:** Depends on your platform (often Ctrl+, or Cmd+,)

Settings are organized into tabs. Changes apply immediately—no need to click "Save" or "OK" in most cases.

## General Tab

### Session Management

**Restore session on startup**
- **What it does:** When enabled, TreeMk reopens the tabs you had open last time, restores cursor positions, and remembers which folder you were working in
- **Default:** Enabled
- **When to use:** Always, unless you prefer starting fresh each time
- **Note:** Also restores window size and position

**Recent Folders**
- **What it does:** File → Recent Folders shows workspaces you've opened recently
- **Number of entries:** Configurable (usually 10-20)
- **How to clear:** Usually a "Clear" button in the Recent Folders menu

### Auto-Save

**Enable auto-save**
- **What it does:** Automatically saves your documents at regular intervals
- **Default:** Often enabled with 60-second interval
- **When to disable:** If you prefer manual control, or auto-save interferes with other tools watching your files

**Auto-save interval**
- **Range:** 10 seconds to 600 seconds (10 minutes)
- **Default:** 60 seconds (1 minute)
- **Recommendation:** 60-120 seconds balances safety and performance

## Editor Tab

### Font and Display

**Font Family**
- **What it does:** Sets the typeface for the editor
- **Default:** Usually a monospace font like "Monospace" or "Courier"
- **Popular choices:**
  - JetBrains Mono (modern, clear ligatures)
  - Fira Code (programmer favorite)
  - Source Code Pro (Adobe's open-source monospace)
  - Consolas (Windows default, clean)
  - SF Mono (macOS system font)
- **Tip:** Try a few. The right font makes typing feel natural

**Font Size**
- **Range:** 8 to 72 points
- **Default:** Usually 10-12 points
- **Recommendation:** 
  - 10-12 for high-DPI screens
  - 12-14 for standard screens
  - 14+ if you prefer larger text or have vision considerations

**Tab Width**
- **Range:** 2 to 8 spaces
- **Default:** 4 spaces
- **When to change:**
  - 2 spaces: Python developers, compact style
  - 4 spaces: Most common, balanced
  - 8 spaces: Old-school Unix style, very indented

### Behavior

**Auto-indent**
- **What it does:** When you press Enter, the next line starts at the same indentation level as the previous line
- **Default:** Enabled
- **When to disable:** Rarely. Auto-indent saves enormous time with lists and nested content

**Auto-close brackets**
- **What it does:** Typing `(` automatically adds `)` and places cursor between them. Same for `[`, `{`, and backticks
- **Default:** Enabled
- **When to disable:** If you find it interferes with your typing flow

**Enable word prediction**
- **What it does:** As you type, TreeMk suggests word completions based on words from all Markdown files in your current directory. Press Tab to accept
- **Default:** Enabled
- **How it works:** 
  - Builds unigram (single word) and bigram (word pair) frequency models
  - Scans all `.md` and `.markdown` files in the current directory
  - Learns vocabulary from your entire workspace, not just the current file
  - Updates prediction model when you open or switch between files
- **When to disable:** If suggestions distract you, or on very slow machines
- **Takes effect:** Immediately across all open documents when changed in preferences

**List continuation**
- **What it does:** Pressing Enter in a list automatically continues the list with the next marker
- **Default:** Enabled
- **Covers:** Bullet lists (`-`, `*`, `+`), numbered lists, task lists (`- [ ]`)
- **Tip:** Press Enter twice on an empty list item to exit list mode

### Visual Guides

**Show line numbers**
- **What it does:** Displays line numbers in the left margin
- **Default:** Usually enabled
- **When to enable:** 
  - Working with long documents
  - Collaborating and discussing specific lines
  - Debugging structured content
- **When to disable:**
  - Writing prose where line numbers distract
  - Small screen where space is precious

**Highlight current line**
- **What it does:** Subtly highlights the line your cursor is on
- **Default:** Usually enabled
- **Why it helps:** Makes it easy to find your cursor after scrolling or switching windows

**Word wrap**
- **What it does:** Long lines wrap to fit in the editor window instead of scrolling horizontally
- **Default:** Usually enabled
- **When to disable:** If you prefer horizontal scrolling, or working with code where line length matters

### Line Breaking

**Enable line breaking**
- **What it does:** Enables Ctrl+Shift+B to break long lines at word boundaries
- **Default:** Usually disabled
- **Requires:** Setting a line break column width

**Line break column**
- **Range:** Usually 60-120 characters
- **Default:** 80 characters (classic terminal width)
- **Common values:**
  - 72: Email standard
  - 80: Programming standard
  - 100: Modern compromise
  - 120: Wide screen comfort
- **How to use:** With line breaking enabled, Ctrl+Shift+B breaks lines longer than this width

## Preview Tab

### Display

**Font Size**
- **Range:** 8 to 24 points
- **Default:** 12-14 points
- **What it affects:** Base font size in the preview. Headings scale proportionally
- **Tip:** Larger preview text can make reviewing easier

**Color Scheme**
- **Options:** Auto / Light / Dark
- **Auto:** Matches application theme
- **Light:** Always white background
- **Dark:** Always dark background
- **Why separate from app theme:** You might write in dark mode but want to preview how documents look in light mode

### Rendering

**Refresh Rate**
- **Range:** 100-2000 milliseconds
- **Default:** Usually 300-500ms
- **What it does:** How long TreeMk waits after you stop typing before updating the preview
- **Lower (100-200ms):** More responsive, uses more CPU
- **Higher (1000-2000ms):** Less CPU usage, slight delay before seeing changes
- **Recommendation:** 300-500ms is a good balance

**Enable LaTeX rendering**
- **What it does:** Renders math formulas like `$E=mc^2$` and `$$\int_0^\infty e^{-x}dx$$`
- **Default:** Enabled
- **Uses:** KaTeX or MathJax (depending on build)
- **When to disable:** If you don't use math and want slightly faster rendering

### Customization

**Custom CSS File**
- **What it does:** Loads your CSS file to style the preview
- **Default:** None
- **How to use:** 
  1. Create a .css file with your styles
  2. Click Browse and select it
  3. Preview updates immediately
- **See also:** [Themes and Appearance](themes-and-appearance.md) for CSS examples

## AI Assistant Tab

*(If AI features are enabled in your build)*

### Enable/Disable AI Features

**Enable AI Assistance**
- **What it does:** Master toggle that controls all AI functionality
- **Default:** Enabled (if AI support is compiled in)
- **When disabled:**
  - AI Assist toolbar button is disabled (grayed out and non-clickable)
  - Edit → AI Assist menu submenu is disabled
  - AI Assist options are completely hidden from editor context menus
  - No AI-related prompts or dialogs can be accessed
  - Provides a clean, distraction-free interface without AI elements
- **When enabled:** All AI features become available through multiple access points
- **Takes effect:** Immediately across all interface elements
- **Use cases for disabling:**
  - You don't use AI features and want a simpler interface
  - Privacy-sensitive work where AI options should not be visible
  - Reducing UI clutter to focus on writing
  - Teaching or demonstration environments where AI should not be accessible

### Provider Settings

**AI Provider**
- **Options:** None / Ollama / OpenAI / Custom
- **None:** Disables AI features
- **Ollama:** Local AI running on your computer
- **OpenAI:** Cloud-based GPT models
- **Custom:** Any OpenAI-compatible API

**Endpoint URL**
- **For Ollama:** Usually `http://localhost:11434`
- **For OpenAI:** Pre-configured
- **For Custom:** Your API endpoint

**API Key**
- **When needed:** OpenAI and most custom endpoints
- **Not needed:** Ollama (local)
- **Security:** Stored securely on your system

**Model Name**
- **For Ollama:** Model you've pulled (e.g., `llama2`, `mistral`)
- **For OpenAI:** `gpt-3.5-turbo`, `gpt-4`, etc.
- **For Custom:** Depends on your provider

### Behavior

**Temperature**
- **Range:** 0.0 to 2.0
- **Default:** Usually 0.7-1.0
- **Lower (0.0-0.5):** More focused, deterministic output
- **Higher (1.0-2.0):** More creative, varied output
- **Recommendation:** 0.7 for most tasks, 1.2+ for creative writing

**Max Tokens**
- **What it does:** Maximum length of AI responses
- **Range:** 256 to 4096 or more
- **Higher:** Longer responses, more cost (for paid APIs)
- **Lower:** Shorter responses, faster

**Timeout**
- **Range:** 10 to 120 seconds
- **What it does:** How long to wait for AI response before giving up
- **Recommendation:** 30-60 seconds for cloud, 60-120 for local

### System Prompts

**Manage prompts:** Tools → System Prompts

**What they are:** Pre-configured AI prompts that appear in Edit → AI Assist menu and toolbar dropdown

**Default prompts:**
- Rephrase
- Fix Grammar
- Make Shorter
- Make Longer
- Simplify Language
- Professional Tone
- Casual Tone
- Summarize
- Translate to Spanish
- Add Examples

**Customization:**
- Add new prompts for your specific needs
- Edit existing prompts to match your style
- Reorder with Move Up/Down buttons
- Delete prompts you don't use

## Shortcuts Tab

*(If customizable shortcuts are implemented)*

**Customizable Shortcuts**
- **What it does:** Remap any keyboard shortcut to your preference
- **How to use:**
  1. Find the action you want to remap
  2. Click on the current shortcut
  3. Press your desired key combination
  4. Conflicts are highlighted
  5. Changes apply immediately

**Categories:**
- File operations
- Editing
- Formatting
- Navigation
- Search
- View
- Help

**Tip:** Export your shortcuts to a file so you can restore them if needed, or share them across machines

## Appearance Tab

### Theme

**Application Theme**
- **Options:** System / Light / Dark
- **System:** Follows OS theme (auto-switches when OS changes)
- **Light:** Always light mode
- **Dark:** Always dark mode
- **Affects:** Entire TreeMk UI (menus, panels, dialogs)

**UI Scaling** *(platform-dependent)*
- **What it does:** Adjusts UI element sizes
- **When to use:** On high-DPI displays or if UI feels too small/large
- **Range:** Usually 100% to 200%

## Export Tab

*(If export settings are configurable)*

**Default Export Format**
- **Options:** HTML / PDF / DOCX / Text
- **What it does:** Pre-selects this format in the export dialog

**Pandoc Options**
- **What it does:** Additional command-line flags passed to Pandoc
- **Examples:**
  - `--toc` - Generate table of contents
  - `--number-sections` - Number headings
  - `--template=mytemplate.html` - Use custom template
- **Advanced:** Requires Pandoc knowledge

**PDF Engine**
- **Options:** pdflatex / xelatex / lualatex
- **Default:** pdflatex
- **When to change:** 
  - xelatex: Better Unicode support
  - lualatex: Modern features, Unicode
  - Most users: stick with pdflatex

## Resetting Settings

**To reset all settings to defaults:**

Most settings dialogs have a "Reset to Defaults" or "Restore Defaults" button.

**Manual reset (advanced):**

Settings are stored in:
- **Linux:** `~/.config/TreeMk/` or `~/.config/DataInquiry/TreeMk.conf`
- **Windows:** `%APPDATA%\TreeMk\` or Registry
- **macOS:** `~/Library/Preferences/`

Delete these files/keys to reset completely (TreeMk must be closed).

## Settings Tips

**Experiment:** Most settings apply immediately. Try different values to find what works.

**Document your changes:** If you customize extensively, note what you changed. Helps when setting up TreeMk on a new machine.

**Profile-specific:** Some users keep different settings files for different workflows (writing vs. coding vs. note-taking).

**Performance:** If TreeMk feels slow:
- Increase preview refresh rate
- Disable word prediction
- Reduce font size
- Disable auto-save or increase interval

**Accessibility:** 
- Larger fonts
- High contrast themes (dark mode often helps)
- Disable distracting features like predictions

## Next Steps

Now that you know all the settings:
- **[Themes and Appearance](themes-and-appearance.md)** for styling details
- **[Keyboard Shortcuts](keyboard-shortcuts.md)** for full shortcut list
- **[Editor Features](editor.md)** to see what each setting enables

Your TreeMk, your way.
