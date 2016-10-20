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

#include "releasemanager.h"

#include "isomd5/libcheckisomd5.h"

#include <QtQml>
#include <QApplication>
#include <QAbstractEventDispatcher>

#include <QJsonDocument>

ReleaseManager::ReleaseManager(QObject *parent)
    : QSortFilterProxyModel(parent), m_sourceModel(new ReleaseListModel(this))
{
    setSourceModel(m_sourceModel);

    qmlRegisterUncreatableType<Release>("MediaWriter", 1, 0, "Release", "");
    qmlRegisterUncreatableType<ReleaseVersion>("MediaWriter", 1, 0, "Version", "");
    qmlRegisterUncreatableType<ReleaseVariant>("MediaWriter", 1, 0, "Variant", "");
    qmlRegisterUncreatableType<ReleaseArchitecture>("MediaWriter", 1, 0, "Architecture", "");
    qmlRegisterUncreatableType<Progress>("MediaWriter", 1, 0, "Progress", "");

    QFile releases(":/releases.json");
    releases.open(QIODevice::ReadOnly);
    onStringDownloaded(releases.readAll());
    releases.close();

    QTimer::singleShot(0, this, &ReleaseManager::fetchReleases);
}

bool ReleaseManager::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    Q_UNUSED(source_parent)
    if (m_frontPage)
        if (source_row < 3)
            return true;
        else
            return false;
    else {
        auto r = get(source_row);
        bool containsArch = false;
        for (auto version : r->versionList()) {
            for (auto variant : version->variantList()) {
                if (variant->arch()->index() == m_filterArchitecture) {
                    containsArch = true;
                    break;
                }
            }
            if (containsArch)
                break;
        }
        return r->isLocal() || (containsArch && (r->name().contains(m_filterText, Qt::CaseInsensitive) || r->summary().contains(m_filterText, Qt::CaseInsensitive)));
    }
}

Release *ReleaseManager::get(int index) const {
    return m_sourceModel->get(index);
}

void ReleaseManager::fetchReleases() {
    m_beingUpdated = true;
    emit beingUpdatedChanged();

    DownloadManager::instance()->fetchPageAsync(this, "https://tmp.elrod.me/script-output.json");
}

bool ReleaseManager::beingUpdated() const {
    return m_beingUpdated;
}

bool ReleaseManager::frontPage() const {
    return m_frontPage;
}

void ReleaseManager::setFrontPage(bool o) {
    if (m_frontPage != o) {
        m_frontPage = o;
        emit frontPageChanged();
        invalidateFilter();
    }
}

QString ReleaseManager::filterText() const {
    return m_filterText;
}

void ReleaseManager::setFilterText(const QString &o) {
    if (m_filterText != o) {
        m_filterText = o;
        emit filterTextChanged();
        invalidateFilter();
    }
}

void ReleaseManager::setLocalFile(const QString &path) {
    for (int i = 0; i < m_sourceModel->rowCount(); i++) {
        Release *r = m_sourceModel->get(i);
        if (r->source() == Release::LOCAL) {
            r->setLocalFile(path);
        }
    }
}

bool ReleaseManager::updateUrl(const QString &release, int version, const QString &status, const QDateTime &releaseDate, const QString &architecture, const QString &url, const QString &sha256, int64_t size) {
    for (int i = 0; i < m_sourceModel->rowCount(); i++) {
        Release *r = get(i);
        if (r->name().toLower().contains(release))
            return r->updateUrl(version, status, releaseDate, architecture, url, sha256, size);
    }
    return false;
}

int ReleaseManager::filterArchitecture() const {
    return m_filterArchitecture;
}

void ReleaseManager::setFilterArchitecture(int o) {
    if (m_filterArchitecture != o && m_filterArchitecture >= 0 && m_filterArchitecture < ReleaseArchitecture::_ARCHCOUNT) {
        m_filterArchitecture = o;
        emit filterArchitectureChanged();
        invalidateFilter();
    }
}

Release *ReleaseManager::selected() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < m_sourceModel->rowCount())
        return m_sourceModel->get(m_selectedIndex);
    return nullptr;
}

int ReleaseManager::selectedIndex() const {
    return m_selectedIndex;
}

void ReleaseManager::setSelectedIndex(int o) {
    if (m_selectedIndex != o) {
        m_selectedIndex = o;
        emit selectedChanged();
    }
}

