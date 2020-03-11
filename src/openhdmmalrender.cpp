#if defined(ENABLE_VIDEO_RENDER)

#include "openhdmmalrender.h"

#include <QQuickWindow>
#include <QApplication>
#include <QCoreApplication>

#if defined(__apple__)
#include <VideoToolbox/VideoToolbox.h>

// copied from Qt source to avoid pulling in an objc++ header
QVideoFrame::PixelFormat QtPixelFormatFromCVFormat(OSType avPixelFormat)
{
    // BGRA <-> ARGB "swap" is intentional:
    // to work correctly with GL_RGBA, color swap shaders
    // (in QSG node renderer etc.).
    switch (avPixelFormat) {
    case kCVPixelFormatType_32ARGB:
        return QVideoFrame::Format_BGRA32;
    case kCVPixelFormatType_32BGRA:
        return QVideoFrame::Format_ARGB32;
    case kCVPixelFormatType_24RGB:
        return QVideoFrame::Format_RGB24;
    case kCVPixelFormatType_24BGR:
        return QVideoFrame::Format_BGR24;
    case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
    case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
        return QVideoFrame::Format_NV12;
    case kCVPixelFormatType_422YpCbCr8:
        return QVideoFrame::Format_UYVY;
    case kCVPixelFormatType_422YpCbCr8_yuvs:
        return QVideoFrame::Format_YUYV;
    default:
        return QVideoFrame::Format_Invalid;
    }
}


class CVPixelBufferVideoBuffer : public QAbstractPlanarVideoBuffer
{
public:
    CVPixelBufferVideoBuffer(CVPixelBufferRef buffer): QAbstractPlanarVideoBuffer(NoHandle), m_buffer(buffer), m_mode(NotMapped) {
        CVPixelBufferRetain(m_buffer);
    }

    virtual ~CVPixelBufferVideoBuffer() {
        CVPixelBufferRelease(m_buffer);
    }

    MapMode mapMode() const {
        return m_mode;
    }

    /*
     * Copied from Qt source.
     *
     * The framework version of this class depends on a different rendering method that we
     * don't use, so we copy the method here instead
     */
    int map(QAbstractVideoBuffer::MapMode mode, int *numBytes, int bytesPerLine[4], uchar *data[4]) {
        const size_t nPlanes = CVPixelBufferGetPlaneCount(m_buffer);

        if (!nPlanes) {
            data[0] = map(mode, numBytes, bytesPerLine);
            return data[0] ? 1 : 0;
        }

        if (mode != QAbstractVideoBuffer::NotMapped && m_mode == QAbstractVideoBuffer::NotMapped) {
            CVPixelBufferLockBaseAddress(m_buffer, mode == QAbstractVideoBuffer::ReadOnly
                                                               ? kCVPixelBufferLock_ReadOnly
                                                               : 0);

            if (numBytes)
                *numBytes = CVPixelBufferGetDataSize(m_buffer);

            if (bytesPerLine) {
                bytesPerLine[0] = CVPixelBufferGetBytesPerRowOfPlane(m_buffer, 0);
                bytesPerLine[1] = CVPixelBufferGetBytesPerRowOfPlane(m_buffer, 1);
            }

            if (data) {
                data[0] = static_cast<uchar*>(CVPixelBufferGetBaseAddressOfPlane(m_buffer, 0));
                data[1] = static_cast<uchar*>(CVPixelBufferGetBaseAddressOfPlane(m_buffer, 1));
            }

            m_mode = mode;
        }

        return nPlanes;
    }

    uchar *map(MapMode mode, int *numBytes, int *bytesPerLine) {
        if (mode != NotMapped && m_mode == NotMapped) {
            CVPixelBufferLockBaseAddress(m_buffer, mode == QAbstractVideoBuffer::ReadOnly
                                                               ? kCVPixelBufferLock_ReadOnly
                                                               : 0);

            const size_t nPlanes = CVPixelBufferGetPlaneCount(m_buffer);

            if (numBytes) {
                *numBytes = CVPixelBufferGetDataSize(m_buffer);
            }

            if (bytesPerLine) {
                *bytesPerLine = CVPixelBufferGetBytesPerRow(m_buffer);
            }

            m_mode = mode;

            return (uchar*)CVPixelBufferGetBaseAddress(m_buffer);
        } else {
            return nullptr;
        }
    }

    void unmap() {
        if (m_mode != NotMapped) {
            CVPixelBufferUnlockBaseAddress(m_buffer, m_mode == QAbstractVideoBuffer::ReadOnly
                                                                   ? kCVPixelBufferLock_ReadOnly
                                                                   : 0);
            m_mode = NotMapped;
        }
    }

private:
    CVPixelBufferRef m_buffer = nullptr;
    MapMode m_mode = NotMapped;
};
#endif

OpenHDMMALRender::OpenHDMMALRender(QQuickItem *parent): QQuickItem(parent) {
    QObject::connect(this, &OpenHDMMALRender::newFrameAvailable, this, &OpenHDMMALRender::onNewVideoContentReceived, Qt::QueuedConnection);
}

void OpenHDMMALRender::paintFrame(uint8_t *buffer_data, size_t buffer_length) {
    if (buffer_length < 1024) {
        return;
    }

    QSize s = m_format.frameSize();
    auto stride = s.width();

    QVideoFrame f(buffer_length, s, stride, m_format.pixelFormat());
    f.map(QAbstractVideoBuffer::MapMode::WriteOnly);
    memcpy(f.bits(), buffer_data, buffer_length);
    f.unmap();

    emit newFrameAvailable(f);
}

#if defined(__apple__)
void OpenHDMMALRender::paintFrame(CVImageBufferRef imageBuffer) {
    int width = CVPixelBufferGetWidth(imageBuffer);
    int height = CVPixelBufferGetHeight(imageBuffer);

    QVideoFrame::PixelFormat format = QtPixelFormatFromCVFormat(CVPixelBufferGetPixelFormatType(imageBuffer));

    QAbstractVideoBuffer *buffer = new CVPixelBufferVideoBuffer(imageBuffer);
    QVideoFrame f(buffer, QSize(width, height), format);
    emit newFrameAvailable(f);
}
#endif

OpenHDMMALRender::~OpenHDMMALRender() {

}

void OpenHDMMALRender::setVideoSurface(QAbstractVideoSurface *surface) {
    if (m_surface && m_surface != surface && m_surface->isActive()) {
        m_surface->stop();
    }

    m_surface = surface;
}

void OpenHDMMALRender::setFormat(int width, int heigth, QVideoFrame::PixelFormat i_format) {
    QSize size(width, heigth);
    QVideoSurfaceFormat format(size, i_format);

    if (m_surface)  {
        if (m_surface->isActive()) {
            m_surface->stop();
        }
        m_format = m_surface->nearestFormat(format);
        m_surface->start(m_format);
    }
}

void OpenHDMMALRender::onNewVideoContentReceived(const QVideoFrame &frame) {
    if (m_surface) {
        m_surface->present(frame);
    }
}

#endif
