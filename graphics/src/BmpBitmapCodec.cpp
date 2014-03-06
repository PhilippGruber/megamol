/*
 * BmpBitmapCodec.cpp
 *
 * Copyright (C) 2009 by Sebastian Grottel.
 * (Copyright (C) 2006 - 2009 by Visualisierungsinstitut Universitaet Stuttgart.)
 * Alle Rechte vorbehalten.
 */

#include "vislib/BmpBitmapCodec.h"
#include <climits>
#include "the/assert.h"
#include "vislib/BitmapImage.h"
#include "vislib/mathfunctions.h"
#include "the/trace.h"


#ifndef _WIN32

/*
 * Declaration of structs from MSDN
 */
typedef struct tagBITMAPFILEHEADER {
  WORD  bfType;
  unsigned int bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  unsigned int bfOffBits;
} BITMAPFILEHEADER;
typedef struct tagBITMAPINFOHEADER {
  unsigned int biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  unsigned int biCompression;
  unsigned int biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  unsigned int biClrUsed;
  unsigned int biClrImportant;
} BITMAPINFOHEADER;
typedef struct tagBITMAPCOREHEADER {
  unsigned int bcSize;
  WORD  bcWidth;
  WORD  bcHeight;
  WORD  bcPlanes;
  WORD  bcBitCount;
} BITMAPCOREHEADER;
typedef long FXPT2DOT30;
typedef struct tagCIEXYZ {
  FXPT2DOT30 ciexyzX;
  FXPT2DOT30 ciexyzY;
  FXPT2DOT30 ciexyzZ;
} CIEXYZ;
typedef struct tagCIEXYZTRIPLE {
  CIEXYZ ciexyzRed;
  CIEXYZ ciexyzGreen;
  CIEXYZ ciexyzBlue;
} CIEXYZTRIPLE;
typedef struct {
  unsigned int        bV4Size;
  LONG         bV4Width;
  LONG         bV4Height;
  WORD         bV4Planes;
  WORD         bV4BitCount;
  unsigned int        bV4V4Compression;
  unsigned int        bV4SizeImage;
  LONG         bV4XPelsPerMeter;
  LONG         bV4YPelsPerMeter;
  unsigned int        bV4ClrUsed;
  unsigned int        bV4ClrImportant;
  unsigned int        bV4RedMask;
  unsigned int        bV4GreenMask;
  unsigned int        bV4BlueMask;
  unsigned int        bV4AlphaMask;
  unsigned int        bV4CSType;
  CIEXYZTRIPLE bV4Endpoints;
  unsigned int        bV4GammaRed;
  unsigned int        bV4GammaGreen;
  unsigned int        bV4GammaBlue;
} BITMAPV4HEADER;
typedef struct {
  unsigned int        bV5Size;
  LONG         bV5Width;
  LONG         bV5Height;
  WORD         bV5Planes;
  WORD         bV5BitCount;
  unsigned int        bV5Compression;
  unsigned int        bV5SizeImage;
  LONG         bV5XPelsPerMeter;
  LONG         bV5YPelsPerMeter;
  unsigned int        bV5ClrUsed;
  unsigned int        bV5ClrImportant;
  unsigned int        bV5RedMask;
  unsigned int        bV5GreenMask;
  unsigned int        bV5BlueMask;
  unsigned int        bV5AlphaMask;
  unsigned int        bV5CSType;
  CIEXYZTRIPLE bV5Endpoints;
  unsigned int        bV5GammaRed;
  unsigned int        bV5GammaGreen;
  unsigned int        bV5GammaBlue;
  unsigned int        bV5Intent;
  unsigned int        bV5ProfileData;
  unsigned int        bV5ProfileSize;
  unsigned int        bV5Reserved;
} BITMAPV5HEADER;
#define BI_RGB        0L
typedef struct tagRGBQUAD {
  uint8_t    rgbBlue;
  uint8_t    rgbGreen;
  uint8_t    rgbRed;
  uint8_t    rgbReserved;
} RGBQUAD;

#endif /* _WIN32 */


/*
 * vislib::graphics::BmpBitmapCodec::BmpBitmapCodec
 */
