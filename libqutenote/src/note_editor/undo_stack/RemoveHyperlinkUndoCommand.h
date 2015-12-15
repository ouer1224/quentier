#ifndef __LIB_QUTE_NOTE__NOTE_EDITOR__UNDO_STACK__REMOVE_HYPERLINK_UNDO_COMMAND_H
#define __LIB_QUTE_NOTE__NOTE_EDITOR__UNDO_STACK__REMOVE_HYPERLINK_UNDO_COMMAND_H

#include "INoteEditorUndoCommand.h"

namespace qute_note {

class RemoveHyperlinkUndoCommand: public INoteEditorUndoCommand
{
public:
    RemoveHyperlinkUndoCommand(const quint64 removedHyperlinkId, NoteEditorPrivate & noteEditor,
                               QUndoCommand * parent = Q_NULLPTR);
    RemoveHyperlinkUndoCommand(const quint64 removedHyperlinkId, NoteEditorPrivate & noteEditor,
                               const QString & text, QUndoCommand * parent = Q_NULLPTR);
    virtual ~RemoveHyperlinkUndoCommand();

    virtual void redoImpl() Q_DECL_OVERRIDE;
    virtual void undoImpl() Q_DECL_OVERRIDE;

private:
    quint64     m_hyperlinkId;
};

} // namespace qute_note

#endif // __LIB_QUTE_NOTE__NOTE_EDITOR__UNDO_STACK__REMOVE_HYPERLINK_UNDO_COMMAND_H