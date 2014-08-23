#ifndef __QUTE_NOTE__CORE__CLIENT__LOCAL_STORAGE__TRANSACTION_H
#define __QUTE_NOTE__CORE__CLIENT__LOCAL_STORAGE__TRANSACTION_H

#include <tools/Linkage.h>
#include <QSqlDatabase>

namespace qute_note {

class QUTE_NOTE_EXPORT Transaction
{
public:
    enum TransactionType {
        Default,
        Immediate,
        Exclusive
    };

    Transaction(QSqlDatabase & db, TransactionType type = Default);
    Transaction(Transaction && other);
    virtual ~Transaction();

    bool commit(QString & errorDescription);

private:
    Transaction() = delete;
    Transaction(const Transaction & other) = delete;
    Transaction & operator=(const Transaction & other) = delete;
    Transaction & operator=(Transaction && other) = delete;

    void init();

    QSqlDatabase & m_db;
    TransactionType m_type;
    bool m_committed;
};

} // namespace qute_note

#endif // __QUTE_NOTE__CORE__CLIENT__LOCAL_STORAGE__TRANSACTION_H
