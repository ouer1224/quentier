/*
 * Copyright 2016-2019 Dmitry Ivanov
 *
 * This file is part of Quentier.
 *
 * Quentier is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * Quentier is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Quentier. If not, see <http://www.gnu.org/licenses/>.
 */

#include "AccountManager.h"
#include "AddAccountDialog.h"
#include "ManageAccountsDialog.h"
#include "AccountModel.h"

#include <lib/preferences/SettingsNames.h>

#include <quentier/utility/ApplicationSettings.h>
#include <quentier/utility/StandardPaths.h>
#include <quentier/utility/Utility.h>
#include <quentier/logging/QuentierLogger.h>

#include <QXmlStreamWriter>

#include <boost/scope_exit.hpp>

namespace quentier {

AccountManager::AccountManager(QObject * parent) :
    QObject(parent),
    m_pAccountModel(new AccountModel(this))
{
    QObject::connect(m_pAccountModel.data(),
                     QNSIGNAL(AccountModel,accountAdded,Account),
                     this,
                     QNSIGNAL(AccountManager,accountAdded,Account));
    QObject::connect(m_pAccountModel.data(),
                     QNSIGNAL(AccountModel,accountDisplayNameChanged,Account),
                     this,
                     QNSLOT(AccountManager,onAccountDisplayNameChanged,Account));
    QObject::connect(m_pAccountModel.data(),
                     QNSIGNAL(AccountModel,accountRemoved,Account),
                     this,
                     QNSIGNAL(AccountManager,accountRemoved,Account));

    detectAvailableAccounts();
}

AccountManager::~AccountManager()
{}

const QVector<Account> & AccountManager::availableAccounts() const
{
    return m_pAccountModel->accounts();
}

AccountModel & AccountManager::accountModel()
{
    return *m_pAccountModel;
}

Account AccountManager::currentAccount()
{
    QNDEBUG("AccountManager::currentAccount");

    Account account = accountFromEnvVarHints();
    if (!account.isEmpty()) {
        return account;
    }

    account = lastUsedAccount();
    if (account.isEmpty())
    {
        ErrorString errorDescription;
        account = createDefaultAccount(errorDescription);
        if (Q_UNLIKELY(account.isEmpty())) {
            ErrorString error(QT_TR_NOOP("Can't initialize the default account"));
            error.appendBase(errorDescription.base());
            error.appendBase(errorDescription.additionalBases());
            error.details() = errorDescription.details();
            throw AccountInitializationException(error);
        }

        updateLastUsedAccount(account);
    }

    return account;
}

int AccountManager::execAddAccountDialog()
{
    QNDEBUG("AccountManager::execAddAccountDialog");

    QWidget * parentWidget = qobject_cast<QWidget*>(parent());

    QScopedPointer<AddAccountDialog> addAccountDialog(
        new AddAccountDialog(m_pAccountModel->accounts(), parentWidget));

    addAccountDialog->setWindowModality(Qt::WindowModal);
    QObject::connect(addAccountDialog.data(),
                     QNSIGNAL(AddAccountDialog,evernoteAccountAdditionRequested,
                              QString,QNetworkProxy),
                     this,
                     QNSIGNAL(AccountManager,evernoteAccountAuthenticationRequested,
                              QString,QNetworkProxy));
    QObject::connect(addAccountDialog.data(),
                     QNSIGNAL(AddAccountDialog,localAccountAdditionRequested,
                              QString,QString),
                     this,
                     QNSLOT(AccountManager,onLocalAccountAdditionRequested,
                            QString,QString));
    return addAccountDialog->exec();
}

int AccountManager::execManageAccountsDialog()
{
    QNDEBUG("AccountManager::execManageAccountsDialog");

    QWidget * parentWidget = qobject_cast<QWidget*>(parent());
    const QVector<Account> & availableAccounts = m_pAccountModel->accounts();

    Account account = currentAccount();
    int currentAccountRow = availableAccounts.indexOf(account);

    QScopedPointer<ManageAccountsDialog> pManageAccountsDialog(
        new ManageAccountsDialog(*this, currentAccountRow, parentWidget));

    pManageAccountsDialog->setWindowModality(Qt::WindowModal);
    QObject::connect(pManageAccountsDialog.data(),
                     QNSIGNAL(ManageAccountsDialog,evernoteAccountAdditionRequested,
                              QString,QNetworkProxy),
                     this,
                     QNSIGNAL(AccountManager,evernoteAccountAuthenticationRequested,
                              QString,QNetworkProxy));
    QObject::connect(pManageAccountsDialog.data(),
                     QNSIGNAL(ManageAccountsDialog,localAccountAdditionRequested,
                              QString,QString),
                     this,
                     QNSLOT(AccountManager,onLocalAccountAdditionRequested,
                            QString,QString));
    QObject::connect(pManageAccountsDialog.data(),
                     QNSIGNAL(ManageAccountsDialog,revokeAuthentication,qevercloud::UserID),
                     this,
                     QNSIGNAL(AccountManager,revokeAuthentication,qevercloud::UserID));
    return pManageAccountsDialog->exec();
}

Account AccountManager::createNewLocalAccount(QString name)
{
    if (!name.isEmpty())
    {
        const QVector<Account> & availableAccounts = m_pAccountModel->accounts();
        for(auto it = availableAccounts.constBegin(),
            end = availableAccounts.constEnd(); it != end; ++it)
        {
            const Account & currentAccount = *it;
            if (currentAccount.type() != Account::Type::Local) {
                continue;
            }

            if (name.compare(currentAccount.name(), Qt::CaseInsensitive) == 0) {
                return Account();
            }
        }
    }
    else
    {
        QString baseName = QStringLiteral("Local account");
        int suffix = 0;
        name = baseName;
        const QVector<Account> & availableAccounts = m_pAccountModel->accounts();
        while(true)
        {
            bool nameClashDetected = false;
            for(auto it = availableAccounts.constBegin(),
                end = availableAccounts.constEnd(); it != end; ++it)
            {
                const Account & currentAccount = *it;
                if (currentAccount.type() != Account::Type::Local) {
                    continue;
                }

                if (name.compare(currentAccount.name(), Qt::CaseInsensitive) == 0) {
                    nameClashDetected = true;
                    ++suffix;
                    name = baseName + QStringLiteral("_") + QString::number(suffix);
                    break;
                }
            }

            if (!nameClashDetected) {
                break;
            }
        }
    }

    ErrorString errorDescription;
    Account account = createLocalAccount(name, name, errorDescription);
    if (Q_UNLIKELY(account.isEmpty())) {
        ErrorString error(QT_TR_NOOP("Can't create a new local account"));
        error.appendBase(errorDescription.base());
        error.appendBase(errorDescription.additionalBases());
        error.details() = errorDescription.details();
        QNWARNING(error);
        Q_EMIT notifyError(error);
        return Account();
    }

    return account;
}

void AccountManager::switchAccount(const Account & account)
{
    // print the entire account because here not only the name but also the type
    // of the account we are switching to matters a lot
    QNDEBUG("AccountManager::switchAccount: " << account);

    // See whether this account is within a list of already available accounts,
    // if not, add it there
    bool accountIsAvailable = false;
    Account::Type::type type = account.type();
    bool isLocal = (account.type() == Account::Type::Local);
    const QVector<Account> & availableAccounts = m_pAccountModel->accounts();
    for(auto it = availableAccounts.constBegin(),
        end = availableAccounts.constEnd(); it != end; ++it)
    {
        const Account & availableAccount = *it;

        if (availableAccount.type() != type) {
            continue;
        }

        if (!isLocal && (availableAccount.id() != account.id())) {
            continue;
        }
        else if (isLocal && (availableAccount.name() != account.name())) {
            continue;
        }

        accountIsAvailable = true;
        break;
    }

    Account complementedAccount = account;
    if (!accountIsAvailable)
    {
        bool res = createAccountInfo(account);
        if (!res) {
            return;
        }

        if (m_pAccountModel->addAccount(account)) {
            Q_EMIT accountAdded(account);
        }
    }
    else
    {
        readComplementaryAccountInfo(complementedAccount);
    }

    updateLastUsedAccount(complementedAccount);
    Q_EMIT switchedAccount(complementedAccount);
}

void AccountManager::onLocalAccountAdditionRequested(QString name,
                                                     QString fullName)
{
    QNDEBUG("AccountManager::onLocalAccountAdditionRequested: "
            << "name = " << name << ", full name = " << fullName);

    // Double-check that no local account with such name already exists
    const QVector<Account> & availableAccounts = m_pAccountModel->accounts();
    for(auto it = availableAccounts.constBegin(),
        end = availableAccounts.constEnd(); it != end; ++it)
    {
        const Account & availableAccount = *it;
        if (availableAccount.type() != Account::Type::Local) {
            continue;
        }

        if (Q_UNLIKELY(availableAccount.name() == name)) {
            ErrorString error(QT_TR_NOOP("Can't add a local account: another "
                                         "account with the same name already "
                                         "exists"));
            QNWARNING(error);
            Q_EMIT notifyError(error);
            return;
        }
    }

    ErrorString errorDescription;
    Account account = createLocalAccount(name, fullName, errorDescription);
    if (Q_UNLIKELY(account.isEmpty())) {
        ErrorString error(QT_TR_NOOP("Can't create a new local account"));
        error.appendBase(errorDescription.base());
        error.appendBase(errorDescription.additionalBases());
        error.details() = errorDescription.details();
        QNWARNING(error);
        Q_EMIT notifyError(error);
        return;
    }

    switchAccount(account);
}

void AccountManager::onAccountDisplayNameChanged(Account account)
{
    QNDEBUG("AccountManager::onAccountDisplayNameChanged: " << account.name());

    bool isLocal = (account.type() ==  Account::Type::Local);
    ErrorString errorDescription;
    QString accountType = evernoteAccountTypeToString(account.evernoteAccountType());
    Q_UNUSED(writeAccountInfo(account.name(), account.displayName(), isLocal,
                              account.id(), accountType, account.evernoteHost(),
                              account.shardId(), errorDescription))

    Q_EMIT accountUpdated(account);
}

void AccountManager::detectAvailableAccounts()
{
    QNDEBUG("AccountManager::detectAvailableAccounts");

    QString appPersistenceStoragePath = applicationPersistentStoragePath();

    QString localAccountsStoragePath = appPersistenceStoragePath +
                                       QStringLiteral("/LocalAccounts");
    QDir localAccountsStorageDir(localAccountsStoragePath);
    QFileInfoList localAccountsDirInfos =
        localAccountsStorageDir.entryInfoList(
            QDir::Filters(QDir::AllDirs | QDir::NoDotAndDotDot));

    QString evernoteAccountsStoragePath = appPersistenceStoragePath +
                                          QStringLiteral("/EvernoteAccounts");
    QDir evernoteAccountsStorageDir(evernoteAccountsStoragePath);
    QFileInfoList evernoteAccountsDirInfos =
        evernoteAccountsStorageDir.entryInfoList(
            QDir::Filters(QDir::AllDirs | QDir::NoDotAndDotDot));

    int numPotentialLocalAccountDirs = localAccountsDirInfos.size();
    int numPotentialEvernoteAccountDirs = evernoteAccountsDirInfos.size();
    int numPotentialAccountDirs = numPotentialLocalAccountDirs +
                                  numPotentialEvernoteAccountDirs;

    QVector<Account> availableAccounts;
    availableAccounts.reserve(numPotentialAccountDirs);

    for(int i = 0; i < numPotentialLocalAccountDirs; ++i)
    {
        const QFileInfo & accountDirInfo = localAccountsDirInfos[i];
        QNTRACE("Examining potential local account dir: "
                << accountDirInfo.absoluteFilePath());

        QFileInfo accountBasicInfoFileInfo(accountDirInfo.absoluteFilePath() +
                                           QStringLiteral("/accountInfo.txt"));
        if (!accountBasicInfoFileInfo.exists()) {
            QNTRACE("Found no accountInfo.txt file in this dir, skipping it");
            continue;
        }

        QString accountName = accountDirInfo.baseName();
        qevercloud::UserID userId = -1;

        Account availableAccount(accountName, Account::Type::Local, userId);
        readComplementaryAccountInfo(availableAccount);
        availableAccounts << availableAccount;
        QNDEBUG("Found available local account: name = "
                << accountName << ", dir "
                << accountDirInfo.absoluteFilePath());
    }

    for(int i = 0; i < numPotentialEvernoteAccountDirs; ++i)
    {
        const QFileInfo & accountDirInfo = evernoteAccountsDirInfos[i];
        QNTRACE("Examining potential Evernote account dir: "
                << accountDirInfo.absoluteFilePath());

        QFileInfo accountBasicInfoFileInfo(accountDirInfo.absoluteFilePath() +
                                           QStringLiteral("/accountInfo.txt"));
        if (!accountBasicInfoFileInfo.exists()) {
            QNTRACE("Found no accountInfo.txt file in this dir, skipping it");
            continue;
        }

        QString accountName = accountDirInfo.fileName();
        qevercloud::UserID userId = -1;

        // The account dir for Evernote accounts is encoded
        // as "<account_name>_<host>_<user_id>"
        int accountNameSize = accountName.size();
        int lastUnderlineIndex = accountName.lastIndexOf(QStringLiteral("_"));
        if ((lastUnderlineIndex < 0) || (lastUnderlineIndex >= accountNameSize))
        {
            QNTRACE("Dir " << accountName
                    << " doesn't seem to be an account dir: it "
                    << "doesn't start with \"local_\" and "
                    << "doesn't contain \"_<user id>\" at the end");
            continue;
        }

        QStringRef userIdStrRef =
            accountName.rightRef(accountNameSize - lastUnderlineIndex - 1);
        bool conversionResult = false;
        userId =
            static_cast<qevercloud::UserID>(userIdStrRef.toInt(&conversionResult));
        if (Q_UNLIKELY(!conversionResult))
        {
            QNTRACE("Skipping dir " << accountName
                    << " as it doesn't seem to end with user id, "
                    << "the attempt to convert it to int fails");
            continue;
        }

        int preLastUnderlineIndex =
            accountName.lastIndexOf(QStringLiteral("_"),
                                    std::max(lastUnderlineIndex - 1, 1));
        if ((preLastUnderlineIndex < 0) ||
            (preLastUnderlineIndex >= lastUnderlineIndex))
        {
            QNTRACE("Dir " << accountName
                    << " doesn't seem to be an account dir: it "
                    << "doesn't start with \"local_\" and doesn't "
                    << "contain \"_<host>_<user id>\" at the end");
            continue;
        }

        QString evernoteHost =
            accountName.mid(preLastUnderlineIndex + 1,
                            lastUnderlineIndex - preLastUnderlineIndex - 1);
        accountName.remove(preLastUnderlineIndex,
                           accountNameSize - preLastUnderlineIndex);

        Account availableAccount(accountName, Account::Type::Evernote, userId,
                                 Account::EvernoteAccountType::Free, evernoteHost);
        readComplementaryAccountInfo(availableAccount);
        availableAccounts << availableAccount;
        QNDEBUG("Found available Evernote account: name = "
                << accountName << ", user id = " << userId
                << ", Evernote account type = "
                << availableAccount.evernoteAccountType()
                << ", Evernote host = "
                << availableAccount.evernoteHost() << ", dir "
                << accountDirInfo.absoluteFilePath());
    }

    m_pAccountModel->setAccounts(availableAccounts);
}

Account AccountManager::createDefaultAccount(ErrorString & errorDescription)
{
    QNDEBUG("AccountManager::createDefaultAccount");

    QString username = getCurrentUserName();
    if (Q_UNLIKELY(username.isEmpty())) {
        QNDEBUG("Couldn't get the current user's name, fallback "
                "to \"Default user\"");
        username = QStringLiteral("Default user");
    }

    QString fullName = getCurrentUserFullName();
    if (Q_UNLIKELY(fullName.isEmpty())) {
        QNDEBUG("Couldn't get the current user's full name, "
                "fallback to \"Defaulr user\"");
        fullName = QStringLiteral("Default user");
    }

    // Need to check whether the default account already exists
    Account account(username, Account::Type::Local, qevercloud::UserID(-1));
    account.setDisplayName(fullName);

    const QVector<Account> & availableAccounts = m_pAccountModel->accounts();
    if (availableAccounts.contains(account)) {
        QNDEBUG("The default account already exists");
        return account;
    }

    return createLocalAccount(username, fullName, errorDescription);
}

Account AccountManager::createLocalAccount(
    const QString & name, const QString & displayName,
    ErrorString & errorDescription)
{
    QNDEBUG("AccountManager::createLocalAccount: name = "
            << name << ", display name = " << displayName);

    bool res = writeAccountInfo(name, displayName, /* is local = */ true,
                                /* user id = */ -1,
                                /* Evernote account type = */ QString(),
                                /* Evernote host = */ QString(),
                                /* shard id = */ QString(), errorDescription);
    if (Q_UNLIKELY(!res)) {
        return Account();
    }

    Account availableAccount(name, Account::Type::Local, qevercloud::UserID(-1));
    availableAccount.setDisplayName(displayName);
    m_pAccountModel->addAccount(availableAccount);

    return Account(name, Account::Type::Local);
}

bool AccountManager::createAccountInfo(const Account & account)
{
    QNDEBUG("AccountManager::createAccountInfo: " << account.name());

    bool isLocal = (account.type() == Account::Type::Local);

    QString evernoteAccountType =
        evernoteAccountTypeToString(account.evernoteAccountType());

    ErrorString errorDescription;
    bool res = writeAccountInfo(account.name(), account.displayName(), isLocal,
                                account.id(), evernoteAccountType,
                                account.evernoteHost(), account.shardId(),
                                errorDescription);
    if (Q_UNLIKELY(!res)) {
        Q_EMIT notifyError(errorDescription);
        return false;
    }

    return true;
}

bool AccountManager::writeAccountInfo(const QString & name,
                                      const QString & displayName,
                                      const bool isLocal,
                                      const qevercloud::UserID id,
                                      const QString & evernoteAccountType,
                                      const QString & evernoteHost,
                                      const QString & shardId,
                                      ErrorString & errorDescription)
{
    QNDEBUG("AccountManager::writeAccountInfo: name = " << name
            << ", display name = " << displayName
            << ", is local = " << (isLocal ? "true" : "false")
            << ", user id = " << id
            << ", Evernote account type = "
            << evernoteAccountType << ", Evernote host = "
            << evernoteHost << ", shard id = " << shardId);

    Account account(name, (isLocal ? Account::Type::Local : Account::Type::Evernote),
                    id, Account::EvernoteAccountType::Free, evernoteHost, shardId);

    QDir accountPersistentStorageDir(accountPersistentStoragePath(account));
    if (!accountPersistentStorageDir.exists())
    {
        bool res = accountPersistentStorageDir.mkpath(
            accountPersistentStorageDir.absolutePath());

        if (Q_UNLIKELY(!res)) {
            errorDescription.setBase(QT_TR_NOOP("Can't create a directory for "
                                                "the account storage"));
            errorDescription.details() = accountPersistentStorageDir.absolutePath();
            QNWARNING(errorDescription);
            return false;
        }
    }

    QFile accountInfo(accountPersistentStorageDir.absolutePath() +
                      QStringLiteral("/accountInfo.txt"));

    bool open = accountInfo.open(QIODevice::WriteOnly);
    if (Q_UNLIKELY(!open))
    {
        errorDescription.setBase(QT_TR_NOOP("Can't open the new account info "
                                            "file for writing"));
        errorDescription.details() = accountInfo.fileName();

        QString errorString = accountInfo.errorString();
        if (!errorString.isEmpty()) {
            errorDescription.details() += QStringLiteral(": ");
            errorDescription.details() += errorString;
        }

        QNWARNING(errorDescription);
        return false;
    }

    QXmlStreamWriter writer(&accountInfo);

    writer.writeStartDocument();

    writer.writeStartElement(QStringLiteral("data"));

    if (!name.isEmpty()) {
        writer.writeStartElement(QStringLiteral("accountName"));
        writer.writeCharacters(name);
        writer.writeEndElement();
    }

    if (!displayName.isEmpty()) {
        writer.writeStartElement(QStringLiteral("displayName"));
        writer.writeCDATA(displayName);
        writer.writeEndElement();
    }

    writer.writeStartElement(QStringLiteral("accountType"));
    writer.writeCharacters(isLocal
                           ? QStringLiteral("Local")
                           : QStringLiteral("Evernote"));
    writer.writeEndElement();

    if (!evernoteAccountType.isEmpty()) {
        writer.writeStartElement(QStringLiteral("evernoteAccountType"));
        writer.writeCharacters(evernoteAccountType);
        writer.writeEndElement();
    }

    if (!evernoteHost.isEmpty()) {
        writer.writeStartElement(QStringLiteral("evernoteHost"));
        writer.writeCharacters(evernoteHost);
        writer.writeEndElement();
    }

    if (!shardId.isEmpty()) {
        writer.writeStartElement(QStringLiteral("shardId"));
        writer.writeCharacters(shardId);
        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    accountInfo.close();
    return true;
}

QString AccountManager::evernoteAccountTypeToString(
    const Account::EvernoteAccountType::type type) const
{
    QString evernoteAccountType;
    switch(type)
    {
    case Account::EvernoteAccountType::Plus:
        evernoteAccountType = QStringLiteral("Plus");
        break;
    case Account::EvernoteAccountType::Premium:
        evernoteAccountType = QStringLiteral("Premium");
        break;
    case Account::EvernoteAccountType::Business:
        evernoteAccountType = QStringLiteral("Business");
        break;
    default:
        evernoteAccountType = QStringLiteral("Free");
        break;
    }

    return evernoteAccountType;
}

void AccountManager::readComplementaryAccountInfo(Account & account)
{
    QNTRACE("AccountManager::readComplementaryAccountInfo: " << account.name());

    if (Q_UNLIKELY(account.isEmpty())) {
        QNDEBUG("The account is empty: " << account.name());
        return;
    }

    if (Q_UNLIKELY(account.name().isEmpty())) {
        QNDEBUG("The account name is empty: " << account.name());
        return;
    }

    QDir accountPersistentStorageDir(accountPersistentStoragePath(account));
    if (!accountPersistentStorageDir.exists()) {
        QNDEBUG("No persistent storage dir exists for this account: "
                << account.name());
        return;
    }

    QFile accountInfo(accountPersistentStorageDir.absolutePath() +
                      QStringLiteral("/accountInfo.txt"));

    bool open = accountInfo.open(QIODevice::ReadOnly);
    if (Q_UNLIKELY(!open))
    {
        ErrorString errorDescription(QT_TR_NOOP("Can't read the complementary "
                                                "account info: can't open file "
                                                "for reading"));
        errorDescription.details() = accountInfo.fileName();

        QString errorString = accountInfo.errorString();
        if (!errorString.isEmpty()) {
            errorDescription.details() += QStringLiteral(": ");
            errorDescription.details() += errorString;
        }

        QNWARNING(errorDescription);
        Q_EMIT notifyError(errorDescription);
        return;
    }

    QXmlStreamReader reader(&accountInfo);

    QString currentElementName;
    while(!reader.atEnd())
    {
        Q_UNUSED(reader.readNext());

        if (reader.isStartDocument()) {
            continue;
        }

        if (reader.isDTD()) {
            continue;
        }

        if (reader.isEndDocument()) {
            continue;
        }

        if (reader.isStartElement()) {
            currentElementName = reader.name().toString();
            continue;
        }

        if (reader.isCharacters())
        {
            if (currentElementName.isEmpty()) {
                continue;
            }

            if (currentElementName == QStringLiteral("evernoteAccountType"))
            {
                QString evernoteAccountType = reader.text().toString();
                if (evernoteAccountType.isEmpty() ||
                    (evernoteAccountType == QStringLiteral("Free")))
                {
                    account.setEvernoteAccountType(Account::EvernoteAccountType::Free);
                }
                else if (evernoteAccountType == QStringLiteral("Plus"))
                {
                    account.setEvernoteAccountType(Account::EvernoteAccountType::Plus);
                }
                else if (evernoteAccountType == QStringLiteral("Premium"))
                {
                    account.setEvernoteAccountType(Account::EvernoteAccountType::Premium);
                }
                else if (evernoteAccountType == QStringLiteral("Business"))
                {
                    account.setEvernoteAccountType(Account::EvernoteAccountType::Business);
                }
            }
            else if (currentElementName == QStringLiteral("evernoteHost"))
            {
                account.setEvernoteHost(reader.text().toString());
            }
            else if (currentElementName == QStringLiteral("shardId"))
            {
                account.setShardId(reader.text().toString());
            }
        }

        if (reader.isCDATA() &&
            (currentElementName == QStringLiteral("displayName")))
        {
            account.setDisplayName(reader.text().toString());
        }
    }

    if (reader.hasError())
    {
        ErrorString errorDescription(QT_TR_NOOP("Can't read the entire "
                                                "complementary account info, "
                                                "error reading XML"));
        errorDescription.details() = reader.errorString();
        QNWARNING(errorDescription);
        Q_EMIT notifyError(errorDescription);
    }

    accountInfo.close();

    QNTRACE("Account after reading in the complementary info: " << account);
}

Account AccountManager::accountFromEnvVarHints()
{
    QNDEBUG("AccountManager::accountFromEnvVarHints");

    if (qEnvironmentVariableIsEmpty(ACCOUNT_NAME_ENV_VAR)) {
        QNDEBUG("Account name environment variable is not set or is empty");
        return Account();
    }

    if (qEnvironmentVariableIsEmpty(ACCOUNT_TYPE_ENV_VAR)) {
        QNDEBUG("Account type environment variable is not set or is empty");
        return Account();
    }

    QByteArray accountType = qgetenv(ACCOUNT_TYPE_ENV_VAR);
    bool isLocal = (accountType == QByteArray("1"));

    if (!isLocal)
    {
        if (qEnvironmentVariableIsEmpty(ACCOUNT_ID_ENV_VAR)) {
            QNDEBUG("Account id environment variable is not set or is empty");
            return Account();
        }

        if (qEnvironmentVariableIsEmpty(ACCOUNT_EVERNOTE_ACCOUNT_TYPE_ENV_VAR)) {
            QNDEBUG("Evernote account type environment variable is not set or "
                    "is empty");
            return Account();
        }

        if (qEnvironmentVariableIsEmpty(ACCOUNT_EVERNOTE_HOST_ENV_VAR)) {
            QNDEBUG("Evernote host environment variable is not set or is empty");
            return Account();
        }
    }

    QString accountName = QString::fromLocal8Bit(qgetenv(ACCOUNT_NAME_ENV_VAR));

    qevercloud::UserID id = -1;
    Account::EvernoteAccountType::type evernoteAccountType =
        Account::EvernoteAccountType::Free;
    QString evernoteHost;

    if (!isLocal)
    {
        bool conversionResult = false;
        id = static_cast<qevercloud::UserID>(
            qEnvironmentVariableIntValue(ACCOUNT_ID_ENV_VAR, &conversionResult));
        if (!conversionResult) {
            QNDEBUG("Could not convert the account id to integer");
            return Account();
        }

        conversionResult = false;
        evernoteAccountType = static_cast<Account::EvernoteAccountType::type>(
            qEnvironmentVariableIntValue(ACCOUNT_EVERNOTE_ACCOUNT_TYPE_ENV_VAR,
                                         &conversionResult));
        if (!conversionResult) {
            QNDEBUG("Could not convert the Evernote account type "
                    "to integer");
            return Account();
        }

        evernoteHost = QString::fromLocal8Bit(qgetenv(ACCOUNT_EVERNOTE_HOST_ENV_VAR));
    }

    return findAccount(isLocal, accountName, id, evernoteAccountType, evernoteHost);
}

Account AccountManager::lastUsedAccount()
{
    QNDEBUG("AccountManager::lastUsedAccount");

    ApplicationSettings appSettings;

    appSettings.beginGroup(ACCOUNT_SETTINGS_GROUP);

    BOOST_SCOPE_EXIT(&appSettings) { \
        appSettings.endGroup();
    } BOOST_SCOPE_EXIT_END

    QVariant name = appSettings.value(LAST_USED_ACCOUNT_NAME);
    if (name.isNull()) {
        QNDEBUG("Can't find last used account's name");
        return Account();
    }

    QString accountName = name.toString();
    if (accountName.isEmpty()) {
        QNDEBUG("Last used account's name is empty");
        return Account();
    }

    QVariant type = appSettings.value(LAST_USED_ACCOUNT_TYPE);
    if (type.isNull()) {
        QNDEBUG("Can't find last used account's type");
        return Account();
    }

    bool isLocal = type.toBool();

    qevercloud::UserID id = -1;
    Account::EvernoteAccountType::type evernoteAccountType =
        Account::EvernoteAccountType::Free;
    QString evernoteHost;

    if (!isLocal)
    {
        QVariant userId = appSettings.value(LAST_USED_ACCOUNT_ID);
        if (userId.isNull()) {
            QNDEBUG("Can't find last used account's id");
            return Account();
        }

        bool conversionResult = false;
        id = userId.toInt(&conversionResult);
        if (!conversionResult) {
            QNDEBUG("Can't convert the last used account's id to int");
            return Account();
        }

        QVariant userEvernoteAccountHost =
            appSettings.value(LAST_USED_ACCOUNT_EVERNOTE_HOST);
        if (userEvernoteAccountHost.isNull()) {
            QNDEBUG("Can't find last used account's Evernote host");
            return Account();
        }
        evernoteHost = userEvernoteAccountHost.toString();

        QVariant userEvernoteAccountType =
            appSettings.value(LAST_USED_ACCOUNT_EVERNOTE_ACCOUNT_TYPE);
        if (userEvernoteAccountType.isNull()) {
            QNDEBUG("Can't find last used account's Evernote account type");
            return Account();
        }

        conversionResult = false;
        evernoteAccountType = static_cast<Account::EvernoteAccountType::type>(
            userEvernoteAccountType.toInt(&conversionResult));
        if (!conversionResult) {
            QNDEBUG("Can't convert the last used account's Evernote account "
                    "type to int");
            return Account();
        }
    }

    return findAccount(isLocal, accountName, id, evernoteAccountType, evernoteHost);
}

Account AccountManager::findAccount(
    const bool isLocal, const QString & accountName,
    const qevercloud::UserID id,
    const Account::EvernoteAccountType::type evernoteAccountType,
    const QString & evernoteHost)
{
    QNDEBUG("AccountManager::findAccount");

    QString appPersistenceStoragePath = applicationPersistentStoragePath();
    QString accountDirName = (isLocal
                              ? (QStringLiteral("LocalAccounts/") + accountName)
                              : (QStringLiteral("EvernoteAccounts/") + accountName +
                                 QStringLiteral("_") + evernoteHost +
                                 QStringLiteral("_") + QString::number(id)));
    QFileInfo accountFileInfo(appPersistenceStoragePath + QStringLiteral("/") +
                              accountDirName + QStringLiteral("/accountInfo.txt"));
    if (!accountFileInfo.exists()) {
        return Account();
    }

    Account account(accountName,
                    (isLocal ? Account::Type::Local : Account::Type::Evernote),
                    id, evernoteAccountType, evernoteHost);
    readComplementaryAccountInfo(account);
    return account;
}

void AccountManager::updateLastUsedAccount(const Account & account)
{
    QNDEBUG("AccountManager::updateLastUsedAccount: " << account.name());

    ApplicationSettings appSettings;

    appSettings.beginGroup(ACCOUNT_SETTINGS_GROUP);

    appSettings.setValue(LAST_USED_ACCOUNT_NAME, account.name());
    appSettings.setValue(LAST_USED_ACCOUNT_TYPE,
                         (account.type() == Account::Type::Local));
    appSettings.setValue(LAST_USED_ACCOUNT_ID, account.id());
    appSettings.setValue(LAST_USED_ACCOUNT_EVERNOTE_ACCOUNT_TYPE,
                         account.evernoteAccountType());
    appSettings.setValue(LAST_USED_ACCOUNT_EVERNOTE_HOST,
                         account.evernoteHost());

    appSettings.endGroup();
}

AccountManager::AccountInitializationException::AccountInitializationException(
        const ErrorString & message) :
    IQuentierException(message)
{}

const QString
AccountManager::AccountInitializationException::exceptionDisplayName() const
{
    return QStringLiteral("AccountInitializationException");
}

} // namespace quentier