void ReleaseManager::onStringDownloaded(const QString &text) {
    QRegExp re("(\\d+)\\s?(\\S+)?");
    auto doc = QJsonDocument::fromJson(text.toUtf8());

    for (auto i : doc.array()) {
        QJsonObject obj = i.toObject();
        QString arch = obj["arch"].toString().toLower();
        QString url = obj["link"].toString();
        QString release = obj["subvariant"].toString().toLower();
        QString versionWithStatus = obj["version"].toString().toLower();
        QString sha256 = obj["sha256"].toString();
        QDateTime releaseDate = QDateTime::fromString((obj["releaseDate"].toString()), "yyyy-MM-dd");
        int64_t size = obj["size"].toString().toLongLong();
        int version;
        QString status;

        if (QStringList{"cloud", "cloud_base", "atomic", "everything", "minimal", "docker", "docker_base"}.contains(release))
            continue;

        if (arch == "armhfp")
            continue;

        if (re.indexIn(versionWithStatus) < 0)
            continue;

        if (release.contains("workstation") && !url.contains("Live"))
            continue;

        if (release.contains("server") && !url.contains("dvd"))
            continue;

        version = re.capturedTexts()[1].toInt();
        status = re.capturedTexts()[2];

        updateUrl(release, version, status, releaseDate, arch, url, sha256, size);
    }

    m_beingUpdated = false;
    emit beingUpdatedChanged();
}

void ReleaseManager::onDownloadError(const QString &message) {
    qWarning() << "Was not able to fetch new releases:" << message << "Retrying in 10 seconds.";

    QTimer::singleShot(10000, this, &ReleaseManager::fetchReleases);
}

QStringList ReleaseManager::architectures() const {
    return ReleaseArchitecture::listAllDescriptions();
}


QVariant ReleaseListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(section); Q_UNUSED(orientation);

    if (role == Qt::UserRole + 1)
        return "release";

    return QVariant();
}

QHash<int, QByteArray> ReleaseListModel::roleNames() const {
    QHash<int, QByteArray> ret;
    ret.insert(Qt::UserRole + 1, "release");
    return ret;
}

int ReleaseListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_releases.count();
}

QVariant ReleaseListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role == Qt::UserRole + 1)
        return QVariant::fromValue(m_releases[index.row()]);

    return QVariant();
}


