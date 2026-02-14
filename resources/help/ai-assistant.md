# AI Assistant

TreeMk integrates AI capabilities to enhance your writing and help you work more efficiently with your notes.

![AI Assistant](images/aiassistant.png)

## Overview

The AI Assistant provides intelligent writing help powered by:

- **Ollama** - Local AI models running on your computer
- **OpenAI** - GPT models via API
- **Compatible Services** - Any OpenAI-compatible endpoint

**Important:** AI assistance is optional and can be completely disabled. When disabled in preferences, all AI-related interface elements (toolbar button, menu items, and context menu options) are hidden or disabled, providing a distraction-free writing environment. See the Configuration section below for details.

## Getting Started

### 1. Configure AI Provider

Go to **Edit → Preferences → AI Assistant**

**For Ollama (Local AI):**

- Install Ollama from https://ollama.ai
- Pull a model: `ollama pull llama2`
- In TreeMk settings:
  - Provider: Ollama
  - Endpoint: `http://localhost:11434`
  - Model: `llama2` (or your preferred model)

**For OpenAI:**

- Get API key from https://platform.openai.com
- In TreeMk settings:
  - Provider: OpenAI
  - API Key: Your OpenAI API key
  - Model: `gpt-3.5-turbo` or `gpt-4`

**For Custom Endpoints:**

- Configure your compatible service URL
- Add authentication if required
- Specify the model name

### 2. Using the AI Assistant

**Method 1: Predefined Prompts (Quick Access)**

1. Select text in your document
2. Click the **AI Assist** toolbar button dropdown (or go to **Edit → AI Assist** menu)
3. Choose from predefined prompts:
   - **Rephrase** - Rewrite text with different wording
   - **Fix Grammar** - Correct grammar and spelling errors
   - **Make Shorter** - Condense text while keeping key points
   - **Make Longer** - Expand and elaborate on ideas
   - **Simplify Language** - Make text easier to understand
   - **Professional Tone** - Convert to formal business style
   - **Casual Tone** - Make text more conversational
   - **Summarize** - Create a brief summary
   - **Translate to Spanish** - Translate selected text
   - **Add Examples** - Include relevant examples
4. The AI processes immediately with the selected prompt

**Method 2: Custom Prompt (Keyboard Shortcut)**

1. Select text in your document
2. Press **Ctrl+Shift+A**
3. The AI assistant dialog opens for custom instructions

**Method 3: Custom Prompt (Toolbar)**

1. Select text
2. Click the main part of the **AI Assist** toolbar button (not the dropdown)
3. Enter your custom prompt in the dialog

**Method 4: Context Menu**

1. Right-click selected text
2. Choose **AI Assist**
3. The AI assistant dialog opens

## Features

### Text Improvement

Ask the AI to:

- Improve clarity and readability
- Fix grammar and spelling
- Enhance writing style
- Make text more concise
- Expand on ideas

### Content Generation

- Generate outlines from topics
- Create summaries of long text
- Draft content based on prompts
- Brainstorm ideas
- Suggest alternatives

### Translation

- Translate to different languages
- Adapt tone and register
- Cultural context adaptation

### Code Assistance

- Explain code snippets
- Suggest improvements
- Generate code examples
- Debug code issues

### Research Help

- Summarize complex topics
- Extract key points
- Organize information
- Create structured notes

## AI Assistant Dialog

### Predefined Prompts

TreeMk includes 10 predefined prompts for common tasks, accessible from:
- The **AI Assist** toolbar button dropdown
- The **Edit → AI Assist** submenu

These prompts work immediately on selected text without opening a dialog:

1. **Rephrase** - Rewrite with different wording while preserving meaning
2. **Fix Grammar** - Correct grammar, spelling, and punctuation
3. **Make Shorter** - Condense text, remove redundancy
4. **Make Longer** - Expand ideas with more detail and explanation
5. **Simplify Language** - Use simpler words and shorter sentences
6. **Professional Tone** - Convert to formal, business-appropriate style
7. **Casual Tone** - Make more conversational and friendly
8. **Summarize** - Create a concise summary of key points
9. **Translate to Spanish** - Translate selected text to Spanish
10. **Add Examples** - Include relevant examples to illustrate points

### Managing Prompts

Access **Tools → System Prompts** to customize:

- **Edit** existing prompts
- **Add** new predefined prompts
- **Delete** prompts you don't use
- **Reorder** prompts with Move Up/Down buttons
- Your custom prompts appear in both toolbar and menu

### Custom Prompt Dialog

When you invoke the AI assistant with **Ctrl+Shift+A** or click the main toolbar button, a dialog appears with:

