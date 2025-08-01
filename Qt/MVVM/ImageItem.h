#pragma once

#include <QObject>
#include <QString>

class ImageItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(int y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QString nom READ nom WRITE setNom NOTIFY nomChanged)

public:
    ImageItem(int x, int y, const QString &type, const QString &nom, QObject *parent = nullptr)
        : QObject(parent), m_x(x), m_y(y), m_type(type), m_nom(nom)
    {}

    int x() const { return m_x; }
    int y() const { return m_y; }
    QString type() const { return m_type; }
    QString nom() const { return m_nom; }

    void setX(int x)
    {
        if (m_x != x)
        {
            m_x = x;
            emit xChanged();
        }
    }

    void setY(int y)
    {
        if (m_y != y)
        {
            m_y = y;
            emit yChanged();
        }
    }

    void setType(const QString &type)
    {
        if (m_type != type)
        {
            m_type = type;
            emit typeChanged();
        }
    }

    void setNom(const QString &nom)
    {
        if (m_nom != nom)
        {
            m_nom = nom;
            emit nomChanged();
        }
    }

signals:
    void xChanged();
    void yChanged();
    void typeChanged();
    void nomChanged();

private:
    int m_x = 0;
    int m_y = 0;
    QString m_type;
    QString m_nom;
}; 