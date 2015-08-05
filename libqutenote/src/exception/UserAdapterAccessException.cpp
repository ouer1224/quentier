#include <qute_note/exception/UserAdapterAccessException.h>

namespace qute_note {

UserAdapterAccessException::UserAdapterAccessException(const QString & message) :
    IQuteNoteException(message)
{}

const QString UserAdapterAccessException::exceptionDisplayName() const
{
    return QString("UserAdapterAccessException");
}

} // namespace qute_note