# Contributing to TreeMk

Thank you for your interest in contributing to TreeMk!

## TreeMk's Philosophy: Excellence Through Integration

TreeMk is built on a powerful principle: **seamlessly integrate Markdown editing with file system navigation, doing both exceptionally well**.

Most Markdown editors treat your file system as an afterthought, hiding it behind proprietary vaults or databases. TreeMk takes the opposite approach—it embraces your file system as a first-class citizen. We combine a powerful Markdown editor with integrated file browsing, wiki-style linking, and rich navigation features, all working directly with your files where they live.

### What Makes TreeMk Special

TreeMk's core strength is the tight integration between editing and file system navigation:

- **Direct file system access**: No proprietary vaults, no databases—just your files organized your way
- **Integrated file explorer**: Browse, navigate, and manage files without leaving the editor
- **Wiki-style linking**: Connect documents naturally, with backlinks showing how your notes relate
- **Navigation history**: Move through your document graph with back/forward navigation
- **File system freedom**: Use any sync tool (Syncthing, Nextcloud, Dropbox), any version control (Git, Mercurial), any backup solution

Your Markdown files remain plain text files in your file system, accessible to any tool, script, or application you choose. TreeMk enhances how you work with them without claiming ownership.

### What We Focus On

We invest our energy in two core areas that work together:

**Markdown Editing Excellence:**

- Smart editor features (auto-indentation, list continuation, word prediction)
- Task list management with easy checkbox toggling
- Rich content insertion (images, formulas, links)
- AI-assisted writing (with your choice of provider)
- Beautiful live preview with LaTeX math and syntax highlighting

**File System Integration:**

- File browsing and navigation within the editor
- Wiki-links that work across your entire directory structure
- Backlinks panel showing document relationships
- Document outline for quick navigation within files
- Search across all files in your workspace
- Recent folders and workspace switching

These two aspects complement each other: great editing tools meet great navigation tools, all working directly with your file system.

### What's Outside Our Scope

Some features, while valuable, are best handled by specialized tools that already excel at them:

- **File synchronization**: Tools like Syncthing, Nextcloud, Dropbox, and rsync are purpose-built for robust, reliable sync across devices. TreeMk works perfectly with any of them.
- **Version control**: Git provides comprehensive versioning, branching, and collaboration far beyond what we could build. Your plain Markdown files integrate seamlessly with any VCS.
- **Cloud storage**: Countless excellent storage solutions exist—use the one you trust. TreeMk just works with files wherever they are.
- **Advanced project management**: Full-featured PM tools offer complex workflow management. TreeMk focuses on note-taking and document navigation.
- **Diagram creation**: Specialized diagramming tools create better diagrams. Reference them from your Markdown; TreeMk will display them.

By keeping TreeMk focused on what it does uniquely well—integrating editing with file system navigation—we ensure every feature receives the attention it deserves. We'd rather have features that work beautifully together than scatter our efforts trying to replace tools that already excel at their specific tasks.

## Ways to Contribute

We welcome contributions that align with TreeMk's philosophy:

- **Report bugs**: Help us make existing features more reliable
- **Polish the user interface**: Improve usability and visual consistency between editor and file browser
- **Enhance documentation**: Make TreeMk easier to learn and use
- **Improve file system integration**: Better browsing, navigation, or link management
- **Refine editing features**: Make the writing experience more intuitive and powerful
- **Strengthen the connection**: Help the editor and file system work even better together

When proposing new features, consider: Does this enhance TreeMk's core mission of integrating Markdown editing with file system navigation? Or would this be better served by using an existing specialized tool alongside TreeMk?

The best contributions often aren't about adding more—they're about making what's already here work even better together.

## Getting Started

1. Fork the repository
2. Clone your fork
3. Create a feature branch
4. Make your changes
5. Submit a pull request

## Development Setup

```bash
git clone https://github.com/YOUR_USERNAME/treemk.git
cd treemk
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## Code Style

- Follow Qt naming conventions
- Add tests for new features
- Update documentation

## Pull Request Process

1. Update relevant documentation
2. Add tests if applicable
3. Ensure all tests pass
4. Describe your changes clearly

## Community

- GitHub Issues: Report bugs and request features
- GitHub Discussions: Ask questions and share ideas

## License

By contributing, you agree your contributions will be licensed under GPL-3.0.

Thank you for helping make TreeMk better!
