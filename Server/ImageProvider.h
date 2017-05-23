#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>

class ImageProvider : public QQuickImageProvider
{
public:
    static ImageProvider* GetInstance();
    virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
    void setImage(QImage image);

private:
    ImageProvider();
    ~ImageProvider();
    static ImageProvider *instance;
    QImage m_image;
};

#endif // IMAGEPROVIDER_H