vislib::graphics::BmpBitmapCodec::BmpBitmapCodec(void)
        : AbstractBitmapCodec() {
    // Intentionally empty
}


/*
 * vislib::graphics::BmpBitmapCodec::~BmpBitmapCodec
 */
vislib::graphics::BmpBitmapCodec::~BmpBitmapCodec(void) {
    // Intentionally empty
}


/*
 * vislib::graphics::BmpBitmapCodec::AutoDetect
 */
int vislib::graphics::BmpBitmapCodec::AutoDetect(const void *mem,
        size_t size) const {
    if (size < 2) return -1; // insufficient preview data
    const char *data = static_cast<const char*>(mem);
    if ((data[0] != 'B') || (data[1] != 'M')) return 0; // wrong magic number
    return 1; // seems ok
}


/*
 * vislib::graphics::BmpBitmapCodec::CanAutoDetect
 */
bool vislib::graphics::BmpBitmapCodec::CanAutoDetect(void) const {
    return true;
}


/*
 * vislib::graphics::BmpBitmapCodec::FileNameExtsA
 */
const char* vislib::graphics::BmpBitmapCodec::FileNameExtsA(void) const {
    return ".bmp";
}


/*
 * vislib::graphics::BmpBitmapCodec::FileNameExtsW
 */
const wchar_t* vislib::graphics::BmpBitmapCodec::FileNameExtsW(void) const {
    return L".bmp";
}


/*
 * vislib::graphics::BmpBitmapCodec::NameA
 */
const char * vislib::graphics::BmpBitmapCodec::NameA(void) const {
    return "Windows Bitmap";
}


/*
 * vislib::graphics::PpmBitmapCodec::NameW
 */
const wchar_t * vislib::graphics::BmpBitmapCodec::NameW(void) const {
    return L"Windows Bitmap";
}


/*
 * vislib::graphics::BmpBitmapCodec::loadFromMemory
 */
bool vislib::graphics::BmpBitmapCodec::loadFromMemory(const void *mem, size_t size) {
    THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO, "Loading BMP ...\n");
    const uint8_t *memBytes = static_cast<const uint8_t *>(mem);

    const BITMAPFILEHEADER *bfh
        = reinterpret_cast<const BITMAPFILEHEADER *>(memBytes);
    if (size < sizeof(BITMAPFILEHEADER)) {
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO, "Failed: insufficient data\n");
        return false; // insufficient data
    }
    if (bfh->bfType != 0x4D42) {
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO, "Failed: wrong magic number\n");
        return false; // wrong magic number
    }
    if (size < bfh->bfSize) {
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO, "Failed: truncated data\n");
        return false; // truncated data
    }
    if (bfh->bfReserved1 != 0) {
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO, "Failed: unexpected extension\n");
        return false; // unexpected extension
    }
    if (bfh->bfReserved2 != 0) {
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO, "Failed: unexpected extension\n");
        return false; // unexpected extension
    }

#if !defined(VISLIB_BMP_LOAD_BY_HAND) && defined(_WIN32)
    HDC screenDC = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

    if (screenDC == NULL) {
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_ERROR,
            "Failed: unable to get screen device context\n");
        return false;
    }

    HBITMAP bmp = ::CreateDIBitmap(screenDC, 
        reinterpret_cast<const BITMAPINFOHEADER*>(memBytes
            + sizeof(BITMAPFILEHEADER)),
        CBM_INIT, memBytes + bfh->bfOffBits,
        reinterpret_cast<const BITMAPINFO*>(memBytes
            + sizeof(BITMAPFILEHEADER)),
        DIB_RGB_COLORS);

    if (bmp == NULL) {
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_ERROR,
            "Failed: unable to create bitmap\n");
        ::DeleteDC(screenDC);

    } else {
        BITMAPINFO info;
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biBitCount = 0;

        if (::GetDIBits(screenDC, bmp, 0, 0, NULL, &info, DIB_RGB_COLORS)
                == 0) {
            THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_ERROR,
                "Failed: unable to query bitmap attributes\n");
            ::DeleteObject(bmp);
            ::DeleteDC(screenDC);
            return false;
        }

        this->image().CreateImage(info.bmiHeader.biWidth,
            info.bmiHeader.biHeight, 3,
            vislib::graphics::BitmapImage::CHANNELTYPE_BYTE);
        this->image().LabelChannelsRGB();
        uint8_t *buf = new uint8_t[(info.bmiHeader.biWidth + 7) * 3];

        info.bmiHeader.biBitCount = 24;
        info.bmiHeader.biCompression = BI_RGB;

        size_t lineDataSize = this->image().Width() * 3;
        for (unsigned int y = 0; y < this->image().Height(); y ++) {
            uint8_t* bmpLine = this->image().PeekDataAs<uint8_t>()
                + (lineDataSize * (this->image().Height() - (y + 1)));
            ::GetDIBits(screenDC, bmp, y, 1, buf, &info, DIB_RGB_COLORS);
            ::memcpy(bmpLine, buf, lineDataSize);
            for (size_t x = 0; x < lineDataSize; x += 3) { // BGR to RGB
                uint8_t b = bmpLine[x];
                bmpLine[x] = bmpLine[x + 2];
                bmpLine[x + 2] = b;
            }
        }

        delete[] buf;
        ::DeleteObject(bmp);
        ::DeleteDC(screenDC);
        return true;
    }

    return false;

