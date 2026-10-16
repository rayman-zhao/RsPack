#include "Exception.h"
#include "Utility.h"
#include "ImageDef.h"
#include "ImageSubArea.h"
#include "ImageSequenceDef.h"
#include "ImageRW.h"
#include "ImageTransform.h"
#include "ImageColor.h"
#include "ImageFilter.h"
#include "ImageMeasure.h"
#include "FileBmp.h"
#include "SequenceMergence.h"
#include "AutofocusOperator.h"
#include "SequenceAutofocus.h"
#include "Compress.h"
#include "AnaglyphRender.h"
#include "MergenceEvaluation.h"
#include "Histogram.h"
#include "ImageAmalgamation.h"
#include "Bayer.h"
#include "RasterPaint.h"

#include "../Include/CMBL.h"

using namespace MBL::Image2D;

void ScaleImage(const unsigned char *srcPixels, int srcWidth, int srcHeight, unsigned char *destPixels, int destWidth, int destHeight, int channels) {
    ImageFormat format;
    switch (channels) {
        case 1: format = IMAGE_FORMAT_INDEX; break;
        case 3: format = IMAGE_FORMAT_RGB; break;
        case 4: format = IMAGE_FORMAT_RGBA; break;
        default: throw MBL::IllegalArgumentException();
    }
    ImageDef8b srcImg(format, const_cast<unsigned char*>(srcPixels), srcWidth, srcHeight);
    ImageDef8b *scaleImg = ScaleImage2Linear(&srcImg, destWidth, destHeight);
    memcpy(destPixels, scaleImg->Pixels, GetBytesOfPixelData(scaleImg));
    delete scaleImg;
}