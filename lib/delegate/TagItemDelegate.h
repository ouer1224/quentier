/*
 * Copyright 2016-2019 Dmitry Ivanov
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

#ifndef QUENTIER_LIB_DELEGATE_TAG_ITEM_DELEGATE_H
#define QUENTIER_LIB_DELEGATE_TAG_ITEM_DELEGATE_H

#include "AbstractStyledItemDelegate.h"

namespace quentier {

class TagItemDelegate: public AbstractStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TagItemDelegate(QObject * parent = nullptr);

private:
    // QStyledItemDelegate interface
    virtual QString displayText(
        const QVariant & value, const QLocale & locale) const override;

    virtual QWidget * createEditor(
        QWidget * parent, const QStyleOptionViewItem & option,
        const QModelIndex & index) const override;

    virtual void paint(
        QPainter * painter, const QStyleOptionViewItem & option,
        const QModelIndex & index) const override;

    virtual void setEditorData(
        QWidget * editor, const QModelIndex & index) const override;

    virtual void setModelData(
        QWidget * editor, QAbstractItemModel * model,
        const QModelIndex & index) const override;

    virtual QSize sizeHint(
        const QStyleOptionViewItem & option,
        const QModelIndex & index) const override;

    virtual void updateEditorGeometry(
        QWidget * editor, const QStyleOptionViewItem & option,
        const QModelIndex & index) const override;

private:
    void drawTagName(
        QPainter * painter, const QModelIndex & index,
        const QStyleOptionViewItem & option) const;

    QSize tagNameSizeHint(
        const QStyleOptionViewItem & option, const QModelIndex & index) const;

private:
    QIcon   m_userIcon;
    QSize   m_userIconSize;
};

} // namespace quentier

#endif // QUENTIER_LIB_DELEGATE_TAG_ITEM_DELEGATE_H