ReleaseListModel::ReleaseListModel(ReleaseManager *parent)
    : QAbstractListModel(parent) {
    // move this to a separate json too
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Workstation", "This is the Linux workstation you've been waiting for.", { "<p>", "Fedora Workstation is a reliable, user-friendly, and powerful operating system for your laptop or desktop computer. It supports a wide range of developers, from hobbyists and students to professionals in corporate environments.", "</p><blockquote><p>", "“The plethora of tools provided by<br /> Fedora allows me to get the job done.<br /> It just works.”", "</p><p align=right> ― <em>Christine Flood, ", "JVM performance engineer", "</em></p></blockquote><h3>", "Sleek user interface", "</h3><p>", "Focus on your code in the GNOME 3 desktop environment. GNOME is built with developer feedback and minimizes distractions, so you can concentrate on what's important.", "</p><h3>", "Complete open source toolbox", "</h3><p>", "Skip the drag of trying to find or build the tools you need. With Fedora's complete set of open source languages, tools, and utilities, everything is a click or command line away. There's even project hosting and repositories like COPR to make your code and builds available quickly to the community.", "</p><h3>", "GNOME Boxes & other virt tools", "</h3><p>", "Get virtual machines up and running quickly to test your code on multiple platforms using GNOME Boxes. Or dig into powerful, scriptable virtualization tools for even more control.", "</p><h3>", "Built-in Docker support", "</h3><p>", "Containerize your own apps, or deploy containerized apps out of the box on Fedora, using the latest technology like Docker.", "</p>" }, Release::PRODUCT, "qrc:/logos/workstation", {}, {} });
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Server", "The latest technology. A stable foundation. Together, for your applications and services.", { "<blockquote><p>", "“The simplicity introduced with rolekit and cockpit have made server deployments a breeze. What took me a few days on other operating systems took less than an hour with Fedora %(rel)s Server. It just works.”", "</p><p align=right> ― <em>Dan Mossor, ", "Systems Engineer", "</em></p></blockquote><p>", "Fedora Server is a short-lifecycle, community-supported server operating system that enables seasoned system administrators experienced with any OS to make use of the very latest server-based technologies available in the open source community.", "</p><h3>", "Easy Administration", "</h3><p>", "Manage your system simply with Cockpit's powerful, modern interface. View and monitor system performance and status, and deploy and manage container-based services.", "</p><h3>", "Server Roles", "</h3><p>", "There's no need to set up your server from scratch when you use server roles. Server roles plug into your Fedora Server system, providing a well-integrated service on top of the Fedora Server platform. Deploy and manage these prepared roles simply using the Rolekit tool.", "</p><h3>", "Database Services", "</h3><p>", "Fedora Server brings with it an enterprise-class, scalable database server powered by the open-source PostgreSQL project.", "</p><h3>", "Complete Enterprise Domain Solution", "</h3><p>", "Level up your Linux network with advanced identity management, DNS, certificate services, Windows(TM) domain integration throughout your environment with FreeIPA, the engine that drives Fedora Server's Domain Controller role.", "</p><blockquote><p>", "“The Docker Role for Fedora Server was simple and fast to install so that you can run your Docker images. This makes a great testbed for beginners and experts with docker so that they can develop their applications on the fly.”", "</p><p align=right> ― <em>John Unland, ", "Information Systems Student", "</em></p></blockquote>" }, Release::PRODUCT, "qrc:/logos/server", {}, {} });
    m_releases.append(new Release {manager(), m_releases.length(), tr("Custom image"), QT_TRANSLATE_NOOP("Release", "Pick a file from your drive(s)"), { QT_TRANSLATE_NOOP("Release", "<p>Here you can choose a OS image from your hard drive to be written to your flash disk</p><p>Currently it is only supported to write raw disk images (.iso or .bin)</p>") }, Release::LOCAL, "qrc:/logos/folder", {}, {} });
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora KDE Plasma Desktop", "A complete, modern desktop built using the KDE Plasma Desktop.", { "<p>", "The Fedora KDE Plasma Desktop Edition is a powerful Fedora-based operating system utilizing the KDE Plasma Desktop as the main user interface.", "</p><p>", "Fedora KDE Plasma Desktop comes with many pre-selected top quality applications that suit all modern desktop use cases - from online communication like web browsing, instant messaging and electronic mail correspondence, through multimedia and entertainment, to an advanced productivity suite, including office applications and enterprise grade personal information management.", "</p><p>", "All KDE applications are well integrated, with a similar look and feel and an easy to use interface, accompanied by an outstanding graphical appearance.", "</p>" }, Release::SPINS, "qrc:/logos/plasma", { "https://spins.stg.fedoraproject.org/en/kde/../static/images/screenshots/screenshot-kde.jpg" }, {} });
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Xfce Desktop", "A complete, well-integrated Xfce Desktop.", { "<p>", "The Fedora Xfce spin showcases the Xfce desktop, which aims to be fast and lightweight, while still being visually appealing and user friendly.", "</p><p>", "Fedora Xfce is a full-fledged desktop using the freedesktop.org standards.", "</p>"} , Release::SPINS, "qrc:/logos/xfce", { "https://spins.stg.fedoraproject.org/en/xfce/../static/images/screenshots/screenshot-xfce.jpg" }, {} });
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora LXDE Desktop", "A light, fast, less-resource hungry desktop environment.", { "<p>", "LXDE, the \"Lightweight X11 Desktop Environment\", is an extremely fast, performant, and energy-saving desktop environment. It maintained by an international community of developers and comes with a beautiful interface, multi-language support, standard keyboard shortcuts and additional features like tabbed file browsing.", "</p><p>", "LXDE is not designed to be powerful and bloated, but to be usable and slim. A main goal of LXDE is to keep computer resource usage low. It is especially designed for computers with low hardware specifications like netbooks, mobile devices (e.g. MIDs) or older computers.", "</p>" }, Release::SPINS, "qrc:/logos/lxde", { "https://spins.stg.fedoraproject.org/en/lxde/../static/images/screenshots/screenshot-lxde.jpg" }, {} });
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora MATE-Compiz Desktop", "A classic Fedora Desktop with an additional 3D Windows Manager.", { "<p>", "The MATE Compiz spin bundles MATE Desktop with Compiz Fusion. MATE Desktop is a lightweight, powerful desktop designed with productivity and performance in mind. The default windows manager is Marco which is usable for all machines and VMs. Compiz Fusion is a beautiful 3D windowing manager with Emerald and GTK+ theming.", "</p><p>", "If you want a powerful, lightweight Fedora desktop with 3D eyecandy you should definitely try the MATE-Compiz spin.", "</p>" }, Release::SPINS, "qrc:/logos/mate", { "https://spins.stg.fedoraproject.org/en/mate-compiz/../static/images/screenshots/screenshot-matecompiz.jpg" }, {} });
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Cinnamon Desktop", "A modern desktop featuring traditional Gnome user experience.", { "<p>", "Cinnamon is a Linux desktop which provides advanced innovative features and a traditional user experience. The desktop layout is similar to Gnome 2. The underlying technology is forked from Gnome Shell. The emphasis is put on making users feel at home and providing them with an easy to use and comfortable desktop experience.", "</p><p>", "Cinnamon is a popular desktop alternative to Gnome 3 and this spin provides the option to quickly try and install this desktop.", "</p>" }, Release::SPINS, "qrc:/logos/cinnamon", { "https://spins.stg.fedoraproject.org/en/cinnamon/../static/images/screenshots/screenshot-cinnamon.jpg" }, {} });
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora SoaS Desktop", "Discover. Reflect. Share. Learn.", { "<p>", "Sugar on a Stick is a Fedora-based operating system featuring the award-winning Sugar Learning Platform and designed to fit on an ordinary USB thumbdrive (\"stick\").", "</p><p>", "Sugar sets aside the traditional “office-desktop” metaphor, presenting a child-friendly graphical environment. Sugar automatically saves your progress to a \"Journal\" on your stick, so teachers and parents can easily pull up \"all collaborative web browsing sessions done in the past week\" or \"papers written with Daniel and Sarah in the last 24 hours\" with a simple query rather than memorizing complex file/folder structures. Applications in Sugar are known as Activities, some of which are described below.", "</p><p>", "It is now deployable for the cost of a stick rather than a laptop; students can take their Sugar on a Stick thumbdrive to any machine - at school, at home, at a library or community center - and boot their customized computing environment without touching the host machine’s hard disk or existing system at all.", "</p>" }, Release::SPINS, "qrc:/logos/soas", { "https://spins.stg.fedoraproject.org/en/soas/../static/images/screenshots/screenshot-soas.jpg" }, {} });
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Astronomy", "Powerful completely open-source and free tool for amateurs and professionals.", { "<p>", "Fedora Astronomy brings a complete open source toolchain to both amateur and professional astronomers.", "</p><p>", "The Spin provides the Fedora KDE desktop enhanced with a complete scientific Python environment and the AstrOmatic software for data analysis. KStars was added to provide a full featured astrophotography tool. As KStars uses the INDI library to control equipment, various telescopes, cameras etc. are supported. Summarized, Fedora Astronomy provides a complete set of software, from the observation planning to the final results.", "</p>" }, Release::LABS, "qrc:/logos/astronomy", { }, {} });
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Design Suite", "Visual design, multimedia production, and publishing suite of free and open source creative tools.", { "<p>", "Looking for a ready-to-go desktop environment brimming with free and open source multimedia production and publishing tools? Try the Design Suite, a Fedora Spin created by designers, for designers.", "</p><p>", "The Design Suite includes the favorite tools of the Fedora Design Team. These are the same programs we use to create all the artwork that you see within the Fedora Project, from desktop backgrounds to CD sleeves, web page designs, application interfaces, flyers, posters and more. From document publication to vector and bitmap editing or 3D modeling to photo management, the Design Suite has an application for you — and you can install thousands more from the Fedora universe of packages.", "</p>" }, Release::LABS, "qrc:/logos/design", { }, {} });
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Games", "A collection and perfect show-case of the best games available in Fedora.", { "<p>", "The Fedora Games spin offers a perfect showcase of the best games available in Fedora. The included games span several genres, from first-person shooters to real-time and turn-based strategy games to puzzle games.", "</p><p>", "Not all the games available in Fedora are included on this spin, but trying out this spin will give you a fair impression of Fedora's ability to run great games.", "</p>" }, Release::LABS, "qrc:/logos/games", {}, {} });
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Robotics Suite", "A wide variety of free and open robotics software packages for beginners and experts in robotics.", { "<p>", "The Fedora Robotics spin provides a wide variety of free and open robotics software packages. These range from hardware accessory libraries for the Hokuyo laser scanners or Katana robotic arm to software frameworks like Fawkes or Player and simulation environments such as Stage and RoboCup Soccer Simulation Server 2D/3D. It also provides a ready to use development environment for robotics including useful libraries such as OpenCV computer vision library, Festival text to speech system and MRPT.", "</p><p>", "The Robotics spin is targeted at people just discovering their interest in robotics as well as experienced roboticists. For the former we provide a readily usable simulation environment with an introductory hands-on demonstration, and for the latter we provide a full development environment, to be used immediately.", "</p>" }, Release::LABS, "qrc:/logos/robotics", {}, {} });
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Scientific", "A bundle of open source scientific and numerical tools used in research.", { "<p>", "Wary of reinstalling all the essential tools for your scientific and numerical work? The answer is here. Fedora Scientific Spin brings together the most useful open source scientific and numerical tools atop the goodness of the KDE desktop environment.", "</p><p>", "Fedora Scientific currently ships with numerous applications and libraries. These range from libraries such as the GNU Scientific library, the SciPy libraries, tools like Octave and xfig to typesetting tools like Kile and graphics programs such as Inkscape. The current set of packages include an IDE, tools and libraries for programming in C, C++, Python, Java and R. Also included along with are libraries for parallel computing such as the OpenMPI and OpenMP. Tools for typesetting, writing and publishing are included.", "</p>" }, Release::LABS, "qrc:/logos/scientific", {}, {} });
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Security Lab", "A safe test environment to work on security auditing, forensics, system rescue and teaching security testing methodologies.", { "<p>", "The Fedora Security Lab provides a safe test environment to work on security auditing, forensics, system rescue and teaching security testing methodologies in universities and other organizations.", "</p><p>", "The spin is maintained by a community of security testers and developers. It comes with the clean and fast Xfce Desktop Environment and a customized menu that provides all the instruments needed to follow a proper test path for security testing or to rescue a broken system. The Live image has been crafted to make it possible to install software while running, and if you are running it from a USB stick created with LiveUSB Creator using the overlay feature, you can install and update software and save your test results permanently.", "</p>" }, Release::LABS, "qrc:/logos/security", {}, {} });
}

