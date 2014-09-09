#include "NoteData.h"
#include "../../Utility.h"
#include <logging/QuteNoteLogger.h>
#include <QDomDocument>

namespace qute_note {

NoteData::NoteData() :
    m_isLocal(true),
    m_thumbnail(),
    m_lazyPlainText(),
    m_lazyPlainTextIsValid(false),
    m_lazyListOfWords(),
    m_lazyListOfWordsIsValid(false),
    m_lazyContainsCheckedToDo(-1),
    m_lazyContainsUncheckedToDo(-1),
    m_lazyContainsEncryption(-1)
{}

NoteData::NoteData(NoteData && other)
{
    m_lazyPlainText = other.m_lazyPlainText;
    other.m_lazyPlainText.clear();

    m_lazyPlainTextIsValid = other.m_lazyPlainTextIsValid;
    other.m_lazyPlainTextIsValid = false;

    m_lazyListOfWords = other.m_lazyListOfWords;
    other.m_lazyListOfWords.clear();

    m_lazyListOfWordsIsValid = other.m_lazyListOfWordsIsValid;
    other.m_lazyListOfWordsIsValid = false;
}

NoteData::~NoteData()
{}

NoteData::NoteData(const qevercloud::Note & other) :
    m_qecNote(other),
    m_isLocal(false),
    m_thumbnail(),
    m_lazyPlainText(),
    m_lazyPlainTextIsValid(false),
    m_lazyListOfWords(),
    m_lazyListOfWordsIsValid(false),
    m_lazyContainsCheckedToDo(-1),
    m_lazyContainsUncheckedToDo(-1),
    m_lazyContainsEncryption(-1)
{}

NoteData & NoteData::operator=(const qevercloud::Note & other)
{
    m_qecNote = other;
    m_lazyPlainTextIsValid = false;    // Mark any existing plain text as invalid but don't free memory
    m_lazyListOfWordsIsValid = false;
    m_lazyContainsCheckedToDo = -1;
    m_lazyContainsUncheckedToDo = -1;
    m_lazyContainsEncryption = -1;
    return *this;
}

NoteData & NoteData::operator=(NoteData && other)
{
    if (this != &other)
    {
        m_qecNote = std::move(other.m_qecNote);
        m_isLocal = std::move(other.m_isLocal);
        m_thumbnail = std::move(other.m_thumbnail);

        m_lazyPlainText = other.m_lazyPlainText;
        other.m_lazyPlainText.clear();

        m_lazyPlainTextIsValid = other.m_lazyPlainTextIsValid;
        other.m_lazyPlainTextIsValid = false;

        m_lazyListOfWords = other.m_lazyListOfWords;
        other.m_lazyListOfWords.clear();

        m_lazyListOfWordsIsValid = other.m_lazyListOfWordsIsValid;
        other.m_lazyListOfWordsIsValid = false;

        m_lazyContainsCheckedToDo   = other.m_lazyContainsCheckedToDo;
        m_lazyContainsUncheckedToDo = other.m_lazyContainsUncheckedToDo;
        m_lazyContainsEncryption    = other.m_lazyContainsEncryption;
    }

    return *this;
}

bool NoteData::ResourceAdditionalInfo::operator==(const NoteData::ResourceAdditionalInfo & other) const
{
    return (localGuid == other.localGuid) &&
            (noteLocalGuid == other.noteLocalGuid) &&
            (isDirty == other.isDirty);
}


bool NoteData::containsToDoImpl(const bool checked) const
{
    int & refLazyContainsToDo = (checked ? m_lazyContainsCheckedToDo : m_lazyContainsUncheckedToDo);
    if (refLazyContainsToDo > (-1)) {
        if (refLazyContainsToDo == 0) {
            return false;
        }
        else {
            return true;
        }
    }

    if (!m_qecNote.content.isSet()) {
        refLazyContainsToDo = 0;
        return false;
    }

    QDomDocument enXmlDomDoc;
    int errorLine = -1, errorColumn = -1;
    QString errorMessage;
    bool res = enXmlDomDoc.setContent(m_qecNote.content.ref(), &errorMessage, &errorLine, &errorColumn);
    if (!res) {
        // TRANSLATOR Explaining the error of XML parsing
        errorMessage += QT_TR_NOOP(". Error happened at line ") +
                        QString::number(errorLine) + QT_TR_NOOP(", at column ") +
                        QString::number(errorColumn);
        QNWARNING("Note content parsing error: " << errorMessage);
        refLazyContainsToDo = 0;
        return false;
    }

    QDomElement docElem = enXmlDomDoc.documentElement();
    QDomNode nextNode = docElem.firstChild();
    while (!nextNode.isNull())
    {
        QDomElement element = nextNode.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            if (tagName == "en-todo")
            {
                QString checkedStr = element.attribute("checked", "false");
                if (checked && (checkedStr == "true")) {
                    refLazyContainsToDo = 1;
                    return true;
                }
                else if (!checked && (checkedStr == "false")) {
                    refLazyContainsToDo = 1;
                    return true;
                }
            }
        }
        nextNode = nextNode.nextSibling();
    }

