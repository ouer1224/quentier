/*
 * Copyright 2017-2019 Dmitry Ivanov
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

#ifndef QUENTIER_LIB_WIDGET_ABSTRACT_FILTER_BY_MODEL_ITEM_WIDGET_H
#define QUENTIER_LIB_WIDGET_ABSTRACT_FILTER_BY_MODEL_ITEM_WIDGET_H

#include <quentier/types/Account.h>
#include <quentier/types/ErrorString.h>
#include <quentier/utility/SuppressWarnings.h>

#include <QWidget>
#include <QPointer>

SAVE_WARNINGS
GCC_SUPPRESS_WARNING(-Wdeprecated-declarations)

#include <boost/bimap.hpp>

RESTORE_WARNINGS

QT_FORWARD_DECLARE_CLASS(FlowLayout)

namespace quentier {

QT_FORWARD_DECLARE_CLASS(ItemModel)
QT_FORWARD_DECLARE_CLASS(NewListItemLineEdit)

/**
 * @brief The AbstractFilterByModelItemWidget class is the base class for filter
 * by model items widget; it handles the boilerplate work of managing the
 * widget's layout and requires its subclasses to implement communication
 * with local storage
 */
class AbstractFilterByModelItemWidget: public QWidget
{
    Q_OBJECT
public:
    explicit AbstractFilterByModelItemWidget(
        const QString & name, QWidget * parent = nullptr);

    const Account & account() const { return m_account; }

    void switchAccount(const Account & account, ItemModel * pItemModel);

    const ItemModel * model() const;

    QStringList itemsInFilter() const;

    QStringList localUidsOfItemsInFilter() const;

    /**
     * @return true if the filter widget has been fully initialized after the
     * most recent account switching, false otherwise
     */
    bool isReady() const;

Q_SIGNALS:
    void notifyError(ErrorString error);

    /**
     * @brief addedItemToFilter signal is emitted when the item is added to
     * the filter
     *
     * @param itemName - the name of the item added to the filted
     */
    void addedItemToFilter(const QString & itemName);

    /**
     * @brief itemRemovedFromFilter signal is emitted when the item is removed
     * from the filter
     *
     * @param itemName - the name of the item removed from the filter
     */
    void itemRemovedFromFilter(const QString & itemName);

    /**
     * @brief cleared signal is emitted when all items are removed from the
     * filter
     */
    void cleared();

    /**
     * @brief updated signal is emitted when the set of items in the filter
     * changes so that it needs to be re-requested and re-processed
     */
    void updated();

    /**
     * @brief ready signal is emitted when the filter widget's initialization
     * is complete
     */
    void ready();

public Q_SLOTS:
    void addItemToFilter(const QString & localUid, const QString & itemName);
    void clear();

    /**
     * @brief update - this slot should be called in case it's necessary to
     * re-fetch the information about all items within the filter because some
     * of them might have been deleted but and it's hard to tell which ones
     * exactly
     */
    void update();

    // The subclass should call these methods in relevant circumstances
    void onItemUpdatedInLocalStorage(
        const QString & localUid, const QString & name);

    void onItemRemovedFromLocalStorage(const QString & localUid);

private Q_SLOTS:
    void onNewItemAdded();
    void onItemRemovedFromList(QString name);
    void onModelReady();

private:
    void persistFilteredItems();
    void restoreFilteredItems();

    void addNewItemWidget();
    void clearLayout();

    NewListItemLineEdit * findNewItemWidget();

private:
    QString                     m_name;
    FlowLayout *                m_pLayout;
    Account                     m_account;
    QPointer<ItemModel>         m_pItemModel;
    bool                        m_isReady;

    typedef boost::bimap<QString, QString> ItemLocalUidToNameBimap;
    ItemLocalUidToNameBimap     m_filteredItemsLocalUidToNameBimap;
};

} // namespace quentier

#endif // QUENTIER_LIB_WIDGET_ABSTRACT_FILTER_BY_MODEL_ITEM_WIDGET_H