#else /* !defined(VISLIB_BMP_LOAD_BY_HAND) && defined(_WIN32) */

    switch (*reinterpret_cast<const unsigned int*>(
            memBytes + sizeof(BITMAPFILEHEADER))) {

        case sizeof(BITMAPINFOHEADER) :
            THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO, "Loading BITMAPINFOHEADER ...\n");
            return this->loadWithBitmapInfoHeader(bfh, memBytes);

        case sizeof(BITMAPCOREHEADER) :
            THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO,
                "Failed: BITMAPCOREHEADER not supported\n");
            break; // unsupported ATM

        case sizeof(BITMAPV4HEADER) :
            THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO,
                "Failed: BITMAPV4HEADER not supported\n");
            break; // unsupported ATM

        case sizeof(BITMAPV5HEADER) :
            THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO,
                "Failed: BITMAPV5HEADER not supported\n");
            break; // unsupported ATM

        default:
            THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO,
                "Failed: unable to find BITMAPINFOHEADER\n");
            break;
    }
    return false; // unsupported subformat
#endif /* !defined(VISLIB_BMP_LOAD_BY_HAND) && defined(_WIN32) */
}


/*
 * vislib::graphics::BmpBitmapCodec::loadFromMemoryImplemented
 */
bool vislib::graphics::BmpBitmapCodec::loadFromMemoryImplemented(void) const {
    return true;
}


/*
 * vislib::graphics::BmpBitmapCodec::saveToMemory
 */
