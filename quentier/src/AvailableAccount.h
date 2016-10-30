#ifndef QUENTIER_AVAILABLE_ACCOUNT_H
#define QUENTIER_AVAILABLE_ACCOUNT_H

#include <QString>

/**
 * @brief The AvailableAccount class is used as a container for a few bits of information
 * related to one of available accounts i.e. one of those to which the user can switch
 */
class AvailableAccount
{
public:
    explicit AvailableAccount();
    explicit AvailableAccount(const QString & username,
                              const QString & userPersistenceStoragePath,
                              const bool isLocal);

    const QString & username() const { return m_username; }
    void setUsername(const QString & username) { m_username = username; }

    const QString & userPersistenceStoragePath() const { return m_userPersistenceStoragePath; }
    void setUserPersistenceStoragePath(const QString & userPersistenceStoragePath)
    { m_userPersistenceStoragePath = userPersistenceStoragePath; }

    bool isLocal() const { return m_isLocal; }
    void setLocal(const bool local) { m_isLocal = local; }

private:
    QString     m_username;
    QString     m_userPersistenceStoragePath;
    bool        m_isLocal;
};

#endif // QUENTIER_AVAILABLE_ACCOUNT_H
