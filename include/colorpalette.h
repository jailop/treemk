#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include <QColor>
#include <QString>

namespace ColorPalette {

struct ThemeColors {
  QColor background;
  QColor backgroundAlt;
  QColor backgroundCode;
  QColor backgroundCodeBlock;
  QColor backgroundLatexInline;
  QColor backgroundLatexBlock;
  QColor backgroundInclusion;
  QColor backgroundInclusionBroken;
  QColor backgroundSelection;
  QColor backgroundCurrentLine;
  
  QColor text;
  QColor textSecondary;
  QColor textSelection;
  QColor textDisabled;
  
  QColor header;
  QColor link;
  QColor wikiLink;
  QColor brokenLink;
  QColor code;
  QColor listMarker;
  QColor blockquote;
  QColor latex;
  QColor strikethrough;
  QColor inclusion;
  QColor inclusionBroken;
  QColor subtleMarkup;
  
  QColor syntaxKeyword;
  QColor syntaxString;
  QColor syntaxComment;
  QColor syntaxNumber;
  QColor syntaxFunction;
  QColor syntaxType;
  
  QColor border;
  QColor borderTable;
  QColor hr;
  
  QColor uiWindow;
  QColor uiWindowText;
  QColor uiBase;
  QColor uiAlternateBase;
  QColor uiButton;
  QColor uiButtonText;
  QColor uiButtonTextDisabled;
  QColor uiHighlight;
  QColor uiHighlightedText;
  QColor uiToolTipBase;
  QColor uiToolTipText;
  
  QColor lineNumberBackground;
  QColor lineNumberText;
  
