# AI Assistant

TreeMk integrates AI capabilities to enhance your writing and help you work more efficiently with your notes.

## Overview

The AI Assistant provides intelligent writing help powered by:
- **Ollama** - Local AI models running on your computer
- **OpenAI** - GPT models via API
- **Compatible Services** - Any OpenAI-compatible endpoint

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

**Method 1: Keyboard Shortcut**
1. Select text in your document
2. Press **Ctrl+Shift+A**
3. The AI assistant dialog opens

**Method 2: Context Menu**
1. Right-click selected text
2. Choose **AI Assist**
3. The AI assistant dialog opens

**Method 3: Menu**
1. Select text
2. Go to **Edit → AI Assist**

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

When you invoke the AI assistant, a dialog appears with:

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

### Best Practices
1. **Use local AI for sensitive content** (medical, legal, personal)
2. **Read provider privacy policies** before using cloud services
3. **Review AI suggestions** - don't accept blindly
4. **Keep API keys secure** - don't share them

## Supported Models

### Ollama Models
Popular models you can run locally:
- **llama2** - General purpose, good balance
- **mistral** - Fast and capable
- **codellama** - Optimized for code
- **phi** - Small but efficient
- **neural-chat** - Conversational
- **dolphin-mixtral** - Advanced reasoning

Download with: `ollama pull <model-name>`

### OpenAI Models
- **gpt-3.5-turbo** - Fast, cost-effective
- **gpt-4** - Most capable, slower, more expensive
- **gpt-4-turbo** - Latest with better performance

### Custom Models
Any model compatible with OpenAI API format works, including:
- Together AI
- Groq
- Anthropic (with adapter)
- Local models via LM Studio

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

## Tips for Better Results

### 1. Be Specific
❌ "Fix this"
✅ "Fix grammar and improve clarity"

### 2. Provide Context
❌ "Translate this"
✅ "Translate to Spanish, maintaining formal academic tone"

### 3. Use Examples
"Rewrite in the style of: [example]"

### 4. Iterate
- Start with general request
- Refine based on results
- Build on previous responses

### 5. Combine with Editing
- Use AI for first draft
- Manually refine output
- Don't rely 100% on AI

## Configuration Options

Access **Edit → Preferences → AI Assistant**:

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

- **Ctrl+Shift+A** - Open AI Assistant
- **Ctrl+Enter** (in dialog) - Send request
- **Esc** (in dialog) - Close without changes

## Advanced Usage

### Chaining Requests
1. Get initial AI response
2. Select that response
3. Request further refinement
4. Build iteratively

### Custom Prompts
Save frequently used prompts:
- Create text snippets for common tasks
- Use macros for repeated workflows
- Build prompt templates

### Integration with Workflow
- Draft with AI, refine manually
- Use for quick summaries
- Generate structure, fill details yourself
- Translate first drafts

## Responsible AI Use

### Guidelines
- **Verify Information**: AI can make mistakes
- **Cite Sources**: Don't pass off AI text as original without disclosure
- **Respect Copyright**: Don't use AI to reproduce copyrighted content
- **Review Output**: Always check for accuracy and appropriateness
- **Consider Ethics**: Use AI responsibly and ethically

### When NOT to Use AI
- Academic work requiring original thought (without disclosure)
- Legal or medical advice
- Decisions with serious consequences
- Content requiring factual accuracy (verify first)

## Next Steps

- Try the AI assistant with different prompts
- Experiment with local and cloud models
- Explore **[Editor Features](editor.md)** for more productivity tools
- Learn **[Keyboard Shortcuts](keyboard-shortcuts.md)** for faster workflow

## Resources

- **Ollama**: https://ollama.ai
- **OpenAI**: https://platform.openai.com
- **Compatible APIs**: Search for "OpenAI compatible API"
- **Model Comparisons**: Research which model fits your needs
