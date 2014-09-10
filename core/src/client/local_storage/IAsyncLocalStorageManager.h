#ifndef __QUTE_NOTE__CLIENT__LOCAL_STORAGE__I_ASYNC_LOCAL_STORAGE_MANAGER_H
#define __QUTE_NOTE__CLIENT__LOCAL_STORAGE__I_ASYNC_LOCAL_STORAGE_MANAGER_H

#include "Lists.h"
#include <tools/Linkage.h>
#include <QString>

namespace qute_note {


/**
 * @brief The IAsyncLocalStorageManager class defines the interfaces for signals and slots
 * used for asynchronous access to local storage database. Typically the interface is
 * slot like "onRequestToDoSmth" + a couple of "complete"/"failed" resulting signals
 * for each public method in LocalStorageManager.
 */
class QUTE_NOTE_EXPORT IAsyncLocalStorageManager
{
public:
    virtual ~IAsyncLocalStorageManager();

protected:
    IAsyncLocalStorageManager() = default;
    IAsyncLocalStorageManager(const IAsyncLocalStorageManager & other) = default;
    IAsyncLocalStorageManager(IAsyncLocalStorageManager && other);
    IAsyncLocalStorageManager & operator=(const IAsyncLocalStorageManager & other) = default;
    IAsyncLocalStorageManager & operator=(IAsyncLocalStorageManager && other);

    // Pure virtual prototypes for signals to be emitted from subclasses:

    // Prototypes for user-related signals:
    virtual void getUserCountComplete(int userCount) = 0;
    virtual void getUserCountFailed(QString errorDescription) = 0;

    virtual void switchUserComplete(qint32 userId) = 0;
    virtual void switchUserFailed(qint32 userId, QString errorDescription) = 0;

    virtual void addUserComplete(QSharedPointer<UserWrapper> user) = 0;
    virtual void addUserFailed(QSharedPointer<UserWrapper> user, QString errorDescription) = 0;

    virtual void updateUserComplete(QSharedPointer<UserWrapper> user) = 0;
    virtual void updateUserFailed(QSharedPointer<UserWrapper> user, QString errorDecription) = 0;

    virtual void findUserComplete(QSharedPointer<UserWrapper> foundUser) = 0;
    virtual void findUserFailed(QSharedPointer<UserWrapper> user, QString errorDescription) = 0;

    virtual void deleteUserComplete(QSharedPointer<UserWrapper> user) = 0;
    virtual void deleteUserFailed(QSharedPointer<UserWrapper> user, QString errorDescription) = 0;

    virtual void expungeUserComplete(QSharedPointer<UserWrapper> user) = 0;
    virtual void expungeUserFailed(QSharedPointer<UserWrapper> user, QString errorDescription) = 0;

    // Prototypes for notebook-related signals:
    virtual void getNotebookCountComplete(int notebookCount) = 0;
    virtual void getNotebookCountFailed(QString errorDescription) = 0;

    virtual void addNotebookComplete(QSharedPointer<Notebook> notebook) = 0;
    virtual void addNotebookFailed(QSharedPointer<Notebook> notebook, QString errorDescription) = 0;

    virtual void updateNotebookComplete(QSharedPointer<Notebook> notebook) = 0;
    virtual void updateNotebookFailed(QSharedPointer<Notebook> notebook, QString errorDescription) = 0;

    virtual void findNotebookComplete(QSharedPointer<Notebook> foundNotebook) = 0;
    virtual void findNotebookFailed(QSharedPointer<Notebook> notebook, QString errorDescription) = 0;

    virtual void findDefaultNotebookComplete(QSharedPointer<Notebook> foundNotebook) = 0;
    virtual void findDefaultNotebookFailed(QSharedPointer<Notebook> notebook, QString errorDescription) = 0;

    virtual void findLastUsedNotebookComplete(QSharedPointer<Notebook> foundNotebook) = 0;
    virtual void findLastUsedNotebookFailed(QSharedPointer<Notebook> notebook, QString errorDescription) = 0;

    virtual void findDefaultOrLastUsedNotebookComplete(QSharedPointer<Notebook> foundNotebook) = 0;
    virtual void findDefaultOrLastUsedNotebookFailed(QSharedPointer<Notebook> notebook, QString errorDescription) = 0;

    virtual void listAllNotebooksComplete(QList<Notebook> foundNotebooks) = 0;
    virtual void listAllNotebooksFailed(QString errorDescription) = 0;

