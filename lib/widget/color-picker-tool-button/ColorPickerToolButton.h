/*
 * Copyright 2015-2019 Dmitry Ivanov
 *
 * This file is part of Quentier.
 *
 * Quentier is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * Quentier is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Quentier. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QUENTIER_LIB_WIDGET_COLOR_PICKER_TOOL_BUTTON_H
#define QUENTIER_LIB_WIDGET_COLOR_PICKER_TOOL_BUTTON_H

#include <QToolButton>
#include <QColor>

QT_FORWARD_DECLARE_CLASS(QMenu)

namespace quentier {

class ColorPickerToolButton: public QToolButton
{
    Q_OBJECT
public:
    explicit ColorPickerToolButton(QWidget * parent = 0);

Q_SIGNALS:
    void colorSelected(QColor color);
    void rejected();

private Q_SLOTS:
    void onColorDialogAction();

private:
    QMenu * m_menu;
};

} // namespace quentier

#endif // QUENTIER_LIB_WIDGET_COLOR_PICKER_TOOL_BUTTON_H
