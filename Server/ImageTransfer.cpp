#include "ImageTransfer.h"

ImageTransfer::ImageTransfer() : QObject()
{
    imageProvider = ImageProvider::GetInstance();
}

void ImageTransfer::setImage(QImage image)
{
    imageProvider->setImage(image);
}
