/*  YUView - YUV player with advanced analytics toolset
*   Copyright (C) 2015  Institut für Nachrichtentechnik
*                       RWTH Aachen University, GERMANY
*
*   YUView is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   YUView is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with YUView.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "yuvsource.h"
#include <QtEndian>
#include <QTime>
#include "stdio.h"

inline quint32 SwapInt32(quint32 arg) {
  quint32 result;
  result = ((arg & 0xFF) << 24) | ((arg & 0xFF00) << 8) | ((arg >> 8) & 0xFF00) | ((arg >> 24) & 0xFF);
  return result;
}

inline quint16 SwapInt16(quint16 arg) {
  quint16 result;
  result = (quint16)(((arg << 8) & 0xFF00) | ((arg >> 8) & 0xFF));
  return result;
}

inline quint32 SwapInt32BigToHost(quint32 arg) {
#if __BIG_ENDIAN__ || IS_BIG_ENDIAN
  return arg;
#else
  return SwapInt32(arg);
#endif
}

inline quint32 SwapInt32LittleToHost(quint32 arg) {
#if __LITTLE_ENDIAN__ || IS_LITTLE_ENDIAN
  return arg;
#else
  return SwapInt32(arg);
#endif
}

inline quint16 SwapInt16LittleToHost(quint16 arg) {
#if __LITTLE_ENDIAN__
  return arg;
#else
  return SwapInt16(arg);
#endif
}

/* Get the number of bytes for a frame with this yuvPixelFormat and the given size
*/
qint64 yuvSource::yuvPixelFormat::bytesPerFrame(QSize frameSize)
{
  if (name == "" || !frameSize.isValid())
    return 0;

  qint64 numSamples = frameSize.height() * frameSize.width();
  unsigned remainder = numSamples % bitsPerPixelDenominator;
  qint64 bits = numSamples / bitsPerPixelDenominator;
  if (remainder == 0) {
    bits *= bitsPerPixelNominator;
  }
  else {
    printf("warning: pixels not divisable by bpp denominator for pixel format '%d' - rounding up\n", name);
    bits = (bits + 1) * bitsPerPixelNominator;
  }
  if (bits % 8 != 0) {
    printf("warning: bits not divisible by 8 for pixel format '%d' - rounding up\n", name);
    bits += 8;
  }

  return bits / 8;
}

/* The default constructor of the YUVFormatList will fill the list with all supported YUV file formats.
 * Don't forget to implement actual support for all of them in the conversion functions.
*/
yuvSource::YUVFormatList::YUVFormatList()
{
  append( yuvSource::yuvPixelFormat()); // "Unknown Pixel Format"
  append( yuvSource::yuvPixelFormat("GBR 12-bit planar", 12, 48, 1, 1, 1, true, 2) );
  append( yuvSource::yuvPixelFormat("RGBA 8-bit", 8, 32, 1, 1, 1, false) );
  append( yuvSource::yuvPixelFormat("RGB 8-bit", 8, 24, 1, 1, 1, false) );
  append( yuvSource::yuvPixelFormat("BGR 8-bit", 8, 24, 1, 1, 1, false) );
  append( yuvSource::yuvPixelFormat("4:4:4 Y'CbCr 16-bit LE planar", 16, 48, 1, 1, 1, true, 2) );
  append( yuvSource::yuvPixelFormat("4:4:4 Y'CbCr 16-bit BE planar", 16, 48, 1, 1, 1, true, 2) );
  append( yuvSource::yuvPixelFormat("4:4:4 Y'CbCr 12-bit LE planar", 12, 48, 1, 1, 1, true, 2) );
  append( yuvSource::yuvPixelFormat("4:4:4 Y'CbCr 12-bit BE planar", 12, 48, 1, 1, 1, true, 2) );
  append( yuvSource::yuvPixelFormat("4:4:4 Y'CbCr 10-bit LE planar", 10, 48, 1, 1, 1, true, 2) );
  append( yuvSource::yuvPixelFormat("4:4:4 Y'CbCr 10-bit BE planar", 10, 48, 1, 1, 1, true, 2) );
  append( yuvSource::yuvPixelFormat("4:4:4 Y'CbCr 8-bit planar", 8, 24, 1, 1, 1, true) );
  append( yuvSource::yuvPixelFormat("4:4:4 Y'CrCb 8-bit planar", 8, 24, 1, 1, 1, true) );
  append( yuvSource::yuvPixelFormat("4:2:2 Y'CbCr 8-bit planar", 8, 16, 1, 2, 1, true) );
  append( yuvSource::yuvPixelFormat("4:2:2 Y'CrCb 8-bit planar", 8, 16, 1, 2, 1, true) );
  append( yuvSource::yuvPixelFormat("4:2:2 8-bit packed", 8, 16, 1, 2, 1, false) );
  append( yuvSource::yuvPixelFormat("4:2:2 10-bit packed 'v210'", 10, 128, 6, 2, 1, false, 2) );
  append( yuvSource::yuvPixelFormat("4:2:2 10-bit packed (UYVY)", 10, 128, 6, 2, 1, true, 2) );
  append( yuvSource::yuvPixelFormat("4:2:0 Y'CbCr 10-bit LE planar", 10, 24, 1, 2, 2, true, 2) );
  append( yuvSource::yuvPixelFormat("4:2:0 Y'CbCr 8-bit planar", 8, 12, 1, 2, 2, true) );
  append( yuvSource::yuvPixelFormat("4:1:1 Y'CbCr 8-bit planar", 8, 12, 1, 4, 1, true) );
  append( yuvSource::yuvPixelFormat("4:0:0 8-bit", 8, 8, 1, 0, 0, true) );
}

