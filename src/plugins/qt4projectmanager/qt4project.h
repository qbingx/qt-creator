/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: http://www.qt-project.org/
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**************************************************************************/

#ifndef QT4PROJECT_H
#define QT4PROJECT_H

#include "qt4projectmanager_global.h"

#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>

#include <QStringList>
#include <QMap>
#include <QFutureInterface>
#include <QTimer>
#include <QFuture>

QT_BEGIN_NAMESPACE
class QMakeGlobals;
QT_END_NAMESPACE

namespace ProjectExplorer { class DeploymentData; }
namespace QtSupport { class ProFileReader; }

namespace Qt4ProjectManager {
class BuildConfigurationInfo;
class MakeStep;
class QMakeStep;
class Qt4BuildConfiguration;
class Qt4Manager;
class Qt4PriFileNode;
class Qt4ProFileNode;
class Qt4RunStep;

namespace Internal {
class CentralizedFolderWatcher;
class FileItem;
class GCCPreprocessor;
class Qt4ProjectFiles;
class Qt4ProjectConfigWidget;
class Qt4ProjectFile;
class Qt4NodesWatcher;
}

class  QT4PROJECTMANAGER_EXPORT Qt4Project : public ProjectExplorer::Project
{
    Q_OBJECT

public:
    Qt4Project(Qt4Manager *manager, const QString &proFile);
    virtual ~Qt4Project();

    QString displayName() const;
    Core::Id id() const;
    Core::IDocument *document() const;
    ProjectExplorer::IProjectManager *projectManager() const;
    Qt4Manager *qt4ProjectManager() const;

    bool supportsKit(ProjectExplorer::Kit *p) const;

    ProjectExplorer::ProjectNode *rootProjectNode() const;
    Qt4ProFileNode *rootQt4ProjectNode() const;
    bool validParse(const QString &proFilePath) const;
    bool parseInProgress(const QString &proFilePath) const;

    virtual QStringList files(FilesMode fileMode) const;
    virtual QString generatedUiHeader(const QString &formFile) const;

    QList<ProjectExplorer::BuildConfigWidget*> subConfigWidgets();

    QList<Qt4ProFileNode *> allProFiles() const;
    QList<Qt4ProFileNode *> applicationProFiles() const;
    bool hasApplicationProFile(const QString &path) const;
    QStringList applicationProFilePathes(const QString &prepend = QString()) const;

    void notifyChanged(const QString &name);

    /// \internal
    QtSupport::ProFileReader *createProFileReader(Qt4ProFileNode *qt4ProFileNode, Qt4BuildConfiguration *bc = 0);
    /// \internal
    QMakeGlobals *qmakeGlobals();
    /// \internal
    void destroyProFileReader(QtSupport::ProFileReader *reader);

    /// \internal
    void scheduleAsyncUpdate(Qt4ProjectManager::Qt4ProFileNode *node);
    /// \internal
    void incrementPendingEvaluateFutures();
    /// \internal
    void decrementPendingEvaluateFutures();
    /// \internal
    bool wasEvaluateCanceled();

    // For Qt4ProFileNode after a on disk change
    void updateFileList();
    void updateCodeModels();

    void watchFolders(const QStringList &l, Qt4PriFileNode *node);
    void unwatchFolders(const QStringList &l, Qt4PriFileNode *node);

    bool needsConfiguration() const;

    void configureAsExampleProject(const QStringList &platforms);

    bool supportsNoTargetPanel() const;

    /// \internal
    QString disabledReasonForRunConfiguration(const QString &proFilePath);

    /// suffix should be unique
    static QString shadowBuildDirectory(const QString &profilePath, const ProjectExplorer::Kit *p,
                                 const QString &suffix);
    /// used by the default implementation of shadowBuildDirectory
    static QString buildNameFor(const ProjectExplorer::Kit *p);

    ProjectExplorer::Target *createTarget(ProjectExplorer::Kit *p, const QList<BuildConfigurationInfo> &infoList);

    void emitBuildDirectoryInitialized();

signals:
    void kitUpdated(Qt4ProjectManager::Qt4ProFileNode *node, bool, bool);
    void buildDirectoryInitialized();
    void proFilesEvaluated();

public slots:
    void scheduleAsyncUpdate();
    void proFileParseError(const QString &errorMessage);
    void update();

protected:
    bool fromMap(const QVariantMap &map);

private slots:
    void asyncUpdate();

    void activeTargetWasChanged();

private:
    void updateRunConfigurations();

    void updateCppCodeModel();
    void updateQmlJSCodeModel();

    static void collectAllfProFiles(QList<Qt4ProFileNode *> &list, Qt4ProFileNode *node);
    static void collectApplicationProFiles(QList<Qt4ProFileNode *> &list, Qt4ProFileNode *node);
    static void findProFile(const QString& fileName, Qt4ProFileNode *root, QList<Qt4ProFileNode *> &list);
    static bool hasSubNode(Qt4PriFileNode *root, const QString &path);

    static bool equalFileList(const QStringList &a, const QStringList &b);

    static QString qmakeVarName(ProjectExplorer::FileType type);

    void updateBuildSystemData();
    void collectData(const Qt4ProFileNode *node, ProjectExplorer::DeploymentData &deploymentData);
    void collectLibraryData(const Qt4ProFileNode *node,
            ProjectExplorer::DeploymentData &deploymentData);

    Qt4Manager *m_manager;
    Qt4ProFileNode *m_rootProjectNode;
    Internal::Qt4NodesWatcher *m_nodesWatcher;

    Internal::Qt4ProjectFile *m_fileInfo;

    // Current configuration
    QString m_oldQtIncludePath;
    QString m_oldQtLibsPath;

    // cached lists of all of files
    Internal::Qt4ProjectFiles *m_projectFiles;

    // cached data during project rescan
    QMakeGlobals *m_qmakeGlobals;
    int m_qmakeGlobalsRefCnt;

    QTimer m_asyncUpdateTimer;
    QFutureInterface<void> *m_asyncUpdateFutureInterface;
    int m_pendingEvaluateFuturesCount;
    enum AsyncUpdateState { NoState, Base, AsyncFullUpdatePending, AsyncPartialUpdatePending, AsyncUpdateInProgress, ShuttingDown };
    AsyncUpdateState m_asyncUpdateState;
    bool m_cancelEvaluate;
    bool m_codeModelCanceled;
    QList<Qt4ProFileNode *> m_partialEvaluate;

    QFuture<void> m_codeModelFuture;

    Internal::CentralizedFolderWatcher *m_centralizedFolderWatcher;

    ProjectExplorer::Target *m_activeTarget;

    friend class Internal::Qt4ProjectFile;
    friend class Internal::Qt4ProjectConfigWidget;
    friend class Qt4Manager; // to schedule a async update if the unconfigured settings change
};

} // namespace Qt4ProjectManager


#endif // QT4PROJECT_H
