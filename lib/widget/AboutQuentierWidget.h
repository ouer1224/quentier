/*
 * Copyright 2017 Dmitry Ivanov
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

#ifndef QUENTIER_LIB_WIDGET_ABOUT_QUENTIER_WIDGET_H
#define QUENTIER_LIB_WIDGET_ABOUT_QUENTIER_WIDGET_H

#include <quentier/utility/Macros.h>

#include <QWidget>

namespace Ui {
class AboutQuentierWidget;
}

namespace quentier {

class AboutQuentierWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AboutQuentierWidget(QWidget * parent = nullptr);
    ~AboutQuentierWidget();

private:
    Ui::AboutQuentierWidget *   m_pUi;
};

} // namespace quentier


#endif // QUENTIER_LIB_WIDGET_ABOUT_QUENTIER_WIDGET_H