ReleaseManager *ReleaseListModel::manager() {
    return qobject_cast<ReleaseManager*>(parent());
}

Release *ReleaseListModel::get(int index) {
    if (index >= 0 && index < m_releases.count())
        return m_releases[index];
    return nullptr;
}


QString Release::sourceString() {
    switch (m_source) {
    case LOCAL:
    case PRODUCT:
        return QString();
    case SPINS:
        return tr("Fedora Spins");
    case LABS:
        return tr("Fedora Labs");
    default:
        return QString();
    }
}

int Release::index() const {
    return m_index;
}

Release::Release(ReleaseManager *parent, int index, const QString &name, const QString &summary, const QStringList &description, Release::Source source, const QString &icon, const QStringList &screenshots, QList<ReleaseVersion *> versions)
    : QObject(parent), m_index(index), m_name(name), m_summary(summary), m_description(description), m_source(source), m_icon(icon), m_screenshots(screenshots), m_versions(versions)
{

}

void Release::setLocalFile(const QString &path) {
    if (m_source != LOCAL)
        return;

    QFileInfo info(QUrl(path).toLocalFile());

    if (!info.exists()) {
        qCritical() << path << "doesn't exist";
        return;
    }

    if (m_versions.count() == 1) {
        m_versions.first()->deleteLater();
        m_versions.removeFirst();
    }

    m_versions.append(new ReleaseVersion(this, QUrl(path).toLocalFile(), info.size()));
    emit versionsChanged();
    emit selectedVersionChanged();
}

