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
class Architecture;

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

    Release(ReleaseManager *parent, int index, const QString &name, const QString &summary, const QString &description, Release::Source source, const QString &icon, const QStringList &screenshots, QList<ReleaseVersion*> versions);

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
    QString m_description {};
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
    int m_number;
    ReleaseVersion::Status m_status { FINAL };
    QDateTime m_releaseDate {};
    QList<ReleaseVariant*> m_variants {};
    int m_selectedVariant { 0 };
};


/**
 * @brief The ReleaseVariant class
 */
class ReleaseVariant : public QObject {
    Q_OBJECT
    Q_PROPERTY(Architecture* arch READ arch CONSTANT)
    Q_PROPERTY(ReleaseVariant::Type type READ type CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)

    Q_PROPERTY(QString url READ url CONSTANT)
    Q_PROPERTY(QString shaHash READ shaHash CONSTANT)
    Q_PROPERTY(QString iso READ iso NOTIFY isoChanged)
    Q_PROPERTY(qreal size READ size CONSTANT) // stored as a 64b int, UI doesn't need the precision and QML doesn't support long ints
    Q_PROPERTY(Progress* progress READ progress CONSTANT)
public:
    enum Type {
        LIVE = 0,
        NETINSTALL,
        FULL
    };
    Q_ENUMS(ReleaseVariant::Type)

    ReleaseVariant(ReleaseVersion *parent, QString url, QString shaHash, int64_t size, Architecture *arch, Type type = LIVE);
    ReleaseVersion *releaseVersion();
    Release *release();

    Architecture *arch() const;
    ReleaseVariant::Type type() const;
    QString name() const;

    QString url() const;
    QString shaHash() const;
    QString iso() const;
    qreal size() const;
    Progress *progress();

signals:
    void isoChanged();

public slots:
    void download();

private:
    Architecture *m_arch { nullptr };
    ReleaseVariant::Type m_type { LIVE };
    QString m_url {};
    QString m_shaHash {};
    QString m_iso {};
    int64_t m_size;
    Progress *m_progress { nullptr };
};


/**
 * @brief The Architecture class
 */
class Architecture : public QObject {
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
    static Architecture *fromId(Id id);
    static Architecture *fromAbbreviation(const QString &abbr);
    static QList<Architecture *> listAll();
    static QStringList listAllDescriptions();

    QStringList abbreviation() const;
    QString description() const;
    QString details() const;
    int index() const;

private:
    Architecture(const QStringList &abbreviation, const QString &description, const QString &details);

    static Architecture m_all[];

    const QStringList m_abbreviation {};
    const QString m_description {};
    const QString m_details {};
};

#endif // RELEASEMANAGER_H
