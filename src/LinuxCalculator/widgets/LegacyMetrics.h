// SPDX-License-Identifier: MIT
#pragma once
#include "KeyLayout.h"
namespace Win7Calc::LegacyMetrics {
inline constexpr int menuHeight=19, displayX=11, displayY=30, displayHeight=50;
inline constexpr int historyExpansion=100, supplementWidth=374;
inline qreal fontScale(const QFont& font) {return qMax(qreal(.75),font.pointSizeF()>0?font.pointSizeF()/9.0:qreal(1));}
}
