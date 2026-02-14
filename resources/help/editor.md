# Editor Features

TreeMk's editor is designed for efficient Markdown writing with smart features that enhance your productivity.

## Core Features

### Multi-Tab Editing

- Open multiple notes simultaneously
- Switch between tabs with **Ctrl+Tab** / **Ctrl+Shift+Tab**
- Close tab with **Ctrl+W**
- Reopen closed tab with **Ctrl+Shift+T**

### Smart Editing

#### Auto-Indent

When you press Enter, TreeMk automatically maintains the indentation level of the previous line.

#### Auto-Close Brackets

Type `(`, `[`, `{`, or `` ` `` and TreeMk automatically adds the closing character.

- `(` becomes `()`
- `[` becomes `[]`
- `{` becomes `{}`
- `` ` `` becomes `` `` ``
- Cursor is placed between the brackets for easy typing

#### List Continuation

When editing lists, pressing Enter automatically continues the list:

- Bullet lists: `- `, `* `, `+ `
- Numbered lists: `1. `, `2. `, etc.
- Task lists: `- [ ]`, `- [x]`

Press Enter on an empty list item to exit list mode.

#### Word Prediction

As you write, TreeMk learns words from your document and suggests completions. As you begin a new word, TreeMK uses the registry to try predicting what the next word will be.

- Type the beginning of a word
- Press **Tab** to accept the suggestion
- Uses unigram and bigram models for context-aware predictions

### Line Operations

- **Join Lines**: **Ctrl+J** - Merges current line with the next
- **Duplicate Line**: **Ctrl+D** - Duplicates the current line
- **Delete Line**: **Ctrl+Shift+K** - Deletes the current line
- **Move Line Up**: **Alt+Up** - Moves line up
- **Move Line Down**: **Alt+Down** - Moves line down

### List Operations

TreeMk provides special operations for working with Markdown lists:

- **Move List Item Up**: **Ctrl+Up** - Swaps current list item with the one above
- **Move List Item Down**: **Ctrl+Down** - Swaps current list item with the one below
- **Indent List Item**: **Tab** - Increases indentation (creates nested list)
- **Dedent List Item**: **Shift+Tab** - Decreases indentation (moves item out)

**Notes:**

- List items can only be swapped with adjacent items at the same indentation level
- Parent and child items at different indentation levels cannot be swapped
- Works with bullet lists (`-`, `*`, `+`), numbered lists, and task lists

### Text Formatting

Insert formatting shortcuts:

- **Bold**: **Ctrl+B** - Wraps selection with `**`
- **Italic**: **Ctrl+I** - Wraps selection with `*`
- **Code**: **Ctrl+`** - Wraps selection with backticks
- **Strikethrough**: **Ctrl+Shift+X** - Wraps selection with `~~`

### Headers

Insert headers quickly:

- **Ctrl+1** through **Ctrl+6** - Insert H1 through H6
- Automatically adds `#` symbols at the beginning of the line

### Insert Operations

#### Images

1. **From File**: **Ctrl+Shift+I** to select and insert an image
1. 
2. **From Clipboard**: **Ctrl+V** to paste images directly
   - Automatically saves to the document's folder
   - Generates unique filename
   - Inserts Markdown image syntax

#### Links
- **Wiki Link**: **Ctrl+K** - Insert `[[]]` link
- **Markdown Link**: **Ctrl+Shift+K** - Insert `[](url)` link

#### Formulas
- **Ctrl+Shift+M** - Opens formula dialog
- Insert inline formulas: `$x = y$`
- Insert block formulas: `$$x = y$$`

### Line Numbers and Display

- Toggle line numbers from **Settings → Editor**
- Current line highlighting
- Word wrap (configurable)
- Customizable fonts and colors

### Find and Replace

- **Find**: **Ctrl+F** - Search in current document
- **Replace**: **Ctrl+H** - Find and replace
- **Find in Files**: **Ctrl+Shift+F** - Search across all documents

## Customization

Access **Edit → Preferences** to customize:

- Editor font and size
- Tab width
- Auto-indent behavior
- Auto-close brackets
- Word prediction
- Line breaking at column width

## Advanced Features

### Session Management

TreeMk remembers:

- Open tabs
- Last opened folder
- Cursor positions
- Window layout

Enable in **Settings → General → Restore session on startup**

### Auto-Save

Automatically saves your work at regular intervals:

- Enable in **Settings → General → Auto-save**
- Configure interval (default: 1 minute)

### AI Assistance

TreeMk integrates with AI providers for intelligent writing help:

**Supported Providers:**

- **Ollama** - Run local AI models (llama2, mistral, etc.)
- **OpenAI** - Use GPT models via API
- **Compatible APIs** - Any OpenAI-compatible endpoint

**How to Use:**

1. Configure in **Edit → Preferences → AI Assistant**
2. Add API endpoint and key (if required)
3. Select your preferred model
4. Use **Ctrl+Shift+A** or right-click → AI Assist

**What AI Can Help With:**

- Improve writing clarity
- Expand ideas
- Summarize text
- Fix grammar and spelling
- Translate content
- Generate outlines

**Privacy Note:** When using cloud AI services, your text is sent to external servers. For privacy-sensitive content, consider using local models with Ollama.

## Next Steps

- Learn about **[Navigation](navigation.md)** features
- Explore **[Preview Features](preview.md)**
- See **[Keyboard Shortcuts](keyboard-shortcuts.md)** reference