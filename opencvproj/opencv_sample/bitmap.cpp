
#include "bitmap.h"
#include <opencv2/opencv.hpp>
using namespace Gdiplus;

#define BEST_BYTE_ALIGNMENT 16
#define COMPUTE_BEST_BYTES_PER_ROW(bpr) ( ( (bpr) + (BEST_BYTE_ALIGNMENT-1) ) & ~(BEST_BYTE_ALIGNMENT-1) )

Bitmap::Bitmap()
    : m_nWidth(0)
    , m_nHeight(0)
    , m_rawData(nullptr)
{
}

Bitmap::Bitmap(int width, int height)
    : m_nWidth(width)
    , m_nHeight(height)
{
    size_t bytesPerRow;

    // Minimum bytes per row is 4 bytes per sample * number of samples.
    bytesPerRow = width * 4;
    // Round to nearest multiple of BEST_BYTE_ALIGNMENT.
    bytesPerRow = COMPUTE_BEST_BYTES_PER_ROW(bytesPerRow);

    m_rawData = (unsigned char*)calloc(1, bytesPerRow * height);
    m_mat = cv::Mat(height, width, CV_8UC4, m_rawData);
}

Bitmap::~Bitmap()
{
    if (m_rawData) {
        free((void*)m_rawData);
        m_rawData = NULL;
    }
}
