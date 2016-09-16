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

ReleaseManager::ReleaseManager(QObject *parent)
    : QSortFilterProxyModel(parent), m_sourceModel(new ReleaseListModel(this))
{
    setSourceModel(m_sourceModel);

    qmlRegisterUncreatableType<Release>("MediaWriter", 1, 0, "Release", "");
    qmlRegisterUncreatableType<ReleaseVersion>("MediaWriter", 1, 0, "Version", "");
    qmlRegisterUncreatableType<ReleaseVariant>("MediaWriter", 1, 0, "Variant", "");
    qmlRegisterUncreatableType<ReleaseArchitecture>("MediaWriter", 1, 0, "Architecture", "");
    qmlRegisterUncreatableType<Progress>("MediaWriter", 1, 0, "Progress", "");
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

bool ReleaseManager::beingUpdated() const {
    return false; // TODO
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
    // until there's a downloader
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Workstation", "This is the Linux workstation you've been waiting for.", "<p>Fedora Workstation is a reliable, user-friendly, and powerful operating system for your laptop or desktop computer. It supports a wide range of developers, from hobbyists and students to professionals in corporate environments.</p><blockquote><p>&#8220;The plethora of tools provided by  Fedora allows me to get the job done.  It just works.&#8221;</p><p align=right> ― <em>Christine Flood, JVM performance engineer</em></p></blockquote><h3>Sleek user interface</h3><p>Focus on your code in the GNOME 3 desktop environment. GNOME is built with developer feedback and minimizes distractions, so you can concentrate on what's important.</p><h3>Complete open source toolbox</h3><p>Skip the drag of trying to find or build the tools you need. With Fedora's complete set of open source languages, tools, and utilities, everything is a click or command line away. There's even project hosting and repositories like COPR to make your code and builds available quickly to the community.</p><h3>GNOME Boxes &amp; other virt tools</h3><p>Get virtual machines up and running quickly to test your code on multiple platforms using GNOME Boxes. Or dig into powerful, scriptable virtualization tools for even more control.</p><h3>Built-in Docker support</h3><p>Containerize your own apps, or deploy containerized apps out of the box on Fedora, using the latest technology like Docker.</p>", Release::PRODUCT, "qrc:/icons/workstation", {}, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/24/Workstation/x86_64/iso/Fedora-Workstation-Live-x86_64-24-1.2.iso", "8e12d7ba1fcf3328b8514d627788ee0146c0eef75a5e27f0674ee1fe4f1feaf6", 1503238553, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/24/Workstation/i386/iso/Fedora-Workstation-Live-i386-24-1.2.iso", "1c96e1529cb25b08dac4ef29cb4f6eafdd9b6bbf008642fb4ae01a5e9bb31255", 1717986918, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("2015-11-03", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Workstation/x86_64/iso/Fedora-Live-Workstation-x86_64-23-10.iso", "a91eca2492ac84909953ef27040f9b61d8525f7ec5e89f6430319f49f9f823fe", 1503238553, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Workstation/i386/iso/Fedora-Live-Workstation-i686-23-10.iso", "1f3fe28a51d0500ac19030b28e4dfb151d4a6368a9c25fb29ac9a3d29d47a838", 1395864371, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Server", "The latest technology. A stable foundation. Together, for your applications and services.", "<blockquote><p>&#8220;The simplicity introduced with rolekit and cockpit have made server deployments a breeze. What took me a few days on other operating systems took less than an hour with Fedora 23 Server. It just works.&#8221;</p><p align=right> ― <em>Dan Mossor, Systems Engineer</em></p></blockquote><p>Fedora Server is a short-lifecycle, community-supported server operating system that enables seasoned system administrators experienced with any OS to make use of the very latest server-based technologies available in the open source community.</p><h3>Easy Administration</h3><p>Manage your system simply with Cockpit's powerful, modern interface. View and monitor system performance and status, and deploy and manage container-based services.</p><h3>Server Roles</h3><p>There's no need to set up your server from scratch when you use server roles. Server roles plug into your Fedora Server system, providing a well-integrated service on top of the Fedora Server platform. Deploy and manage these prepared roles simply using the Rolekit tool.</p><h3>Database Services</h3><p>Fedora Server brings with it an enterprise-class, scalable database server powered by the open-source PostgreSQL project.</p><h3>Complete Enterprise Domain Solution</h3><p>Level up your Linux network with advanced identity management, DNS, certificate services, Windows(TM) domain integration throughout your environment with FreeIPA, the engine that drives Fedora Server's Domain Controller role.</p><blockquote><p>&#8220;The Docker Role for Fedora Server was simple and fast to install so that you can run your Docker images. This makes a great testbed for beginners and experts with docker so that they can develop their applications on the fly.&#8221;</p><p align=right> ― <em>John Unland, Information Systems Student</em></p></blockquote>", Release::PRODUCT, "qrc:/icons/server", {}, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/24/Server/x86_64/iso/Fedora-Server-dvd-x86_64-24-1.2.iso", "1c0971d4c1a37bb06ec603ed3ded0af79e22069499443bb2d47e501c9ef42ae8", 1825361100, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("2015-11-03", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Server/x86_64/iso/Fedora-Server-DVD-x86_64-23.iso", "30758dc821d1530de427c9e35212bd79b058bd4282e64b7b34ae1a40c87c05ae", 2147483648, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Server/i386/iso/Fedora-Server-DVD-i386-23.iso", "aa2125b6351480ce82ace619925d897d0588195a3287ef74fb203b6eb34cbccf", 2254857830, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Custom OS...", "Pick a file from your drive(s)", "<p>Here you can choose a OS image from your hard drive to be written to your flash disk</p><p>Currently it is only supported to write raw disk images (.iso or .bin)</p>", Release::LOCAL, "qrc:/icons/folder", {}, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "", "", 0, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora KDE Plasma Desktop", "A complete, modern desktop built using the KDE Plasma Desktop.", "<p>The Fedora KDE Plasma Desktop Edition is a powerful Fedora-based operating system utilizing the KDE Plasma Desktop as the main user interface.</p><p>Fedora KDE Plasma Desktop comes with many pre-selected top quality applications that suit all modern desktop use cases - from online communication like web browsing, instant messaging and electronic mail correspondence, through multimedia and entertainment, to an advanced productivity suite, including office applications and enterprise grade personal information management.</p><p>All KDE applications are well integrated, with a similar look and feel and an easy to use interface, accompanied by an outstanding graphical appearance.</p>", Release::SPINS, "qrc:/icons/plasma", { "http://spins.stg.fedoraproject.org/en/kde/../static/images/screenshots/screenshot-kde.jpg" }, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/24/Spins/x86_64/iso/Fedora-KDE-Live-x86_64-24-1.2.iso", "28bcde6e4c2e5471ef982fcf15930fbc4181ccf4bd47efad8f915c7d89c29bbb", 1288490188, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/24/Spins/i386/iso/Fedora-KDE-Live-i386-24-1.2.iso", "70a1833b2bbc8309acef36b354384412b12c05bde6aa1e5a1403febe652b50ab", 1288490188, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("2015-11-03", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Live/x86_64/Fedora-Live-KDE-x86_64-23-10.iso", "ef7e5ed9eee6dbcde1e0a4d69c76ce6fb552f75ccad879fa0f93031ceb950f27", 1288490188, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Live/i386/Fedora-Live-KDE-i686-23-10.iso", "60f7e4efbe04cf89918df01e218042b698dccc5767d47208b9f46c6cd4ceb49b", 1288490188, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Xfce Desktop", "A complete, well-integrated Xfce Desktop.", "<p>The Fedora Xfce spin showcases the Xfce desktop, which aims to be fast and lightweight, while still being visually appealing and user friendly.</p><p>Fedora Xfce is a full-fledged desktop using the freedesktop.org standards.</p>", Release::SPINS, "qrc:/icons/xfce", { "http://spins.stg.fedoraproject.org/en/xfce/../static/images/screenshots/screenshot-xfce.jpg" }, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/24/Spins/x86_64/iso/Fedora-Xfce-Live-x86_64-24-1.2.iso", "71a1ab11cf92660bd8508ed14248e8341abac99002c7c3db71f4403e32bb6a9c", 960495616, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/24/Spins/i386/iso/Fedora-Xfce-Live-i386-24-1.2.iso", "8837a634a65db89a42344213aa83f6fe251d4df30e02ca8826c5ded183a7b9f1", 934281216, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("2015-11-03", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Live/x86_64/Fedora-Live-Xfce-x86_64-23-10.iso", "a24e48a604c81f8e3c3fbdd48a907d7168d0bc5310a0072f8b844aa799dd3365", 960495616, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Live/i386/Fedora-Live-Xfce-i686-23-10.iso", "9111100e47742bd62a4b3ecaf79b985921601ac1d7616bb5ea0d924b4cfda8ba", 934281216, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora LXDE Desktop", "A light, fast, less-resource hungry desktop environment.", "<p>LXDE, the \"Lightweight X11 Desktop Environment\", is an extremely fast, performant, and energy-saving desktop environment. It maintained by an international community of developers and comes with a beautiful interface, multi-language support, standard keyboard shortcuts and additional features like tabbed file browsing.</p><p>LXDE is not designed to be powerful and bloated, but to be usable and slim. A main goal of LXDE is to keep computer resource usage low. It is especially designed for computers with low hardware specifications like netbooks, mobile devices (e.g. MIDs) or older computers.</p>", Release::SPINS, "qrc:/icons/lxde", { "http://spins.stg.fedoraproject.org/en/lxde/../static/images/screenshots/screenshot-lxde.jpg" }, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/24/Spins/x86_64/iso/Fedora-LXDE-Live-x86_64-24-1.2.iso", "74400c0b08adadb249b224466468b3b50e72157abf266d845137f3988fe407ad", 877658112, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/24/Spins/i386/iso/Fedora-LXDE-Live-i386-24-1.2.iso", "ab497657c6a9108373415f43e052e9a0aa80e449627a385bda63345818651e52", 1010827264, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("2015-11-03", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Live/x86_64/Fedora-Live-LXDE-x86_64-23-10.iso", "9b2acffef7ee8d8445fab427ef06afb0e888448241f761fc59aec59f53c7b3f0", 877658112, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Live/i386/Fedora-Live-LXDE-i686-23-10.iso", "0298e4ef3f514911105d3cfaa29bb35f08bcc0319386de703b89a43c88eade15", 1010827264, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora MATE-Compiz Desktop", "A classic Fedora Desktop with an additional 3D Windows Manager.", "<p>The MATE Compiz spin bundles MATE Desktop with Compiz Fusion. MATE Desktop is a lightweight, powerful desktop designed with productivity and performance in mind. The default windows manager is Marco which is usable for all machines and VMs. Compiz Fusion is a beautiful 3D windowing manager with Emerald and GTK+ theming.</p><p>If you want a powerful, lightweight Fedora desktop with 3D eyecandy you should definitely try the MATE-Compiz spin.</p>", Release::SPINS, "qrc:/icons/mate", { "http://spins.stg.fedoraproject.org/en/mate-compiz/../static/images/screenshots/screenshot-matecompiz.jpg" }, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/24/Spins/x86_64/iso/Fedora-MATE_Compiz-Live-x86_64-24-1.2.iso", "0ad0b5c2ac188330de7f3614781ce5480e1214bd64d08821cfef23030128876f", 1395864371, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/24/Spins/i386/iso/Fedora-MATE_Compiz-Live-i386-24-1.2.iso", "1e350f8a2e5509cd53d3ecade36d5df260830e59e924535c130f15b06ea578b9", 1288490188, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("2015-11-03", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Live/x86_64/Fedora-Live-MATE_Compiz-x86_64-23-10.iso", "5cc5dd3b4c8dfa3b57cc9700404cb1d4036265691af7f28714456b5983d57737", 1395864371, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Live/i386/Fedora-Live-MATE_Compiz-i686-23-10.iso", "f33b7c0320796a907a471324b5934152371e6fc3d291fcb5e63b664e797ca2ed", 1288490188, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Cinnamon Desktop", "A modern desktop featuring traditional Gnome user experience.", "<p>Cinnamon is a Linux desktop which provides advanced innovative features and a traditional user experience. The desktop layout is similar to Gnome 2. The underlying technology is forked from Gnome Shell. The emphasis is put on making users feel at home and providing them with an easy to use and comfortable desktop experience.</p><p>Cinnamon is a popular desktop alternative to Gnome 3 and this spin provides the option to quickly try and install this desktop.</p>", Release::SPINS, "qrc:/icons/cinnamon", { "http://spins.stg.fedoraproject.org/en/cinnamon/../static/images/screenshots/screenshot-cinnamon.jpg" }, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/24/Spins/x86_64/iso/Fedora-Cinnamon-Live-x86_64-24-1.2.iso", "df601acd9eebc21ba2a5a1949db072d34f1a04fd762da1dca0299a59bf97a0f8", 1288490188, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/24/Spins/i386/iso/Fedora-Cinnamon-Live-i386-24-1.2.iso", "7912efc41e75bf79c35c6bbd5b072dd456b4836cae0fc69ac24890bfaccbe9b7", 1288490188, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("2015-11-03", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Live/x86_64/Fedora-Live-Cinnamon-x86_64-23-10.iso", "4585ff18d8f7b019f9f15119ecb6ee8ddeb947cba4c4d649d6689032ef57cca9", 1288490188, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Live/i386/Fedora-Live-Cinnamon-i686-23-10.iso", "5bfe789ed8fbcbf6c22e1c30b8a38e373206303ff9a12b7f2dd108ade33473b8", 1288490188, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora SoaS Desktop", "Discover. Reflect. Share. Learn.", "<p>Sugar on a Stick is a Fedora-based operating system featuring the award-winning Sugar Learning Platform and designed to fit on an ordinary USB thumbdrive (\"stick\").</p><p>Sugar sets aside the traditional “office-desktop” metaphor, presenting a child-friendly graphical environment. Sugar automatically saves your progress to a \"Journal\" on your stick, so teachers and parents can easily pull up \"all collaborative web browsing sessions done in the past week\" or \"papers written with Daniel and Sarah in the last 24 hours\" with a simple query rather than memorizing complex file/folder structures. Applications in Sugar are known as Activities, some of which are described below.</p><p>It is now deployable for the cost of a stick rather than a laptop; students can take their Sugar on a Stick thumbdrive to any machine - at school, at home, at a library or community center - and boot their customized computing environment without touching the host machine’s hard disk or existing system at all.</p>", Release::SPINS, "qrc:/icons/soas", { "http://spins.stg.fedoraproject.org/en/soas/../static/images/screenshots/screenshot-soas.jpg" }, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://dl.fedoraproject.org/pub/alt/unofficial/releases/24/x86_64/Fedora-SoaS-Live-x86_64-24-20160614.n.0.iso", "ba1dbd4bac36660f8f5b6ef9acaa18bcfb117413bc2b557b05a876778f4fa777", 732954624, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://dl.fedoraproject.org/pub/alt/unofficial/releases/24/i386/Fedora-SoaS-Live-i386-24-20160614.n.0.iso", "2af7c621681c3f4978e71a25bfd608fa66d2b441db51806b9ab639901c8eec58", 708837376, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("2015-11-03", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Live/x86_64/Fedora-Live-SoaS-x86_64-23-10.iso", "cdc364a5afdad91e615cf30aca8cd0c7ad9091e0d485bab4dcfc802a83600207", 732954624, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/fedora/linux/releases/23/Live/i386/Fedora-Live-SoaS-i686-23-10.iso", "f1cc96b9c07f182409e74b0346ffdafece15eddb91926637759fb3d3460ff128", 708837376, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Astronomy", "Powerful completely open-source and free tool for amateurs and professionals.", "<p>Fedora Astronomy brings a complete open source toolchain to both amateur and professional astronomers.</p><p>The Spin provides the Fedora KDE desktop enhanced with a complete scientific Python environment and the AstrOmatic software for data analysis. KStars was added to provide a full featured astrophotography tool. As KStars uses the INDI library to control equipment, various telescopes, cameras etc. are supported. Summarized, Fedora Astronomy provides a complete set of software, from the observation planning to the final results.</p>", Release::LABS, "qrc:/icons/astronomy", { }, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/24/Labs/x86_64/iso/Fedora-Astronomy_KDE-Live-x86_64-24-1.2.iso", "eeb21b6801494668d2031a0c341cfd266b633ae33d7ce5cd3ea1ec308e70224a", 2477785088, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/24/Labs/i386/iso/Fedora-Astronomy_KDE-Live-i386-24-1.2.iso", "e5724016044f1f0b836d8551a10ae6e0b987552e1fa934e58d0aca3a61ab185d", 2631925760, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Design Suite", "Visual design, multimedia production, and publishing suite of free and open source creative tools.", "<p>Looking for a ready-to-go desktop environment brimming with free and open source multimedia production and publishing tools? Try the Design Suite, a Fedora Spin created by designers, for designers.</p><p>The Design Suite includes the favorite tools of the Fedora Design Team. These are the same programs we use to create all the artwork that you see within the Fedora Project, from desktop backgrounds to CD sleeves, web page designs, application interfaces, flyers, posters and more. From document publication to vector and bitmap editing or 3D modeling to photo management, the Design Suite has an application for you — and you can install thousands more from the Fedora universe of packages.</p>", Release::LABS, "qrc:/icons/design", { }, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://dl.fedoraproject.org/pub/alt/unofficial/releases/24/x86_64/Fedora-Design_suite-Live-x86_64-24-20160614.n.0.iso", "d9a44a18e7433e8d523fa38d7c0f71199b5866af57d17f3c1e433cdd098373cd", 1932735283, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://dl.fedoraproject.org/pub/alt/unofficial/releases/24/i386/Fedora-Design_suite-Live-i386-24-20160614.n.0.iso", "77ac8c0ec235604ea2a49ad475356d243636f460410038879504fc82665c1651", 2040109465, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("2015-11-03", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/23/Spins/x86_64/Fedora-Live-Design_suite-x86_64-23-10.iso", "beb5b9129a19d494064269d2b4be398f6724ff0128adc245d4e5414b4ea1196c", 1932735283, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/23/Spins/i386/Fedora-Live-Design_suite-i686-23-10.iso", "8cdc37d92a0c2322ad3789b2c9f7960311e85d0f3628b82fd530d54cf7c45110", 2040109465, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Games", "A collection and perfect show-case of the best games available in Fedora.", "<p>The Fedora Games spin offers a perfect showcase of the best games available in Fedora. The included games span several genres, from first-person shooters to real-time and turn-based strategy games to puzzle games.</p><p>Not all the games available in Fedora are included on this spin, but trying out this spin will give you a fair impression of Fedora's ability to run great games.</p>", Release::LABS, "qrc:/icons/games", {}, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/24/Labs/x86_64/iso/Fedora-Games-Live-x86_64-24-1.2.iso", "84ebef0dfc7a15af5164ed8c40f8bd7ea4fbdf8046457f3fe7e6278d2fcd5510", 3865470566, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/24/Labs/i386/iso/Fedora-Games-Live-i386-24-1.2.iso", "a9cefe5d295ac9a0bfb61185917694fa81b2ca51bf0b547b8d8ecf892b191a25", 4080218931, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("2015-11-03", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/23/Spins/x86_64/Fedora-Live-Games-x86_64-23-10.iso", "5b4a9264f176fb79e3e6de280ade23af80cda65112e8dc9cfc8c44fcd60b0eb4", 4187593113, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/23/Spins/i386/Fedora-Live-Games-i686-23-10.iso", "fa9d4003094e85e2f667a7a065dbd1f59903ad61a3a3154aabc0db2ebe68093a", 3972844748, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Robotics Suite", "A wide variety of free and open robotics software packages for beginners and experts in robotics.", "<p>The Fedora Robotics spin provides a wide variety of free and open robotics software packages. These range from hardware accessory libraries for the Hokuyo laser scanners or Katana robotic arm to software frameworks like Fawkes or Player and simulation environments such as Stage and RoboCup Soccer Simulation Server 2D/3D. It also provides a ready to use development environment for robotics including useful libraries such as OpenCV computer vision library, Festival text to speech system and MRPT.</p><p>The Robotics spin is targeted at people just discovering their interest in robotics as well as experienced roboticists. For the former we provide a readily usable simulation environment with an introductory hands-on demonstration, and for the latter we provide a full development environment, to be used immediately.</p>", Release::LABS, "qrc:/icons/robotics", {}, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/24/Labs/x86_64/iso/Fedora-Robotics-Live-x86_64-24-1.2.iso", "fd5a0718f85499d0fe69f9c9a8fcf00eb32ec06e8b7520ddd063433a462ce6b7", 2576980377, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/24/Labs/i386/iso/Fedora-Robotics-Live-i386-24-1.2.iso", "1e689c0b67def88d4a94e600c559f2e30fa92a10a6669081d87117227327702e", 2899102924, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("2015-11-03", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/23/Spins/x86_64/Fedora-Live-Robotics-x86_64-23-10.iso", "71008e7035cc4ac79da7166786450ac2d73df5dab2240070af8e52e81aab11ea", 2684354560, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/23/Spins/i386/Fedora-Live-Robotics-i686-23-10.iso", "c76a71ef18bedf07e6c41e6a26a740562121c32e32acd5200c255f3c47ada0a8", 2684354560, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Scientific", "A bundle of open source scientific and numerical tools used in research.", "<p>Wary of reinstalling all the essential tools for your scientific and numerical work? The answer is here. Fedora Scientific Spin brings together the most useful open source scientific and numerical tools atop the goodness of the KDE desktop environment.</p><p>Fedora Scientific currently ships with numerous applications and libraries. These range from libraries such as the GNU Scientific library, the SciPy libraries, tools like Octave and xfig to typesetting tools like Kile and graphics programs such as Inkscape. The current set of packages include an IDE, tools and libraries for programming in C, C++, Python, Java and R. Also included along with are libraries for parallel computing such as the OpenMPI and OpenMP. Tools for typesetting, writing and publishing are included.</p>", Release::LABS, "qrc:/icons/scientific", {}, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/24/Labs/x86_64/iso/Fedora-Scientific_KDE-Live-x86_64-24-1.2.iso", "6cfdba696cbd69e2878f4c6f4ee5cf7852dc6b6265ddf85477c48e67822d0aeb", 3113851289, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/24/Labs/i386/iso/Fedora-Scientific_KDE-Live-i386-24-1.2.iso", "68536eafea38c6c83303355f897eea4034c01b1c28d8ffd3e3ccfc5f51983307", 3435973836, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("2015-11-03", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/23/Spins/x86_64/Fedora-Live-Scientific_KDE-x86_64-23-10.iso", "255b73a16feb8b44cdf546338ce48a3085be858dfeccfca1df03b87ff7d57934", 2899102924, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/23/Spins/i386/Fedora-Live-Scientific_KDE-i686-23-10.iso", "72669c5fa57ab298d73cf545c88050977cdbaf8f2ee573e6146651cb4a156b53", 2791728742, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.append(new Release {manager(), m_releases.length(), "Fedora Security Lab", "A safe test environment to work on security auditing, forensics, system rescue and teaching security testing methodologies.", "<p>The Fedora Security Lab provides a safe test environment to work on security auditing, forensics, system rescue and teaching security testing methodologies in universities and other organizations.</p><p>The spin is maintained by a community of security testers and developers. It comes with the clean and fast Xfce Desktop Environment and a customized menu that provides all the instruments needed to follow a proper test path for security testing or to rescue a broken system. The Live image has been crafted to make it possible to install software while running, and if you are running it from a USB stick created with LiveUSB Creator using the overlay feature, you can install and update software and save your test results permanently.</p>", Release::LABS, "qrc:/icons/security", {}, {} });
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 24, {}, ReleaseVersion::FINAL, QDateTime::fromString("2016-06-21", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/24/Labs/x86_64/iso/Fedora-Security-Live-x86_64-24-1.2.iso", "522b432295b380afcaf91310672e34f23a3b2219f0209b7d3cc6d99d03959807", 1181116006, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/24/Labs/i386/iso/Fedora-Security-Live-i386-24-1.2.iso", "5e66b08452932735009e053ea094fb19a423ac33934294bd6be19087154d2a4f", 1288490188, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
    m_releases.last()->addVersion(new ReleaseVersion {m_releases.last(), 23, {}, ReleaseVersion::FINAL, QDateTime::fromString("2015-11-03", "yyyy-MM-dd")});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/23/Spins/x86_64/Fedora-Live-Security-x86_64-23-10.iso", "fe712e118b72ac5727196a371dd4bf3472f84cc1b22a6c05d90af7a4cf3abd12", 985661440, ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64)});
    m_releases.last()->versionList().last()->addVariant(new ReleaseVariant {m_releases.last()->versionList().last(), "http://download.fedoraproject.org/pub/alt/releases/23/Spins/i386/Fedora-Live-Security-i686-23-10.iso", "2a41ea039b6bfac18f6e45ca0d474f566fd4f70365ba6377dfaaf488564ffe98", 960495616, ReleaseArchitecture::fromId(ReleaseArchitecture::X86)});
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
        return "Fedora Spins";
    case LABS:
        return "Fedora Labs";
    default:
        return QString();
    }
}

int Release::index() const {
    return m_index;
}

Release::Release(ReleaseManager *parent, int index, const QString &name, const QString &summary, const QString &description, Release::Source source, const QString &icon, const QStringList &screenshots, QList<ReleaseVersion *> versions)
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

    m_versions.append(new ReleaseVersion(this, QUrl(path).toLocalFile()));
    emit versionsChanged();
    emit selectedVersionChanged();
}

