# Workflows: Real-World Use Cases

TreeMk adapts to different ways of working. Here are proven workflows from actual users.

## Research Workflow

You're researching a topic—maybe for a paper, a project, or just curiosity. Notes pile up. Connections form. TreeMk helps you see the web.

**The Pattern:**

1. **Start with a central note.** Create `quantum-computing.md` as your main topic.

2. **Branch out as you read.** When you encounter a concept, create a note: `qubits.md`, `superposition.md`, `entanglement.md`. Link from your main note: `[[qubits]]`, `[[superposition]]`.

3. **Connect related ideas.** In `qubits.md`, mention `[[superposition]]` and `[[measurement]]`. Links form naturally as you write.

4. **Use backlinks to discover patterns.** Open `superposition.md`. Check the **Backlinks** panel. You'll see every note that references it. That's your research web visualized.

5. **Search across everything.** Press **Ctrl+Shift+F** and search for "decoherence". TreeMk shows you every note mentioning it with context. You just found connections you forgot about.

6. **Navigate your history.** **Alt+Left** and **Alt+Right** let you retrace your reading path. The **History** panel shows where you've been. Your research trail is preserved.

**Why This Works:**

Traditional folders force hierarchy. But research doesn't work that way. Ideas connect sideways, not just top-down. TreeMk's wiki links and backlinks let your notes form organic networks.

## Documentation Workflow

You're maintaining project documentation. Code changes. Docs need to stay in sync. Copy-paste breaks down. TreeMk's file inclusion saves you.

**The Pattern:**

1. **Keep code separate from docs.** Your project structure:
   ```
   project/
   ├── src/
   │   ├── api.py
   │   └── database.py
   └── docs/
       ├── api-guide.md
       └── database-guide.md
   ```

2. **Include code in documentation.** In `api-guide.md`:
   ````markdown
   # API Guide
   
   Here's the implementation:
   
   ```python
   [[!../src/api.py]]
   ```
   ````

3. **Let preview show live code.** Open `api-guide.md`. The preview displays your actual `api.py` content. Change the code? The preview updates.

4. **Share reusable sections.** Create `docs/common/authentication.md` with your auth explanation. Include it in multiple guides: `[[!common/authentication]]`.

5. **Export to share.** Press **Ctrl+E** and export to HTML or PDF. Included files render as complete documents. Recipients see everything, not broken references.

**Why This Works:**

Single source of truth. Documentation reads from actual code. No sync issues. No outdated examples. Just living docs that reflect reality.

## Journal Workflow

Daily notes. Thoughts, tasks, meetings. You want a record, searchable and browsable, without complex systems.

**The Pattern:**

1. **Create daily notes.** Press **Alt+D** to insert today's date. Use it as a filename: `2026-02-14.md`. Or create a template:
   ```markdown
   # 2026-02-14
   
   ## Tasks
   - [ ] 
   
   ## Notes
   
   ## Ideas
   ```

2. **Link to projects.** In today's note: "Meeting about [[project-alpha]]". Link to your project note. Now `project-alpha.md` shows this meeting in its backlinks.

3. **Reference previous entries.** "Following up on [[2026-02-10]]". Journal entries link to each other. Your timeline is navigable.

4. **Search your past.** Can't remember when you met someone? **Ctrl+Shift+F** search for their name. TreeMk finds every mention across all journal entries with dates.

5. **Review your week.** Open seven recent journal files in tabs. **Ctrl+Tab** cycles through them. Your week in review.

**Why This Works:**

No database, no app lock-in. Just markdown files named by date. Works with any backup system. Readable in any text editor. TreeMk adds search and links to make it powerful.

## Zettelkasten Method

The Zettelkasten method: atomic notes, each with one idea, densely linked. TreeMk was basically built for this.

**The Pattern:**

1. **One idea per note.** Create `202602141830-superposition-in-quantum-systems.md`. The timestamp makes it unique. The title describes the single idea.

2. **Write atomically.** Keep notes short—one concept explained clearly. Maybe 100-300 words. If you're covering multiple ideas, split into multiple notes.

3. **Link heavily.** Reference related ideas: `[[202602121420-qubits]]`, `[[202602130915-measurement-problem]]`. Your notes become a densely connected network.