bool Release::updateUrl(int version, const QString &status, const QDateTime &releaseDate, const QString &architecture, const QString &url, const QString &sha256, int64_t size) {
    for (auto i : m_versions) {
        if (i->number() == version)
            return i->updateUrl(status, releaseDate, architecture, url, sha256, size);
    }
    ReleaseVersion::Status s = status == "alpha" ? ReleaseVersion::ALPHA : status == "beta" ? ReleaseVersion::BETA : ReleaseVersion::FINAL;
    auto ver = new ReleaseVersion(this, version, { }, s, releaseDate);
    auto variant = new ReleaseVariant(ver, url, sha256, size, ReleaseArchitecture::fromAbbreviation(architecture));
    ver->addVariant(variant);
    addVersion(ver);
    return true;
}

QString Release::name() const {
    return m_name;
}

QString Release::summary() const {
    return tr(m_summary.toUtf8());
}

QString Release::description() const {
    QString result;
    for (auto i : m_description) {
        // there is a %(rel)s formatting string in the translation texts, get rid of that
        // get rid of in-translation break tags too
        result.append(tr(i.toUtf8()).replace("\%(rel)s ", "").replace("<br />", ""));
    }
    return result;
}

Release::Source Release::source() const {
    return m_source;
}

bool Release::isLocal() const {
    return m_source == Release::LOCAL;
}

