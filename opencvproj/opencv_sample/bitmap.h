
#ifndef _BITMAP_
#define _BITMAP_

#pragma once

#include <opencv2/opencv.hpp>

using uint8_t = unsigned char;

namespace Gdiplus
{
    ///////////////////////////////////////////////////////////////////////////////
    // GDI++ Bitmap stub
    //
    class Bitmap
    {
    public:
        Bitmap();
        Bitmap(int width, int height);
        ~Bitmap();

        int GetWidth();
        int GetHeight();
        cv::Mat getCvMat();

        uint8_t* GetRawData();

    private:
        int m_nWidth;
        int m_nHeight;
        uint8_t* m_rawData;
        cv::Mat m_mat;
    };

    inline int Bitmap::GetWidth()
    {
        return m_nWidth;
    }

    inline int Bitmap::GetHeight()
    {
        return m_nHeight;
    }

    inline cv::Mat Bitmap::getCvMat()
    {
        return m_mat;
    }

    inline uint8_t* Bitmap::GetRawData()
    {
        return m_rawData;
    }

} // GDI namespace stub

#endif //_BITMAP_