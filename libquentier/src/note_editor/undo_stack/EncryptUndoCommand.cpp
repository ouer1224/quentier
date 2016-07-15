#include "EncryptUndoCommand.h"
#include "../NoteEditor_p.h"
#include <quentier/logging/QuentierLogger.h>

namespace quentier {

#define GET_PAGE() \
    NoteEditorPage * page = qobject_cast<NoteEditorPage*>(m_noteEditorPrivate.page()); \
    if (Q_UNLIKELY(!page)) { \
        QNLocalizedString error = QT_TR_NOOP("can't undo/redo the text encryption: can't get note editor page"); \
        QNWARNING(error); \
        emit notifyError(error); \
        return; \
    }

EncryptUndoCommand::EncryptUndoCommand(NoteEditorPrivate & noteEditorPrivate, const Callback & callback, QUndoCommand * parent) :
    INoteEditorUndoCommand(noteEditorPrivate, parent),
    m_callback(callback)
{
    setText(tr("Encrypt selected text"));
}

EncryptUndoCommand:: EncryptUndoCommand(NoteEditorPrivate & noteEditorPrivate, const Callback & callback,
                                        const QString & text, QUndoCommand * parent) :
    INoteEditorUndoCommand(noteEditorPrivate, text, parent),
    m_callback(callback)
{}

EncryptUndoCommand::~EncryptUndoCommand()
{}

void EncryptUndoCommand::redoImpl()
{
    QNDEBUG("EncryptUndoCommand::redoImpl");

    GET_PAGE()
    page->executeJavaScript("encryptDecryptManager.redo();", m_callback);
}

void EncryptUndoCommand::undoImpl()
{
    QNDEBUG("EncryptUndoCommand::undoImpl");

    GET_PAGE()
    page->executeJavaScript("encryptDecryptManager.undo();", m_callback);
}

} // namespace quentier