4. **Create index notes.** Make `quantum-computing-index.md` that lists key notes:
   ```markdown
   # Quantum Computing Index
   
   - [[202602141830-superposition-in-quantum-systems]]
   - [[202602131205-quantum-entanglement-basics]]
   - [[202602121420-qubits]]
   ```

5. **Follow the trail.** Open an index note. Click a link. Check its backlinks. See what else references it. Click another link. You're traversing your knowledge graph.

6. **Discover emergent structure.** After months, look at a note's backlinks. Twenty other notes reference it. You've discovered a central concept without planning it.

**Why This Works:**

Zettelkasten requires tools that don't impose structure. TreeMk doesn't. It gives you links and backlinks. The structure emerges from your thinking, not from folder hierarchies.

## Multi-Project Workflow

You work on several projects. Each needs its own notes. You don't want them mixed.

**The Pattern:**

1. **One folder per project.** Create separate directories:
   ```
   ~/notes/work-projects/
   ~/notes/personal/
   ~/notes/learning/quantum-physics/
   ```

2. **Open workspaces by folder.** Press **Ctrl+O** and open `work-projects/`. TreeMk shows only those files. Switch to a different project? **Ctrl+O** again, open `personal/`. Clean context switch.

3. **Use Recent Folders.** **File → Recent Folders** lists your workspaces. One click switches projects. TreeMk closes old tabs, opens last session's tabs for the new workspace.

4. **Cross-reference when needed.** Sometimes projects connect. Use absolute paths: `[[/home/user/notes/personal/idea.md]]`. Or copy notes between folders when ideas migrate.

5. **Sync separately.** Each folder can use different sync systems. Maybe work notes sync to company storage, personal to Dropbox, learning to Git. TreeMk doesn't care—just folders.

**Why This Works:**

Clean separation without complexity. No tags to manage, no filters to configure. Just folders. Simple, obvious, compatible with every other tool.

## Mixed Workflow: Practical Daily Use

Real life isn't one workflow. It's all of them, mixed. Here's how a real user might work:

**Morning:** Open work project workspace. Check `today.md` journal note. See tasks: `- [ ] Finish [[api-docs]]`. Click the link.

**Mid-morning:** Writing `api-docs.md`. Include live code: `[[!../src/api.py]]`. Include shared auth section: `[[!common/auth-guide]]`. Preview looks good.

**Afternoon:** Research task. Create `user-authentication-research.md`. Link to existing notes: `[[oauth-basics]]`, `[[jwt-tokens]]`. Add new findings. Check backlinks—three other notes reference OAuth. Read them for context.

**Evening:** Switch to personal workspace. Open `learning/quantum-physics.md`. Follow links through `[[qubits]]` → `[[superposition]]` → `[[measurement]]`. History panel shows my learning path. Add new note with timestamp: `202602141945-decoherence-effects.md`.

**Night:** Quick journal entry in `2026-02-14.md`. Mention `[[project-alpha]]` and `[[quantum-physics]]`. Tomorrow I'll see these connections in backlinks.

**Next week:** Export `api-docs.md` to PDF. Email to team. File inclusion means they see current code. Search all notes for "authentication" to audit my knowledge. Find gaps. Create notes to fill them.

This is TreeMk in practice: flexible, unobtrusive, powerful.

## Tips Across All Workflows

**Name notes descriptively.** "Quantum Superposition Basics" beats "Note 23". You'll thank yourself when searching.

**Link early, link often.** Don't wait until notes are "complete" to link them. Link while writing. Connections are the point.

**Use the preview.** See your work rendered. Spot broken links. Check that includes work. Preview is your feedback loop.

**Trust search.** Don't over-organize with folders. Flat folder + good search often beats complex hierarchies.

**Check backlinks regularly.** They reveal unexpected connections. Serendipity is a feature.

**Export when ready to share.** TreeMk format (Markdown + wiki links) is for you. Export format (PDF/HTML/DOCX) is for others.

## Next Steps

- **[Getting Started](getting-started.md)** if you're new
- **[Navigation](navigation.md)** to master links and backlinks
- **[Editor Features](editor.md)** for writing techniques
- **[Exporting](exporting.md)** to share your work

Your workflow, your way. TreeMk adapts.
