# Navigation Features

TreeMk provides powerful navigation features to help you organize and explore your notes efficiently.

## Wiki-Style Links

### Creating Links

Create links between your notes using double brackets:

```markdown
[[note-name]]
```

### Following Links

- **Ctrl+Click** on a link to open the target note
- If the file doesn't exist, TreeMk will ask if you want to create it
- Links automatically adapt to your folder structure

### Link Syntax Variations

```markdown
[[simple-link]]                    # Links to simple-link.md
[[folder/note]]                    # Links to file in subfolder
[[note#section]]                   # Links to specific heading
[[custom text|actual-file-name]]   # Display text different from filename
```

## File Tree Navigation

### File Explorer

![File Explorer](images/filepanel.png)


The left panel shows your folder structure:

- Browse folders and files
- Right-click for context menu (New, Rename, Delete)
- Drag files to the editor to create links
- **F5** to refresh the tree

### Quick Operations

- **F2**: Rename selected file
- **Delete**: Delete selected file (with confirmation)
- **Double-click**: Open file in editor
- **Right-click**: Access full context menu
- **Drag & Drop**: Drag files from tree to editor to create `[[wiki-links]]`

### Clipboard Support

- Paste images directly into notes with **Ctrl+V**
- Images are auto-saved to the document folder
- Automatic filename generation with timestamps
- Markdown syntax inserted automatically

## Outline Panel

### Document Structure

The Outline tab shows all headings in the current document:

- Click any heading to jump to that section
- Hierarchical view of document structure
- Auto-updates as you edit

### Heading Levels

TreeMk recognizes all Markdown heading levels:

- `# H1` - Main document title
- `## H2` - Major sections
- `### H3` - Subsections
- And so on...

## Backlinks

### What Are Backlinks?

Backlinks show all notes that link to the current document.

### Using Backlinks

1. Open any note
2. Switch to the **Backlinks** tab
3. See all notes that reference this one
4. Click any backlink to open that note

### Benefits

- Discover connections between notes
- Find all references to a topic
- Navigate your knowledge graph
- Build a networked note system

## Quick Open

### Fast File Access

Press **Ctrl+P** to open the Quick Open dialog:

- Start typing a filename
- Fuzzy search matches parts of names
- Arrow keys to select
- **Enter** to open

### Fuzzy Matching

Type any part of the filename:

- `gett` matches "getting-started.md"
- `key sho` matches "keyboard-shortcuts.md"
- Works with partial matches

## Search in Files

### Full-Text Search

Press **Ctrl+Shift+F** to search across all files:

- Enter search term
- See all matches with context
- Click any result to open that file
- Search is case-insensitive by default

### Search Features

- Preview matching lines
- Jump directly to matches
- Search entire workspace
- Regular expression support

## Tab Navigation

### Multiple Tabs

Work with multiple notes simultaneously:

- **Ctrl+Tab**: Next tab
- **Ctrl+Shift+Tab**: Previous tab
- **Ctrl+W**: Close current tab
- **Ctrl+Shift+T**: Reopen closed tab

### Tab Management

- Tabs remember their position
- Session restoration on restart
- Close tabs from other folders when switching workspaces
- Drag tabs to reorder

## Recent Folders

### Workspace Switching

Access recently opened folders:

- **File → Recent Folders** menu
- Quick switching between projects
- Recently used folders at the top
- Clear history option available

## Tips for Efficient Navigation

1. **Use Wiki Links**: Create a web of interconnected notes
2. **Leverage Backlinks**: Discover unexpected connections
3. **Quick Open**: Fastest way to jump between files
4. **Outline Panel**: Navigate long documents quickly
5. **Tab Groups**: Keep related notes open together
6. **Search**: Find anything across your entire workspace

## Keyboard Shortcuts

For a complete list of navigation shortcuts, see **[Keyboard Shortcuts](keyboard-shortcuts.md)**.

## Next Steps

- Learn about **[Preview Features](preview.md)** for rendering
- Explore **[Editor Features](editor.md)** for editing power
- Return to **[User Guide](index.md)** for overview