QString Release::icon() const {
    return m_icon;
}

QStringList Release::screenshots() const {
    return m_screenshots;
}

QString Release::prerelease() const {
    if (m_versions.empty() || m_versions.first()->status() == ReleaseVersion::FINAL)
        return "";
    return m_versions.first()->name();
}

QQmlListProperty<ReleaseVersion> Release::versions() {
    return QQmlListProperty<ReleaseVersion>(this, m_versions);
}

QList<ReleaseVersion *> Release::versionList() const {
    return m_versions;
}

QStringList Release::versionNames() const {
    QStringList ret;
    for (auto i : m_versions) {
        ret.append(i->name());
    }
    return ret;
}

void Release::addVersion(ReleaseVersion *version) {
    for (int i = 0; i < m_versions.count(); i++) {
        if (m_versions[i]->number() < version->number()) {
            m_versions.insert(i, version);
            emit versionsChanged();
            if (version->status() != ReleaseVersion::FINAL && m_selectedVersion >= i) {
                m_selectedVersion++;
                emit selectedVersionChanged();
            }
            return;
        }
    }
    m_versions.append(version);
    emit versionsChanged();
    emit selectedVersionChanged();
}

ReleaseVersion *Release::selectedVersion() const {
    if (m_selectedVersion >= 0 && m_selectedVersion < m_versions.count())
        return m_versions[m_selectedVersion];
    return nullptr;
}

int Release::selectedVersionIndex() const {
    return m_selectedVersion;
}

void Release::setSelectedVersionIndex(int o) {
    if (m_selectedVersion != o && m_selectedVersion >= 0 && m_selectedVersion < m_versions.count()) {
        m_selectedVersion = o;
        emit selectedVersionChanged();
    }
}


ReleaseVersion::ReleaseVersion(Release *parent, int number, QList<ReleaseVariant *> variants, ReleaseVersion::Status status, QDateTime releaseDate)
    : QObject(parent), m_number(number), m_status(status), m_releaseDate(releaseDate), m_variants(variants)
{
    if (status != FINAL)
        emit parent->prereleaseChanged();
}

ReleaseVersion::ReleaseVersion(Release *parent, const QString &file, int64_t size)
    : QObject(parent), m_variants({ new ReleaseVariant(this, file, size) })
{

}

Release *ReleaseVersion::release() {
    return qobject_cast<Release*>(parent());
}

bool ReleaseVersion::updateUrl(const QString &status, const QDateTime &releaseDate, const QString &architecture, const QString &url, const QString &sha256, int64_t size) {
    Status s = status == "alpha" ? ALPHA : status == "beta" ? BETA : FINAL;
    if (s <= m_status) {
        m_status = s;
        emit statusChanged();
        if (s == FINAL)
            emit release()->prereleaseChanged();
    }
    else {
        return false;
    }
    if (m_releaseDate != releaseDate && releaseDate.isValid()) {
        m_releaseDate = releaseDate;
        emit releaseDateChanged();
    }
    for (auto i : m_variants) {
        if (i->arch() == ReleaseArchitecture::fromAbbreviation(architecture))
            return i->updateUrl(url, sha256, size);
    }
    m_variants.append(new ReleaseVariant(this, url, sha256, size, ReleaseArchitecture::fromAbbreviation(architecture)));
    return true;
}

int ReleaseVersion::number() const {
    return m_number;
}

QString ReleaseVersion::name() const {
    switch (m_status) {
    case ALPHA:
        return tr("%1 Alpha").arg(m_number);
    case BETA:
        return tr("%1 Beta").arg(m_number);
    case RELEASE_CANDIDATE:
        return tr("%1 Release Candidate").arg(m_number);
    default:
        return QString("%1").arg(m_number);
    }
}

ReleaseVariant *ReleaseVersion::selectedVariant() const {
    if (m_selectedVariant >= 0 && m_selectedVariant < m_variants.count())
        return m_variants[m_selectedVariant];
    return nullptr;
}

