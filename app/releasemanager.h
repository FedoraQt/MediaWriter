/*
 * Fedora Media Writer
 * Copyright (C) 2016 Martin Bříza <mbriza@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef RELEASEMANAGER_H
#define RELEASEMANAGER_H

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QQmlListProperty>

#include <QDateTime>

#include "utilities.h"

class ReleaseManager;
class ReleaseListModel;
class Release;
class ReleaseVersion;
class ReleaseVariant;
class ReleaseArchitecture;

/*
 * Architecture - singleton (x86, x86_64, etc)
 *
 * Release -> Version -> Variant
 *
 * Server  -> 24      -> Full    -> x86_64
 *                               -> i686
 *                    -> Netinst -> x86_64
 *                               -> i686
 *         -> 23      -> Full    -> x86_64
 *                               -> i686
 *                    -> Netinst -> x86_64
 *                               -> i686
 */


/**
 * @brief The ReleaseManager class
 */
class ReleaseManager : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(bool frontPage READ frontPage WRITE setFrontPage NOTIFY frontPageChanged)
    Q_PROPERTY(bool beingUpdated READ beingUpdated NOTIFY beingUpdatedChanged)

    Q_PROPERTY(int filterArchitecture READ filterArchitecture WRITE setFilterArchitecture NOTIFY filterArchitectureChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

    Q_PROPERTY(Release* selected READ selected NOTIFY selectedChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedChanged)

    Q_PROPERTY(QStringList architectures READ architectures CONSTANT)
public:
    explicit ReleaseManager(QObject *parent = 0);
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    Q_INVOKABLE Release *get(int index) const;

    bool beingUpdated() const;

    bool frontPage() const;
    void setFrontPage(bool o);

    QString filterText() const;
    void setFilterText(const QString &o);

    Q_INVOKABLE void setLocalFile(const QString &path);

    QStringList architectures() const;
    int filterArchitecture() const;
    void setFilterArchitecture(int o);

    Release *selected() const;
    int selectedIndex() const;
    void setSelectedIndex(int o);

signals:
    void beingUpdatedChanged();
    void frontPageChanged();
    void filterTextChanged();
    void filterArchitectureChanged();
    void selectedChanged();

private:
    ReleaseListModel *m_sourceModel { nullptr };
    bool m_frontPage { true };
    QString m_filterText {};
    int m_filterArchitecture { 0 };
    int m_selectedIndex { 0 };
};


/**
 * @brief The ReleaseListModel class
 */
class ReleaseListModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit ReleaseListModel(ReleaseManager *parent = 0);
    ReleaseManager *manager();

    Q_INVOKABLE Release *get(int index);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
private:

    QList<Release*> m_releases {};
};


/**
 * @brief The Release class
 */
class Release : public QObject {
    Q_OBJECT
    Q_PROPERTY(int index READ index CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString summary READ summary CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)

    Q_PROPERTY(Source source READ source CONSTANT)
    Q_PROPERTY(bool isLocal READ isLocal CONSTANT)
    Q_PROPERTY(QString category READ sourceString CONSTANT)

    Q_PROPERTY(QString icon READ icon CONSTANT)
    Q_PROPERTY(QStringList screenshots READ screenshots CONSTANT)

    Q_PROPERTY(QQmlListProperty<ReleaseVersion> versions READ versions NOTIFY versionsChanged)
    Q_PROPERTY(QStringList versionNames READ versionNames NOTIFY versionsChanged)
    Q_PROPERTY(ReleaseVersion* version READ selectedVersion NOTIFY selectedVersionChanged)
    Q_PROPERTY(int versionIndex READ selectedVersionIndex WRITE setSelectedVersionIndex NOTIFY selectedVersionChanged)
public:
    enum Source {
        LOCAL,
        PRODUCT,
        SPINS,
        LABS
    };
    Q_ENUMS(Source)
    Q_INVOKABLE QString sourceString();

    Release(ReleaseManager *parent, int index, const QString &name, const QString &summary, const QStringList &description, Release::Source source, const QString &icon, const QStringList &screenshots, QList<ReleaseVersion*> versions);
    void setLocalFile(const QString &path);

    int index() const;
    QString name() const;
    QString summary() const;
    QString description() const;
    Release::Source source() const;
    bool isLocal() const;
    QString icon() const;
    QStringList screenshots() const;

    void addVersion(ReleaseVersion *version);
    QQmlListProperty<ReleaseVersion> versions();
    QList<ReleaseVersion*> versionList() const;
    QStringList versionNames() const;
    ReleaseVersion *selectedVersion() const;
    int selectedVersionIndex() const;
    void setSelectedVersionIndex(int o);

signals:
    void versionsChanged();
    void selectedVersionChanged();
private:
    int m_index { 0 };
    QString m_name {};
    QString m_summary {};
    QStringList m_description {};
    Release::Source m_source { LOCAL };
    QString m_icon {};
    QStringList m_screenshots {};
    QList<ReleaseVersion *> m_versions {};
    int m_selectedVersion { 0 };
};


/**
 * @brief The ReleaseVersion class
 */
class ReleaseVersion : public QObject {
    Q_OBJECT
    Q_PROPERTY(int number READ number CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)

    Q_PROPERTY(ReleaseVersion::Status status READ status CONSTANT)
    Q_PROPERTY(QDateTime releaseDate READ releaseDate CONSTANT)

