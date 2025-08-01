#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include "Image.h"

class ImageViewModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum ImageRoles {
        TypeRole = Qt::UserRole + 1,
        IdentifiantRole,
        TitreRole,
        ImageObjectRole
    };

    explicit ImageViewModel(QObject *parent = nullptr);

    // QAbstractListModel interfaces
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Methods for C++
public:
    void addImage(const QString &type, int identifiant, const QString &titre);
    void removeImage(int index);
    void clearImages();
    Image* getImage(int index) const;
    int getImageCount() const;

public slots:
    // Methods for QML
    Q_INVOKABLE void addImageQml(const QString &type, int identifiant, const QString &titre);
    Q_INVOKABLE void removeImageQml(int index);
    Q_INVOKABLE void clearImagesQml();

private slots:
    void onImageChanged();

private:
    QList<Image*> m_images;
    void connectImageSignals(Image* image);
    void disconnectImageSignals(Image* image);
};