int ReleaseVersion::selectedVariantIndex() const {
    return m_selectedVariant;
}

void ReleaseVersion::setSelectedVariantIndex(int o) {
    if (m_selectedVariant != o && m_selectedVariant >= 0 && m_selectedVariant < m_variants.count()) {
        m_selectedVariant = o;
        emit selectedVariantChanged();
    }
}

ReleaseVersion::Status ReleaseVersion::status() const {
    return m_status;
}

QDateTime ReleaseVersion::releaseDate() const {
    return m_releaseDate;
}

void ReleaseVersion::addVariant(ReleaseVariant *v) {
    m_variants.append(v);
    emit variantsChanged();
    if (m_variants.count() == 1)
        emit selectedVariantChanged();
}

QQmlListProperty<ReleaseVariant> ReleaseVersion::variants() {
    return QQmlListProperty<ReleaseVariant>(this, m_variants);
}

QList<ReleaseVariant *> ReleaseVersion::variantList() const {
    return m_variants;
}


ReleaseVariant::ReleaseVariant(ReleaseVersion *parent, QString url, QString shaHash, int64_t size, ReleaseArchitecture *arch, ReleaseVariant::Type type)
    : QObject(parent), m_arch(arch), m_type(type), m_url(url), m_shaHash(shaHash), m_size(size)
{

}

ReleaseVariant::ReleaseVariant(ReleaseVersion *parent, const QString &file, int64_t size)
    : QObject(parent), m_iso(file), m_arch(ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)), m_size(size)
{
    m_status = READY;
}

bool ReleaseVariant::updateUrl(const QString &url, const QString &sha256, int64_t size) {
    bool changed = false;
    if (!url.isEmpty() && m_url.toUtf8().trimmed() != url.toUtf8().trimmed()) {
        qWarning() << "Url" << m_url << "changed to" << url;
        m_url = url;
        emit urlChanged();
        changed = true;
    }
    if (!sha256.isEmpty() && m_shaHash.trimmed() != sha256.trimmed()) {
        qWarning() << "SHA256 hash of" << url << "changed from" << m_shaHash << "to" << sha256;
        m_shaHash = sha256;
        emit shaHashChanged();
        changed = true;
    }
    if (size != 0 && m_size != size) {
        m_size = size;
        emit sizeChanged();
        changed = true;
    }
    return changed;
}

ReleaseVersion *ReleaseVariant::releaseVersion() {
    return qobject_cast<ReleaseVersion*>(parent());
}

Release *ReleaseVariant::release() {
    return releaseVersion()->release();
}

ReleaseArchitecture *ReleaseVariant::arch() const {
    return m_arch;
}

ReleaseVariant::Type ReleaseVariant::type() const {
    return m_type;
}

QString ReleaseVariant::name() const {
    return m_arch->description();
}

QString ReleaseVariant::url() const {
    return m_url;
}

QString ReleaseVariant::shaHash() const {
    return m_shaHash;
}

QString ReleaseVariant::iso() const {
    return m_iso;
}

qreal ReleaseVariant::size() const {
    return m_size;
}

Progress *ReleaseVariant::progress() {
    if (!m_progress)
        m_progress = new Progress(this, 0.0, size());

    return m_progress;
}

ReleaseVariant::Status ReleaseVariant::status() const {
    return m_status;
}

QString ReleaseVariant::statusString() const {
    return m_statusStrings[m_status];
}

void ReleaseVariant::onFileDownloaded(const QString &path, const QString &hash) {
    m_iso = path;
    emit isoChanged();

    if (m_progress)
        m_progress->setValue(size());
    setStatus(DOWNLOAD_VERIFYING);
    m_progress->setValue(0.0/0.0, 1.0);

    if (!shaHash().isEmpty() && shaHash() != hash) {
        qWarning() << "Computed SHA256 hash of" << path << " - " << hash << "does not match expected" << shaHash();
        setErrorString(tr("The downloaded image is corrupted"));
        setStatus(FAILED_DOWNLOAD);
    }

    qApp->eventDispatcher()->processEvents(QEventLoop::AllEvents);

    int checkResult = mediaCheckFile(QDir::toNativeSeparators(path).toLocal8Bit(), &ReleaseVariant::staticOnMediaCheckAdvanced, this);
    if (checkResult == ISOMD5SUM_CHECK_FAILED) {
        qWarning() << "Internal MD5 media check of" << path << "failed with status" << checkResult;
        QFile::remove(path);
        setErrorString(tr("The downloaded image is corrupted"));
        setStatus(FAILED_DOWNLOAD);
    }
    else {
        setStatus(READY);

        if (QFile(m_iso).size() != m_size) {
            m_size = QFile(m_iso).size();
            emit sizeChanged();
        }
    }
}