    Q_PROPERTY(QQmlListProperty<ReleaseVariant> variants READ variants NOTIFY variantsChanged)
    Q_PROPERTY(ReleaseVariant* variant READ selectedVariant NOTIFY selectedVariantChanged)
    Q_PROPERTY(int variantIndex READ selectedVariantIndex WRITE setSelectedVariantIndex NOTIFY selectedVariantChanged)

public:
    enum Status {
        FINAL,
        ALPHA,
        BETA,
        RELEASE_CANDIDATE
    };
    Q_ENUMS(ReleaseVersion::Status)

    ReleaseVersion(Release *parent, int number, QList<ReleaseVariant*> variants, ReleaseVersion::Status status = FINAL, QDateTime releaseDate = QDateTime());
    ReleaseVersion(Release *parent, const QString &file, int64_t size);
    Release *release();

    int number() const;
    QString name() const;
    ReleaseVersion::Status status() const;
    QDateTime releaseDate() const;

    void addVariant(ReleaseVariant *v);
    QQmlListProperty<ReleaseVariant> variants();
    QList<ReleaseVariant*> variantList() const;
    ReleaseVariant *selectedVariant() const;
    int selectedVariantIndex() const;
    void setSelectedVariantIndex(int o);

signals:
    void variantsChanged();
    void selectedVariantChanged();

private:
    int m_number { 0 };
    ReleaseVersion::Status m_status { FINAL };
    QDateTime m_releaseDate {};
    QList<ReleaseVariant*> m_variants {};
    int m_selectedVariant { 0 };
};


/**
 * @brief The ReleaseVariant class
 */
class ReleaseVariant : public QObject, public DownloadReceiver {
    Q_OBJECT
    Q_PROPERTY(ReleaseArchitecture* arch READ arch CONSTANT)
    Q_PROPERTY(ReleaseVariant::Type type READ type CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)

    Q_PROPERTY(QString url READ url CONSTANT)
    Q_PROPERTY(QString shaHash READ shaHash CONSTANT)
    Q_PROPERTY(QString iso READ iso NOTIFY isoChanged)
    Q_PROPERTY(qreal size READ size CONSTANT) // stored as a 64b int, UI doesn't need the precision and QML doesn't support long ints
    Q_PROPERTY(Progress* progress READ progress CONSTANT)

    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged)
public:
    enum Type {
        LIVE = 0,
        NETINSTALL,
        FULL
    };
    Q_ENUMS(Type)
    enum Status {
        PREPARING = 0,
        DOWNLOADING,
        DOWNLOAD_VERIFYING,
        READY,
        WRITING,
        FINISHED,
        FAILED_DOWNLOAD,
        FAILED
    };
    Q_ENUMS(Status)
    const QStringList m_statusStrings {
        tr("Preparing"),
        tr("Downloading"),
        tr("Checking the download"),
        tr("Ready to write"),
        tr("Writing"),
        tr("Finished!"),
        tr("Download failed"),
        tr("Error")
    };

    ReleaseVariant(ReleaseVersion *parent, QString url, QString shaHash, int64_t size, ReleaseArchitecture *arch, Type type = LIVE);
    ReleaseVariant(ReleaseVersion *parent, const QString &file, int64_t size);

    ReleaseVersion *releaseVersion();
    Release *release();

    ReleaseArchitecture *arch() const;
    ReleaseVariant::Type type() const;
    QString name() const;

    QString url() const;
    QString shaHash() const;
    QString iso() const;
    qreal size() const;
    Progress *progress();

    Status status() const;
    QString statusString() const;
    void setStatus(Status s);
    QString errorString() const;
    void setErrorString(const QString &o);

    // DownloadReceiver interface
    void onFileDownloaded(const QString &path) override;
    virtual void onDownloadError(const QString &message) override;

    static int staticOnMediaCheckAdvanced(void *data, long long offset, long long total);
    int onMediaCheckAdvanced(long long offset, long long total);

signals:
    void isoChanged();
    void statusChanged();
    void errorStringChanged();

public slots:
    void download();
    void resetStatus();

private:
    QString m_iso {};
    ReleaseArchitecture *m_arch { nullptr };
    ReleaseVariant::Type m_type { LIVE };
    QString m_url {};
    QString m_shaHash {};
    int64_t m_size { 0 };
    Status m_status { PREPARING };
    QString m_error {};

    Progress *m_progress { nullptr };
};


/**
 * @brief The ReleaseArchitecture class
 */
class ReleaseArchitecture : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList abbreviation READ abbreviation CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QString details READ details CONSTANT)
public:
    enum Id {
        X86_64 = 0,
        X86,
        ARM,
        _ARCHCOUNT,
    };
    static ReleaseArchitecture *fromId(Id id);
    static ReleaseArchitecture *fromAbbreviation(const QString &abbr);
    static QList<ReleaseArchitecture *> listAll();
    static QStringList listAllDescriptions();

    QStringList abbreviation() const;
    QString description() const;
    QString details() const;
    int index() const;

private:
    ReleaseArchitecture(const QStringList &abbreviation, const char *description, const char *details);

    static ReleaseArchitecture m_all[];

    const QStringList m_abbreviation {};
    const char *m_description {};
    const char *m_details {};
};

#endif // RELEASEMANAGER_H
