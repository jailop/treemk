#ifndef REGEXPATTERNS_H
#define REGEXPATTERNS_H

namespace RegexPatterns {

/**
 * Wiki and Markdown link patterns
 *
 * WIKI_LINK: Matches [[target]], [[!target]], [[target|display]],
 * [[!target|display]]
 *   - Capture group 1: Inclusion marker "!" (optional)
 *   - Capture group 2: Target (page name or path)
 *   - Capture group 4: Display text (optional)
 *   - Supports spaces and path separators (/ or \)
 *
 * WIKI_LINK_SIMPLE: Like WIKI_LINK but excludes inclusion syntax
 *
 * MARKDOWN_LINK: Matches [text](url)
 *   - Capture group 1: Link text
 *   - Capture group 2: URL or path
 *
 * MARKDOWN_LINK_WITH_IMAGE: Matches [text](url) or ![alt](image.png)
 *   - Capture group 1: Image marker "!" (optional)
 *   - Capture group 2: Link text or alt text
 *   - Capture group 3: URL or image path
 *
 * MARKDOWN_LINK_IMAGE: Matches optional image marker before link
 *
 * INCLUSION_PATTERN: Matches [[!target]] or [[!target|display]]
 *   - Specifically for inclusion links only
 *
 * URL: Matches http:// or https:// URLs
 */
constexpr char WIKI_LINK[] = "\\[\\[(!)?([^|\\]]+)(\\|([^\\]]+))?\\]\\]";
constexpr char WIKI_LINK_SIMPLE[] = "\\[\\[([^|\\]!]+)(\\|([^\\]]+))?\\]\\]";
constexpr char MARKDOWN_LINK[] = "\\[([^\\]]+)\\]\\(([^\\)]+)\\)";
constexpr char MARKDOWN_LINK_WITH_IMAGE[] =
    "(!)?\\[([^\\]]+)\\]\\(([^\\)]+)\\)";
constexpr char MARKDOWN_LINK_IMAGE[] = "!?\\[([^\\]]+)\\]\\(([^\\)]+)\\)";
constexpr char INCLUSION_PATTERN[] = "\\[\\[!([^|\\]]+)(\\|([^\\]]+))?\\]\\]";
constexpr char URL[] = "(https?://[^\\s]+)";

/**
 * List item patterns
 *
 * LIST_ITEM: Matches list items with bullet (-, *, +) or number (1. 2. 3.)
 *   - Capture group 1: Leading whitespace (indentation)
 *   - Capture group 2: Marker (bullet or number with dot)
 *   - Requires space after marker
 *
 * LIST_ITEM_NO_PLUS: Like LIST_ITEM but requires only single space
 *
 * UNORDERED_LIST: Matches only bullet lists (-, *, +)
 *
 * ORDERED_LIST: Matches only numbered lists (1. 2. 3.)
 *
 * LIST_MARKER_ONLY: Matches empty list items (marker with no content)
 */
constexpr char LIST_ITEM[] = R"(^(\s*)([-*+]|[0-9]+\.)\s+)";
constexpr char LIST_ITEM_NO_PLUS[] = R"(^(\s*)([-*+]|[0-9]+\.)\s)";
constexpr char UNORDERED_LIST[] = R"(^(\s*)([-*+])\s)";
constexpr char ORDERED_LIST[] = R"(^(\s*)(\d+\.)\s)";
constexpr char LIST_MARKER_ONLY[] = R"(^\s*[-*+]\s*$)";

/**
 * Task list patterns
 *
 * TASK_ITEM: Matches task items "- [ ] task" or "- [x] done"
 *   - Capture group 1: Leading whitespace (indentation)
 *   - Capture group 2: Bullet marker (-, *, +)
 *   - Capture group 3: Checkbox state (" ", "x", "X", ".")
 *
 * TASK_ITEM_SHORT: Shorter version without leading whitespace requirement
 *
 * TASK_CHECKBOX: Matches the checkbox part "- [ ]" or "- [x]"
 *
 * TASK_CHECKBOX_CHECKED: Matches only checked or in-progress tasks
 */
constexpr char TASK_ITEM[] = R"(^(\s*)([-*+])\s+\[([ xX.]?)\]\s*)";
constexpr char TASK_ITEM_SHORT[] = R"(([-*+])\s+\[([ xX.]?)\])";
constexpr char TASK_CHECKBOX[] = R"([-*+] \[([ xX.]?)\])";
constexpr char TASK_CHECKBOX_CHECKED[] = R"(([-*+])\s+\[([xX.]?)\])";

/**
 * Markdown formatting patterns
 *
 * HEADER: Matches ATX-style headers "# Title" through "###### Title"
 *   - Capture group 1: Hash marks (# to ######)
 *   - Capture group 2: Header text
 *
 * HEADER_PREFIX: Matches just the header prefix "# " through "###### "
 *
 * HEADER_H1: Matches only level 1 headers "# Title"
 *
 * HEADER_ANY: Matches any header level
 *
 * BOLD: Matches **bold** or __bold__ text
 *
 * ITALIC: Matches *italic* or _italic_ text
 *
 * STRIKETHROUGH: Matches ~~strikethrough~~ text
 *
 * INLINE_CODE: Matches `code` text
 *
 * CODE_FENCE: Matches code block fences ```language
 *
 * QUOTE: Matches blockquotes "> quote" or ">> nested"
 *
 * HORIZONTAL_RULE: Matches horizontal rules --- or *** or ___
 */
constexpr char HEADER[] = R"(^(#{1,6})\s+(.+)$)";
constexpr char HEADER_PREFIX[] = R"(^(#{1,6})\s)";
constexpr char HEADER_H1[] = R"(^#\s+(.+)$)";
constexpr char HEADER_ANY[] = R"(^#{1,6}\s+(.+)$)";
constexpr char BOLD[] = R"((\\*\\*|__)(?=\\S)(.+?)(?<=\\S)\\1)";
constexpr char ITALIC[] =
    R"((?<!\\*)\\*(?=\\S)(.+?)(?<=\\S)\\*(?!\\*)|(?<!_)_(?=\\S)(.+?)(?<=\\S)_(?!_))";
constexpr char STRIKETHROUGH[] = "~~(.*?)~~";
constexpr char STRIKETHROUGH_MARKER[] = "~~";
constexpr char INLINE_CODE[] = "`[^`]+`";
constexpr char CODE_FENCE[] = R"(^```(\w+)?)";
constexpr char CODE_BLOCK_FENCE[] = R"(^```.*$)";
constexpr char QUOTE[] = R"(^(\s*>+)\s)";
constexpr char HORIZONTAL_RULE[] = R"(^([-*_]\s*){3,}$)";

/**
 * LaTeX equation patterns
 *
 * LATEX_BLOCK: Matches block equations $$equation$$
 *
 * LATEX_INLINE: Matches inline equations $equation$
 *   - Does not match across newlines
 */
constexpr char LATEX_BLOCK[] = R"(\$\$[^$]+\$\$)";
constexpr char LATEX_INLINE[] = R"(\$[^$\n]+\$)";

/**
 * Code highlighting patterns
 *
 * WORD_BOUNDARY: Matches words with 3+ letters
 *
 * NUMBER: Matches integers and floats (42, 3.14)
 *
 * NUMBER_C: Matches C-style numbers with suffixes (42L, 3.14f)
 *
 * NUMBER_JAVA: Matches Java-style numbers with suffixes (42L, 3.14d)
 *
 * FUNCTION_CALL: Matches function calls myFunction(
 *   - Capture group 1: Function name
 *
 * FUNCTION_CALL_JS: Like FUNCTION_CALL but allows $ in identifiers
 *
 * STRING_PATTERN: Matches "double" or 'single' quoted strings
 *
 * STRING_PATTERN_BACKTICK: Includes `backtick` strings
 *
 * MULTILINE_COMMENT: Matches C-style comment blocks (slash-star ... star-slash)
 */
constexpr char WORD_BOUNDARY[] = R"(\b[a-zA-Z]{3,}\b)";
constexpr char NUMBER[] = R"(\b\d+(\.\d+)?\b)";
constexpr char NUMBER_C[] = R"(\b\d+(\.\d+)?[fFuUlL]*\b)";
constexpr char NUMBER_JAVA[] = R"(\b\d+(\.\d+)?[fFdDlL]*\b)";
constexpr char FUNCTION_CALL[] = R"(\b([a-zA-Z_][a-zA-Z0-9_]*)\s*(?=\())";
constexpr char FUNCTION_CALL_JS[] = R"(\b([a-zA-Z_$][a-zA-Z0-9_$]*)\s*(?=\())";
constexpr char STRING_PATTERN[] = R"(("[^"]*"|'[^']*'))";
constexpr char STRING_PATTERN_BACKTICK[] = R"((`[^`]*`|'[^']*'|"[^"]*"))";
constexpr char MULTILINE_COMMENT[] = R"(/\*.*\*/)";