QString Release::name() const {
    return m_name;
}

QString Release::summary() const {
    return m_summary;
}

QString Release::description() const {
    return m_description;
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
    m_versions.append(version);
    emit versionsChanged();
    if (m_versions.count() == 1)
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

}

ReleaseVersion::ReleaseVersion(Release *parent, const QString &file)
    : QObject(parent), m_variants({ new ReleaseVariant(this, file) })
{

}

Release *ReleaseVersion::release() {
    return qobject_cast<Release*>(parent());
}

int ReleaseVersion::number() const {
    return m_number;
}

QString ReleaseVersion::name() const {
    QString ret = QString("%1").arg(m_number);
    switch (m_status) {
    case ALPHA:
        ret.append(" Alpha");
        break;
    case BETA:
        ret.append(" Beta");
        break;
    case RELEASE_CANDIDATE:
        ret.append(" Release Candidate");
        break;
    default:
        break;
    }
    return ret;
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

ReleaseVariant::ReleaseVariant(ReleaseVersion *parent, const QString &file)
    : QObject(parent), m_iso(file), m_arch(ReleaseArchitecture::fromId(ReleaseArchitecture::X86_64))
{
    m_status = READY;
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

void ReleaseVariant::onFileDownloaded(const QString &path) {
    m_iso = path;
    emit isoChanged();
    if (m_progress)
        m_progress->setValue(size());
    setStatus(DOWNLOAD_VERIFYING);
    m_progress->setValue(0.0/0.0, 1.0);
    if (shaHash().isEmpty() || mediaCheckFile(path.toLocal8Bit(), &ReleaseVariant::staticOnMediaCheckAdvanced, this)) {
        setStatus(READY);
    }
    else {
        setErrorString("The downloaded image is corrupted");
        setStatus(FAILED_DOWNLOAD);
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
    m_progress->setValue(offset, total);
    qApp->eventDispatcher()->processEvents(QEventLoop::AllEvents);
    return 0;
}

void ReleaseVariant::download() {
    resetStatus();
    setStatus(DOWNLOADING);
    QString ret = DownloadManager::instance()->downloadFile(this, url(), DownloadManager::dir(), progress());
    if (!ret.isEmpty()) {
        m_iso = ret;
        emit isoChanged();

        setStatus(READY);
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
    {{"x86_64"}, "Intel 64bit", "ISO format image for Intel, AMD and other compatible PCs (64-bit)"},
    {{"x86"}, "Intel 32bit", "ISO format image for Intel, AMD and other compatible PCs (32-bit)"},
    {{"armv7hl"}, "ARM v7", "ARM PLACEHOLDER TEXT"},
};

ReleaseArchitecture::ReleaseArchitecture(const QStringList &abbreviation, const QString &description, const QString &details)
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
    return m_description;
}

QString ReleaseArchitecture::details() const {
    return m_details;
}

int ReleaseArchitecture::index() const {
    return this - m_all;
}
