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

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Utility.h"
#include "SymbolsUnpacker.h"

#include <lib/utility/HumanReadableVersionInfo.h>
#include <VersionInfo.h>

#include <quentier/utility/VersionInfo.h>
#include <quentier/utility/Utility.h>

#include <QDir>
#include <QDesktopServices>
#include <QThreadPool>

#include <QStandardPaths>

#define QNSIGNAL(className, methodName, ...) &className::methodName
#define QNSLOT(className, methodName, ...) &className::methodName

MainWindow::MainWindow(const QString & quentierSymbolsFileLocation,
                       const QString & libquentierSymbolsFileLocation,
                       const QString & stackwalkBinaryLocation,
                       const QString & minidumpLocation,
                       QWidget * parent) :
    QMainWindow(parent),
    m_pUi(new Ui::MainWindow),
    m_numPendingSymbolsUnpackers(0),
    m_minidumpLocation(),
    m_stackwalkBinary(),
    m_unpackedSymbolsRootPath(),
    m_symbolsUnpackingErrors(),
    m_output(),
    m_error()
{
    m_pUi->setupUi(this);
    setWindowTitle(tr("Quentier crashed"));

    m_minidumpLocation = nativePathToUnixPath(minidumpLocation);
    m_pUi->minidumpFilePathLineEdit->setText(m_minidumpLocation);

    m_stackwalkBinary = nativePathToUnixPath(stackwalkBinaryLocation);
    QFileInfo stackwalkBinaryInfo(m_stackwalkBinary);
    if (Q_UNLIKELY(!stackwalkBinaryInfo.exists())) {
        m_pUi->stackTracePlainTextEdit->setPlainText(
            tr("Error: minidump stackwalk utility file doesn't exist") +
            QStringLiteral(": ") + QDir::toNativeSeparators(m_stackwalkBinary));
        return;
    }
    else if (Q_UNLIKELY(!stackwalkBinaryInfo.isFile())) {
        m_pUi->stackTracePlainTextEdit->setPlainText(
            tr("Error: the path to minidump stackwalk utility doesn't point to "
               "an actual file") +
            QStringLiteral(": ") + QDir::toNativeSeparators(m_stackwalkBinary));
        return;
    }

    QString tmpDirPath =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    m_unpackedSymbolsRootPath =
        tmpDirPath + QStringLiteral("/Quentier_debugging_symbols/symbols");
    bool res = quentier::removeDir(m_unpackedSymbolsRootPath);
    if (Q_UNLIKELY(!res)) {
        m_pUi->stackTracePlainTextEdit->setPlainText(
            tr("Error: the directory containing the unpacked debugging symbols "
               "already exists and can't be removed") +
            QStringLiteral(": ") +
            QDir::toNativeSeparators(m_unpackedSymbolsRootPath));
        return;
    }

    QDir unpackRootDir(m_unpackedSymbolsRootPath);
    res = unpackRootDir.mkpath(m_unpackedSymbolsRootPath);
    if (!res) {
        m_pUi->stackTracePlainTextEdit->setPlainText(
            tr("Error: the directory for the unpacked debugging symbols can't "
               "be created") +
            QStringLiteral(": ") +
            QDir::toNativeSeparators(m_unpackedSymbolsRootPath));
        return;
    }

    QString output = QStringLiteral("Version info:\n\n");
    output += versionInfos();
    output += QStringLiteral("\n\n");
    output += tr("Loading debugging symbols, please wait");
    output += QStringLiteral("...");
    m_pUi->stackTracePlainTextEdit->setPlainText(output);

    SymbolsUnpacker * pQuentierSymbolsUnpacker =
        new SymbolsUnpacker(quentierSymbolsFileLocation, m_unpackedSymbolsRootPath);
    QObject::connect(pQuentierSymbolsUnpacker,
                     QNSIGNAL(SymbolsUnpacker,finished,bool,QString),
                     this,
                     QNSLOT(MainWindow,onSymbolsUnpackerFinished,bool,QString));
    ++m_numPendingSymbolsUnpackers;

    SymbolsUnpacker * pLibquentierSymbolsUnpacker =
        new SymbolsUnpacker(libquentierSymbolsFileLocation, m_unpackedSymbolsRootPath);
    QObject::connect(pLibquentierSymbolsUnpacker,
                     QNSIGNAL(SymbolsUnpacker,finished,bool,QString),
                     this,
                     QNSLOT(MainWindow,onSymbolsUnpackerFinished,bool,QString));
    ++m_numPendingSymbolsUnpackers;

    QThreadPool::globalInstance()->start(pQuentierSymbolsUnpacker);
    QThreadPool::globalInstance()->start(pLibquentierSymbolsUnpacker);
}

