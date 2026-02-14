# Frequently Asked Questions

Honest answers to common questions about TreeMk.

## General Questions

### What exactly is TreeMk?

TreeMk is a desktop Markdown editor with integrated file browsing and wiki-style linking. Built with Qt6 and C++, it's designed to work directly with the file system rather than requiring a proprietary vault structure.

Think of it as a text editor that understands connections between notes, with a file tree always visible on the side. No import, no export, no database—just Markdown files where they already live.

### Why use TreeMk alongside Obsidian?

TreeMk isn't designed to replace Obsidian—it's designed to complement existing tools.

**TreeMk works alongside other editors:**

- Edit project documentation in TreeMk while coding in VS Code
- Keep technical notes in TreeMk, personal knowledge base in Obsidian
- Use TreeMk for work files in their project folders, Obsidian for personal vault
- Switch between tools based on the task at hand

**TreeMk's specific strengths:**

- No vault requirement—works with files in existing project structures
- Integrated file tree shows actual directory structure
- Simpler, focused interface without feature overload
- Direct file system access without import/export steps

**When TreeMk fits best:**

- Documentation alongside code repositories
- Technical notes that need to stay with project files
- Situations where vault-based organization doesn't make sense
- Users who value simplicity and file system transparency

TreeMk doesn't compete with Obsidian's graph view, plugins, or mobile apps. It solves a different problem: editing Markdown files where they naturally live, without reorganizing into a vault.

### How does TreeMk work with VS Code?

VS Code is a programming IDE. TreeMk is a note-taking editor. They serve different purposes and work well together.

**VS Code excels at:**

- Coding with Markdown documentation alongside
- Powerful extension ecosystem
- Excellent Git integration
- Multi-language development workflows

**TreeMk excels at:**

- Dedicated note-taking interface (outline, backlinks, history panels)
- Live preview optimized for reading, not just viewing
- Wiki-style linking with automatic backlink tracking
- Focused UI without development tool complexity

**Typical workflow:** Use VS Code for coding, TreeMk for note-taking and documentation review. Keep a project's `/docs` folder open in TreeMk while working on code in VS Code. Both editors see the same files—no conflicts, no duplication.

## Technical Questions

### What platforms does TreeMk support?

**Officially supported:**

- Linux (primary development platform)
- Windows 10/11
- macOS (10.15+)

**Reality check:** TreeMk is developed primarily on Linux. Windows and macOS builds exist but receive less testing. Platform-specific issues should be reported—fixes depend on maintainer availability and resources.

### What are the system requirements?

**Minimum:**

- 4GB RAM
- 100MB disk space
- 1280×720 display

**Recommended:**

- 8GB RAM (for large workspaces with thousands of files)
- SSD for better file indexing performance
- 1920×1080 or higher display

**Reality:** TreeMk runs on modest hardware. Performance depends on workspace size and search/indexing frequency.

### How does TreeMk handle large file sets?

Honestly? It depends.

**What works well:**

