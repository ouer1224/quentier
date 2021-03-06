/*
 * Copyright 2020 Dmitry Ivanov
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

#include "GitHubUpdateChecker.h"

#include <lib/network/NetworkReplyFetcher.h>
#include <lib/update/UpdateInfo.h>

#include <quentier/logging/QuentierLogger.h>

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSysInfo>

namespace quentier {

GitHubUpdateChecker::GitHubUpdateChecker(QObject * parent) :
    IUpdateChecker(parent),
    m_host(QStringLiteral("api.github.com")),
    m_scheme(QStringLiteral("https")),
    m_currentBuildCreationDateTime(QDateTime::fromString(
        QUENTIER_BUILD_TIMESTAMP,
        Qt::ISODate))
{}

GitHubUpdateChecker::~GitHubUpdateChecker()
{
    QNDEBUG("GitHubUpdateChecker::~GitHubUpdateChecker");
}

void GitHubUpdateChecker::checkForUpdates()
{
    QNDEBUG("GitHubUpdateChecker::checkForUpdates");

    if (m_inProgress) {
        QNDEBUG("Checking for updates is already in progress");
        return;
    }

    QUrl url;
    url.setHost(m_host);
    url.setScheme(m_scheme);
    url.setPath(QStringLiteral("/repos/d1vanov/quentier/releases"));

    auto * pListReleasesReplyFetcher = new NetworkReplyFetcher(
        url,
        NETWORK_REPLY_FETCHER_DEFAULT_TIMEOUT_MSEC,
        this);

    QObject::connect(
        pListReleasesReplyFetcher,
        &NetworkReplyFetcher::finished,
        this,
        &GitHubUpdateChecker::onReleasesListed);

    m_inProgress = true;
    pListReleasesReplyFetcher->start();
}

void GitHubUpdateChecker::onReleasesListed(
    bool status, QByteArray fetchedData, ErrorString errorDescription)
{
    QNDEBUG("GitHubUpdateChecker::onReleasesListed: status = "
        << (status ? "true" : "false") << ", error description = "
        << errorDescription << ", fetched data size = " << fetchedData.size());

    auto * pFetcher = qobject_cast<NetworkReplyFetcher*>(sender());
    if (pFetcher) {
        pFetcher->disconnect(this);
        pFetcher->deleteLater();
        pFetcher = nullptr;
    }

    m_inProgress = false;

    if (Q_UNLIKELY(!status))
    {
        ErrorString error(QT_TR_NOOP("Failed to list releases from GitHub"));
        error.appendBase(errorDescription.base());
        error.appendBase(errorDescription.additionalBases());
        error.details() = errorDescription.details();
        QNWARNING(error);
        Q_EMIT failure(error);
        return;
    }

    if (Q_UNLIKELY(!m_currentBuildCreationDateTime.isValid())) {
        ErrorString error(
            QT_TR_NOOP("Failed to parse current build creation time from string"));
        error.details() = QUENTIER_BUILD_TIMESTAMP;
        QNWARNING(error);
        Q_EMIT failure(error);
    }

    QJsonParseError jsonParseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(
        fetchedData,
        &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError)
    {
        ErrorString error(
            QT_TR_NOOP("Failed to parse list releases response from GitHub to "
                       "json"));
        QNWARNING(error);
        Q_EMIT failure(error);
        return;
    }

    parseListedReleases(jsonDoc);

    if (!m_pLatestReleaseInfo) {
        Q_EMIT noUpdatesAvailable();
        return;
    }

    Q_EMIT updatesAvailable(m_pLatestReleaseInfo->m_htmlUrl);
}

void GitHubUpdateChecker::parseListedReleases(const QJsonDocument & jsonDoc)
{
    QUrl latestReleaseUrl;
    QDateTime latestReleaseCreationDateTime;

    QRegularExpression versionedReleaseRegex(
        QStringLiteral("^v\\d+\\.\\d+(\\.\\d+)?(-\\S*)?$"));
    Q_ASSERT(versionedReleaseRegex.isValid());

    QJsonArray releases = jsonDoc.array();
    for(const auto release: releases)
    {
        if (Q_UNLIKELY(!release.isObject())) {
            QNWARNING("Skipping json field which is not an object although it "
                << "should be a GitHub release: " <<  release);
            continue;
        }

        auto releaseObject = release.toObject();

        auto prereleaseValue = releaseObject.value(QStringLiteral("prerelease"));
        if (Q_UNLIKELY(prereleaseValue == QJsonValue::Undefined)) {
            QNWARNING("GitHub release has no prerelease field: "
                << release);
            continue;
        }

        auto nameValue = releaseObject.value(QStringLiteral("name"));
        if (Q_UNLIKELY(nameValue == QJsonValue::Undefined)) {
            QNWARNING("GitHub release has no name field: " << release);
            continue;
        }

        auto name = nameValue.toString();

        if (prereleaseValue.toBool() &&
            name.contains(QStringLiteral("continuous-")) &&
            !m_useContinuousUpdateChannel)
        {
            QNDEBUG("Skipping release " << name
                << " as checking for continuous releases is switched off");
            continue;
        }

        auto createdAtValue = releaseObject.value(QStringLiteral("created_at"));
        if (Q_UNLIKELY(createdAtValue == QJsonValue::Undefined))
        {
            QNWARNING("GitHub release has no created_at field: "
                << release);
            continue;
        }

        QDateTime createdAt = QDateTime::fromString(
            createdAtValue.toString(),
            Qt::ISODate);
        if (!createdAt.isValid())
        {
            QNWARNING("Failed to parse datetime from created_at field of "
                << "GitHub release: " << createdAtValue.toString());
            continue;
        }

        if (m_currentBuildCreationDateTime >= createdAt) {
            QNDEBUG("Skipping release " << name << " as its creation time "
                << createdAt << " is no greater than Quentier build time: "
                << m_currentBuildCreationDateTime);
            continue;
        }

        auto targetCommitValue = releaseObject.value(
            QStringLiteral("target_commitish"));
        if (Q_UNLIKELY(targetCommitValue == QJsonValue::Undefined)) {
            QNWARNING("GitHub release has no target_committish field: "
                << release);
            continue;
        }

        auto targetCommit = targetCommitValue.toString();

        if (targetCommit.startsWith(QUENTIER_BUILD_GIT_COMMIT)) {
            QNDEBUG("Skipping release " << name << " as its target commit matches "
                << "the build commit of Quentier: " << QUENTIER_BUILD_GIT_COMMIT);
            continue;
        }

        // If we got here, it seems the release was created after the current
        // build of Quentier, now need to figure out if it matches the specified
        // update channel

        auto tagNameValue = releaseObject.value(QStringLiteral("tag_name"));
        if (Q_UNLIKELY(tagNameValue == QJsonValue::Undefined)) {
            QNWARNING("GitHub release has no tag_name field: " << release);
            continue;
        }

        auto tagName = tagNameValue.toString();

        // Versioned releases are assumed to be created only from master branch
        bool isVersionedRelease = versionedReleaseRegex.match(tagName).hasMatch();
        if (isVersionedRelease && (m_updateChannel != QStringLiteral("master"))) {
            QNDEBUG("Skipping versioned release " << tagName
                << " as update channel is not master but " << m_updateChannel);
            continue;
        }

        if (!isVersionedRelease &&
            !tagName.contains(m_updateChannel, Qt::CaseInsensitive))
        {
            QNDEBUG("Skipping release " << tagName
                << " not matching the current update channel "
                <<  m_updateChannel);
            continue;
        }

        // If we got here, this release seems to match the one we are looking
        // for; need to check whether we don't have a later release already
        if (latestReleaseCreationDateTime.isValid() &&
            (latestReleaseCreationDateTime > createdAt))
        {
            QNDEBUG("Skipping release " << tagName
                << " as its creation datetime " << createdAt
                << " is not later than the creation datetime "
                << latestReleaseCreationDateTime << " of already found release");
            continue;
        }

        // Now need to check whether the release contains the required assets
        if (!checkReleaseAssets(releaseObject)) {
            continue;
        }

        // That's it, we found a new appropriate release, need to update latest
        // release info

        auto htmlUrlValue = releaseObject.value(QStringLiteral("html_url"));
        if (Q_UNLIKELY(htmlUrlValue == QJsonValue::Undefined)) {
            QNWARNING("GitHub release has no html_url field: " << release);
            continue;
        }

        QUrl htmlUrl = QUrl(htmlUrlValue.toString());
        if (Q_UNLIKELY(!htmlUrl.isValid())) {
            QNWARNING("GitHub release's html_url field is not a valid url: "
                << release);
            continue;
        }

        latestReleaseUrl = htmlUrl;
        latestReleaseCreationDateTime = createdAt;
    }

    if (!latestReleaseUrl.isValid()) {
        QNDEBUG("Found no appropriate releases to update to");
        m_pLatestReleaseInfo.reset();
        return;
    }

    QNDEBUG("Found appropriate release: creation datetime = "
        << latestReleaseCreationDateTime << ", html url = " << latestReleaseUrl);

    m_pLatestReleaseInfo = std::make_unique<GitHubReleaseInfo>(
        latestReleaseUrl,
        latestReleaseCreationDateTime);
}

bool GitHubUpdateChecker::checkReleaseAssets(
    const QJsonObject & releaseObject) const
{
    QRegularExpression assetNameRegex;

    auto kernelType = QSysInfo::kernelType();
    if (kernelType == QStringLiteral("linux")) {
        assetNameRegex.setPattern(QStringLiteral("(.*)\\.AppImage"));
    }
    else if (kernelType == QStringLiteral("darwin")) {
        assetNameRegex.setPattern(QStringLiteral("Quentier_mac_x86_64\\.zip"));
    }
    else if (kernelType.startsWith(QStringLiteral("win")))
    {
        auto arch = QSysInfo::currentCpuArchitecture();
        if (arch.endsWith(QStringLiteral("64"))) {
            assetNameRegex.setPattern(
                QStringLiteral("((.*)windows(.*)x64.zip)|((.*)x64.exe)"));
        }
        else {
            assetNameRegex.setPattern(
                QStringLiteral("((.*)windows(.*)x86.zip)|((.*)Win32.exe)"));
        }
    }
    else
    {
        QNWARNING("Failed to determine kernel type: " << kernelType);
        return true;
    }

    Q_ASSERT(assetNameRegex.isValid());

    auto assetsValue = releaseObject.value(QStringLiteral("assets"));
    if (Q_UNLIKELY(assetsValue == QJsonValue::Undefined)) {
        QNWARNING("GitHub release appears to have no assets: " << releaseObject);
        return false;
    }

    if (Q_UNLIKELY(!assetsValue.isArray())) {
        QNWARNING("GitHub release assets are not organized into an array: "
            << releaseObject);
        return false;
    }

    bool foundMatchingAsset = false;

    auto assetsArray = assetsValue.toArray();
    for(const auto asset: assetsArray)
    {
        if (Q_UNLIKELY(!asset.isObject())) {
            QNWARNING("Skipping release asset field which is not an object "
                << "although it should be a GitHub release asset: " << asset);
            continue;
        }

        auto assetObject = asset.toObject();

        auto assetNameValue = assetObject.value(QStringLiteral("name"));
        if (Q_UNLIKELY(assetNameValue == QJsonValue::Undefined)) {
            QNWARNING("GitHub release asset has no name field: " << asset);
            continue;
        }

        auto assetName = assetNameValue.toString();
        if (assetNameRegex.match(assetName).hasMatch()) {
            QNDEBUG("Found matching asset: pattern = "
                << assetNameRegex.pattern() << ", asset name = " << assetName);
            foundMatchingAsset = true;
            break;
        }
    }

    return foundMatchingAsset;
}

} // namespace quentier
