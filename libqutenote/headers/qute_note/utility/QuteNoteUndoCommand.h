#ifndef __LIB_QUTE_NOTE__UTILITY__QUTE_NOTE_UNDO_COMMAND_H
#define __LIB_QUTE_NOTE__UTILITY__QUTE_NOTE_UNDO_COMMAND_H

#include <qute_note/utility/Qt4Helper.h>
#include <QUndoCommand>

namespace qute_note {

/**
 * @brief The QuteNoteUndoCommand class has the sole purpose of working around
 * one quirky aspect of Qt's undo/redo framework: when you push QUndoCommand
 * to QUndoStack, it calls "redo" method of that command. This class offers
 * subclasses to implement their own methods for actual "undo" and "redo" commands while
 * ignoring the attempts to "redo" anything if there were no previous "undo" call
 * prior to that.
 *
 * The rationale behind the current behaviour seems to be the compliance with
 * "command pattern behaviour" when you create the command to execute the action
 * instead of just executing it immediately. This design is enforced by Qt's undo/redo
 * framework, there's no option to choose not to call "redo" when pushing to the stack.
 *
 * One thing which this design fails to see is the fact that the command may be
 * already executed externally by the moment the QUndoCommand
 * can be created. Suppose we can get the information about how to undo (and then again redo)
 * that command. We create the corresponding QUndoCommand, set up the stuff for its
 * undo/redo methods and push it to QUndoStack for future use... but at the same time
 * QUndoStack calls "redo" method of the command. Really not the behaviour you'd like to have.
 */
class QuteNoteUndoCommand: public QUndoCommand
{
public:
    QuteNoteUndoCommand(QUndoCommand * parent = Q_NULLPTR);
    QuteNoteUndoCommand(const QString & text, QUndoCommand * parent = Q_NULLPTR);
    virtual ~QuteNoteUndoCommand();

    virtual void undo() Q_DECL_OVERRIDE Q_DECL_FINAL;
    virtual void redo() Q_DECL_OVERRIDE Q_DECL_FINAL;

    bool onceUndoExecuted() const { return m_onceUndoExecuted; }

protected:
    virtual void undoImpl() = 0;
    virtual void redoImpl() = 0;

private:
    bool    m_onceUndoExecuted;
};

} // namespace qute_note

#endif // __LIB_QUTE_NOTE__UTILITY__QUTE_NOTE_UNDO_COMMAND_H
