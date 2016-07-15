/*
 * Copyright 2016 Dmitry Ivanov
 *
 * This file is part of libquentier
 *
 * libquentier is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * libquentier is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libquentier. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIB_QUENTIER_ENML_ENML_CONVERTER_P_H
#define LIB_QUENTIER_ENML_ENML_CONVERTER_P_H

#include <quentier/enml/ENMLConverter.h>
#include <quentier/utility/Qt4Helper.h>
#include <quentier/utility/QNLocalizedString.h>
#include <QtGlobal>
#include <QStringList>
#include <QFlag>

QT_FORWARD_DECLARE_CLASS(QXmlStreamReader)
QT_FORWARD_DECLARE_CLASS(QXmlStreamWriter)
QT_FORWARD_DECLARE_CLASS(QXmlStreamAttributes)

namespace quentier {

QT_FORWARD_DECLARE_CLASS(IResource)
QT_FORWARD_DECLARE_CLASS(DecryptedTextManager)
QT_FORWARD_DECLARE_CLASS(HTMLCleaner)

class ENMLConverterPrivate
{
public:
    ENMLConverterPrivate();
    ~ENMLConverterPrivate();

    typedef ENMLConverter::NoteContentToHtmlExtraData NoteContentToHtmlExtraData;
    typedef ENMLConverter::SkipHtmlElementRule SkipHtmlElementRule;

    bool htmlToNoteContent(const QString & html,
                           const QVector<SkipHtmlElementRule> & skipRules,
                           QString & noteContent,
                           DecryptedTextManager & decryptedTextManager,
                           QNLocalizedString & errorDescription) const;
    bool noteContentToHtml(const QString & noteContent, QString & html,
                           QNLocalizedString & errorDescription,
                           DecryptedTextManager & decryptedTextManager,
                           NoteContentToHtmlExtraData & extraData) const;

    bool validateEnml(const QString & enml, QNLocalizedString & errorDescription) const;

    static bool noteContentToPlainText(const QString & noteContent, QString & plainText,
                                       QNLocalizedString & errorMessage);

    static bool noteContentToListOfWords(const QString & noteContent, QStringList & listOfWords,
                                         QNLocalizedString & errorMessage, QString * plainText = Q_NULLPTR);

    static QStringList plainTextToListOfWords(const QString & plainText);

    static QString toDoCheckboxHtml(const bool checked, const quint64 idNumber);

    static QString encryptedTextHtml(const QString & encryptedText, const QString & hint,
                                     const QString & cipher, const size_t keyLength,
                                     const quint64 enCryptIndex);

    static QString decryptedTextHtml(const QString & decryptedText, const QString & encryptedText,
                                     const QString & hint, const QString & cipher,
                                     const size_t keyLength, const quint64 enDecryptedIndex);

    static QString resourceHtml(const IResource & resource, QNLocalizedString & errorDescription);

    static void escapeString(QString & string, const bool simplify);

private:
    static bool isForbiddenXhtmlTag(const QString & tagName);
    static bool isForbiddenXhtmlAttribute(const QString & attributeName);
    static bool isEvernoteSpecificXhtmlTag(const QString & tagName);
    static bool isAllowedXhtmlTag(const QString & tagName);

    // convert <div> element with decrypted text to ENML <en-crypt> tag
    bool decryptedTextToEnml(QXmlStreamReader & reader,
                             DecryptedTextManager & decryptedTextManager,
                             QXmlStreamWriter & writer, QNLocalizedString & errorDescription) const;

    // convert ENML en-crypt tag to HTML <object> tag
    bool encryptedTextToHtml(const QXmlStreamAttributes & enCryptAttributes,
                             const QStringRef & encryptedTextCharacters, const quint64 enCryptIndex, const quint64 enDecryptedIndex,
                             QXmlStreamWriter & writer, DecryptedTextManager & decryptedTextManager, bool & convertedToEnCryptNode) const;

    // convert ENML <en-media> tag to HTML <object> tag
    static bool resourceInfoToHtml(const QXmlStreamAttributes & attributes, QXmlStreamWriter & writer,
                                   QNLocalizedString & errorDescription);

    void toDoTagsToHtml(const QXmlStreamReader & reader, const quint64 enToDoIndex,
                        QXmlStreamWriter & writer) const;

    static void decryptedTextHtml(const QString & decryptedText, const QString & encryptedText,
                                  const QString & hint, const QString & cipher, const size_t keyLength, const quint64 enDecryptedIndex,
                                  QXmlStreamWriter & writer);

    class ShouldSkipElementResult: public Printable
    {
    public:
        enum type
        {
            SkipWithContents = 0x0,
            SkipButPreserveContents = 0x1,
            ShouldNotSkip = 0x2
        };

        Q_DECLARE_FLAGS(Types, type)

        virtual QTextStream & print(QTextStream & strm) const Q_DECL_OVERRIDE;
    };

    ShouldSkipElementResult::type shouldSkipElement(const QString & elementName,
                                                    const QXmlStreamAttributes & attributes,
                                                    const QVector<SkipHtmlElementRule> & skipRules) const;

private:
    Q_DISABLE_COPY(ENMLConverterPrivate)

private:
    mutable HTMLCleaner *   m_pHtmlCleaner;
    mutable QString         m_cachedConvertedXml;   // Cached memory for the html converted to valid xml
};

} // namespace quentier

QUENTIER_DECLARE_PRINTABLE(QXmlStreamAttributes)
QUENTIER_DECLARE_PRINTABLE(QVector<quentier::ENMLConverter::SkipHtmlElementRule>)

#endif // LIB_QUENTIER_ENML_ENML_CONVERTER_P_H