    virtual void listAllSharedNotebooksComplete(QList<SharedNotebookWrapper> foundSharedNotebooks) = 0;
    virtual void listAllSharedNotebooksFailed(QString errorDescription) = 0;

    virtual void listSharedNotebooksPerNotebookGuidComplete(QString notebookGuid, QList<SharedNotebookWrapper> foundSharedNotebooks) = 0;
    virtual void listSharedNotebooksPerNotebookGuidFailed(QString notebookGuid, QString errorDescription) = 0;

    virtual void expungeNotebookComplete(QSharedPointer<Notebook> notebook) = 0;
    virtual void expungeNotebookFailed(QSharedPointer<Notebook> notebook, QString errorDescription) = 0;

    // Prototypes for linked notebook-related signals:
    virtual void getLinkedNotebookCountComplete(int linkedNotebookCount) = 0;
    virtual void getLinkedNotebookCountFailed(QString errorDescription) = 0;

    virtual void addLinkedNotebookComplete(QSharedPointer<LinkedNotebook> linkedNotebook) = 0;
    virtual void addLinkedNotebookFailed(QSharedPointer<LinkedNotebook> linkedNotebook, QString errorDescription) = 0;

    virtual void updateLinkedNotebookComplete(QSharedPointer<LinkedNotebook> linkedNotebook) = 0;
    virtual void updateLinkedNotebookFailed(QSharedPointer<LinkedNotebook> linkedNotebook, QString errorDescription) = 0;

    virtual void findLinkedNotebookComplete(QSharedPointer<LinkedNotebook> foundLinkedNotebook) = 0;
    virtual void findLinkedNotebookFailed(QSharedPointer<LinkedNotebook> linkedNotebook, QString errorDescription) = 0;

    virtual void listAllLinkedNotebooksComplete(QList<LinkedNotebook> foundLinkedNotebooks) = 0;
    virtual void listAllLinkedNotebooksFailed(QString errorDescription) = 0;

    virtual void expungeLinkedNotebookComplete(QSharedPointer<LinkedNotebook> linkedNotebook) = 0;
    virtual void expungeLinkedNotebookFailed(QSharedPointer<LinkedNotebook> linkedNotebook, QString errorDescription) = 0;

    // Prototypes for note-related signals:
    virtual void getNoteCountComplete(int noteCount) = 0;
    virtual void getNoteCountFailed(QString errorDescription) = 0;

    virtual void addNoteComplete(Note note, Notebook notebook) = 0;
    virtual void addNoteFailed(Note note, Notebook notebook, QString errorDescription) = 0;

    virtual void updateNoteComplete(Note note, Notebook notebook) = 0;
    virtual void updateNoteFailed(Note note, Notebook notebook, QString errorDescription) = 0;

    virtual void findNoteComplete(Note foundNote, bool withResourceBinaryData) = 0;
    virtual void findNoteFailed(Note note, bool withResourceBinaryData, QString errorDescription) = 0;

    virtual void listAllNotesPerNotebookComplete(Notebook notebook, bool withResourceBinaryData, QList<Note> foundNotes) = 0;
    virtual void listAllNotesPerNotebookFailed(Notebook notebook, bool withResourceBinaryData, QString errorDescription) = 0;

    virtual void deleteNoteComplete(Note note) = 0;
    virtual void deleteNoteFailed(Note note, QString errorDescription) = 0;

    virtual void expungeNoteComplete(Note note) = 0;
    virtual void expungeNoteFailed(Note note, QString errorDescription) = 0;

    // Prototypes for tag-related signals:
    virtual void getTagCountComplete(int tagCount) = 0;
    virtual void getTagCountFailed(QString errorDescription) = 0;

    virtual void addTagComplete(QSharedPointer<Tag> tag) = 0;
    virtual void addTagFailed(QSharedPointer<Tag> tag, QString errorDescription) = 0;

    virtual void updateTagComplete(QSharedPointer<Tag> tag) = 0;
    virtual void updateTagFailed(QSharedPointer<Tag> tag, QString errorDescription) = 0;

    virtual void linkTagWithNoteComplete(QSharedPointer<Tag> tag, QSharedPointer<Note> note) = 0;
    virtual void linkTagWithNoteFailed(QSharedPointer<Tag> tag, QSharedPointer<Note> note, QString errorDescription) = 0;