bool vislib::graphics::BmpBitmapCodec::saveToMemory(vislib::RawStorage& outmem) const {
    BitmapImage bmiStore;
    const BitmapImage *img = &this->image();

    if (img->GetChannelCount() == 0) {
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_ERROR,
            "Fail to save bmp: image has no channels\n");
        return false;
    }

    if (img->GetChannelType() != BitmapImage::CHANNELTYPE_BYTE) {
        BitmapImage tmpl(1, 1, 3, BitmapImage::CHANNELTYPE_BYTE);
        tmpl.LabelChannelsRGB();
        bmiStore.ConvertFrom(this->image(), tmpl);
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO, "Image converted to RGB8");
        img = & bmiStore;
    }

    unsigned int bpp = img->GetChannelCount();
    unsigned int ridx = UINT_MAX;
    unsigned int gidx = UINT_MAX;
    unsigned int bidx = UINT_MAX;

    if (bpp == 0) {
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_ERROR,
            "Fail to save bmp: image has no channels\n");
        return false;
    }
    for (unsigned int i = 0; i < bpp; i++) {
        switch (img->GetChannelLabel(i)) {
            case BitmapImage::CHANNEL_RED:
                if (ridx == UINT_MAX) ridx = i;
                break;
            case BitmapImage::CHANNEL_GREEN:
                if (gidx == UINT_MAX) gidx = i;
                break;
            case BitmapImage::CHANNEL_BLUE:
                if (bidx == UINT_MAX) bidx = i;
                break;
            case BitmapImage::CHANNEL_GRAY:
                if (ridx == UINT_MAX) ridx = i;
                if (gidx == UINT_MAX) gidx = i;
                if (bidx == UINT_MAX) bidx = i;
                break;
#ifndef _WIN32
            default:
                // nothing to do here
                break;
#endif /* !_WIN32 */
        }
    }
    if ((ridx == UINT_MAX) && (gidx == UINT_MAX) && (bidx == UINT_MAX)) {
        ridx = 0;
        gidx = ((bpp == 3) ? 1 : 0);
        bidx = ((bpp == 3) ? 2 : 0);
    }

    size_t slsize = img->Width() * 3;
    size_t slcut = 0;
    if (slsize % 4) {
        slcut = 4 - (slsize % 4);
    }
    size_t headersSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    outmem.EnforceSize(headersSize + (slsize + slcut) * img->Height());

    BITMAPFILEHEADER *bfh = outmem.As<BITMAPFILEHEADER>();
    BITMAPINFOHEADER *bih
        = outmem.AsAt<BITMAPINFOHEADER>(sizeof(BITMAPFILEHEADER));

    bfh->bfType = 0x4D42; //BM
    bfh->bfSize = static_cast<unsigned int>(headersSize
        + (slsize + slcut) * img->Height());
    bfh->bfReserved1 = 0;
    bfh->bfReserved2 = 0;
    bfh->bfOffBits = static_cast<unsigned int>(headersSize);

    bih->biSize = sizeof(BITMAPINFOHEADER);
    bih->biWidth = img->Width();
    bih->biHeight = -static_cast<LONG>(img->Height());
    bih->biPlanes = 1;
    bih->biBitCount = 24;
    bih->biCompression = BI_RGB;
    bih->biSizeImage = 0;
    bih->biXPelsPerMeter = 0;
    bih->biYPelsPerMeter = 0;
    bih->biClrUsed = 0;
    bih->biClrImportant = 0;

    const uint8_t *imgDat = img->PeekDataAs<uint8_t>();
    uint8_t *bmpDat = outmem.AsAt<uint8_t>(headersSize);
    for (unsigned int y = 0; y < img->Height(); y++) {
        for (unsigned int x = 0; x < img->Width(); x++, bmpDat += 3, imgDat += bpp) {
            bmpDat[2] = ((ridx == UINT_MAX) ? 0 : imgDat[ridx]);
            bmpDat[1] = ((gidx == UINT_MAX) ? 0 : imgDat[gidx]);
            bmpDat[0] = ((bidx == UINT_MAX) ? 0 : imgDat[bidx]);
        }
        bmpDat += slcut;
    }

    return true;
}


/*
 * vislib::graphics::BmpBitmapCodec::saveToMemoryImplemented
 */
bool vislib::graphics::BmpBitmapCodec::saveToMemoryImplemented(void) const {
    return true;
}


#if defined(VISLIB_BMP_LOAD_BY_HAND) || !defined(_WIN32)

/*
 * vislib::graphics::BmpBitmapCodec::loadWithBitmapInfoHeader
 */
