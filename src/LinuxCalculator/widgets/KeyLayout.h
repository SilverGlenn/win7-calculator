// SPDX-License-Identifier: MIT
#pragma once
#include <QRect>
#include <QSize>
#include <QString>
#include <optional>
#include "model/CalculatorState.h"
namespace Win7Calc {
enum class KeyAppearance { Number, Operator, Memory };
struct KeySpec {
 QString id, text, accessibleName;
 QRect rect;
 std::optional<CalculationManager::Command> command;
 KeyAppearance appearance = KeyAppearance::Operator;
};
QSize clientSize(Mode,Panel,bool historyVisible,qreal logicalScale=1);
QRect scaledRect(QRect,qreal);
std::vector<KeySpec> keyLayout(Mode,bool inverse=false);
}
