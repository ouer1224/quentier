#include "RenameResourceDelegate.h"
#include "../NoteEditor_p.h"
#include "../GenericResourceImageWriter.h"
#include "../dialogs/RenameResourceDialog.h"
#include <qute_note/logging/QuteNoteLogger.h>
#include <QScopedPointer>
#include <QBuffer>
#include <QImage>

namespace qute_note {

#define GET_PAGE() \
    NoteEditorPage * page = qobject_cast<NoteEditorPage*>(m_noteEditor.page()); \
    if (Q_UNLIKELY(!page)) { \
        QString error = QT_TR_NOOP("Can't rename resource: can't get note editor page"); \
        QNWARNING(error); \
        emit notifyError(error); \
        return; \
    }

RenameResourceDelegate::RenameResourceDelegate(const ResourceWrapper & resource, NoteEditorPrivate & noteEditor,
                                               GenericResourceImageWriter * pGenericResourceImageWriter,
                                               const bool performingUndo) :
    m_noteEditor(noteEditor),
    m_pGenericResourceImageWriter(pGenericResourceImageWriter),
    m_resource(resource),
    m_oldResourceName(resource.displayName()),
    m_newResourceName(),
    m_shouldGetResourceNameFromDialog(true),
    m_performingUndo(performingUndo)
#ifdef USE_QT_WEB_ENGINE
    ,
    m_genericResourceImageWriterRequestId(),
    m_newGenericResourceImageFilePath()
#endif
{}

void RenameResourceDelegate::start()
{
    QNDEBUG("RenameResourceDelegate::start");

    if (m_noteEditor.isModified()) {
        QObject::connect(&m_noteEditor, QNSIGNAL(NoteEditorPrivate,convertedToNote,Note),
                         this, QNSLOT(RenameResourceDelegate,onOriginalPageConvertedToNote,Note));
        m_noteEditor.convertToNote();
    }
    else {
        doStart();
    }
}

void RenameResourceDelegate::startWithPresetNames(const QString & oldResourceName, const QString & newResourceName)
{
    QNDEBUG("RenameResourceDelegate::startWithPresetNames: old resource name = " << oldResourceName
            << ", new resource name = " << newResourceName);

    m_oldResourceName = oldResourceName;
    m_newResourceName = newResourceName;
    m_shouldGetResourceNameFromDialog = false;

    start();
}

void RenameResourceDelegate::onOriginalPageConvertedToNote(Note note)
{
    QNDEBUG("RenameResourceDelegate::onOriginalPageConvertedToNote");

    Q_UNUSED(note)

    QObject::disconnect(&m_noteEditor, QNSIGNAL(NoteEditorPrivate,convertedToNote,Note),
                        this, QNSLOT(RenameResourceDelegate,onOriginalPageConvertedToNote,Note));

    doStart();
}

void RenameResourceDelegate::doStart()
{
    QNDEBUG("RenameResourceDelegate::doStart");

    if (Q_UNLIKELY(!m_resource.hasDataHash())) {
        QString error = QT_TR_NOOP("Can't rename resource: resource to rename doesn't have the data hash set");
        QNWARNING(error);
        emit notifyError(error);
        return;
    }

    if (m_shouldGetResourceNameFromDialog)
    {
        raiseRenameResourceDialog();
    }
    else
    {
        m_resource.setDisplayName(m_newResourceName);

#ifdef USE_QT_WEB_ENGINE
        buildAndSaveGenericResourceImage();
#else
        emit finished(m_oldResourceName, m_newResourceName, m_resource, m_performingUndo, QString());
#endif
    }
}

void RenameResourceDelegate::raiseRenameResourceDialog()
{
    QNDEBUG("RenameResourceDelegate::raiseRenameResourceDialog");

    QScopedPointer<RenameResourceDialog> pRenameResourceDialog(new RenameResourceDialog(m_oldResourceName, &m_noteEditor));
    pRenameResourceDialog->setWindowModality(Qt::WindowModal);
    QObject::connect(pRenameResourceDialog.data(), QNSIGNAL(RenameResourceDialog,accepted,QString),
                     this, QNSLOT(RenameResourceDelegate,onRenameResourceDialogFinished,QString));
    QNTRACE("Will exec rename resource dialog now");
    int res = pRenameResourceDialog->exec();
    if (res == QDialog::Rejected) {
        QNTRACE("Cancelled renaming the resource");
        emit cancelled();
    }
}

void RenameResourceDelegate::onRenameResourceDialogFinished(QString newResourceName)
{
    QNDEBUG("RenameResourceDelegate::onRenameResourceDialogFinished: new resource name = " << newResourceName);

    if (newResourceName.isEmpty()) {
        QNTRACE("New resource name is empty, treating it as cancellation");
        emit cancelled();
        return;
    }

    if (newResourceName == m_oldResourceName) {
        QNTRACE("The new resource name is equal to the old one, treating it as cancellation");
        emit cancelled();
        return;
    }

    m_newResourceName = newResourceName;
    m_resource.setDisplayName(m_newResourceName);
    m_noteEditor.replaceResourceInNote(m_resource);

#ifdef USE_QT_WEB_ENGINE
    buildAndSaveGenericResourceImage();
#else
    emit finished(m_oldResourceName, m_newResourceName, m_resource, m_performingUndo, QString());
#endif
}

#ifdef USE_QT_WEB_ENGINE
void RenameResourceDelegate::buildAndSaveGenericResourceImage()
{
    QNDEBUG("RenameResourceDelegate::buildAndSaveGenericResourceImage");

    QImage resourceImage = m_noteEditor.buildGenericResourceImage(m_resource);

    QByteArray imageData;
    QBuffer buffer(&imageData);
    Q_UNUSED(buffer.open(QIODevice::WriteOnly));
    resourceImage.save(&buffer, "PNG");

    m_genericResourceImageWriterRequestId = QUuid::createUuid();

    QNDEBUG("Emitting request to write generic resource image for resource with local uid "
            << m_resource.localUid() << ", request id " << m_genericResourceImageWriterRequestId);

    QObject::connect(this, QNSIGNAL(RenameResourceDelegate,saveGenericResourceImageToFile,QString,QByteArray,QString,QByteArray,QString,QUuid),
                     m_pGenericResourceImageWriter, QNSLOT(GenericResourceImageWriter,onGenericResourceImageWriteRequest,QString,QByteArray,QString,QByteArray,QString,QUuid));
    QObject::connect(m_pGenericResourceImageWriter, QNSIGNAL(GenericResourceImageWriter,genericResourceImageWriteReply,bool,QByteArray,QString,QString,QUuid),
                     this, QNSLOT(RenameResourceDelegate,onGenericResourceImageWriterFinished,bool,QByteArray,QString,QString,QUuid));

    emit saveGenericResourceImageToFile(m_resource.localUid(), imageData, "png", m_resource.dataHash(), m_resource.displayName(), m_genericResourceImageWriterRequestId);
}

void RenameResourceDelegate::onGenericResourceImageWriterFinished(bool success, QByteArray resourceHash, QString filePath,
                                                                  QString errorDescription, QUuid requestId)
{
    if (requestId != m_genericResourceImageWriterRequestId) {
        return;
    }

    QNDEBUG("RenameResourceDelegate::onGenericResourceImageWriterFinished: success = " << (success ? "true" : "false")
            << ", resource hash = " << resourceHash << ", file path = " << filePath << ", error description = "
            << errorDescription << ", request id = " << requestId);

    QObject::disconnect(this, QNSIGNAL(RenameResourceDelegate,saveGenericResourceImageToFile,QString,QByteArray,QString,QByteArray,QString,QUuid),
                        m_pGenericResourceImageWriter, QNSLOT(GenericResourceImageWriter,onGenericResourceImageWriteRequest,QString,QByteArray,QString,QByteArray,QString,QUuid));
    QObject::disconnect(m_pGenericResourceImageWriter, QNSIGNAL(GenericResourceImageWriter,genericResourceImageWriteReply,bool,QByteArray,QString,QString,QUuid),
                        this, QNSLOT(RenameResourceDelegate,onGenericResourceImageWriterFinished,bool,QByteArray,QString,QString,QUuid));

    if (Q_UNLIKELY(!success)) {
        errorDescription = QT_TR_NOOP("Can't rename generic resource: can't write generic resource image to file") + QString(": ") + errorDescription;
        QNWARNING(errorDescription);
        emit notifyError(errorDescription);
        return;
    }

    m_newGenericResourceImageFilePath = filePath;

    QString javascript = "updateImageResourceSrc('" + QString::fromLocal8Bit(resourceHash) + "', '" + filePath + "');";

    GET_PAGE()
    page->executeJavaScript(javascript, JsCallback(*this, &RenameResourceDelegate::onGenericResourceImageUpdated));
}

void RenameResourceDelegate::onGenericResourceImageUpdated(const QVariant & data)
{
    QNDEBUG("RenameResourceDelegate::onGenericResourceImageUpdated");

    Q_UNUSED(data)

    emit finished(m_oldResourceName, m_newResourceName, m_resource, m_performingUndo, m_newGenericResourceImageFilePath);
}

#endif

} // namespace qute_note