  QColor statusSuccess;
  QColor statusError;
  QColor statusWarning;
  QColor statusInfo;
};

inline const ThemeColors& getLightTheme() {
  static ThemeColors light{
    .background = QColor(252, 250, 248),
    .backgroundAlt = QColor(247, 243, 240),
    .backgroundCode = QColor(245, 240, 245),
    .backgroundCodeBlock = QColor(242, 237, 242),
    .backgroundLatexInline = QColor(250, 240, 250),
    .backgroundLatexBlock = QColor(245, 235, 245),
    .backgroundInclusion = QColor(240, 250, 240),
    .backgroundInclusionBroken = QColor(255, 240, 240),
    .backgroundSelection = QColor(200, 220, 255),
    .backgroundCurrentLine = QColor(255, 255, 230),
    
    .text = QColor(70, 70, 80),
    .textSecondary = QColor(130, 130, 140),
    .textSelection = QColor(40, 40, 50),
    .textDisabled = QColor(180, 180, 185),
    
    .header = QColor(100, 120, 200),
    .link = QColor(120, 140, 230),
    .wikiLink = QColor(80, 180, 120),
    .brokenLink = QColor(220, 100, 110),
    .code = QColor(180, 100, 140),
    .listMarker = QColor(100, 170, 100),
    .blockquote = QColor(140, 140, 150),
    .latex = QColor(170, 130, 100),
    .strikethrough = QColor(160, 160, 165),
    .inclusion = QColor(70, 160, 90),
    .inclusionBroken = QColor(200, 80, 90),
    .subtleMarkup = QColor(180, 180, 190),
    
    .syntaxKeyword = QColor(140, 110, 200),
    .syntaxString = QColor(110, 170, 120),
    .syntaxComment = QColor(150, 150, 160),
    .syntaxNumber = QColor(210, 150, 110),
    .syntaxFunction = QColor(120, 150, 210),
    .syntaxType = QColor(180, 140, 110),
    
    .border = QColor(220, 215, 225),
    .borderTable = QColor(215, 210, 220),
    .hr = QColor(210, 205, 215),
    
    .uiWindow = QColor(245, 243, 248),
    .uiWindowText = QColor(70, 70, 80),
    .uiBase = QColor(252, 250, 248),
    .uiAlternateBase = QColor(247, 243, 240),
    .uiButton = QColor(242, 240, 245),
    .uiButtonText = QColor(70, 70, 80),
    .uiButtonTextDisabled = QColor(180, 180, 185),
    .uiHighlight = QColor(150, 170, 230),
    .uiHighlightedText = QColor(255, 255, 255),
    .uiToolTipBase = QColor(255, 255, 235),
    .uiToolTipText = QColor(70, 70, 80),
    
    .lineNumberBackground = QColor(245, 243, 248),
    .lineNumberText = QColor(150, 150, 160),
    
    .statusSuccess = QColor(110, 180, 130),
    .statusError = QColor(220, 120, 130),
    .statusWarning = QColor(230, 170, 110),
    .statusInfo = QColor(130, 160, 220)
  };
  return light;
}

inline const ThemeColors& getDarkTheme() {
  static ThemeColors dark{
    .background = QColor(40, 40, 45),
    .backgroundAlt = QColor(50, 50, 55),
    .backgroundCode = QColor(55, 50, 60),
    .backgroundCodeBlock = QColor(50, 47, 55),
    .backgroundLatexInline = QColor(60, 50, 65),
    .backgroundLatexBlock = QColor(58, 48, 63),
    .backgroundInclusion = QColor(50, 65, 55),
    .backgroundInclusionBroken = QColor(70, 50, 55),
    .backgroundSelection = QColor(80, 100, 140),
    .backgroundCurrentLine = QColor(60, 60, 65),
    
    .text = QColor(220, 215, 225),
    .textSecondary = QColor(170, 165, 180),
    .textSelection = QColor(245, 245, 250),
    .textDisabled = QColor(120, 120, 125),
    
    .header = QColor(200, 180, 255),
    .link = QColor(180, 200, 255),
    .wikiLink = QColor(150, 230, 200),
    .brokenLink = QColor(255, 160, 180),
    .code = QColor(240, 180, 210),
    .listMarker = QColor(180, 220, 180),
    .blockquote = QColor(180, 180, 190),
    .latex = QColor(250, 230, 210),
    .strikethrough = QColor(160, 160, 170),
    .inclusion = QColor(160, 240, 200),
    .inclusionBroken = QColor(240, 150, 170),
    .subtleMarkup = QColor(120, 120, 130),
    
    .syntaxKeyword = QColor(210, 180, 240),
    .syntaxString = QColor(180, 220, 190),
    .syntaxComment = QColor(130, 130, 145),
    .syntaxNumber = QColor(240, 200, 170),
    .syntaxFunction = QColor(180, 200, 240),
    .syntaxType = QColor(240, 210, 170),
    
    .border = QColor(80, 80, 90),
    .borderTable = QColor(85, 85, 95),
    .hr = QColor(90, 90, 100),
    
    .uiWindow = QColor(50, 50, 55),
    .uiWindowText = QColor(220, 215, 225),
    .uiBase = QColor(40, 40, 45),
    .uiAlternateBase = QColor(50, 50, 55),
    .uiButton = QColor(55, 55, 60),
    .uiButtonText = QColor(220, 215, 225),
    .uiButtonTextDisabled = QColor(120, 120, 125),
    .uiHighlight = QColor(130, 150, 210),
    .uiHighlightedText = QColor(255, 255, 255),
    .uiToolTipBase = QColor(35, 35, 40),
    .uiToolTipText = QColor(220, 215, 225),
    
    .lineNumberBackground = QColor(50, 50, 55),
    .lineNumberText = QColor(150, 150, 160),
    
    .statusSuccess = QColor(150, 220, 180),
    .statusError = QColor(255, 180, 190),
    .statusWarning = QColor(250, 210, 160),
    .statusInfo = QColor(170, 190, 240)
  };
  return dark;
}

inline QString toRgbString(const QColor& color) {
  return QString("rgb(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue());
}

inline QString toHexString(const QColor& color) {
  return color.name();
}

inline QString toCssRgba(const QColor& color, int alpha = 255) {
  return QString("rgba(%1, %2, %3, %4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(alpha);
}

}

#endif
