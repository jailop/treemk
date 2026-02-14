# Writing with TreeMk's Editor

Welcome to TreeMk's editor—a thoughtfully designed writing environment that gets out of your way while providing powerful features when you need them. Let's explore how to make the most of it.

## Getting Started with Multiple Documents

Picture this: you're working on a project that spans multiple notes. Maybe you're drafting a blog post while referencing your research notes and a todo list. TreeMk handles this naturally with tabs.

Open a new file with **Ctrl+N**, and it appears as a new tab alongside your current document. Switch between them quickly using **Ctrl+Tab** (forward) or **Ctrl+Shift+Tab** (backward)—just like your web browser. When you're done with a tab, close it with **Ctrl+W**. Accidentally closed something? **Ctrl+Shift+T** brings it right back.

## Smart Writing Assistance

### The Editor That Anticipates Your Needs

TreeMk pays attention to what you're doing and helps you maintain consistency. When you press Enter at the end of an indented line, the next line starts at the same indentation level. No need to manually align your text—the editor remembers where you are.

Type an opening parenthesis `(`, bracket `[`, brace `{`, or backtick `` ` ``, and TreeMk immediately adds the closing character, placing your cursor between them. It's a small thing, but when you're typing mathematical expressions like `(a + b) * (c + d)` or code snippets with multiple `[[nested]]` structures, these micro-optimizations add up.

### Lists That Write Themselves

Start a list by typing `- ` or `* ` or `+ ` at the beginning of a line. Press Enter, and TreeMk continues the list automatically. Working with numbered lists? Type `1. ` and each Enter gives you the next number. Creating a task list? Type `- [ ]` for a checkbox or `- [x]` for a completed task—TreeMk keeps adding checkboxes as you go.

When you've finished your list, just press Enter twice on an empty list item, and TreeMk understands you're done.

### Words That Complete Themselves

As you write, TreeMk quietly learns the vocabulary of your workspace. It builds a model of word patterns—both individual words (unigrams) and common word pairs (bigrams)—from all Markdown files in your current directory. When you start typing a word, TreeMk might suggest a completion based on this collective vocabulary. Press **Tab** to accept it, or just keep typing to ignore it.

This isn't just about saving keystrokes. When you're working on a project with specialized terminology, technical documentation with long variable names, or a collection of related notes, word prediction becomes a genuine productivity boost. The prediction model learns from all your `.md` and `.markdown` files in the folder, so terms you use frequently across different documents become readily available.

You can enable or disable word prediction in **Edit → Preferences → Editor**. When disabled, no predictions appear as you type. This setting applies immediately across all open documents.

## Working with Lines and Text

### Moving Things Around

You're editing a paragraph and realize the second sentence should come first. Select the line and press **Alt+Up** to move it up, or **Alt+Down** to move it down. Need a copy of a line? **Ctrl+D** duplicates it immediately. Want it gone? **Ctrl+Shift+K** deletes the entire line.

Have two short lines that should be one? Place your cursor on the first line and press **Ctrl+J** to join them. Got a line that's too long? **Ctrl+Shift+B** intelligently breaks it at word boundaries (if you've enabled line breaking in settings).

For more complex operations, **Ctrl+Shift+J** joins either your selected lines or all lines in a paragraph—perfect for when you've written prose in short, wrapped lines and want to consolidate them.

### Mastering List Operations

Lists deserve special treatment. When you're reorganizing ideas, you can move entire list items up and down with **Ctrl+Up** and **Ctrl+Down**. TreeMk is smart about this—it only swaps items at the same indentation level, so you won't accidentally make a parent item a child of its former child.

Want to nest an idea deeper? Press **Tab** to indent it. Changed your mind? **Shift+Tab** brings it back out. This works with bullet lists, numbered lists, and task lists alike.

## Formatting Your Text

Select any text and press **Ctrl+B** to make it **bold**—TreeMk wraps it in `**asterisks**`. **Ctrl+I** makes it *italic* with single `*asterisks*`. **Ctrl+`** turns it into `inline code` with backticks. Need to show something is obsolete? **Ctrl+Shift+X** applies ~~strikethrough~~ with tildes.

Headers are just as quick. Place your cursor on a line and press **Ctrl+1** through **Ctrl+6** to make it a heading at that level. The appropriate number of `#` symbols appears at the start of the line.

## Inserting Rich Content

### Images, Naturally

Working with images should be effortless. Press **Ctrl+Shift+I** to select an image file from your computer, and TreeMk inserts the Markdown syntax for you. But here's the really nice part: copy an image to your clipboard from anywhere—a screenshot, an image from a website, a picture from another document—and just paste it with **Ctrl+V**. TreeMk saves the image to an `./images` subdirectory (creating it if needed), prompts you for a filename, and inserts the reference. All files in the same directory share the same `./images` folder, keeping your workspace organized. No fuss.

### Linking Documents Together

Press **Ctrl+K** to insert a wiki-style link: `[[]]`. Type the name of another note, and you're done. TreeMk will find it (or offer to create it) when you click the link. For traditional Markdown links to websites or files, **Ctrl+Shift+K** gives you `[text](url)` with your cursor ready to fill in the details.

### Mathematical Expressions

Need to write an equation? Press **Ctrl+Shift+M** to open the formula dialog. You can insert inline math like `$E = mc^2$` or display equations like `$$\int_0^\infty e^{-x} dx = 1$$`. The preview panel renders them beautifully using LaTeX notation.

