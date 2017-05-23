#include "ImageProvider.h"

ImageProvider* ImageProvider::instance = nullptr;
ImageProvider::ImageProvider() : QQuickImageProvider(QQuickImageProvider::Image)
{

}

ImageProvider::~ImageProvider()
{

}

ImageProvider *ImageProvider::GetInstance()
{
    if (!instance)
    {
        instance = new ImageProvider();
    }
    return instance;
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    return m_image;
}

void ImageProvider::setImage(QImage image)
{
    m_image = image;
}