MainWindow::~MainWindow()
{
    delete m_pUi;
}

void MainWindow::onMinidumpStackwalkReadyReadStandardOutput()
{
    QProcess * pStackwalkProcess = qobject_cast<QProcess*>(sender());
    if (Q_UNLIKELY(!pStackwalkProcess)) {
        m_pUi->stackTracePlainTextEdit->setPlainText(
            tr("Error: can't cast the invoker of minidump stackwalk stdout "
               "update to QProcess"));
        return;
    }

    m_output += readData(*pStackwalkProcess, /* from stdout = */ true);

    QString output;

    if (!m_symbolsUnpackingErrors.isEmpty()) {
        output = m_symbolsUnpackingErrors;
    }

    output += m_output;

    m_pUi->stackTracePlainTextEdit->setPlainText(output);
}

void MainWindow::onMinidumpStackwalkReadyReadStandardError()
{
    QProcess * pStackwalkProcess = qobject_cast<QProcess*>(sender());
    if (Q_UNLIKELY(!pStackwalkProcess)) {
        m_pUi->stackTracePlainTextEdit->setPlainText(
            tr("Error: can't cast the invoker of minidump stackwalk stderr "
               "update to QProcess"));
        return;
    }

    m_error += readData(*pStackwalkProcess, /* from stdout = */ false);
}

void MainWindow::onMinidumpStackwalkProcessFinished(
    int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)

    QString output;

    if (!m_symbolsUnpackingErrors.isEmpty()) {
        output = m_symbolsUnpackingErrors;
        output += QStringLiteral("\n");
    }

    output += QStringLiteral("Version info:\n\n");
    output += versionInfos();
    output += QStringLiteral("\n\n");
    output += tr("Stacktrace extraction finished, exit code") +
              QStringLiteral(": ") +
              QString::number(exitCode) +
              QStringLiteral("\n");
    output += m_output;
    output += QStringLiteral("\n\n");
    output += m_error;

    m_pUi->stackTracePlainTextEdit->setPlainText(output);
}

void MainWindow::onSymbolsUnpackerFinished(bool status, QString errorDescription)
{
    if (m_numPendingSymbolsUnpackers != 0) {
        --m_numPendingSymbolsUnpackers;
    }

    if (!status)
    {
        if (m_symbolsUnpackingErrors.isEmpty()) {
            m_symbolsUnpackingErrors = tr("Errors detected during symbols unpacking") +
                                       QStringLiteral(":\n\n");
        }

        m_symbolsUnpackingErrors += errorDescription;
        m_symbolsUnpackingErrors += QStringLiteral("\n");
    }

    if (m_numPendingSymbolsUnpackers != 0) {
        return;
    }

    QProcess * pStackwalkProcess = new QProcess(this);
    QObject::connect(pStackwalkProcess,
                     QNSIGNAL(QProcess,readyReadStandardOutput),
                     this,
                     QNSLOT(MainWindow,onMinidumpStackwalkReadyReadStandardOutput));
    QObject::connect(pStackwalkProcess,
                     QNSIGNAL(QProcess,readyReadStandardError),
                     this,
                     QNSLOT(MainWindow,onMinidumpStackwalkReadyReadStandardError));
    QObject::connect(pStackwalkProcess,
                     SIGNAL(finished(int,QProcess::ExitStatus)),
                     this,
                     SLOT(onMinidumpStackwalkProcessFinished(int,QProcess::ExitStatus)));

    QStringList stackwalkArgs;
    stackwalkArgs.reserve(2);
    stackwalkArgs << QDir::fromNativeSeparators(m_minidumpLocation);
    stackwalkArgs << m_unpackedSymbolsRootPath;

    pStackwalkProcess->start(m_stackwalkBinary, stackwalkArgs, QIODevice::ReadOnly);
}

QString MainWindow::readData(QProcess & process, const bool fromStdout)
{
    QByteArray output = (fromStdout
                         ? process.readAllStandardOutput()
                         : process.readAllStandardError());
    return QString::fromUtf8(output);
}

QString MainWindow::versionInfos() const
{
    QString result = QStringLiteral("libquentier: ");

    result += quentier::libquentierRuntimeInfo();
    result += QStringLiteral("\n");

    result += QStringLiteral("Quentier: ");
    result += quentier::quentierVersion();
    result += QStringLiteral(", build info: ");
    result += quentier::quentierBuildInfo();

    result += QStringLiteral("\n\nBuilt with Qt ");
    result += QString::fromUtf8(QT_VERSION_STR);
    result += QStringLiteral(", uses Qt ");
    result += QString::fromUtf8(qVersion());
    return result;
}
