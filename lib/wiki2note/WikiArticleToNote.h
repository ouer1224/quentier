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

#ifndef QUENTIER_LIB_WIKI2NOTE_WIKI_ARTICLE_TO_NOTE_H
#define QUENTIER_LIB_WIKI2NOTE_WIKI_ARTICLE_TO_NOTE_H

#include <quentier/utility/Macros.h>
#include <quentier/types/Note.h>

#include <QObject>
#include <QHash>
#include <QUrl>

namespace quentier {

QT_FORWARD_DECLARE_CLASS(ENMLConverter)
QT_FORWARD_DECLARE_CLASS(NetworkReplyFetcher)

class WikiArticleToNote: public QObject
{
    Q_OBJECT
public:
    explicit WikiArticleToNote(
        ENMLConverter & enmlConverter,
        QObject * parent = Q_NULLPTR);

    bool isStarted() const { return m_started; }
    bool isFinished() const { return m_finished; }

    const Note & note() const { return m_note; }

Q_SIGNALS:
    void finished(bool status, ErrorString errorDescription, Note note);

public Q_SLOTS:
    void start(QByteArray wikiPageContent);

private:
    void finishWithError(ErrorString errorDescription);

private:
    ENMLConverter &     m_enmlConverter;
    Note    m_note;

    bool    m_started;
    bool    m_finished;

    QHash<QString, NetworkReplyFetcher*> m_imageDataFetchersByResourceLocalUid;
};

} // namespace quentier

#endif // QUENTIER_LIB_WIKI2NOTE_WIKI_ARTICLE_TO_NOTE_H