**Input Area:**

- Your selected text (automatically filled)
- Or empty for new prompts

**Prompt Field:**

- Enter your instruction (e.g., "Improve this paragraph")
- Use natural language
- Be specific about what you want

**Action Buttons:**

- **Send** - Submit your request to AI
- **Insert** - Insert AI response into document
- **Replace** - Replace selected text with response
- **Copy** - Copy response to clipboard
- **Close** - Close the dialog

## Example Prompts

### Using Predefined Prompts

The quickest way to use AI assistance:

1. Select text you want to improve
2. Click **AI Assist** dropdown on toolbar
3. Choose a predefined action (e.g., "Fix Grammar")
4. AI processes and replaces your text automatically

### Custom Prompt Examples

For more specific requests, use the custom dialog (**Ctrl+Shift+A**):

### Writing Improvement
```
Make this paragraph more concise

Improve the clarity of this explanation

Fix any grammar or spelling errors
```

### Content Creation
```
Create an outline for a blog post about [topic]

Summarize this article in 3 bullet points

Expand this idea into a full paragraph
```

### Translation
```
Translate this to Spanish

Translate to French, keeping a formal tone

Convert this technical text to simple language
```

### Analysis
```
What are the main points in this text?

List the key takeaways from this section

Identify any logical gaps in this argument
```

## Privacy & Security

### Local AI (Ollama)

- **Most Private**: All processing happens on your computer
- **No Internet Required**: Works completely offline
- **No Data Sharing**: Your text never leaves your machine
- **Recommended For**: Sensitive documents, personal notes

### Cloud AI (OpenAI)

- **Convenience**: Fast, powerful models
- **Internet Required**: Sends text to external servers
- **Privacy Consideration**: Your text is processed by third party
- **Best For**: Non-sensitive content

## Troubleshooting

### "Connection Failed"

**Ollama:**

- Check Ollama is running: `ollama list`
- Verify endpoint: `http://localhost:11434`
- Restart Ollama service

**OpenAI:**

- Verify API key is correct
- Check internet connection
- Confirm account has credits

### "Model Not Found"

**Ollama:**

- Pull the model: `ollama pull <model>`
- Check available models: `ollama list`

**OpenAI:**

- Verify model name spelling
- Check model availability for your account

### "Slow Response"

- Larger models take longer
- Try a smaller/faster model
- For Ollama, check system resources
- For cloud APIs, check network speed

### "Invalid Response"

- Try rephrasing your prompt
- Ensure model supports your language
- Check if request is too complex
- Try with different model

## Configuration Options

Access **Edit → Preferences → AI Assistant**:

**Enable AI Assistance:**

- **What it does:** Master toggle for all AI features
- **When disabled:** 
  - AI Assist toolbar button becomes disabled (grayed out)
  - Edit → AI Assist menu submenu becomes disabled
  - AI Assist options are hidden from editor context menus
  - No AI-related interface elements are shown
- **When to disable:** 
  - If you don't use AI features and want a cleaner interface
  - For privacy-sensitive work where you don't want AI options visible
  - To reduce interface clutter
- **When enabled:** All AI features become available based on your provider settings

**Provider Settings:**

- Select provider (Ollama, OpenAI, Custom)
- Configure endpoint URL
- Set API key (if required)

**Model Settings:**

- Choose model
- Set temperature (creativity level)
- Configure max tokens (response length)
- Timeout settings

**Behavior:**

- Default action (insert/replace)
- Keyboard shortcut customization
- Response formatting

## Keyboard Shortcuts

- **Ctrl+Shift+A** - Open AI Assistant dialog for custom prompt
- **Ctrl+Enter** (in dialog) - Send request
- **Esc** (in dialog) - Close without changes

Use the toolbar dropdown or **Edit → AI Assist** menu for quick access to predefined prompts.

## Advanced Usage

### Chaining Requests

1. Get initial AI response
2. Select that response
3. Request further refinement
4. Build iteratively

### Custom Prompts

Create reusable prompts via **Tools → System Prompts**:

- Add prompts for your specific workflows
- Edit existing prompts to match your style
- Reorder to put most-used prompts at the top
- Prompts appear in both toolbar dropdown and Edit menu
- Build a library of task-specific prompts

### Integration with Workflow

**Quick Improvements:**
- Select text, click dropdown, choose prompt
- No dialog interruption for common tasks
- Fast iterations with predefined actions

**Custom Refinements:**
- Use **Ctrl+Shift+A** for specific instructions
- Draft with AI, refine manually
- Use for quick summaries
- Generate structure, fill details yourself
- Translate first drafts