    virtual void findTagComplete(QSharedPointer<Tag> tag) = 0;
    virtual void findTagFailed(QSharedPointer<Tag> tag, QString errorDescription) = 0;

    virtual void listAllTagsPerNoteComplete(QList<Tag> foundTags, QSharedPointer<Note> note) = 0;
    virtual void listAllTagsPerNoteFailed(QSharedPointer<Note> note, QString errorDescription) = 0;

    virtual void listAllTagsComplete(QList<Tag> foundTags) = 0;
    virtual void listAllTagsFailed(QString errorDescription) = 0;

    virtual void deleteTagComplete(QSharedPointer<Tag> tag) = 0;
    virtual void deleteTagFailed(QSharedPointer<Tag> tag, QString errorDescription) = 0;

    virtual void expungeTagComplete(QSharedPointer<Tag> tag) = 0;
    virtual void expungeTagFailed(QSharedPointer<Tag> tag, QString errorDescription) = 0;

    // Prototypes for resource-related signals
    virtual void getResourceCountComplete(int resourceCount) = 0;
    virtual void getResourceCountFailed(QString errorDescription) = 0;

    virtual void addResourceComplete(QSharedPointer<ResourceWrapper> resource, QSharedPointer<Note> note) = 0;
    virtual void addResourceFailed(QSharedPointer<ResourceWrapper> resource, QSharedPointer<Note> note,
                                   QString errorDescription) = 0;

    virtual void updateResourceComplete(QSharedPointer<ResourceWrapper> resource, QSharedPointer<Note> note) = 0;
    virtual void updateResourceFailed(QSharedPointer<ResourceWrapper> resource, QSharedPointer<Note> note,
                                      QString errorDescription) = 0;

    virtual void findResourceComplete(QSharedPointer<ResourceWrapper> resource, bool withBinaryData) = 0;
    virtual void findResourceFailed(QSharedPointer<ResourceWrapper> resource, bool withBinaryData,
                                    QString errorDescription) = 0;

    virtual void expungeResourceComplete(QSharedPointer<ResourceWrapper> resource) = 0;
    virtual void expungeResourceFailed(QSharedPointer<ResourceWrapper> resource,
                                       QString errorDescription) = 0;

    // Prototypes for saved search-related signals:
    virtual void getSavedSearchCountComplete(int savedSearchCount) = 0;
    virtual void getSavedSearchCountFailed(QString errorDescription) = 0;

    virtual void addSavedSearchComplete(QSharedPointer<SavedSearch> search) = 0;
    virtual void addSavedSearchFailed(QSharedPointer<SavedSearch> search, QString errorDescription) = 0;

    virtual void updateSavedSearchComplete(QSharedPointer<SavedSearch> search) = 0;
    virtual void updateSavedSearchFailed(QSharedPointer<SavedSearch> search, QString errorDescription) = 0;

    virtual void findSavedSearchComplete(QSharedPointer<SavedSearch> search) = 0;
    virtual void findSavedSearchFailed(QSharedPointer<SavedSearch> search, QString errorDescription) = 0;

    virtual void listAllSavedSearchesComplete(QList<SavedSearch> foundSearches) = 0;
    virtual void listAllSavedSearchesFailed(QString errorDescription) = 0;

    virtual void expungeSavedSearchComplete(QSharedPointer<SavedSearch> search) = 0;
    virtual void expungeSavedSearchFailed(QSharedPointer<SavedSearch> search,
                                          QString errorDescription) = 0;

    // Pure virtual prototypes for slots to be invoked:

    // Pure virtual prototypes for user-related slots:
    virtual void onGetUserCountRequest() = 0;
    virtual void onSwitchUserRequest(QString username, qint32 userId, bool startFromScratch) = 0;
    virtual void onAddUserRequest(QSharedPointer<UserWrapper> user) = 0;
    virtual void onUpdateUserRequest(QSharedPointer<UserWrapper> user) = 0;
    virtual void onFindUserRequest(QSharedPointer<UserWrapper> user) = 0;
    virtual void onDeleteUserRequest(QSharedPointer<UserWrapper> user) = 0;
    virtual void onExpungeUserRequest(QSharedPointer<UserWrapper> user) = 0;

