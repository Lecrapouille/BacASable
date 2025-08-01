#include "ViewModel.h"

ImageViewModel::ImageViewModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ImageViewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_images.size();
}

QVariant ImageViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_images.size())
        return QVariant();

    Image *image = m_images.at(index.row());

    switch (role) {
    case TypeRole:
        return image->type();
    case IdentifiantRole:
        return image->identifiant();
    case TitreRole:
        return image->titre();
    case ImageObjectRole:
        return QVariant::fromValue(image);
    default:
        return QVariant();
    }
}

bool ImageViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_images.size())
        return false;

    Image *image = m_images.at(index.row());

    switch (role) {
    case TypeRole:
        image->setType(value.toString());
        break;
    case IdentifiantRole:
        image->setIdentifiant(value.toInt());
        break;
    case TitreRole:
        image->setTitre(value.toString());
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, {role});
    return true;
}

Qt::ItemFlags ImageViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QHash<int, QByteArray> ImageViewModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[IdentifiantRole] = "identifiant";
    roles[TitreRole] = "titre";
    roles[ImageObjectRole] = "imageObject";
    return roles;
}

// Méthodes pour C++
void ImageViewModel::addImage(const QString &type, int identifiant, const QString &titre)
{
    beginInsertRows(QModelIndex(), m_images.size(), m_images.size());
    Image *image = new Image(type, identifiant, titre, this);
    connectImageSignals(image);
    m_images.append(image);
    endInsertRows();
}

void ImageViewModel::removeImage(int index)
{
    if (index < 0 || index >= m_images.size())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    Image *image = m_images.takeAt(index);
    disconnectImageSignals(image);
    image->deleteLater();
    endRemoveRows();
}

void ImageViewModel::clearImages()
{
    beginResetModel();
    for (Image *image : m_images) {
        disconnectImageSignals(image);
        image->deleteLater();
    }
    m_images.clear();
    endResetModel();
}

Image* ImageViewModel::getImage(int index) const
{
    if (index < 0 || index >= m_images.size())
        return nullptr;
    return m_images.at(index);
}

int ImageViewModel::getImageCount() const
{
    return m_images.size();
}

// Méthodes pour QML
void ImageViewModel::addImageQml(const QString &type, int identifiant, const QString &titre)
{
    addImage(type, identifiant, titre);
}

void ImageViewModel::removeImageQml(int index)
{
    removeImage(index);
}

void ImageViewModel::clearImagesQml()
{
    clearImages();
}

void ImageViewModel::onImageChanged()
{
    Image *image = qobject_cast<Image*>(sender());
    if (!image)
        return;

    int index = m_images.indexOf(image);
    if (index >= 0) {
        QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex);
    }
}

void ImageViewModel::connectImageSignals(Image* image)
{
    connect(image, &Image::typeChanged, this, &ImageViewModel::onImageChanged);
    connect(image, &Image::identifiantChanged, this, &ImageViewModel::onImageChanged);
    connect(image, &Image::titreChanged, this, &ImageViewModel::onImageChanged);
}

void ImageViewModel::disconnectImageSignals(Image* image)
{
    disconnect(image, &Image::typeChanged, this, &ImageViewModel::onImageChanged);
    disconnect(image, &Image::identifiantChanged, this, &ImageViewModel::onImageChanged);
    disconnect(image, &Image::titreChanged, this, &ImageViewModel::onImageChanged);
}
