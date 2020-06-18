/*
 *  Copyright (C) 2011 Felix Geyer <debfx@fobos.de>
 *  Copyright (C) 2017 KeePassXC Team <team@keepassxc.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Config.h"
#include "constants.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryFile>

using namespace ARs;

/*
 * Map of configuration file settings that are either deprecated, or have
 * had their name changed.  Entries in the map are of the form
 *     {oldName, newName}
 * Set newName to empty string to remove the setting from the file.
 */
static const QMap<QString, QString> deprecationMap = {
    // >2.3.4
    {QStringLiteral("security/hidepassworddetails"), QStringLiteral("security/HidePasswordPreviewPanel")},
    // >2.3.4
    {QStringLiteral("GUI/HideDetailsView"), QStringLiteral("GUI/HidePreviewPanel")},
    // >2.3.4
    {QStringLiteral("GUI/DetailSplitterState"), QStringLiteral("GUI/PreviewSplitterState")},
    // >2.3.4
    {QStringLiteral("security/IconDownloadFallbackToGoogle"), QStringLiteral("security/IconDownloadFallback")},
};

Config *Config::m_instance(nullptr);

QVariant Config::get(const QString &key)
{
    return (m_settings->value(key, m_defaults.value(key)));
}

QVariant Config::get(const QString &key, const QVariant &defaultValue)
{
    return (m_settings->value(key, defaultValue));
}

bool Config::hasAccessError()
{
    return (m_settings->status() & QSettings::AccessError);
}

QString Config::getFileName()
{
    return (m_settings->fileName());
}

void Config::set(const QString &key, const QVariant &value)
{
    if (m_settings->contains(key) && (m_settings->value(key) == value)) return;

    const bool surpressSignal = !m_settings->contains(key) && m_defaults.value(key) == value;
    m_settings->setValue(key, value);

    if (!surpressSignal) emit changed(key);
}

/**
 * Sync configuration with persistent storage.
 *
 * Usually, you don't need to call this method manually, but if you are writing
 * configurations after an emitted \link QCoreApplication::aboutToQuit() signal,
 * use it to guarantee your config values are persisted.
 */
void Config::sync()
{
    m_settings->sync();
}

void Config::resetToDefaults()
{
    for (const auto &setting : m_defaults.keys()) {
        m_settings->setValue(setting, m_defaults.value(setting));
    }
}

void Config::upgrade()
{
    const auto keys = deprecationMap.keys();
    for (const auto &setting : keys) {
        if (m_settings->contains(setting)) {
            if (!deprecationMap.value(setting).isEmpty())
                // Add entry with new name and old entry's value
                m_settings->setValue(deprecationMap.value(setting), m_settings->value(setting));

            m_settings->remove(setting);
        }
    }

    /*
     * // > 2.3.4
     * if (m_settings->value("AutoSaveAfterEveryChange").toBool()) {
     *  m_settings->setValue("AutoSaveOnExit", true);
     * }
     *
     * // Setting defaults for 'hide window on copy' behavior, keeping the user's original setting
     * if (m_settings->value("HideWindowOnCopy").isNull()) {
     *  m_settings->setValue("HideWindowOnCopy", m_settings->value("MinimizeOnCopy").toBool());
     *  m_settings->setValue("MinimizeOnCopy", true);
     * }
     */
}

Config::Config(const QString &fileName, QObject *parent)
    : QObject(parent)
{
    init(fileName);
}

Config::Config(QObject *parent)
    : QObject(parent)
{
    // Check if portable config is present. If not, find it in user's directory
    QString portablePath = QCoreApplication::applicationDirPath() + "/arsenic.ini";

    if (QFile::exists(portablePath)) {
        init(portablePath);
    } else {
        QString userPath;
        QString homePath = QDir::homePath();

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
        // we can't use QStandardPaths on X11 as it uses XDG_DATA_HOME instead of XDG_CONFIG_HOME
        QByteArray env = qgetenv("XDG_CONFIG_HOME");

        if (env.isEmpty()) {
            userPath = homePath;
            userPath += "/.config";
        } else if (env[0] == '/') {
            userPath = QFile::decodeName(env);
        } else {
            userPath = homePath;
            userPath += '/';
            userPath += QFile::decodeName(env);
        }

        userPath += "/arsenic/";
#else
        userPath = QDir::fromNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
        // storageLocation() appends the application name ("/arsenic") to the end
        userPath += "/";
#endif

#ifdef QT_DEBUG
        userPath += "arsenic_debug.ini";
#else
        userPath += "arsenic.ini";
#endif

        init(userPath);
    }
}

Config::~Config() {}

void Config::init(const QString &fileName)
{
    m_settings.reset(new QSettings(fileName, QSettings::IniFormat));
    upgrade();
    connect(qApp, &QCoreApplication::aboutToQuit, this, &Config::sync);

    m_defaults.insert("GUI/showPassword", DEFAULT_SHOW_PSW);
    m_defaults.insert("GUI/deleteFinished", DEFAULT_DELETE_FINSHED);
    m_defaults.insert("GUI/darkTheme", DEFAULT_DARK_THEME);
    m_defaults.insert("GUI/Language", DEFAULT_LANGUAGE);
    m_defaults.insert("GUI/showToolbar", DEFAULT_SHOW_TOOLBAR);
    m_defaults.insert("GUI/lastDirectory", DEFAULT_LASTDIRECTORY);
    m_defaults.insert("GUI/currentIndexTab", DEFAULT_INDEX_TAB);
    m_defaults.insert("GUI/AddEncrypted", DEFAULT_ADD_ENCRYPTED);
    m_defaults.insert("GUI/AddDecrypted", DEFAULT_ADD_DECRYPTED);

    m_defaults.insert("CRYPTO/argonMemory", DEFAULT_ARGON_MEM_LIMIT);
    m_defaults.insert("CRYPTO/argonItr", DEFAULT_ARGON_ITR_LIMIT);

    m_defaults.insert("SECURITY/clearclipboard", DEFAULT_CLEAR_CLIPBOARD);
    m_defaults.insert("SECURITY/clearclipboardtimeout", DEFAULT_CLIPBOARD_TIMEOUT);
}

Config *Config::instance()
{
    if (!m_instance) m_instance = new Config(qApp);

    return (m_instance);
}

void Config::createConfigFromFile(const QString &file)
{
    Q_ASSERT(!m_instance);
    m_instance = new Config(file, qApp);
}

void Config::createTempFileInstance()
{
    Q_ASSERT(!m_instance);
    auto *tmpFile = new QTemporaryFile();
    bool openResult = tmpFile->open();
    Q_ASSERT(openResult);
    Q_UNUSED(openResult);
    m_instance = new Config(tmpFile->fileName(), qApp);
    tmpFile->setParent(m_instance);
}