/* Put all the names of the yuvPixelFormats into a list and return it
*/
QStringList yuvSource::YUVFormatList::getFormatedNames()
{
  QStringList l;
  for (int i = 0; i < count(); i++)
  {
    l.append( at(i).name );
  }
  return l;
}

yuvSource::yuvPixelFormat yuvSource::YUVFormatList::getFromName(QString name)
{
  for (int i = 0; i < count(); i++)
  {
    if ( at(i) == name )
      return at(i);
  }
  // If the format could not be found, we return the "Unknown Pixel Format" format (which is index 0)
  return at(0);
}

// Initialize the static yuvFormatList
yuvSource::YUVFormatList yuvSource::yuvFormatList;

yuvSource::yuvSource()
{
  // preset internal values
  srcPixelFormat = yuvFormatList.getFromName("Unknown Pixel Format");
  interpolationMode = NearestNeighborInterpolation;
  componentDisplayMode = DisplayAll;
  yuvColorConversionType = YUVC709ColorConversionType;
  lumaScale = 1;
  lumaOffset = 125;
  chromaScale = 1;
  chromaOffset = 128;
  lumaInvert = false;
  chromaInvert = false;
}

yuvSource::~yuvSource()
{
}

void yuvSource::convert2YUV444(QByteArray &sourceBuffer, int lumaWidth, int lumaHeight, QByteArray &targetBuffer)
{
  if (srcPixelFormat == "Unknown Pixel Format") {
    // Unknown format. We cannot convert this.
    return;
  }

  const int componentWidth = lumaWidth;
  const int componentHeight = lumaHeight;
  // TODO: make this compatible with 10bit sequences
  const int componentLength = componentWidth*componentHeight; // number of bytes per luma frames
  const int horiSubsampling = srcPixelFormat.subsamplingHorizontal;
  const int vertSubsampling = srcPixelFormat.subsamplingVertical;
  const int chromaWidth = horiSubsampling == 0 ? 0 : lumaWidth / horiSubsampling;
  const int chromaHeight = vertSubsampling == 0 ? 0 : lumaHeight / vertSubsampling;
  const int chromaLength = chromaWidth * chromaHeight; // number of bytes per chroma frame
  // make sure target buffer is big enough (YUV444 means 3 byte per sample)
  int targetBufferLength = 3 * componentWidth * componentHeight * srcPixelFormat.bytePerComponentSample;
  if (targetBuffer.size() != targetBufferLength)
    targetBuffer.resize(targetBufferLength);

  // TODO: keep unsigned char for 10bit? use short?
  if (chromaLength == 0) {
    const unsigned char *srcY = (unsigned char*)sourceBuffer.data();
    unsigned char *dstY = (unsigned char*)targetBuffer.data();
    unsigned char *dstU = dstY + componentLength;
    memcpy(dstY, srcY, componentLength);
    memset(dstU, 128, 2 * componentLength);
  }
  else if (srcPixelFormat == "4:2:2 8-bit packed") {
    const unsigned char *srcY = (unsigned char*)sourceBuffer.data();
    unsigned char *dstY = (unsigned char*)targetBuffer.data();
    unsigned char *dstU = dstY + componentLength;
    unsigned char *dstV = dstU + componentLength;

    int y;
#pragma omp parallel for default(none) shared(dstY,dstU,dstV,srcY)
    for (y = 0; y < componentHeight; y++) {
      for (int x = 0; x < componentWidth; x++) {
        dstY[x + y*componentWidth] = srcY[((x + y*componentWidth) << 1) + 1];
        dstU[x + y*componentWidth] = srcY[((((x >> 1) << 1) + y*componentWidth) << 1)];
        dstV[x + y*componentWidth] = srcY[((((x >> 1) << 1) + y*componentWidth) << 1) + 2];
      }
    }
  }
  else if (srcPixelFormat == "4:2:2 10-bit packed (UYVY)") {
    const quint32 *srcY = (quint32*)sourceBuffer.data();
    quint16 *dstY = (quint16*)targetBuffer.data();
    quint16 *dstU = dstY + componentLength;
    quint16 *dstV = dstU + componentLength;

    int i;
#define BIT_INCREASE 6
#pragma omp parallel for default(none) shared(dstY,dstU,dstV,srcY)
    for (i = 0; i < ((componentLength + 5) / 6); i++) {
      const int srcPos = i * 4;
      const int dstPos = i * 6;
      quint32 srcVal;
      srcVal = SwapInt32BigToHost(srcY[srcPos]);
      dstV[dstPos] = dstV[dstPos + 1] = (srcVal & 0xffc00000) >> (22 - BIT_INCREASE);
      dstY[dstPos] = (srcVal & 0x003ff000) >> (12 - BIT_INCREASE);
      dstU[dstPos] = dstU[dstPos + 1] = (srcVal & 0x00000ffc) << (BIT_INCREASE - 2);
      srcVal = SwapInt32BigToHost(srcY[srcPos + 1]);
      dstY[dstPos + 1] = (srcVal & 0xffc00000) >> (22 - BIT_INCREASE);
      dstV[dstPos + 2] = dstV[dstPos + 3] = (srcVal & 0x003ff000) >> (12 - BIT_INCREASE);
      dstY[dstPos + 2] = (srcVal & 0x00000ffc) << (BIT_INCREASE - 2);
      srcVal = SwapInt32BigToHost(srcY[srcPos + 2]);
      dstU[dstPos + 2] = dstU[dstPos + 3] = (srcVal & 0xffc00000) >> (22 - BIT_INCREASE);
      dstY[dstPos + 3] = (srcVal & 0x003ff000) >> (12 - BIT_INCREASE);
      dstV[dstPos + 4] = dstV[dstPos + 5] = (srcVal & 0x00000ffc) << (BIT_INCREASE - 2);
      srcVal = SwapInt32BigToHost(srcY[srcPos + 3]);
      dstY[dstPos + 4] = (srcVal & 0xffc00000) >> (22 - BIT_INCREASE);
      dstU[dstPos + 4] = dstU[dstPos + 5] = (srcVal & 0x003ff000) >> (12 - BIT_INCREASE);
      dstY[dstPos + 5] = (srcVal & 0x00000ffc) << (BIT_INCREASE - 2);
    }
  }
  else if (srcPixelFormat == "4:2:2 10-bit packed 'v210'") {
    const quint32 *srcY = (quint32*)sourceBuffer.data();
    quint16 *dstY = (quint16*)targetBuffer.data();
    quint16 *dstU = dstY + componentLength;
    quint16 *dstV = dstU + componentLength;

    int i;
#define BIT_INCREASE 6
#pragma omp parallel for default(none) shared(dstY,dstU,dstV,srcY)
    for (i = 0; i < ((componentLength + 5) / 6); i++) {
      const int srcPos = i * 4;
      const int dstPos = i * 6;
      quint32 srcVal;
      srcVal = SwapInt32LittleToHost(srcY[srcPos]);
      dstV[dstPos] = dstV[dstPos + 1] = (srcVal & 0x3ff00000) >> (20 - BIT_INCREASE);
      dstY[dstPos] = (srcVal & 0x000ffc00) >> (10 - BIT_INCREASE);
      dstU[dstPos] = dstU[dstPos + 1] = (srcVal & 0x000003ff) << BIT_INCREASE;
      srcVal = SwapInt32LittleToHost(srcY[srcPos + 1]);
      dstY[dstPos + 1] = (srcVal & 0x000003ff) << BIT_INCREASE;
      dstU[dstPos + 2] = dstU[dstPos + 3] = (srcVal & 0x000ffc00) >> (10 - BIT_INCREASE);
      dstY[dstPos + 2] = (srcVal & 0x3ff00000) >> (20 - BIT_INCREASE);
      srcVal = SwapInt32LittleToHost(srcY[srcPos + 2]);
      dstU[dstPos + 4] = dstU[dstPos + 5] = (srcVal & 0x3ff00000) >> (20 - BIT_INCREASE);
      dstY[dstPos + 3] = (srcVal & 0x000ffc00) >> (10 - BIT_INCREASE);
      dstV[dstPos + 2] = dstV[dstPos + 3] = (srcVal & 0x000003ff) << BIT_INCREASE;
      srcVal = SwapInt32LittleToHost(srcY[srcPos + 3]);
      dstY[dstPos + 4] = (srcVal & 0x000003ff) << BIT_INCREASE;
      dstV[dstPos + 4] = dstV[dstPos + 5] = (srcVal & 0x000ffc00) >> (10 - BIT_INCREASE);
      dstY[dstPos + 5] = (srcVal & 0x3ff00000) >> (20 - BIT_INCREASE);
    }
  }
  else if (srcPixelFormat == "4:2:0 Y'CbCr 8-bit planar" && interpolationMode == BiLinearInterpolation) {
    // vertically midway positioning - unsigned rounding
    const unsigned char *srcY = (unsigned char*)sourceBuffer.data();
    const unsigned char *srcU = srcY + componentLength;
    const unsigned char *srcV = srcU + chromaLength;
    const unsigned char *srcUV[2] = { srcU, srcV };
    unsigned char *dstY = (unsigned char*)targetBuffer.data();
    unsigned char *dstU = dstY + componentLength;
    unsigned char *dstV = dstU + componentLength;
    unsigned char *dstUV[2] = { dstU, dstV };

    const int dstLastLine = (componentHeight - 1)*componentWidth;
    const int srcLastLine = (chromaHeight - 1)*chromaWidth;

    memcpy(dstY, srcY, componentLength);

    int c;
    for (c = 0; c < 2; c++) {
      //NSLog(@"%i", omp_get_num_threads());
      // first line
      dstUV[c][0] = srcUV[c][0];
      int i;
#pragma omp parallel for default(none) shared(dstUV,srcUV) firstprivate(c)
      for (i = 0; i < chromaWidth - 1; i++) {
        dstUV[c][i * 2 + 1] = (((int)(srcUV[c][i]) + (int)(srcUV[c][i + 1]) + 1) >> 1);
        dstUV[c][i * 2 + 2] = srcUV[c][i + 1];
      }
      dstUV[c][componentWidth - 1] = dstUV[c][componentWidth - 2];

      int j;
#pragma omp parallel for default(none) shared(dstUV,srcUV) firstprivate(c)
      for (j = 0; j < chromaHeight - 1; j++) {
        const int dstTop = (j * 2 + 1)*componentWidth;
        const int dstBot = (j * 2 + 2)*componentWidth;
        const int srcTop = j*chromaWidth;
        const int srcBot = (j + 1)*chromaWidth;
        dstUV[c][dstTop] = ((3 * (int)(srcUV[c][srcTop]) + (int)(srcUV[c][srcBot]) + 2) >> 2);
        dstUV[c][dstBot] = (((int)(srcUV[c][srcTop]) + 3 * (int)(srcUV[c][srcBot]) + 2) >> 2);
        for (int i = 0; i < chromaWidth - 1; i++) {
          const int tl = srcUV[c][srcTop + i];
          const int tr = srcUV[c][srcTop + i + 1];
          const int bl = srcUV[c][srcBot + i];
          const int br = srcUV[c][srcBot + i + 1];
          dstUV[c][dstTop + i * 2 + 1] = ((6 * tl + 6 * tr + 2 * bl + 2 * br + 8) >> 4);
          dstUV[c][dstBot + i * 2 + 1] = ((2 * tl + 2 * tr + 6 * bl + 6 * br + 8) >> 4);
          dstUV[c][dstTop + i * 2 + 2] = ((3 * tr + br + 2) >> 2);
          dstUV[c][dstBot + i * 2 + 2] = ((tr + 3 * br + 2) >> 2);
        }
        dstUV[c][dstTop + componentWidth - 1] = dstUV[c][dstTop + componentWidth - 2];
        dstUV[c][dstBot + componentWidth - 1] = dstUV[c][dstBot + componentWidth - 2];
      }

      dstUV[c][dstLastLine] = srcUV[c][srcLastLine];
#pragma omp parallel for default(none) shared(dstUV,srcUV) firstprivate(c)
      for (i = 0; i < chromaWidth - 1; i++) {
        dstUV[c][dstLastLine + i * 2 + 1] = (((int)(srcUV[c][srcLastLine + i]) + (int)(srcUV[c][srcLastLine + i + 1]) + 1) >> 1);
        dstUV[c][dstLastLine + i * 2 + 2] = srcUV[c][srcLastLine + i + 1];
      }
      dstUV[c][dstLastLine + componentWidth - 1] = dstUV[c][dstLastLine + componentWidth - 2];
    }
  }
  else if (srcPixelFormat == "4:2:0 Y'CbCr 8-bit planar" && interpolationMode == InterstitialInterpolation) {
    // interstitial positioning - unsigned rounding, takes 2 times as long as nearest neighbour
    const unsigned char *srcY = (unsigned char*)sourceBuffer.data();
    const unsigned char *srcU = srcY + componentLength;
    const unsigned char *srcV = srcU + chromaLength;
    const unsigned char *srcUV[2] = { srcU, srcV };
    unsigned char *dstY = (unsigned char*)targetBuffer.data();
    unsigned char *dstU = dstY + componentLength;
    unsigned char *dstV = dstU + componentLength;
    unsigned char *dstUV[2] = { dstU, dstV };

    const int dstLastLine = (componentHeight - 1)*componentWidth;
    const int srcLastLine = (chromaHeight - 1)*chromaWidth;

    memcpy(dstY, srcY, componentLength);

    int c;
    for (c = 0; c < 2; c++) {
      // first line
      dstUV[c][0] = srcUV[c][0];

      int i;
#pragma omp parallel for default(none) shared(dstUV,srcUV) firstprivate(c)
      for (i = 0; i < chromaWidth - 1; i++) {
        dstUV[c][2 * i + 1] = ((3 * (int)(srcUV[c][i]) + (int)(srcUV[c][i + 1]) + 2) >> 2);
        dstUV[c][2 * i + 2] = (((int)(srcUV[c][i]) + 3 * (int)(srcUV[c][i + 1]) + 2) >> 2);
      }
      dstUV[c][componentWidth - 1] = srcUV[c][chromaWidth - 1];

      int j;
#pragma omp parallel for default(none) shared(dstUV,srcUV) firstprivate(c)
      for (j = 0; j < chromaHeight - 1; j++) {
        const int dstTop = (j * 2 + 1)*componentWidth;
        const int dstBot = (j * 2 + 2)*componentWidth;
        const int srcTop = j*chromaWidth;
        const int srcBot = (j + 1)*chromaWidth;
        dstUV[c][dstTop] = ((3 * (int)(srcUV[c][srcTop]) + (int)(srcUV[c][srcBot]) + 2) >> 2);
        dstUV[c][dstBot] = (((int)(srcUV[c][srcTop]) + 3 * (int)(srcUV[c][srcBot]) + 2) >> 2);
        for (int i = 0; i < chromaWidth - 1; i++) {
          const int tl = srcUV[c][srcTop + i];
          const int tr = srcUV[c][srcTop + i + 1];
          const int bl = srcUV[c][srcBot + i];
          const int br = srcUV[c][srcBot + i + 1];
          dstUV[c][dstTop + i * 2 + 1] = (9 * tl + 3 * tr + 3 * bl + br + 8) >> 4;
          dstUV[c][dstBot + i * 2 + 1] = (3 * tl + tr + 9 * bl + 3 * br + 8) >> 4;
          dstUV[c][dstTop + i * 2 + 2] = (3 * tl + 9 * tr + bl + 3 * br + 8) >> 4;
          dstUV[c][dstBot + i * 2 + 2] = (tl + 3 * tr + 3 * bl + 9 * br + 8) >> 4;
        }
        dstUV[c][dstTop + componentWidth - 1] = ((3 * (int)(srcUV[c][srcTop + chromaWidth - 1]) + (int)(srcUV[c][srcBot + chromaWidth - 1]) + 2) >> 2);
        dstUV[c][dstBot + componentWidth - 1] = (((int)(srcUV[c][srcTop + chromaWidth - 1]) + 3 * (int)(srcUV[c][srcBot + chromaWidth - 1]) + 2) >> 2);
      }

      dstUV[c][dstLastLine] = srcUV[c][srcLastLine];
#pragma omp parallel for default(none) shared(dstUV,srcUV) firstprivate(c)
      for (i = 0; i < chromaWidth - 1; i++) {
        dstUV[c][dstLastLine + i * 2 + 1] = ((3 * (int)(srcUV[c][srcLastLine + i]) + (int)(srcUV[c][srcLastLine + i + 1]) + 2) >> 2);
        dstUV[c][dstLastLine + i * 2 + 2] = (((int)(srcUV[c][srcLastLine + i]) + 3 * (int)(srcUV[c][srcLastLine + i + 1]) + 2) >> 2);
      }
      dstUV[c][dstLastLine + componentWidth - 1] = srcUV[c][srcLastLine + chromaWidth - 1];
    }
  } /*else if (pixelFormatType == YUVC_420YpCbCr8PlanarPixelFormat && self.chromaInterpolation == 3) {
    // interstitial positioning - correct signed rounding - takes 6/5 times as long as unsigned rounding
    const unsigned char *srcY = (unsigned char*)sourceBuffer->data();
    const unsigned char *srcU = srcY + componentLength;
    const unsigned char *srcV = srcU + chromaLength;
    unsigned char *dstY = (unsigned char*)targetBuffer->data();
    unsigned char *dstU = dstY + componentLength;
    unsigned char *dstV = dstU + componentLength;

    memcpy(dstY, srcY, componentLength);

    unsigned char *dstC = dstU;
    const unsigned char *srcC = srcU;
    int c;
    for (c = 0; c < 2; c++) {
    // first line
    unsigned char *endLine = dstC + componentWidth;
    unsigned char *endComp = dstC + componentLength;
    *dstC++ = *srcC;
    while (dstC < (endLine-1)) {
    *dstC++ = thresAddAndShift( 3*(int)(*srcC) +   (int)(*(srcC+1)), 512, 1, 2, 2);
    *dstC++ = thresAddAndShift(   (int)(*srcC) + 3*(int)(*(srcC+1)), 512, 1, 2, 2);
    srcC++;
    }
    *dstC++ = *srcC++;
    srcC -= chromaWidth;

    while (dstC < endComp - 2*componentWidth) {
    endLine = dstC + componentWidth;
    *(dstC)                = thresAddAndShift( 3*(int)(*srcC) +   (int)(*(srcC+chromaWidth)), 512, 1, 2, 2);
    *(dstC+componentWidth) = thresAddAndShift(   (int)(*srcC) + 3*(int)(*(srcC+chromaWidth)), 512, 1, 2, 2);
    dstC++;
    while (dstC < endLine-1) {
    int tl = (int)*srcC;
    int tr = (int)*(srcC+1);
    int bl = (int)*(srcC+chromaWidth);
    int br = (int)*(srcC+chromaWidth+1);
    *(dstC)                  = thresAddAndShift(9*tl + 3*tr + 3*bl +   br, 2048, 7, 8, 4);
    *(dstC+1)                = thresAddAndShift(3*tl + 9*tr +   bl + 3*br, 2048, 7, 8, 4);
    *(dstC+componentWidth)   = thresAddAndShift(3*tl +   tr + 9*bl + 3*br, 2048, 7, 8, 4);
    *(dstC+componentWidth+1) = thresAddAndShift(  tl + 3*tr + 3*bl + 9*br, 2048, 7, 8, 4);
    srcC++;
    dstC+=2;
    }
    *(dstC)                = thresAddAndShift( 3*(int)(*srcC) +   (int)(*(srcC+chromaWidth)), 512, 1, 2, 2);
    *(dstC+componentWidth) = thresAddAndShift(   (int)(*srcC) + 3*(int)(*(srcC+chromaWidth)), 512, 1, 2, 2);
    dstC++;
    srcC++;
    dstC += componentWidth;
    }

    endLine = dstC + componentWidth;
    *dstC++ = *srcC;
    while (dstC < (endLine-1)) {
    *dstC++ = thresAddAndShift( 3*(int)(*srcC) +   (int)(*(srcC+1)), 512, 1, 2, 2);
    *dstC++ = thresAddAndShift(   (int)(*srcC) + 3*(int)(*(srcC+1)), 512, 1, 2, 2);
    srcC++;
    }
    *dstC++ = *srcC++;

    dstC = dstV;
    srcC = srcV;
    }
    }*/ else if (srcPixelFormat.planar && srcPixelFormat.bitsPerSample == 8) {
    // sample and hold interpolation
    const bool reverseUV = (srcPixelFormat == "4:4:4 Y'CrCb 8-bit planar") || (srcPixelFormat == "4:2:2 Y'CrCb 8-bit planar");
    const unsigned char *srcY = (unsigned char*)sourceBuffer.data();
    const unsigned char *srcU = srcY + componentLength + (reverseUV ? chromaLength : 0);
    const unsigned char *srcV = srcY + componentLength + (reverseUV ? 0 : chromaLength);
    unsigned char *dstY = (unsigned char*)targetBuffer.data();
    unsigned char *dstU = dstY + componentLength;
    unsigned char *dstV = dstU + componentLength;
    int horiShiftTmp = 0;
    int vertShiftTmp = 0;
    while (((1 << horiShiftTmp) & horiSubsampling) != 0) horiShiftTmp++;
    while (((1 << vertShiftTmp) & vertSubsampling) != 0) vertShiftTmp++;
    const int horiShift = horiShiftTmp;
    const int vertShift = vertShiftTmp;

    memcpy(dstY, srcY, componentLength);

    if (2 == horiSubsampling && 2 == vertSubsampling) {
      int y;
#pragma omp parallel for default(none) shared(dstV,dstU,srcV,srcU)
      for (y = 0; y < chromaHeight; y++) {
        for (int x = 0; x < chromaWidth; x++) {
          dstU[2 * x + 2 * y*componentWidth] = dstU[2 * x + 1 + 2 * y*componentWidth] = srcU[x + y*chromaWidth];
          dstV[2 * x + 2 * y*componentWidth] = dstV[2 * x + 1 + 2 * y*componentWidth] = srcV[x + y*chromaWidth];
        }
        memcpy(&dstU[(2 * y + 1)*componentWidth], &dstU[(2 * y)*componentWidth], componentWidth);
        memcpy(&dstV[(2 * y + 1)*componentWidth], &dstV[(2 * y)*componentWidth], componentWidth);
      }
    }
    else if ((1 << horiShift) == horiSubsampling && (1 << vertShift) == vertSubsampling) {
      int y;
#pragma omp parallel for default(none) shared(dstV,dstU,srcV,srcU)
      for (y = 0; y < componentHeight; y++) {
        for (int x = 0; x < componentWidth; x++) {
          //dstY[x + y*componentWidth] = srcY[x + y*componentWidth];
          dstU[x + y*componentWidth] = srcU[(x >> horiShift) + (y >> vertShift)*chromaWidth];
          dstV[x + y*componentWidth] = srcV[(x >> horiShift) + (y >> vertShift)*chromaWidth];
        }
      }
    }
    else {
      int y;
#pragma omp parallel for default(none) shared(dstV,dstU,srcV,srcU)
      for (y = 0; y < componentHeight; y++) {
        for (int x = 0; x < componentWidth; x++) {
          //dstY[x + y*componentWidth] = srcY[x + y*componentWidth];
          dstU[x + y*componentWidth] = srcU[x / horiSubsampling + y / vertSubsampling*chromaWidth];
          dstV[x + y*componentWidth] = srcV[x / horiSubsampling + y / vertSubsampling*chromaWidth];
        }
      }
    }
  }
    else if (srcPixelFormat == "4:2:0 Y'CbCr 10-bit LE planar") {
      // TODO: chroma interpolation for 4:2:0 10bit planar
      const unsigned short *srcY = (unsigned short*)sourceBuffer.data();
      const unsigned short *srcU = srcY + componentLength;
      const unsigned short *srcV = srcU + chromaLength;
      unsigned short *dstY = (unsigned short*)targetBuffer.data();
      unsigned short *dstU = dstY + componentLength;
      unsigned short *dstV = dstU + componentLength;

      int y;
#pragma omp parallel for default(none) shared(dstY,dstV,dstU,srcY,srcV,srcU)
      for (y = 0; y < componentHeight; y++) {
        for (int x = 0; x < componentWidth; x++) {
          //dstY[x + y*componentWidth] = MIN(1023, CFSwapInt16LittleToHost(srcY[x + y*componentWidth])) << 6; // clip value for data which exceeds the 2^10-1 range
          //     dstY[x + y*componentWidth] = SwapInt16LittleToHost(srcY[x + y*componentWidth])<<6;
          //    dstU[x + y*componentWidth] = SwapInt16LittleToHost(srcU[x/2 + (y/2)*chromaWidth])<<6;
          //    dstV[x + y*componentWidth] = SwapInt16LittleToHost(srcV[x/2 + (y/2)*chromaWidth])<<6;

          dstY[x + y*componentWidth] = qFromLittleEndian(srcY[x + y*componentWidth]);
          dstU[x + y*componentWidth] = qFromLittleEndian(srcU[x / 2 + (y / 2)*chromaWidth]);
          dstV[x + y*componentWidth] = qFromLittleEndian(srcV[x / 2 + (y / 2)*chromaWidth]);
        }
      }
    }
    else if (srcPixelFormat == "4:4:4 Y'CbCr 12-bit BE planar"
      || srcPixelFormat == "4:4:4 Y'CbCr 16-bit BE planar")
    {
      // Swap the input data in 2 byte pairs.
      // BADC -> ABCD
      const char *src = (char*)sourceBuffer.data();
      char *dst = (char*)targetBuffer.data();
      int i;
#pragma omp parallel for default(none) shared(src,dst)
      for (i = 0; i < srcPixelFormat.bytesPerFrame( QSize(componentWidth, componentHeight) ); i+=2)
      {
        dst[i] = src[i + 1];
        dst[i + 1] = src[i];
      }
    }
    else if (srcPixelFormat == "4:4:4 Y'CbCr 10-bit LE planar")
    {
      const unsigned short *srcY = (unsigned short*)sourceBuffer.data();
      const unsigned short *srcU = srcY + componentLength;
      const unsigned short *srcV = srcU + componentLength;
      unsigned short *dstY = (unsigned short*)targetBuffer.data();
      unsigned short *dstU = dstY + componentLength;
      unsigned short *dstV = dstU + componentLength;
      int y;
#pragma omp parallel for default(none) shared(dstY,dstV,dstU,srcY,srcV,srcU)
      for (y = 0; y < componentHeight; y++)
      {
        for (int x = 0; x < componentWidth; x++)
        {
          dstY[x + y*componentWidth] = qFromLittleEndian(srcY[x + y*componentWidth]);
          dstU[x + y*componentWidth] = qFromLittleEndian(srcU[x + y*chromaWidth]);
          dstV[x + y*componentWidth] = qFromLittleEndian(srcV[x + y*chromaWidth]);
        }
      }

    }
    else if (srcPixelFormat == "4:4:4 Y'CbCr 10-bit BE planar")
    {
      const unsigned short *srcY = (unsigned short*)sourceBuffer.data();
      const unsigned short *srcU = srcY + componentLength;
      const unsigned short *srcV = srcU + chromaLength;
      unsigned short *dstY = (unsigned short*)targetBuffer.data();
      unsigned short *dstU = dstY + componentLength;
      unsigned short *dstV = dstU + componentLength;
      int y;
#pragma omp parallel for default(none) shared(dstY,dstV,dstU,srcY,srcV,srcU)
      for (y = 0; y < componentHeight; y++)
      {
        for (int x = 0; x < componentWidth; x++)
        {
          dstY[x + y*componentWidth] = qFromBigEndian(srcY[x + y*componentWidth]);
          dstU[x + y*componentWidth] = qFromBigEndian(srcU[x + y*chromaWidth]);
          dstV[x + y*componentWidth] = qFromBigEndian(srcV[x + y*chromaWidth]);
        }
      }

    }

    else {
      printf("Unhandled pixel format: %d\n", srcPixelFormat);
    }

    return;
}

