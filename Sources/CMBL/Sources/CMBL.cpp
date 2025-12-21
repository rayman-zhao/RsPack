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

void ScaleImage(const unsigned char *srcRGB, int srcWidth, int srcHeight, unsigned char *destRGB, int destWidth, int destHeight) {
    ImageDef8b srcImg(IMAGE_FORMAT_RGB, const_cast<unsigned char*>(srcRGB), srcWidth, srcHeight);
    ImageDef8b *scaleImg = ScaleImage2Linear(&srcImg, destWidth, destHeight);
    memcpy(destRGB, scaleImg->Pixels, GetBytesOfPixelData(scaleImg));
    delete scaleImg;
}