/*
 *  Copyright (C) 2017 KeePassXC Team <team@keepassxc.org>
 *  Copyright (C) 2012 Felix Geyer <debfx@fobos.de>
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

#include "clipboard.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTimer>

#include "Config.h"

Clipboard *Clipboard::m_instance(nullptr);
#ifdef Q_OS_MACOS
QPointer<MacPasteboard> Clipboard::m_pasteboard(nullptr);
#endif

Clipboard::Clipboard(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this))
{
#ifdef Q_OS_MACOS
    if (!m_pasteboard) {
        m_pasteboard = new MacPasteboard();
    }
#endif
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), SLOT(clearClipboard()));
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(clearCopiedText()));
}

void Clipboard::setText(const QString &text)
{
    QClipboard *clipboard = QApplication::clipboard();

    QMimeData *mime = new QMimeData;
#ifdef Q_OS_MACOS
    mime->setText(text);
    mime->setData("application/x-nspasteboard-concealed-type", text.toUtf8());
    clipboard->setMimeData(mime, QClipboard::Clipboard);
#else
    const QString secretStr = "secret";
    QByteArray secretBa     = secretStr.toUtf8();
    mime->setText(text);
    mime->setData("x-kde-passwordManagerHint", secretBa);
    clipboard->setMimeData(mime, QClipboard::Clipboard);

    if (clipboard->supportsSelection()) {
        clipboard->setMimeData(mime, QClipboard::Selection);
    }
#endif

    if (config()->get(Config::SECURITY_clearclipboard).toBool()) {
        int timeout = config()->get(Config::SECURITY_clearclipboardtimeout).toInt();
        if (timeout > 0) {
            m_lastCopied = text;
            m_timer->start(timeout * 1000);
        }
    }
}

void Clipboard::clearCopiedText()
{
    if (m_timer->isActive()) {
        m_timer->stop();
        clearClipboard();
    }
}

void Clipboard::clearClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (!clipboard) {
        qWarning("Unable to access the clipboard.");
        return;
    }

    if (clipboard->text(QClipboard::Clipboard) == m_lastCopied) {
        clipboard->clear(QClipboard::Clipboard);
    }

    if (clipboard->supportsSelection() && (clipboard->text(QClipboard::Selection) == m_lastCopied)) {
        clipboard->clear(QClipboard::Selection);
    }

    m_lastCopied.clear();
}

Clipboard *Clipboard::instance()
{
    if (!m_instance) {
        m_instance = new Clipboard(qApp);
    }

    return m_instance;
}
