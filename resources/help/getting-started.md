# Your First Steps with TreeMk

Welcome to TreeMk! If you're coming from proprietary note apps with locked-in databases, you're about to experience something refreshingly different. TreeMk works directly with your files—plain Markdown files sitting in regular folders on your computer. No import, no export, no database. Just your notes, accessible to you and any tool you choose.

Let's get you started.

## Opening Your Notes

TreeMk doesn't work with individual files—it works with entire folders. Think of a folder as your workspace, your project, or your knowledge base. You might have one folder for work notes, another for personal journals, and another for a writing project.

Press **Ctrl+O** (or choose **File → Open Folder** from the menu), then select any folder on your computer. TreeMk immediately shows you all its files in the tree on the left side of the window. That's your file browser—a live view of what's actually on your disk.

Don't have a notes folder yet? No problem. Create one wherever you like—maybe `Documents/Notes` or `~/notes` or anywhere else that makes sense to you. TreeMk doesn't care where your files live.

## Creating Your First Note

See that file tree on the left? Right-click anywhere in it and choose **New File**. TreeMk asks for a name—let's call it `welcome.md`. The `.md` extension means it's a Markdown file, but you can name it anything you like.

TreeMk creates the file and opens it in the editor. The cursor blinks at you, ready for your first words. Go ahead, type something:

```markdown
# My First Note

Today I learned that TreeMk works with regular files. This means:

- I can edit my notes with any text editor
- I can sync them with Dropbox, Git, or whatever I want
- I can search them with grep
- They're mine, forever

This feels liberating.
```

As you type, notice the preview panel on the right. It updates automatically, showing you what your Markdown will look like when rendered. Your heading appears large and bold. Your list gets proper bullets. The magic happens in real-time.

## Connecting Ideas

Now let's create another note. Right-click in the file tree again, choose **New File**, and call it `markdown-tips.md`. In this new note, type:

```markdown
# Markdown Tips

Markdown is wonderfully simple. Here are the basics I use most:

**Bold** text with double asterisks
*Italic* text with single asterisks
`code` with backticks
```

Now go back to your `welcome.md` note (click its tab at the top). Let's link these notes together. Add a line:

```markdown
I should learn more about [[markdown-tips]].
```

See what happened? Those double brackets create a wiki-style link. **Ctrl+Click** on it, and TreeMk opens the `markdown-tips.md` note. No hunting through file dialogs, no typing full paths—just natural connections between your ideas.

Now go to the **Backlinks** tab in the left sidebar (it's the link icon). You'll see that `welcome.md` links to this note. TreeMk tracks these connections automatically, helping you discover how your ideas relate to each other.

## Understanding Your Workspace

Take a moment to explore the interface. You're looking at three main areas:

**Left Sidebar** has four tabs (hover to see what each does):
- **Files** (folder icon): Your file tree, showing the actual files and folders on disk
- **Outline** (list icon): All the headings in your current document—click any heading to jump there
- **Backlinks** (link icon): Which other notes link to this one (incredibly useful for discovering connections)
- **History** (clock icon): Recent files you've opened, so you can retrace your steps

**Center Editor** is where you write:
- Multiple tabs let you work on several notes at once
- Line numbers help you navigate (toggle them in settings if you prefer)
- The editor highlights your Markdown syntax as you type
- It even learns your vocabulary and suggests words as you write

**Right Preview** shows your rendered Markdown:
- Updates live as you type
- Displays formatted text, code blocks with syntax highlighting, math equations, diagrams
- Press **Ctrl+R** to toggle it on or off
- Press **Ctrl+\\** to cycle through editor-only, preview-only, or both views

## Quick Markdown Reference

You don't need to memorize much. Here's what you'll use most often:

```markdown
# Big Heading
## Medium Heading
### Smaller Heading

**bold text** or __bold text__
*italic text* or _italic text_
~~strikethrough~~
`inline code`

- bullet list
- another item
  - nested item

1. numbered list
2. second item

[website link](https://example.com)
[[internal link to another note]]

![image](path/to/image.png)
```

Code blocks use triple backticks:

````markdown
```python
print("Hello, World!")
```
````

Math equations use dollar signs:

```markdown
Inline math: $E = mc^2$

Display math:
$$
\int_0^\infty e^{-x} dx = 1
$$
```

## A Few Power Moves

Before you dive deeper, here are some shortcuts that'll quickly become second nature:

- **Ctrl+P**: Quick open—start typing any filename and TreeMk finds it instantly
- **Ctrl+Shift+F**: Search across all your notes—find that idea you wrote last month
- **Ctrl+K**: Insert a `[[wiki link]]` to another note
- **Ctrl+V**: Paste an image from your clipboard—TreeMk saves it automatically
- **Alt+Left** and **Alt+Right**: Navigate backward and forward through files you've opened (like a browser)
- **Alt+D**: Insert today's date
- **F1**: See all keyboard shortcuts

## What's Next?

You've created notes, linked them together, and explored the interface. You're ready to build your knowledge base. Here's where to go from here:

- **[Writing with TreeMk's Editor](editor.md)**: Discover all the smart features that make writing effortless
- **[Navigating Your Notes](navigation.md)**: Master wiki links, backlinks, search, and history
- **[Preview Features](preview.md)**: Learn about formulas, diagrams, code highlighting, and more
- **[Keyboard Shortcuts](keyboard-shortcuts.md)**: Speed up your workflow with shortcuts

Welcome aboard. Your notes are yours again.