bool vislib::graphics::BmpBitmapCodec::loadWithBitmapInfoHeader(
        const void *header, const uint8_t *dat) {
    const BITMAPFILEHEADER *bfh
        = static_cast<const BITMAPFILEHEADER*>(header);
    const BITMAPINFOHEADER *bih = reinterpret_cast<const BITMAPINFOHEADER*>(
        dat + sizeof(BITMAPFILEHEADER));

    THE_ASSERT(bih->biSize == sizeof(BITMAPINFOHEADER));
    // bih->biWidth
    // bih->biHeight
    if (bih->biPlanes != 1) {
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO, "Failed: biPlanes != 1\n");
        return false;
    }
    if ((bih->biBitCount != 1) && (bih->biBitCount != 2)
            && (bih->biBitCount != 4) && (bih->biBitCount != 8)
            && (bih->biBitCount != 16) && (bih->biBitCount != 24)
            && (bih->biBitCount != 32)) {
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO,
            "Failed: biBitCount == %d unsupported\n", bih->biBitCount);
        return false;
    }
    if (bih->biCompression != BI_RGB) {
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO,
            "Failed: biCompression != BI_RGB is unsupported\n");
        return false;
    }
    // bih->biSizeImage is not relevant at all
    // bih->biXPelsPerMeter
    // bih->biYPelsPerMeter
    if (bih->biClrUsed > (1U << bih->biBitCount)) {
        THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO,
            "Failed: biClrUsed requeste more colours than allowed\n");
        return false;
    }
    // bih->biClrImportant

    switch (bih->biBitCount) {
        case 1: {
            unsigned int colPalSize = bih->biClrUsed;
            if (colPalSize == 0) {
                colPalSize = 2;
            }
            const RGBQUAD *colPal = reinterpret_cast<const RGBQUAD *>(
                dat + sizeof(BITMAPFILEHEADER) + bih->biSize);

            int stride = bih->biWidth;
            if (stride % 8) {
                stride += 8 - (stride % 8);
            }
            stride /= 8;
            if (stride % 4) {
                stride += 4 - (stride % 4);
            }
            return this->loadBitmap1(bih->biWidth, bih->biHeight,
                stride, colPal, colPalSize, dat + bfh->bfOffBits);
        }
        case 4: {
            unsigned int colPalSize = bih->biClrUsed;
            if (colPalSize == 0) {
                colPalSize = 16;
            }
            const RGBQUAD *colPal = reinterpret_cast<const RGBQUAD *>(
                dat + sizeof(BITMAPFILEHEADER) + bih->biSize);

            int stride = bih->biWidth;
            if (stride % 2) {
                stride++;
            }
            stride /= 2;
            if (stride % 4) {
                stride += 4 - (stride % 4);
            }
            return this->loadBitmap4(bih->biWidth, bih->biHeight,
                stride, colPal, colPalSize, dat + bfh->bfOffBits);
        }
        case 8: {
            unsigned int colPalSize = bih->biClrUsed;
            if (colPalSize == 0) {
                colPalSize = 256;
            }
            const RGBQUAD *colPal = reinterpret_cast<const RGBQUAD *>(
                dat + sizeof(BITMAPFILEHEADER) + bih->biSize);

            int stride = bih->biWidth;
            if (stride % 4) {
                stride += 4 - (stride % 4);
            }
            return this->loadBitmap8(bih->biWidth, bih->biHeight,
                stride, colPal, colPalSize, dat + bfh->bfOffBits);
        }
        case 16: {
            int stride = (bih->biWidth * 2);
            if (stride % 4) {
                stride += 4 - (stride % 4);
            }
            return this->loadBitmap16(bih->biWidth, bih->biHeight,
                stride, dat + bfh->bfOffBits);
        }
        case 24: {
            int stride = (bih->biWidth * 3);
            if (stride % 4) {
                stride += 4 - (stride % 4);
            }
            return this->loadBitmap24(bih->biWidth, bih->biHeight,
                stride, dat + bfh->bfOffBits);
        }
        case 32:
            return this->loadBitmap32(bih->biWidth, bih->biHeight,
                dat + bfh->bfOffBits);
        default:
            THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_INFO,
                "Failed: biBitCount == %d not supported\n",
                bih->biBitCount);
    }

    return false; // Not supported
}


/*
 * vislib::graphics::BmpBitmapCodec::loadBitmap1
 */