void ReleaseVariant::onDownloadError(const QString &message) {
    setErrorString(message);
    setStatus(FAILED_DOWNLOAD);
}

int ReleaseVariant::staticOnMediaCheckAdvanced(void *data, long long offset, long long total) {
    ReleaseVariant *v = static_cast<ReleaseVariant*>(data);
    return v->onMediaCheckAdvanced(offset, total);
}

int ReleaseVariant::onMediaCheckAdvanced(long long offset, long long total) {
    qApp->eventDispatcher()->processEvents(QEventLoop::AllEvents);
    m_progress->setValue(offset, total);
    return 0;
}

void ReleaseVariant::download() {
    if (url().isEmpty() && !iso().isEmpty()) {
        setStatus(READY);
    }
    else {
        resetStatus();
        setStatus(DOWNLOADING);
        if (m_size)
            m_progress->setTo(m_size);
        QString ret = DownloadManager::instance()->downloadFile(this, url(), DownloadManager::dir(), progress());
        if (!ret.isEmpty()) {
            m_iso = ret;
            emit isoChanged();

            setStatus(READY);

            if (QFile(m_iso).size() != m_size) {
                m_size = QFile(m_iso).size();
                emit sizeChanged();
            }
        }
    }
}

void ReleaseVariant::resetStatus() {
    if (!m_iso.isEmpty()) {
        setStatus(READY);
    }
    else {
        setStatus(PREPARING);
        if (m_progress)
            m_progress->setValue(0.0);
    }
    setErrorString(QString());
    emit statusChanged();
}

void ReleaseVariant::setStatus(Status s) {
    if (m_status != s) {
        m_status = s;
        emit statusChanged();
    }
}

QString ReleaseVariant::errorString() const {
    return m_error;
}

void ReleaseVariant::setErrorString(const QString &o) {
    if (m_error != o) {
        m_error = o;
        emit errorStringChanged();
    }
}


ReleaseArchitecture ReleaseArchitecture::m_all[] = {
    {{"x86_64"}, QT_TR_NOOP("Intel 64bit"), QT_TR_NOOP("ISO format image for Intel, AMD and other compatible PCs (64-bit)")},
    {{"x86", "i386", "i686"}, QT_TR_NOOP("Intel 32bit"), QT_TR_NOOP("ISO format image for Intel, AMD and other compatible PCs (32-bit)")},
    {{"armv7hl", "armhfp"}, QT_TR_NOOP("ARM v7"), QT_TR_NOOP("LZMA-compressed raw image for ARM v7-A machines like the Raspberry Pi 2 and 3")},
};

ReleaseArchitecture::ReleaseArchitecture(const QStringList &abbreviation, const char *description, const char *details)
    : m_abbreviation(abbreviation), m_description(description), m_details(details)
{

}

ReleaseArchitecture *ReleaseArchitecture::fromId(ReleaseArchitecture::Id id) {
    if (id >= 0 && id < _ARCHCOUNT)
        return &m_all[id];
    return nullptr;
}

ReleaseArchitecture *ReleaseArchitecture::fromAbbreviation(const QString &abbr) {
    for (int i = 0; i < _ARCHCOUNT; i++) {
        if (m_all[i].abbreviation().contains(abbr, Qt::CaseInsensitive))
            return &m_all[i];
    }
    return nullptr;
}

QList<ReleaseArchitecture *> ReleaseArchitecture::listAll() {
    QList<ReleaseArchitecture *> ret;
    for (int i = 0; i < _ARCHCOUNT; i++) {
        ret.append(&m_all[i]);
    }
    return ret;
}

QStringList ReleaseArchitecture::listAllDescriptions() {
    QStringList ret;
    for (int i = 0; i < _ARCHCOUNT; i++) {
        ret.append(m_all[i].description());
    }
    return ret;
}

QStringList ReleaseArchitecture::abbreviation() const {
    return m_abbreviation;
}

QString ReleaseArchitecture::description() const {
    return tr(m_description);
}

QString ReleaseArchitecture::details() const {
    return tr(m_details);
}

int ReleaseArchitecture::index() const {
    return this - m_all;
}
