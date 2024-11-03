// SPDX-FileCopyrightText: 2014 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QClipboard>
#include <QVariant>

class ClipboardPrivate;

class Clipboard : public QObject
{
    Q_OBJECT
    /**
     * Controls the state this object will be monitoring and extracting its contents from.
     */
    Q_PROPERTY(QClipboard::Mode mode READ mode WRITE setMode NOTIFY modeChanged)

    /**
     * Provides the contents currently in the clipboard and lets modify them.
     */
    Q_PROPERTY(QVariant content READ content WRITE setContent NOTIFY contentChanged)

    /**
     * Figure out the nature of the contents in the clipboard.
     */
    Q_PROPERTY(QStringList formats READ formats NOTIFY contentChanged)

public:
    explicit Clipboard(QObject *parent = nullptr);

    QClipboard::Mode mode() const;
    void setMode(QClipboard::Mode mode);

    Q_SCRIPTABLE QVariant contentFormat(const QString &format) const;
    QVariant content() const;
    void setContent(const QVariant &content);

    QStringList formats() const;

    Q_SCRIPTABLE void clear();

Q_SIGNALS:
    void modeChanged(QClipboard::Mode mode);
    void contentChanged();

private Q_SLOTS:
    void clipboardChanged(QClipboard::Mode m);

private:
    QClipboard *m_clipboard;
    QClipboard::Mode m_mode;
};

#endif
