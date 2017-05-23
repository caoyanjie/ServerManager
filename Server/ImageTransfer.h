#ifndef IMAGETRANSFER_H
#define IMAGETRANSFER_H

#include <QObject>
#include "ImageProvider.h"

class ImageTransfer : public QObject
{
    Q_OBJECT
public:
    explicit ImageTransfer();

private:
    ImageProvider *imageProvider;

signals:

public slots:
    void setImage(QImage image);
};

#endif // IMAGETRANSFER_H