    refLazyContainsToDo = 0;
    return false;
}

void NoteData::setContent(const QString &content)
{
    m_qecNote.content = content;
    m_lazyPlainTextIsValid = false;    // Mark any existing plain text as invalid but don't free memory
    m_lazyListOfWordsIsValid = false;
    m_lazyContainsCheckedToDo = -1;
    m_lazyContainsUncheckedToDo = -1;
    m_lazyContainsEncryption = -1;
}

void NoteData::clear()
{
    m_qecNote = qevercloud::Note();
    m_lazyPlainTextIsValid = false;    // Mark any existing plain text as invalid but don't free memory
    m_lazyListOfWordsIsValid = false;
    m_lazyContainsCheckedToDo = -1;
    m_lazyContainsUncheckedToDo = -1;
    m_lazyContainsEncryption = -1;
}

bool NoteData::checkParameters(QString & errorDescription) const
{
    if (m_qecNote.guid.isSet() && !CheckGuid(m_qecNote.guid.ref())) {
        errorDescription = QT_TR_NOOP("Note's guid is invalid");
        return false;
    }

    if (m_qecNote.updateSequenceNum.isSet() && !CheckUpdateSequenceNumber(m_qecNote.updateSequenceNum)) {
        errorDescription = QT_TR_NOOP("Note's update sequence number is invalid");
        return false;
    }

    if (m_qecNote.title.isSet())
    {
        int titleSize = m_qecNote.title->size();

        if ( (titleSize < qevercloud::EDAM_NOTE_TITLE_LEN_MIN) ||
             (titleSize > qevercloud::EDAM_NOTE_TITLE_LEN_MAX) )
        {
            errorDescription = QT_TR_NOOP("Note's title length is invalid");
            return false;
        }
    }

    if (m_qecNote.content.isSet())
    {
        int contentSize = m_qecNote.content->size();

        if ( (contentSize < qevercloud::EDAM_NOTE_CONTENT_LEN_MIN) ||
             (contentSize > qevercloud::EDAM_NOTE_CONTENT_LEN_MAX) )
        {
            errorDescription = QT_TR_NOOP("Note's content length is invalid");
            return false;
        }
    }

    if (m_qecNote.contentHash.isSet())
    {
        size_t contentHashSize = m_qecNote.contentHash->size();

        if (contentHashSize != qevercloud::EDAM_HASH_LEN) {
            errorDescription = QT_TR_NOOP("Note's content hash size is invalid");
            return false;
        }
    }

    if (m_qecNote.notebookGuid.isSet() && !CheckGuid(m_qecNote.notebookGuid.ref())) {
        errorDescription = QT_TR_NOOP("Note's notebook guid is invalid");
        return false;
    }

    if (m_qecNote.tagGuids.isSet())
    {
        int numTagGuids = m_qecNote.tagGuids->size();

        if (numTagGuids > qevercloud::EDAM_NOTE_TAGS_MAX) {
            errorDescription = QT_TR_NOOP("Note has too many tags, max allowed ");
            errorDescription.append(QString::number(qevercloud::EDAM_NOTE_TAGS_MAX));
            return false;
        }
    }

    if (m_qecNote.resources.isSet())
    {
        int numResources = m_qecNote.resources->size();

        if (numResources > qevercloud::EDAM_NOTE_RESOURCES_MAX) {
            errorDescription = QT_TR_NOOP("Note has too many resources, max allowed ");
            errorDescription.append(QString::number(qevercloud::EDAM_NOTE_RESOURCES_MAX));
            return false;
        }
    }


    if (m_qecNote.attributes.isSet())
    {
        const qevercloud::NoteAttributes & attributes = m_qecNote.attributes;

#define CHECK_NOTE_ATTRIBUTE(name) \
    if (attributes.name.isSet()) { \
        int name##Size = attributes.name->size(); \
        \
        if ( (name##Size < qevercloud::EDAM_ATTRIBUTE_LEN_MIN) || \
             (name##Size > qevercloud::EDAM_ATTRIBUTE_LEN_MAX) ) \
        { \
            errorDescription = QT_TR_NOOP("Note attributes' " #name " field has invalid size"); \
            return false; \
        } \
    }

        CHECK_NOTE_ATTRIBUTE(author);
        CHECK_NOTE_ATTRIBUTE(source);
        CHECK_NOTE_ATTRIBUTE(sourceURL);
        CHECK_NOTE_ATTRIBUTE(sourceApplication);

#undef CHECK_NOTE_ATTRIBUTE

        if (attributes.contentClass.isSet())
        {
            int contentClassSize = attributes.contentClass->size();
            if ( (contentClassSize < qevercloud::EDAM_NOTE_CONTENT_CLASS_LEN_MIN) ||
                 (contentClassSize > qevercloud::EDAM_NOTE_CONTENT_CLASS_LEN_MAX) )
            {
                errorDescription = QT_TR_NOOP("Note attributes' content class has invalid size");
                return false;
            }
        }

        if (attributes.applicationData.isSet())
        {
            const qevercloud::LazyMap & applicationData = attributes.applicationData;

            if (applicationData.keysOnly.isSet())
            {
                const QSet<QString> & keysOnly = applicationData.keysOnly;
                foreach(const QString & key, keysOnly) {
                    int keySize = key.size();
                    if ( (keySize < qevercloud::EDAM_APPLICATIONDATA_NAME_LEN_MIN) ||
                         (keySize > qevercloud::EDAM_APPLICATIONDATA_NAME_LEN_MAX) )
                    {
                        errorDescription = QT_TR_NOOP("Note's attributes application data has invalid key (in keysOnly part)");
                        return false;
                    }
                }
            }

            if (applicationData.fullMap.isSet())
            {
                const QMap<QString, QString> & fullMap = applicationData.fullMap;
                for(QMap<QString, QString>::const_iterator it = fullMap.constBegin(); it != fullMap.constEnd(); ++it)
                {
                    int keySize = it.key().size();
                    if ( (keySize < qevercloud::EDAM_APPLICATIONDATA_NAME_LEN_MIN) ||
                         (keySize > qevercloud::EDAM_APPLICATIONDATA_NAME_LEN_MAX) )
                    {
                        errorDescription = QT_TR_NOOP("Note's attributes application data has invalid key (in fullMap part)");
                        return false;
                    }

                    int valueSize = it.value().size();
                    if ( (valueSize < qevercloud::EDAM_APPLICATIONDATA_VALUE_LEN_MIN) ||
                         (valueSize > qevercloud::EDAM_APPLICATIONDATA_VALUE_LEN_MAX) )
                    {
                        errorDescription = QT_TR_NOOP("Note's attributes application data has invalid value");
                        return false;
                    }

                    int sumSize = keySize + valueSize;
                    if (sumSize > qevercloud::EDAM_APPLICATIONDATA_ENTRY_LEN_MAX) {
                        errorDescription = QT_TR_NOOP("Note's attributes application data has invalid entry size");
                        return false;
                    }
                }
            }
        }

        if (attributes.classifications.isSet())
        {
            const QMap<QString, QString> & classifications = attributes.classifications;
            for(QMap<QString, QString>::const_iterator it = classifications.constBegin();
                it != classifications.constEnd(); ++it)
            {
                const QString & value = it.value();
                if (!value.startsWith("CLASSIFICATION_")) {
                    errorDescription = QT_TR_NOOP("Note's attributes classifications has invalid classification value");
                    return false;
                }
            }
        }
    }

    return true;
}

} // namespace qute_note
