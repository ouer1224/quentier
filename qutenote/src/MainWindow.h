#ifndef __QUTE_NOTE__MAINWINDOW_H
#define __QUTE_NOTE__MAINWINDOW_H

#include <QtCore>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QMainWindow>
#else
#include <QMainWindow>
#endif
#include <QTextListFormat>

namespace Ui {
class MainWindow;
}

QT_FORWARD_DECLARE_CLASS(EvernoteOAuthBrowser)
QT_FORWARD_DECLARE_CLASS(EvernoteServiceManager)
QT_FORWARD_DECLARE_CLASS(QUrl)
QT_FORWARD_DECLARE_CLASS(AskConsumerKeyAndSecret)
QT_FORWARD_DECLARE_CLASS(AskUserNameAndPassword)
QT_FORWARD_DECLARE_CLASS(QTextCharFormat)

namespace qute_note {
QT_FORWARD_DECLARE_CLASS(NoteEditor)
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget * pParentWidget = nullptr);
    virtual ~MainWindow();

    EvernoteOAuthBrowser * OAuthBrowser();

private:
    void connectActionsToEditorSlots();

    void checkAndSetupConsumerKeyAndSecret();
    void checkAndSetupUserNameAndPassword();
    void checkAndSetupOAuthTokenAndSecret();

    enum ECredentialsToCheck
    {
        CHECK_CONSUMER_KEY_AND_SECRET,
        CHECK_USER_NAME_AND_PASSWORD,
        CHECK_OAUTH_TOKEN_AND_SECRET
    };

    void checkAndSetupCredentials(ECredentialsToCheck credentials);

public Q_SLOTS:
    void onSetStatusBarText(QString message, const int duration = 0);
    void onShowAuthWebPage(QUrl url);
    void onRequestUsernameAndPassword();
    void show();

private Q_SLOTS:
    void noteTextBold();
    void noteTextItalic();
    void noteTextUnderline();
    void noteTextStrikeThrough();
    void noteTextAlignLeft();
    void noteTextAlignCenter();
    void noteTextAlignRight();
    void noteTextAddHorizontalLine();
    void noteTextIncreaseIndentation();
    void noteTextDecreaseIndentation();
    void noteTextInsertUnorderedList();
    void noteTextInsertOrderedList();

    void noteChooseTextColor();
    void noteChooseSelectedTextColor();

    void noteTextSpellCheck() { /* TODO: implement */ }
    void noteTextInsertToDoCheckBox();

    void noteTextInsertTableDialog();
    void noteTextInsertTable(int rows, int columns, double width, bool relativeWidth);

    void onShowNoteSource();

    void onNoteContentChanged();

private:
    void insertList(const QTextListFormat::Style style);

    enum ESelectedAlignment { ALIGNED_LEFT, ALIGNED_CENTER, ALIGNED_RIGHT };
    void setAlignButtonsCheckedState(const ESelectedAlignment alignment);

    void checkThemeIconsAndSetFallbacks();

    void updateNoteHtmlView();

private:
    Ui::MainWindow * m_pUI;
    AskConsumerKeyAndSecret * m_pAskConsumerKeyAndSecretWidget;
    AskUserNameAndPassword  * m_pAskUserNameAndPasswordWidget;
    EvernoteServiceManager  * m_pManager;
    EvernoteOAuthBrowser * m_pOAuthBrowser;
    QWidget * m_currentStatusBarChildWidget;
    qute_note::NoteEditor * m_pNoteEditor;
};

#endif // __QUTE_NOTE__MAINWINDOW_H