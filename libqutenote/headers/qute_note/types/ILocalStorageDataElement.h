#ifndef __LIB_QUTE_NOTE__TYPES__I_LOCAL_STORAGE_DATA_ELEMENT_H
#define __LIB_QUTE_NOTE__TYPES__I_LOCAL_STORAGE_DATA_ELEMENT_H

#include <qute_note/utility/Linkage.h>
#include <qute_note/utility/Qt4Helper.h>
#include <QString>
#include <QUuid>

namespace qute_note {

class QUTE_NOTE_EXPORT ILocalStorageDataElement
{
public:
    virtual const QString localUid() const = 0;
    virtual void setLocalUid(const QString & guid) = 0;
    virtual void unsetLocalUid() = 0;

    virtual ~ILocalStorageDataElement() {}
};

#define _DEFINE_LOCAL_UID_GETTER(type) \
    const QString type::localUid() const { \
        if (d->m_localUid.isNull()) { \
            return QString(); \
        } \
        else { \
            QString result = d->m_localUid.toString(); \
            result.remove(0,1); \
            result.remove(result.size()-1,1); \
            return result; \
        } \
    }

#define _DEFINE_LOCAL_UID_SETTER(type) \
    void type::setLocalUid(const QString & guid) { \
        d->m_localUid = guid; \
    }

#define _DEFINE_LOCAL_UID_UNSETTER(type) \
    void type::unsetLocalUid() { \
        d->m_localUid = QUuid(); \
    }

#define QN_DECLARE_LOCAL_UID \
    virtual const QString localUid() const Q_DECL_OVERRIDE; \
    virtual void setLocalUid(const QString & guid) Q_DECL_OVERRIDE; \
    virtual void unsetLocalUid() Q_DECL_OVERRIDE;

#define QN_DEFINE_LOCAL_UID(type) \
    _DEFINE_LOCAL_UID_GETTER(type) \
    _DEFINE_LOCAL_UID_SETTER(type) \
    _DEFINE_LOCAL_UID_UNSETTER(type)

} // namespace qute_note

#endif // __LIB_QUTE_NOTE__TYPES__I_LOCAL_STORAGE_DATA_ELEMENT_H