/**
 * Utility patterns
 *
 * NEWLINE_BETWEEN_TEXT: Matches single newline between text (not double)
 *
 * FILENAME_INVALID_CHARS: Matches characters invalid in filenames
 *
 * IMAGE_FILENAME: Matches image_123.png format
 *   - Capture group 1: The number
 *
 * WHITESPACE_MULTIPLE: Matches any whitespace (spaces, tabs, newlines)
 *
 * NON_WORD_CHARS: Matches non-word characters (excludes _ and -)
 *
 * LEADING_TRAILING_DASH: Matches dashes at start or end
 */
constexpr char NEWLINE_BETWEEN_TEXT[] = R"(([^\n])\n([^\n]))";
constexpr char FILENAME_INVALID_CHARS[] = R"([<>:"/\\|?*])";
constexpr char IMAGE_FILENAME[] = R"(^image_(\d+)\.png$)";
constexpr char WHITESPACE_MULTIPLE[] = R"(\s+)";
constexpr char NON_WORD_CHARS[] = R"([^\w\-_])";
constexpr char LEADING_TRAILING_DASH[] = R"(^-+|-+$)";

/**
 * HTML/Preview processing patterns
 *
 * HTML_HEADING: Matches HTML heading tags <h1>text</h1> through <h6>text</h6>
 *   - Capture group 1: Tag name (h1-h6)
 *   - Capture group 2: Attributes (optional)
 *   - Capture group 3: Heading text content
 *
 * HTML_ANCHOR: Matches HTML anchor links <a href="url">text</a>
 *   - Capture group 1: URL/href attribute
 *   - Capture group 2: Link text
 *
 * HTML_CODE: Matches HTML code tags <code>content</code> with optional attributes
 *   - Capture group 1: Attributes (optional)
 *   - Capture group 2: Code content
 *
 * LATEX_DISPLAY_EQUATION: Matches md4c output for display equations
 *   - Captures content between <x-equation type="display"> tags
 *
 * LATEX_INLINE_EQUATION: Matches md4c output for inline equations
 *   - Captures content between <x-equation> tags
 *
 * MD4C_WIKILINK: Matches md4c output for wiki links
 *   - Capture group 1: Target/data-target attribute
 *   - Capture group 2: Link text
 *
 * MD4C_WIKILINK_INCLUSION: Matches md4c output for inclusion wiki links [[!target]]
 *   - Capture group 1: Target (without the ! marker)
 *   - Capture group 2: Display text
 */
constexpr char HTML_HEADING[] = "<(h[1-6])([^>]*)>([^<]+)</\\1>";
constexpr char HTML_ANCHOR[] = "<a href=\\\"([^\\\"]*)\\\"?>([^<]*)</a>";
constexpr char HTML_CODE[] = "<code([^>]*)>([^<]*)</code>";
constexpr char LATEX_DISPLAY_EQUATION[] = "<x-equation type=\\\"display\\\">([^<]*)</x-equation>";
constexpr char LATEX_INLINE_EQUATION[] = "<x-equation>([^<]*)</x-equation>";
constexpr char MD4C_WIKILINK[] = "<x-wikilink data-target=\\\"([^\\\"]+)\\\">([^<]+)</x-wikilink>";
constexpr char MD4C_WIKILINK_INCLUSION[] = "<x-wikilink data-target=\\\"!([^\\\"]+)\\\">([^<]+)</x-wikilink>";

}  // namespace RegexPatterns

#endif  // REGEXPATTERNS_H
