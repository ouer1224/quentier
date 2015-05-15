#include "EncryptedAreaPlugin.h"
#include "ui_EncryptedAreaPlugin.h"
#include "NoteDecryptionDialog.h"
#include <logging/QuteNoteLogger.h>
#include <client/types/IResource.h>
#include <QIcon>
#include <QMouseEvent>

namespace qute_note {

EncryptedAreaPlugin::EncryptedAreaPlugin(QWidget * parent) :
    INoteEditorPlugin(parent),
    m_pUI(new Ui::EncryptedAreaPlugin),
    m_hint(),
    m_cipher(),
    m_keyLength(0)
{
    m_pUI->setupUi(this);

    if (!QIcon::hasThemeIcon("security-high")) {
        QIcon lockIcon;
        lockIcon.addFile(":/encrypted_area_icons/png/lock-16x16", QSize(16, 16));
        lockIcon.addFile(":/encrypted_area_icons/png/lock-24x24", QSize(24, 24));
        lockIcon.addFile(":/encrypted_area_icons/png/lock-32x32", QSize(32, 32));
        lockIcon.addFile(":/encrypted_area_icons/png/lock-48x48", QSize(48, 48));
        m_pUI->iconPushButton->setIcon(lockIcon);
    }

    QAction * showEncryptedTextAction = new QAction(this);
    showEncryptedTextAction->setText(QObject::tr("Show encrypted text") + "...");
    QObject::connect(showEncryptedTextAction, SIGNAL(triggered()), this, SLOT(decrypt()));
    m_pUI->toolButton->addAction(showEncryptedTextAction);

    QAction * decryptTextPermanentlyAction = new QAction(this);
    decryptTextPermanentlyAction->setText(QObject::tr("Decrypt text permanently") + "...");
    QObject::connect(decryptTextPermanentlyAction, SIGNAL(triggered()), this, SLOT(decryptAndRemember()));
    m_pUI->toolButton->addAction(decryptTextPermanentlyAction);
}

EncryptedAreaPlugin::~EncryptedAreaPlugin()
{
    delete m_pUI;
}

EncryptedAreaPlugin * EncryptedAreaPlugin::clone() const
{
    return new EncryptedAreaPlugin();
}

bool EncryptedAreaPlugin::initialize(const QString & mimeType, const QUrl & url,
                                     const QStringList & parameterNames,
                                     const QStringList & parameterValues,
                                     const IResource * resource, QString & errorDescription)
{
    QNDEBUG("EncryptedAreaPlugin::initialize: mime type = " << mimeType
            << ", url = " << url.toString() << ", parameter names = " << parameterNames.join(", ")
            << ", parameter values = " << parameterValues.join(", "));

    Q_UNUSED(resource);

    const int numParameterValues = parameterValues.size();

    int cipherIndex = parameterNames.indexOf("cipher");
    if (cipherIndex < 0) {
        errorDescription = QT_TR_NOOP("cipher attribute was not found within object with encrypted text");
        return false;
    }

    if (numParameterValues <= cipherIndex) {
        errorDescription = QT_TR_NOOP("no value was found for cipher attribute");
        return false;
    }

    int keyLengthIndex = parameterNames.indexOf("length");
    if (keyLengthIndex < 0) {
        errorDescription = QT_TR_NOOP("length attribute was not found within object with encrypted text");
        return false;
    }

    if (numParameterValues <= keyLengthIndex) {
        errorDescription = QT_TR_NOOP("no value was found for length attribute");
        return false;
    }

    QString keyLengthString = parameterValues[keyLengthIndex];
    bool conversionResult = false;
    int keyLength = keyLengthString.toInt(&conversionResult);
    if (!conversionResult) {
        errorDescription = QT_TR_NOOP("can't extract integer value from length attribute: ") + keyLengthString;
        return false;
    }
    else if (keyLength < 0) {
        errorDescription = QT_TR_NOOP("key length is negative: ") + keyLengthString;
        return false;
    }

    m_keyLength = static_cast<size_t>(keyLength);

    m_cipher = parameterValues[cipherIndex];

    int hintIndex = parameterNames.indexOf("hint");
    if ((hintIndex < 0) || (numParameterValues <= hintIndex)) {
        m_hint.clear();
    }
    else {
        m_hint = parameterValues[hintIndex];
    }

    QNTRACE("Initialized encrypted area plugin: cipher = " << m_cipher
            << ", length = " << m_keyLength << ", hint = " << m_hint);
    return true;
}

QStringList EncryptedAreaPlugin::mimeTypes() const
{
    return QStringList();
}

QHash<QString, QStringList> EncryptedAreaPlugin::fileExtensions() const
{
    return QHash<QString, QStringList>();
}

QStringList EncryptedAreaPlugin::specificAttributes() const
{
    return QStringList() << "en-crypt";
}

QString EncryptedAreaPlugin::name() const
{
    return "EncryptedAreaPlugin";
}

QString EncryptedAreaPlugin::description() const
{
    return QObject::tr("Encrypted area plugin - note editor plugin used for the display and convenient work "
                       "with encrypted text within notes");
}

void EncryptedAreaPlugin::mouseReleaseEvent(QMouseEvent * mouseEvent)
{
    QWidget::mouseReleaseEvent(mouseEvent);

    if (!mouseEvent) {
        return;
    }

    const QPoint & pos = mouseEvent->pos();
    QWidget * child = childAt(pos);
    if (!child) {
        return;
    }

    if (child == m_pUI->iconPushButton) {
        decrypt();
    }
}

void EncryptedAreaPlugin::decrypt()
{
    raiseNoteDecryptionDialog(false);
}

void EncryptedAreaPlugin::decryptAndRemember()
{
    raiseNoteDecryptionDialog(true);
}

void EncryptedAreaPlugin::raiseNoteDecryptionDialog(const bool shouldRememberPassphrase)
{
    QScopedPointer<NoteDecryptionDialog> pDecryptionDialog(new NoteDecryptionDialog(this));
    pDecryptionDialog->setWindowModality(Qt::WindowModal);

    pDecryptionDialog->setHint(m_hint);
    pDecryptionDialog->setRememberPassphraseDefaultState(false);

    int res = pDecryptionDialog->exec();
    if (res == QDialog::Accepted)
    {
        QString passphrase = pDecryptionDialog->passphrase();
        // TODO: attempt to use it to decrypt stuff, return with error if no luck
        bool shouldRememberPassphrase = pDecryptionDialog->rememberPassphrase();
        if (shouldRememberPassphrase) {
            emit rememberPassphraseForSession(m_cipher, passphrase);
        }
    }
}

} // namespace qute_note