bool vislib::graphics::BmpBitmapCodec::loadBitmap1(int width, int height,
        int stride, const void *colPalDat, unsigned int colPalSize,
        const uint8_t *dat) {
    RGBQUAD black = {0, 0, 0, 0};
    const RGBQUAD *colPal = static_cast<const RGBQUAD *>(colPalDat);
    if (colPalSize == 0) {
        colPalSize = 1;
        colPal = &black;
    }

    this->image().CreateImage(width, vislib::math::Abs(height), 3,
        vislib::graphics::BitmapImage::CHANNELTYPE_BYTE);
    this->image().LabelChannelsRGB();

    int y = 0;
    int yEnd = height;
    int yStep = 1;
    if (yEnd < 0) {
        y = -yEnd - 1;
        yEnd = -1;
        yStep = -1;
    }

    for (; y != yEnd; y += yStep, dat += stride) {
        const uint8_t *imgLine = dat;
        uint8_t* bmpLine = this->image().PeekDataAs<uint8_t>()
            + (width * 3 * (this->image().Height() - (y + 1)));
        for (int x = 0; x < width; x++, bmpLine += 3) {
            unsigned int idx = (imgLine[0] >> (7 - (x % 8))) % 2;
            if ((x % 8) == 7) imgLine++;
            if (idx >= colPalSize) {
                THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_ERROR,
                    "Colour index %u out of palette size %u\n", idx,
                    colPalSize);
                idx = 0;
            }
            bmpLine[0] = colPal[idx].rgbRed;
            bmpLine[1] = colPal[idx].rgbGreen;
            bmpLine[2] = colPal[idx].rgbBlue;
        }
    }

    return true;
}


/*
 * vislib::graphics::BmpBitmapCodec::loadBitmap4
 */
bool vislib::graphics::BmpBitmapCodec::loadBitmap4(int width, int height,
        int stride, const void *colPalDat, unsigned int colPalSize,
        const uint8_t *dat) {
    RGBQUAD black = {0, 0, 0, 0};
    const RGBQUAD *colPal = static_cast<const RGBQUAD *>(colPalDat);
    if (colPalSize == 0) {
        colPalSize = 1;
        colPal = &black;
    }

    this->image().CreateImage(width, vislib::math::Abs(height), 3,
        vislib::graphics::BitmapImage::CHANNELTYPE_BYTE);
    this->image().LabelChannelsRGB();

    int y = 0;
    int yEnd = height;
    int yStep = 1;
    if (yEnd < 0) {
        y = -yEnd - 1;
        yEnd = -1;
        yStep = -1;
    }

    for (; y != yEnd; y += yStep, dat += stride) {
        const uint8_t *imgLine = dat;
        uint8_t* bmpLine = this->image().PeekDataAs<uint8_t>()
            + (width * 3 * (this->image().Height() - (y + 1)));
        for (int x = 0; x < width; x++, bmpLine += 3) {
            unsigned int idx;
            if (x % 2) {
                idx = imgLine[0] % 16;
                imgLine++;
            } else {
                idx = imgLine[0] >> 4;
            }
            if (idx >= colPalSize) {
                THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_ERROR,
                    "Colour index %u out of palette size %u\n", idx,
                    colPalSize);
                idx = 0;
            }
            bmpLine[0] = colPal[idx].rgbRed;
            bmpLine[1] = colPal[idx].rgbGreen;
            bmpLine[2] = colPal[idx].rgbBlue;
        }
    }

    return true;
}


/*
 * vislib::graphics::BmpBitmapCodec::loadBitmap8
 */
bool vislib::graphics::BmpBitmapCodec::loadBitmap8(int width, int height,
        int stride, const void *colPalDat, unsigned int colPalSize,
        const uint8_t *dat) {
    RGBQUAD black = {0, 0, 0, 0};
    const RGBQUAD *colPal = static_cast<const RGBQUAD *>(colPalDat);
    if (colPalSize == 0) {
        colPalSize = 1;
        colPal = &black;
    }

    this->image().CreateImage(width, vislib::math::Abs(height), 3,
        vislib::graphics::BitmapImage::CHANNELTYPE_BYTE);
    this->image().LabelChannelsRGB();

    int y = 0;
    int yEnd = height;
    int yStep = 1;
    if (yEnd < 0) {
        y = -yEnd - 1;
        yEnd = -1;
        yStep = -1;
    }

    for (; y != yEnd; y += yStep, dat += stride) {
        const uint8_t *imgLine = dat;
        uint8_t* bmpLine = this->image().PeekDataAs<uint8_t>()
            + (width * 3 * (this->image().Height() - (y + 1)));
        for (int x = 0; x < width; x++, bmpLine += 3, imgLine++) {
            unsigned int idx = imgLine[0];
            if (idx >= colPalSize) {
                THE_TRACE(THE_TRCCHL_DEFAULT, THE_TRCLVL_ERROR,
                    "Colour index %u out of palette size %u\n", idx,
                    colPalSize);
                idx = 0;
            }
            bmpLine[0] = colPal[idx].rgbRed;
            bmpLine[1] = colPal[idx].rgbGreen;
            bmpLine[2] = colPal[idx].rgbBlue;
        }
    }

    return true;
}


