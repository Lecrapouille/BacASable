#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include "ImageItem.h"

class Image : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(int identifiant READ identifiant WRITE setIdentifiant NOTIFY identifiantChanged)
    Q_PROPERTY(QString titre READ titre WRITE setTitre NOTIFY titreChanged)
    Q_PROPERTY(QList<ImageItem*> items READ items NOTIFY itemsChanged)

public:
    Image(const QString &type, int identifiant, const QString &titre, QObject *parent = nullptr)
        : QObject(parent), m_type(type), m_identifiant(identifiant), m_titre(titre)
    {}

    QString type() const { return m_type; }
    int identifiant() const { return m_identifiant; }
    QString titre() const { return m_titre; }
    QList<ImageItem*> items() const { return m_items; }

    void setType(const QString &type)
    {
        if (m_type != type)
        {
            m_type = type;
            emit typeChanged();
        }
    }

    void setIdentifiant(int identifiant)
    {
        if (m_identifiant != identifiant)
        {
            m_identifiant = identifiant;
            emit identifiantChanged();
        }
    }

    void setTitre(const QString &titre)
    {
        if (m_titre != titre)
        {
            m_titre = titre;
            emit titreChanged();
        }
    }

    void addItem(ImageItem *item)
    {
        if (item && !m_items.contains(item))
        {
            m_items.append(item);
            emit itemsChanged();
        }
    }

    void removeItem(ImageItem *item)
    {
        if (m_items.removeOne(item))
        {
            emit itemsChanged();
        }
    }

    void clearItems()
    {
        if (!m_items.isEmpty())
        {
            m_items.clear();
            emit itemsChanged();
        }
    }

    int itemCount() const
    {
        return m_items.size();
    }

signals:
    void typeChanged();
    void identifiantChanged();
    void titreChanged();
    void itemsChanged();

private:
    QString m_type;
    int m_identifiant = 0;
    QString m_titre;
    QList<ImageItem*> m_items;
}; 