- Up to 1,000 files: Smooth, no noticeable lag
- File tree navigation: Fast even with 10,000+ files (Qt's file system model is efficient)
- Individual file editing: Always fast regardless of workspace size

**What can be slow:**

- Full-text search across 5,000+ files: Expect a few seconds
- Initial backlink indexing on startup: Takes longer with more files (runs in background)
- Wiki-link resolution in deeply nested directories: Slightly slower but usually acceptable

**TreeMk doesn't scale to 100,000 files gracefully.** For workspaces that large, consider organizing into separate smaller workspaces or using specialized tools designed for massive datasets.

### How do wiki links work across directories?

When typing `[[note]]`, TreeMk searches for matching files using a depth-based strategy from the current file's location:

**Search pattern:**

- Depth 0 (current directory): Searches current folder + all subdirectories recursively
- Depth 1 (one up): Goes up one folder, searches that folder + all its subdirectories
- Depth 2 (default): Goes up two folders, searches those folders + all their subdirectories

**File matching priority:**

1. Exact match: `note.md` or `note.markdown`
2. Fuzzy match: Any file containing "note" in the name
3. Case-insensitive matching

**Search depth:** Configurable in settings (default: 2 levels up). Setting it to 0 limits search to current directory tree only. Higher values search more of the workspace but take longer.

**Performance:** Link resolution uses caching. First click may take milliseconds for deeply nested structures, subsequent clicks are instant. Typical workspaces (hundreds of files) feel instant.

**Limitation:** Search starts from the current file's location, not the workspace root. This is intentional—it keeps link resolution fast and predictable by searching nearby files first.

**Technical approach:** The link resolver navigates up the folder hierarchy. At each level, it searches the directory and all subdirectories with recursive flags. Search stops when a match is found or the maximum depth is reached. The algorithm prioritizes closer files (same directory first) before searching parent directories, making relative wiki links like `[[../other-folder/note]]` work as expected. Home directory boundaries are enforced to prevent escaping outside the user's file system area.

### How are backlinks tracked?

TreeMk builds an index of all `[[wiki-links]]` in files within the configured search depth:

1. **On startup:** Scans Markdown files within search depth (background thread, doesn't block UI)
2. **On file save:** Updates index for that file
3. **On file add/delete:** Adjusts index

**Scope:** Only files within the configured search depth are indexed. Default depth of 2 means going up 2 folders from the workspace root, then indexing those folders and all their subdirectories. This prevents indexing the entire hard drive while still catching most relevant links.

**Where is the index stored?** In memory while running, rebuilt on next startup. No persistent database. This means:

- No database to corrupt or maintain
- Always reflects actual file state  
- Rebuild time on large workspaces (mitigated by background processing)

**Why no persistent database?** Simplicity and reliability. A 1-second rebuild on startup is preferable to risking index corruption or stale data.

**Technical approach:** The link parse maintains a register mapping each file path to its outgoing wiki links (forward links). On startup, it spawns a background thread that recursively scans directories. Each Markdown file is parsed with regular expressions. The algorithm inverts this map to create backlinks: for each target file, it stores which source files link to it. Lookup is O(1) using hash tables. The depth parameter limits how far up the directory tree to search, preventing full filesystem scans. Index updates on file save are incremental—only the changed file is re-parsed, keeping the UI responsive.

### Can TreeMk work with Git repositories?

Yes! TreeMk edits files in place. Git doesn't care what editor is used.

**What TreeMk does:**

- Edits Markdown files in place
- Saves them as regular files
- Doesn't interfere with Git

**What TreeMk doesn't do:**

- Git commits, pushes, or pulls
- Show Git status or diffs
- Resolve merge conflicts

TreeMk is for writing, Git (command line or GUI) is for version control. They coexist peacefully.

### What about sync and cloud storage?

TreeMk doesn't do sync. **This is intentional.**

Files are regular Markdown files. Any existing sync solution works:

- Dropbox
- Google Drive
- Syncthing
- Git
- rsync
- iCloud
- NextCloud

This approach means TreeMk doesn't build a mediocre sync system—users employ best-in-class solutions that already work.

**Conflict handling:** If a sync tool creates conflict files, TreeMk sees them as separate files. Conflicts should be resolved with the sync tool, not TreeMk.

## Feature Questions

### Does TreeMk support plugins?

No. TreeMk doesn't have a plugin system.

**Why not?** Plugins add complexity, security risks, and maintenance burden. For a small project, building a solid core is prioritized over managing a plugin ecosystem.

**Can TreeMk be extended?** The code is open source (GPL-3.0). It can be forked, modified, and rebuilt. However, there's no maintained plugin API.

### Will TreeMk get mobile apps?

Probably not.

Building quality mobile apps is a completely different engineering effort. TreeMk is a desktop application. Attempting mobile would dilute focus and resources.

**Alternatives:** Any Markdown editor on mobile can edit synced files. Notes are portable Markdown—they're not locked to TreeMk.

### Can TreeMk handle images and attachments?

**Images:** Yes. Paste from clipboard (Ctrl+V), drag-drop, or link manually. TreeMk saves them in the document folder.

**PDFs:** TreeMk won't display them inline, but linking to them works. Clicking opens them with the system's default application.

**Other attachments:** Link to any file. TreeMk opens it with the default application.

**Reality:** TreeMk is text-focused. Heavy multimedia management requires other tools.

### How does word prediction work?

TreeMk offers context-aware word completion based on the current document's content.

**What it does:**

- Suggests completions as you type
- Press Tab to accept a suggestion
- Learns from words already in the document
- Updates predictions as the document evolves

**Models used:**

- Unigram: Tracks single word frequency
- Bigram: Tracks two-word sequences (e.g., "quantum" followed by "mechanics")

**Scope:** Only markdown document in the current directory are analyzed, not the entire workspace. This keeps predictions relevant and fast.

**Technical approach:** The predictor maintains two tables: one for word frequencies and one for bigram pairs. When the document changes, it tokenizes the text using regular expressions. Each word increments its unigram count; each pair of adjacent words increments their bigram count. On keystroke, the predictor looks at the current word being typed and the previous word. It queries the bigram table first (higher priority for context), then falls back to unigram matches. Suggestions are ranked by frequency and filtered by prefix match. The entire model rebuilds on document change, which is fast for typical document sizes (< 100KB text). Memory usage is O(unique words), typically a few KB per document.

### How does AI integration work?

TreeMk integrates with AI providers through a plugin-like provider system, currently supporting Ollama (local) and OpenAI-compatible APIs (cloud).

**What it provides:**

- Text transformation (rephrase, fix grammar, translate)
- Custom prompts via system prompt dialog
- Selected text or full document as context
- Results inserted in-place or shown for review

**Providers supported:**

- **Ollama:** Local inference (llama2, mistral, etc.)
- **OpenAI:** Cloud API (GPT-3.5, GPT-4)
- **Custom:** Any OpenAI-compatible endpoint

**Configuration:** Edit → Preferences → AI Assistant sets the provider, endpoint URL, API key, model name, and parameters (temperature, max tokens).

### Does TreeMk have end-to-end encryption?

No. Files are stored as plain Markdown on the file system.

**For encryption needs:**

- Use full-disk encryption (BitLocker, FileVault, LUKS)
- Store files in encrypted containers (VeraCrypt)
- Use encrypted cloud storage

TreeMk is not a security tool. It's a text editor. Encryption is the operating system's responsibility.

## Stability and Maturity Questions

### Is TreeMk production-ready?

**It works.** It is used daily for real work, but it is not stable yet.

**However:** TreeMk is a community project, not a commercial product. Expect:

- Core features functional
- Regular bug fixes
- No guaranteed support response time
- Occasional bugs in edge cases

**Recommendation:** Test with a workspace first. If it fits the workflow, use it. Backups are recommended (as always).

### How often is TreeMk updated?

**Reality:** Updates happen when maintainers have time. Sometimes weekly, sometimes monthly, sometimes longer gaps.

TreeMk is maintained by volunteers with day jobs. Bugs get fixed, features get added, improvements happen—but on volunteer schedules, not corporate release calendars.

**Check activity:** The GitHub repository commit history shows the real picture.

### What if TreeMk development stops?

Notes are Markdown files. **They're always accessible.**

If TreeMk disappears tomorrow:

- Files remain readable in any text editor
- Links are visible as `[[note]]` syntax
- Conversion to another tool's format is possible

This is the beauty of plain text: No vendor lock-in. No trap.

### Where can help be found?

**Documentation:** Help menu (F1) has comprehensive guides

**GitHub Issues:** Report bugs, request features, ask questions at the repository

## Licensing and Development

### Is TreeMk really open source?

Yes. GPL-3.0 license. Full source code is available.

**What this means:**

- Free to use
- Free to modify
- Free to redistribute
- Must keep source open if distributing modified versions

**Commercial use:** Allowed. TreeMk can be used for business. No fees, no restrictions.

### Can contributions be made?

**Yes!** See the contributing guidelines.

**Realistic expectations:**

- Pull requests are reviewed when maintainers have time
- Code quality standards matter (C++/Qt best practices)
- Small, focused changes merge faster than big rewrites
- Documentation contributions are especially welcome

**Not sure where to start?** Look for "good first issue" labels in GitHub Issues.

## The Honest Bottom Line

### Should TreeMk be adopted?

**Try it if:**

- File system flexibility is valued
- Simpler, focused tools are preferred
- Plain Markdown is comfortable
- Files are already organized in directories
- Working alongside other tools sounds appealing

**Skip it if:**

- Mobile apps are required
- Extensive plugins are needed
- Polished commercial software is preferred
- Tolerance for occasional rough edges is low

**Best approach:** Install it, test with a small workspace, see if it fits the workflow. TreeMk isn't for everyone. That's acceptable.

### What's TreeMk's long-term future?

**Honest answer:** Unknown.

TreeMk exists as long as maintainers have time and interest. It might grow, it might stay small, it might eventually stop being updated.

**But notes are Markdown files.** They'll outlive any software. That's the point.

Use TreeMk as long as it serves the purpose. When it doesn't, switch to something else. Notes remain accessible.

## Still Have Questions?

Check the [GitHub repository](https://github.com/jailop/treemk) for:

- Issue tracker for bug reports
- Discussions for questions
- Releases for latest versions

Not everything can be answered, but attempts will be made.