### Including External Files

Here's where things get interesting. Imagine you're writing documentation and want to include an actual source code file, not just a copy that could get out of date. In a code block, type `[[!filename]]` where `filename` is your source file:

````markdown
```python
[[!calculate.py]]
```
````

When the preview renders, TreeMk reads `calculate.py` and displays its contents right there in your code block. Update the source file, and the preview updates automatically. This works with any text file—Python scripts, configuration files, snippets of prose—anything you want to include dynamically.

You can also use `[[!filename]]` outside of code blocks to embed entire Markdown documents. Perfect for reusable snippets, shared sections, or modular documentation. Use `[[!snippet|Custom Title]]` if you want to override the display title.

### Dates and Times

Writing a journal entry? Meeting notes? Press **Alt+D** to insert today's date. TreeMk remembers your preferred format (YYYY-MM-DD, DD/MM/YYYY, "February 14, 2026", etc.) and uses it next time. Similarly, **Alt+T** inserts the current time in your preferred format. Small conveniences that make daily note-taking feel effortless.

## Finding What You Need

### Quick Find

Press **Ctrl+F** to search within the current document. A search bar appears at the top. As you type, TreeMk highlights all matches in the editor.

**Search Options:**
- **Case Sensitive** - Toggle to match exact capitalization
- **Whole Words** - Find "note" without matching "notes" or "denote"
- **Regular Expressions** - Use regex patterns for complex searches
- **F3** or **Ctrl+G** - Jump to next match
- **Shift+F3** or **Ctrl+Shift+G** - Jump to previous match

**Regex Examples:**
- `\d{4}-\d{2}-\d{2}` - Find dates (2026-02-14)
- `TODO:.*` - Find all TODO comments
- `^#+ ` - Find all headings
- `\[\[.*\]\]` - Find all wiki links

### Find and Replace

Press **Ctrl+H** to open find-and-replace. Same options as Quick Find, plus:

**Replace Options:**
- **Replace** - Replace current match
- **Replace All** - Replace every match in document
- **Preview** - See what will change before committing

**Use Case:** Rename a term throughout your document. Search for "quantum mechanics" and replace with "QM" everywhere. One click.

### Search Across Files

Need to find something across all your notes? Press **Ctrl+Shift+F**.

**What it searches:**
- All `.md` and `.markdown` files in your workspace
- Text content (not filenames)
- Respects `.gitignore` if present

**Search Results:**
- Shows each match with context (surrounding lines)
- Click any result to jump to that file and line
- Results grouped by file
- Live updates as you type

**Power Tip:** Use regex here too. Find all notes mentioning a person: `(Alice|Bob|Charlie)`. Find all empty headings: `^#+\s*$`.

## Making TreeMk Yours

The editor adapts to your preferences. Open **Edit → Preferences** to customize fonts, tab widths, and colors. Toggle auto-indent, auto-close brackets, or word prediction based on how you like to work. 

**Line Breaking:** If you prefer lines to break at a specific column (like 80 characters for email-style text), go to **Edit → Preferences → Editor** and:
1. Enable **Line Breaking**
2. Set **Line Break Column** to your preferred width (60-120 characters)
3. Press **Ctrl+Shift+B** to break long lines at word boundaries

Want TreeMk to remember your work session? Enable **Settings → General → Restore session on startup**, and next time you launch TreeMk, all your tabs, cursor positions, and window layout return exactly as you left them.

Enable **Settings → General → Auto-save** to automatically save your work every minute (or whatever interval you prefer). Never lose a thought to an unexpected interruption.

## Writing with AI Assistance

Sometimes you need a second opinion on your writing, or a quick translation, or help expanding a rough outline into full sentences. TreeMk integrates with AI services to provide exactly that kind of help.

Configure your preferred provider in **Edit → Preferences → AI Assistant**:

- **Ollama** runs AI models locally on your computer—completely private, no internet required
- **OpenAI** uses GPT models via their API—powerful, but sends your text to their servers
- Any **OpenAI-compatible endpoint** works too

Once configured, select any text and access AI assistance through:

- **Ctrl+Shift+A** - Open AI assistant dialog for custom prompts
- **Toolbar button** - Click the AI Assist button (or its dropdown for predefined prompts)
- **Edit menu** - Choose **Edit → AI Assist** and select a predefined prompt
- **Context menu** - Right-click selected text and choose **AI Assist**

Ask it to improve clarity, fix grammar, translate to another language, expand your ideas, or summarize long sections. The AI processes your request and returns suggestions you can insert, replace, or just copy for reference.

For privacy-sensitive work—personal journals, confidential notes, anything you wouldn't want leaving your computer—stick with Ollama. For everything else, cloud AI services offer convenience and power.

**Note:** If you prefer not to use AI features, you can disable them completely in **Edit → Preferences → AI Assistant** by unchecking **Enable AI Assistance**. When disabled, all AI-related UI elements (toolbar button, menu items, and context menu options) are hidden or disabled, giving you a distraction-free writing environment.

## Where to Go From Here

You've learned how TreeMk's editor helps you write more effectively. Now explore how to organize and navigate your notes with **[Navigation](navigation.md)** features, see how the **[Preview](preview.md)** panel renders your Markdown beautifully, or dive into the full **[Keyboard Shortcuts](keyboard-shortcuts.md)** reference to discover even more ways to work efficiently.

Happy writing!