/*
 * Copyright 2019 Dmitry Ivanov
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

#ifndef QUENTIER_WIKI2ACCOUNT_FETCH_NOTES_H
#define QUENTIER_WIKI2ACCOUNT_FETCH_NOTES_H

#include <quentier/types/Notebook.h>
#include <quentier/types/Tag.h>

namespace quentier {

QT_FORWARD_DECLARE_CLASS(LocalStorageManagerAsync)

bool FetchNotes(
    const QList<Notebook> & notebooks, const QList<Tag> & tags,
    const quint32 minTagsPerNote, const quint32 numNotes,
    LocalStorageManagerAsync & localStorageManager);

} // namespace quentier

#endif // QUENTIER_WIKI2ACCOUNT_FETCH_NOTES_H
