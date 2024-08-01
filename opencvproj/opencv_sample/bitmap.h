
#ifndef _BITMAP_
#define _BITMAP_

#pragma once

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

        uint8_t* GetRawData();

    protected:
        int m_nWidth;
        int m_nHeight;
        uint8_t* m_rawData;
    };

    inline int Bitmap::GetWidth()
    {
        return m_nWidth;
    }

    inline int Bitmap::GetHeight()
    {
        return m_nHeight;
    }

} // GDI namespace stub

#endif //_BITMAP_