    // Pure virtual prototypes for notebook-related slots:
    virtual void onGetNotebookCountRequest() = 0;
    virtual void onAddNotebookRequest(QSharedPointer<Notebook> notebook) = 0;
    virtual void onUpdateNotebookRequest(QSharedPointer<Notebook> notebook) = 0;
    virtual void onFindNotebookRequest(QSharedPointer<Notebook> notebook) = 0;
    virtual void onFindDefaultNotebookRequest(QSharedPointer<Notebook> notebook) = 0;
    virtual void onFindLastUsedNotebookRequest(QSharedPointer<Notebook> notebook) = 0;
    virtual void onFindDefaultOrLastUsedNotebookRequest(QSharedPointer<Notebook> notebook) = 0;
    virtual void onListAllNotebooksRequest() = 0;
    virtual void onListAllSharedNotebooksRequest() = 0;
    virtual void onListSharedNotebooksPerNotebookGuidRequest(QString notebookGuid) = 0;
    virtual void onExpungeNotebookRequest(QSharedPointer<Notebook> notebook) = 0;

    // Pure virtual prototypes for linked notebook-related slots:
    virtual void onGetLinkedNotebookCountRequest() = 0;
    virtual void onAddLinkedNotebookRequest(QSharedPointer<LinkedNotebook> linkedNotebook) = 0;
    virtual void onUpdateLinkedNotebookRequest(QSharedPointer<LinkedNotebook> linkedNotebook) = 0;
    virtual void onFindLinkedNotebookRequest(QSharedPointer<LinkedNotebook> linkedNotebook) = 0;
    virtual void onListAllLinkedNotebooksRequest() = 0;
    virtual void onExpungeLinkedNotebookRequest(QSharedPointer<LinkedNotebook> linkedNotebook) = 0;

    // Pure virtual prototypes for note-related slots:
    virtual void onGetNoteCountRequest() = 0;
    virtual void onAddNoteRequest(Note note, Notebook notebook) = 0;
    virtual void onUpdateNoteRequest(Note note, Notebook notebook) = 0;
    virtual void onFindNoteRequest(Note note, bool withResourceBinaryData) = 0;
    virtual void onListAllNotesPerNotebookRequest(Notebook notebook, bool withResourceBinaryData) = 0;
    virtual void onDeleteNoteRequest(Note note) = 0;
    virtual void onExpungeNoteRequest(Note note) = 0;

    // Pure virtual prototypes for tag-related slots:
    virtual void onGetTagCountRequest() = 0;
    virtual void onAddTagRequest(QSharedPointer<Tag> tag) = 0;
    virtual void onUpdateTagRequest(QSharedPointer<Tag> tag) = 0;
    virtual void onLinkTagWithNoteRequest(QSharedPointer<Tag> tag, QSharedPointer<Note> note) = 0;
    virtual void onFindTagRequest(QSharedPointer<Tag> tag) = 0;
    virtual void onListAllTagsPerNoteRequest(QSharedPointer<Note> note) = 0;
    virtual void onListAllTagsRequest() = 0;
    virtual void onDeleteTagRequest(QSharedPointer<Tag> tag) = 0;
    virtual void onExpungeTagRequest(QSharedPointer<Tag> tag) = 0;

    // Pure virtual prototypes for resource-related slots:
    virtual void onGetResourceCountRequest() = 0;
    virtual void onAddResourceRequest(QSharedPointer<ResourceWrapper> resource, QSharedPointer<Note> note) = 0;
    virtual void onUpdateResourceRequest(QSharedPointer<ResourceWrapper> resource, QSharedPointer<Note> note) = 0;
    virtual void onFindResourceRequest(QSharedPointer<ResourceWrapper> resource, bool withBinaryData) = 0;
    virtual void onExpungeResourceRequest(QSharedPointer<ResourceWrapper> resource) = 0;

    // Pure virtual prototypes for saved search-related methods:
    virtual void onGetSavedSearchCountRequest() = 0;
    virtual void onAddSavedSearchRequest(QSharedPointer<SavedSearch> search) = 0;
    virtual void onUpdateSavedSearchRequest(QSharedPointer<SavedSearch> search) = 0;
    virtual void onFindSavedSearchRequest(QSharedPointer<SavedSearch> search) = 0;
    virtual void onListAllSavedSearchesRequest() = 0;
    virtual void onExpungeSavedSearch(QSharedPointer<SavedSearch> search) = 0;
};

}

#endif // __QUTE_NOTE__CLIENT__LOCAL_STORAGE__I_ASYNC_LOCAL_STORAGE_MANAGER_H