/*
 * vislib::graphics::BmpBitmapCodec::loadBitmap16
 */
bool vislib::graphics::BmpBitmapCodec::loadBitmap16(int width, int height,
        int stride, const uint8_t *dat) {
    this->image().CreateImage(width, vislib::math::Abs(height), 3,
        vislib::graphics::BitmapImage::CHANNELTYPE_BYTE);
    this->image().LabelChannelsRGB();

    int y = 0;
    int yEnd = height;
    int yStep = 1;
    if (yEnd < 0) {
        y = -yEnd - 1;
        yEnd = -1;
        yStep = -1;
    }

    for (; y != yEnd; y += yStep, dat += stride) {
        const uint8_t *imgLine = dat;
        uint8_t* bmpLine = this->image().PeekDataAs<uint8_t>()
            + (width * 3 * (this->image().Height() - (y + 1)));
        for (int x = 0; x < width; x++, bmpLine += 3, imgLine += 2) {
            bmpLine[0] = static_cast<uint8_t>(static_cast<float>(
                (imgLine[1] >> 2) % 32) * 255.0f / 31.0f);
            bmpLine[1] = static_cast<uint8_t>(static_cast<float>(
                ((imgLine[1] % 4) << 3) + (imgLine[0] >> 5))
                * 255.0f / 31.0f);
            bmpLine[2] = static_cast<uint8_t>(static_cast<float>(imgLine[0] % 32)
                * 255.0f / 31.0f);
        }
    }

    return true;
}


/*
 * vislib::graphics::BmpBitmapCodec::loadBitmap24
 */
bool vislib::graphics::BmpBitmapCodec::loadBitmap24(int width, int height,
        int stride, const uint8_t *dat) {
    this->image().CreateImage(width, vislib::math::Abs(height), 3,
        vislib::graphics::BitmapImage::CHANNELTYPE_BYTE);
    this->image().LabelChannelsRGB();

    int y = 0;
    int yEnd = height;
    int yStep = 1;
    if (yEnd < 0) {
        y = -yEnd - 1;
        yEnd = -1;
        yStep = -1;
    }

    size_t lineDataSize = this->image().Width() * 3;
    for (; y != yEnd; y += yStep, dat += stride) {
        uint8_t* bmpLine = this->image().PeekDataAs<uint8_t>()
            + (lineDataSize * (this->image().Height() - (y + 1)));
        memcpy(bmpLine, dat, lineDataSize);
        for (size_t x = 0; x < lineDataSize; x += 3) { // BGR to RGB
            uint8_t b = bmpLine[x];
            bmpLine[x] = bmpLine[x + 2];
            bmpLine[x + 2] = b;
        }
    }

    return true;
}


/*
 * vislib::graphics::BmpBitmapCodec::loadBitmap32
 */
bool vislib::graphics::BmpBitmapCodec::loadBitmap32(int width, int height,
        const uint8_t *dat) {
    this->image().CreateImage(width, vislib::math::Abs(height), 4,
        vislib::graphics::BitmapImage::CHANNELTYPE_BYTE);
    this->image().LabelChannelsRGBA();

    int y = 0;
    int yEnd = height;
    int yStep = 1;
    if (yEnd < 0) {
        y = -yEnd - 1;
        yEnd = -1;
        yStep = -1;
    }

    for (; y != yEnd; y += yStep, dat += width * 4) {
        uint8_t* bmpLine = this->image().PeekDataAs<uint8_t>()
            + (width * 4 * (this->image().Height() - (y + 1)));
        memcpy(bmpLine, dat, width * 4);
        for (int x = 0; x < width * 4; x += 4) { // BGRA to RGBA
            uint8_t b = bmpLine[x];
            bmpLine[x] = bmpLine[x + 2];
            bmpLine[x + 2] = b;
        }
    }

    return true;
}

#endif /* defined(VISLIB_BMP_LOAD_BY_HAND) || !defined(_WIN32) */
