/*
 * Copyright 2017-2019 Dmitry Ivanov
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

#include "NetworkProxySettingsHelpers.h"
#include "SettingsNames.h"
#include <quentier/types/Account.h>
#include <quentier/utility/Macros.h>
#include <quentier/utility/ApplicationSettings.h>
#include <quentier/logging/QuentierLogger.h>
#include <QUrl>
#include <QScopedPointer>
#include <algorithm>

namespace quentier {

void parseNetworkProxySettings(const Account & currentAccount,
                               QNetworkProxy::ProxyType & type, QString & host,
                               int & port, QString & user, QString & password)
{
    // Initialize with "empty" values
    type = QNetworkProxy::DefaultProxy;
    host.resize(0);
    port = -1;
    user.resize(0);
    password.resize(0);

    QScopedPointer<ApplicationSettings> pSyncSettings;
    if (currentAccount.isEmpty()) {
        QNDEBUG(QStringLiteral("parseNetworkProxySettings: "
                               "using application-wise settings"));
        pSyncSettings.reset(new ApplicationSettings);
    }
    else {
        QNDEBUG(QStringLiteral("parseNetworkProxySettings: "
                               "using account-specific settings"));
        pSyncSettings.reset(new ApplicationSettings(currentAccount,
                                                    QUENTIER_SYNC_SETTINGS));
    }

    pSyncSettings->beginGroup(SYNCHRONIZATION_NETWORK_PROXY_SETTINGS);

    // 1) Parse network proxy type

    if (pSyncSettings->contains(SYNCHRONIZATION_NETWORK_PROXY_TYPE))
    {
        QVariant data = pSyncSettings->value(SYNCHRONIZATION_NETWORK_PROXY_TYPE);
        bool convertedToInt = false;
        int proxyType = data.toInt(&convertedToInt);
        if (convertedToInt)
        {
            // NOTE: it is unsafe to just cast int to QNetworkProxy::ProxyType,
            // it can be out of range; hence, checking for each available proxy
            // type manually
            switch(proxyType)
            {
            case QNetworkProxy::NoProxy:
                type = QNetworkProxy::NoProxy;
                break;
            case QNetworkProxy::DefaultProxy:
                type = QNetworkProxy::DefaultProxy;
                break;
            case QNetworkProxy::Socks5Proxy:
                type = QNetworkProxy::Socks5Proxy;
                break;
            case QNetworkProxy::HttpProxy:
                type = QNetworkProxy::HttpProxy;
                break;
            case QNetworkProxy::HttpCachingProxy:
                type = QNetworkProxy::HttpCachingProxy;
                break;
            case QNetworkProxy::FtpCachingProxy:
                type = QNetworkProxy::FtpCachingProxy;
                break;
            default:
                {
                    QNWARNING(QStringLiteral("Unrecognized network proxy type: ")
                              << proxyType << QStringLiteral(", fallback to "
                                                             "the default proxy "
                                                             "type"));
                    type = QNetworkProxy::DefaultProxy;
                    break;
                }
            }
        }
        else
        {
            QNWARNING(QStringLiteral("Failed to convert the network proxy type ")
                      << QStringLiteral("to int: ") << data
                      << QStringLiteral(", fallback to the default proxy type"));
            type = QNetworkProxy::DefaultProxy;
        }
    }
    else
    {
        QNDEBUG(QStringLiteral("No network proxy type was found within the settings"));
    }

    // 2) Parse network proxy host

    if (pSyncSettings->contains(SYNCHRONIZATION_NETWORK_PROXY_HOST))
    {
        QString data = pSyncSettings->value(SYNCHRONIZATION_NETWORK_PROXY_HOST).toString();
        if (!data.isEmpty())
        {
            QUrl url(data);
            if (Q_UNLIKELY(!url.isValid())) {
                QNWARNING(QStringLiteral("Network proxy host read from app ")
                          << QStringLiteral("settings does not appear to be ")
                          << QStringLiteral("the valid URL: ") << data);
            }
            else {
                host = data;
            }
        }
    }

    if (host.isEmpty()) {
        QNDEBUG(QStringLiteral("No host is specified within the settings"));
    }

    // 3) Parse network proxy port

    if (pSyncSettings->contains(SYNCHRONIZATION_NETWORK_PROXY_PORT))
    {
        QVariant data = pSyncSettings->value(SYNCHRONIZATION_NETWORK_PROXY_PORT);
        bool convertedToInt = false;
        int proxyPort = data.toInt(&convertedToInt);
        if (convertedToInt) {
            port = proxyPort;
        }
        else {
            QNWARNING(QStringLiteral("Failed to convert the network proxy port ")
                      << QStringLiteral("to int: ") << data);
        }
    }
    else
    {
        QNDEBUG(QStringLiteral("No network proxy port was found within the settings"));
    }

    // 4) Parse network proxy username

    if (pSyncSettings->contains(SYNCHRONIZATION_NETWORK_PROXY_USER)) {
        user = pSyncSettings->value(SYNCHRONIZATION_NETWORK_PROXY_USER).toString();
    }
    else {
        QNDEBUG(QStringLiteral("No network proxy username was found within "
                               "the settings"));
    }

    // 5) Parse network proxy password

    if (pSyncSettings->contains(SYNCHRONIZATION_NETWORK_PROXY_PASSWORD)) {
        password =
            pSyncSettings->value(SYNCHRONIZATION_NETWORK_PROXY_PASSWORD).toString();
    }
    else {
        QNDEBUG(QStringLiteral("No network proxy password was found within "
                               "the settings"));
    }

    pSyncSettings->endGroup();

    QNDEBUG(QStringLiteral("Result: network proxy type = ") << type
            << QStringLiteral(", host = ") << host
            << QStringLiteral(", port = ") << port
            << QStringLiteral(", username = ") << user
            << QStringLiteral(", password: ")
            << (password.isEmpty()
                ? QStringLiteral("<empty>")
                : QStringLiteral("not empty")));
}

void persistNetworkProxySettingsForAccount(const Account & account,
                                           const QNetworkProxy & proxy)
{
    QNDEBUG(QStringLiteral("persistNetworkProxySettingsForAccount: account = ")
            << account.name() << QStringLiteral("\nProxy type = ")
            << proxy.type() << QStringLiteral(", proxy host = ")
            << proxy.hostName() << QStringLiteral(", proxy port = ")
            << proxy.port() << QStringLiteral(", proxy user = ") << proxy.user());

    QScopedPointer<ApplicationSettings> pSyncSettings;
    if (account.isEmpty()) {
        QNDEBUG(QStringLiteral("Persisting application-wise proxy settings"));
        pSyncSettings.reset(new ApplicationSettings);
    }
    else {
        QNDEBUG(QStringLiteral("Persisting account-specific settings"));
        pSyncSettings.reset(new ApplicationSettings(account, QUENTIER_SYNC_SETTINGS));
    }

    pSyncSettings->beginGroup(SYNCHRONIZATION_NETWORK_PROXY_SETTINGS);

    pSyncSettings->setValue(SYNCHRONIZATION_NETWORK_PROXY_TYPE, proxy.type());
    pSyncSettings->setValue(SYNCHRONIZATION_NETWORK_PROXY_HOST, proxy.hostName());
    pSyncSettings->setValue(SYNCHRONIZATION_NETWORK_PROXY_PORT, proxy.port());
    pSyncSettings->setValue(SYNCHRONIZATION_NETWORK_PROXY_USER, proxy.user());
    pSyncSettings->setValue(SYNCHRONIZATION_NETWORK_PROXY_PASSWORD, proxy.password());

    pSyncSettings->endGroup();
}

void restoreNetworkProxySettingsForAccount(const Account & account)
{
    QNDEBUG(QStringLiteral("restoreNetworkProxySettingsForAccount: account = ")
            << account.name());

    QNetworkProxy::ProxyType type = QNetworkProxy::NoProxy;
    QString host;
    int port = 0;
    QString user;
    QString password;

    parseNetworkProxySettings(account, type, host, port, user, password);

    QNetworkProxy proxy(type);
    proxy.setHostName(host);
    proxy.setPort(static_cast<quint16>(std::max(port, 0)));
    proxy.setUser(user);
    proxy.setPassword(password);

    QNTRACE(QStringLiteral("Setting the application proxy extracted from app "
                           "settings"));
    QNetworkProxy::setApplicationProxy(proxy);
}

} // namespace quentier
