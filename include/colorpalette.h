#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include <QColor>
#include <QString>

namespace ColorPalette {

struct ThemeColors {
  QColor background = QColor(252, 250, 248);
  QColor backgroundAlt = QColor(247, 243, 240);
  QColor backgroundCode = QColor(245, 240, 245);
  QColor backgroundCodeBlock = QColor(242, 237, 242);
  QColor backgroundLatexInline = QColor(250, 240, 250);
  QColor backgroundLatexBlock = QColor(245, 235, 245);
  QColor backgroundInclusion = QColor(240, 250, 240);
  QColor backgroundInclusionBroken = QColor(255, 240, 240);
  QColor backgroundSelection = QColor(200, 220, 255);
  QColor backgroundCurrentLine = QColor(255, 255, 230);
  
  QColor text = QColor(70, 70, 80);
  QColor textSecondary = QColor(130, 130, 140);
  QColor textSelection = QColor(40, 40, 50);
  QColor textDisabled = QColor(180, 180, 185);
  
  QColor header = QColor(100, 120, 200);
  QColor link = QColor(120, 140, 230);
  QColor wikiLink = QColor(80, 180, 120);
  QColor brokenLink = QColor(220, 100, 110);
  QColor code = QColor(180, 100, 140);
  QColor listMarker = QColor(100, 170, 100);
  QColor blockquote = QColor(140, 140, 150);
  QColor latex = QColor(170, 130, 100);
  QColor strikethrough = QColor(160, 160, 165);
  QColor inclusion = QColor(70, 160, 90);
  QColor inclusionBroken = QColor(200, 80, 90);
  QColor subtleMarkup = QColor(180, 180, 190);
  
  QColor syntaxKeyword = QColor(140, 110, 200);
  QColor syntaxString = QColor(110, 170, 120);
  QColor syntaxComment = QColor(150, 150, 160);
  QColor syntaxNumber = QColor(210, 150, 110);
  QColor syntaxFunction = QColor(120, 150, 210);
  QColor syntaxType = QColor(180, 140, 110);
  
  QColor border = QColor(220, 215, 225);
  QColor borderTable = QColor(215, 210, 220);
  QColor hr = QColor(210, 205, 215);
  
  QColor uiWindow = QColor(245, 243, 248);
  QColor uiWindowText = QColor(70, 70, 80);
  QColor uiBase = QColor(252, 250, 248);
  QColor uiAlternateBase = QColor(247, 243, 240);
  QColor uiButton = QColor(242, 240, 245);
  QColor uiButtonText = QColor(70, 70, 80);
  QColor uiButtonTextDisabled = QColor(180, 180, 185);
  QColor uiHighlight = QColor(150, 170, 230);
  QColor uiHighlightedText = QColor(255, 255, 255);
  QColor uiToolTipBase = QColor(255, 255, 235);
  QColor uiToolTipText = QColor(70, 70, 80);
  
  QColor lineNumberBackground = QColor(245, 243, 248);
  QColor lineNumberText = QColor(150, 150, 160);
  
  QColor statusSuccess = QColor(110, 180, 130);
  QColor statusError = QColor(220, 120, 130);
  QColor statusWarning = QColor(230, 170, 110);
  QColor statusInfo = QColor(130, 160, 220);
};

inline const ThemeColors& getLightTheme() {
  static ThemeColors light;
  return light;
}

inline const ThemeColors& getDarkTheme() {
  static ThemeColors dark{
    QColor(40, 40, 45),
    QColor(50, 50, 55),
    QColor(55, 50, 60),
    QColor(50, 47, 55),
    QColor(60, 50, 65),
    QColor(58, 48, 63),
    QColor(50, 65, 55),
    QColor(70, 50, 55),
    QColor(80, 100, 140),
    QColor(60, 60, 65),
    
    QColor(220, 215, 225),
    QColor(170, 165, 180),
    QColor(245, 245, 250),
    QColor(120, 120, 125),
    
    QColor(200, 180, 255),
    QColor(180, 200, 255),
    QColor(150, 230, 200),
    QColor(255, 160, 180),
    QColor(240, 180, 210),
    QColor(180, 220, 180),
    QColor(180, 180, 190),
    QColor(250, 230, 210),
    QColor(160, 160, 170),
    QColor(160, 240, 200),
    QColor(240, 150, 170),
    QColor(120, 120, 130),
    
    QColor(210, 180, 240),
    QColor(180, 220, 190),
    QColor(130, 130, 145),
    QColor(240, 200, 170),
    QColor(180, 200, 240),
    QColor(240, 210, 170),
    
    QColor(80, 80, 90),
    QColor(85, 85, 95),
    QColor(90, 90, 100),
    
    QColor(50, 50, 55),
    QColor(220, 215, 225),
    QColor(40, 40, 45),
    QColor(50, 50, 55),
    QColor(55, 55, 60),
    QColor(220, 215, 225),
    QColor(120, 120, 125),
    QColor(130, 150, 210),
    QColor(255, 255, 255),
    QColor(35, 35, 40),
    QColor(220, 215, 225),
    
    QColor(50, 50, 55),
    QColor(150, 150, 160),
    
    QColor(150, 220, 180),
    QColor(255, 180, 190),
    QColor(250, 210, 160),
    QColor(170, 190, 240)
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
