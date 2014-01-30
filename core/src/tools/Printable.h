#ifndef __QUTE_NOTE__TOOLS_PRINTABLE_H
#define __QUTE_NOTE__TOOLS_PRINTABLE_H

#include <QString>
#include <QTextStream>

namespace qute_note {

/**
 * @brief The Printable class is the interface for QuteNote's internal classes
 * which should be able to write themselves into QTextStream and/or convert to QString
 */
class Printable {
public:
    virtual ~Printable();

    virtual QTextStream & Print(QTextStream & strm) const = 0;

    virtual const QString ToQString() const;

    friend QTextStream & operator << (QTextStream & strm,
                                      const Printable & printable);
};

} // namespace qute_note

// QTextStream operators for existing classes not inheriting from Printable

namespace evernote {
namespace edam {

QT_FORWARD_DECLARE_CLASS(BusinessUserInfo)
QT_FORWARD_DECLARE_CLASS(PremiumInfo)
QT_FORWARD_DECLARE_CLASS(Accounting)

}
}

QTextStream & operator << (QTextStream & strm, const evernote::edam::BusinessUserInfo & info);
QTextStream & operator << (QTextStream & strm, const evernote::edam::PremiumInfo & info);
QTextStream & operator << (QTextStream & strm, const evernote::edam::Accounting & accounting);

template<class T>
const QString ToQString(const T & object)
{
    QString str;
    QTextStream strm(&str, QIODevice::WriteOnly);
    strm << object;
    return std::move(str);
}

#endif // __QUTE_NOTE__TOOLS_PRINTABLE_H
