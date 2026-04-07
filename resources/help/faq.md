# Frequently Asked Questions

Answers to common questions about TreeMk.

**What exactly is TreeMk?**

TreeMk is a desktop Markdown editor with integrated file browsing and wiki-style linking. Built with Qt6 and C++, it's designed to work directly with the file system. Think of it as a text editor that understands connections between notes, with a file tree always visible on the side.

**What platforms does TreeMk support?**

- Linux and FreeBSD (primary development platforms)
- Windows 10/11
- macOS (10.15+)

TreeMk is developed primarily on Linux and FreeBSD. Windows and macOS builds exist but receive less testing and lack a better integration with the OS. Contributions are specially welcome to improve user experience in Windows and MacOS.

**How are backlinks tracked?**

TreeMk builds an index of all `[[wiki-links]]` in files within the configured search depth:

1. On startup: Scans Markdown files within search depth (background thread, doesn't block UI)
2. On file save: Updates index for that file
3. On file add/delete: Adjusts index

Only files within the configured search depth are indexed. Default depth of 2 means going up 2 folders from the workspace root, then indexing those folders and all their subdirectories. This prevents indexing the entire hard drive while still catching most relevant links.

Where is the index stored? In memory while running, rebuilt on next startup. No persistent database. This means:

- No database to corrupt or maintain
- Always reflects actual file state  
- Rebuild time on large workspaces (mitigated by background processing)

**What about sync and cloud storage?**

TreeMk doesn't do sync. This is intentional.

Files are regular Markdown files. Any existing sync solution works.

**Does TreeMk support plugins?**

No. TreeMk doesn't have a plugin system.

**Will TreeMk get mobile apps?**

Probably not.

Building mobile apps represents a different engineering undertaking. TreeMk is currently a desktop application. Furthermore, the concept of having direct control over local files is well-established in desktop environments, but not in mobile devices. Based on solid reasoning, both iPhone and Android devices consider direct access to the file system as a security vulnerability.

**Can TreeMk handle images and attachments?**

Images: Yes. Paste from clipboard (Ctrl+V), drag-drop, or link manually. TreeMk saves them in the document folder and render the images in the preview panel.

PDFs: TreeMk won't display them inline, but linking to them works. Clicking opens them with the system's default application.

Other attachments: Link to any file. TreeMk opens it with the default application.

TreeMk is text-focused. Heavy multimedia management requires other tools.

**How does word prediction work?**

TreeMk offers context-aware word completion based on the current document folder's content.

What it does:

- Suggests completions as you type
- Press Tab to accept a suggestion
- Learns from words already in the document
- Updates predictions as the document evolves

Models used:

- Unigram: Tracks single word frequency
- Bigram: Tracks two-word sequences (e.g., "quantum" followed by "mechanics")

Only markdown document in the current directory are analyzed, not the entire workspace. This keeps predictions relevant and fast.

**How does AI integration work?**

TreeMk integrates with AI providers through a provider configuration, currently supporting Ollama and OpenAI-compatible APIs.

What it provides:

- Text transformation (rephrase, fix grammar, translate)
- Custom prompts via system prompt dialog
- Selected text or full document as context
- Results inserted in-place or shown for review

Basic text editing tasks, such as grammar checking, summarization, or translation into other languages, don't require the most advanced models. A local model running through Ollama can handle most needs. For more advanced AI-assisted text editing features, such as full text generation, it's better to use a different tool.

**Is TreeMk production-ready?**

It works. It's used daily for real work, but many things still need or could be improved.

**Can contributions be made?**

Yes, they are very welcome. See the contributing guidelines.

Realistic expectations:

- Pull requests are reviewed when maintainer have time
- Code quality standards matter (C++/Qt best practices)
- Small, focused changes merge faster than big rewrites
- Documentation contributions are especially welcome

**Should TreeMk be adopted?**

Try it if:

- File system flexibility is valued
- Simpler, focused tools are preferred
- Plain Markdown is comfortable
- Files are already organized in directories
- Working alongside other tools sounds appealing

Skip it if:

- Mobile apps are required
- Extensive plugins are needed
- Polished commercial software is preferred
- Tolerance for occasional rough edges is low

