/**
 * @file
 *
 * @brief  使用CMP压缩方法，在内存中对图像进行压缩和解压缩。
 */

#include <memory.h>
#include <math.h>
#include "Exception.h"
#include "Utility.h"
#include "ImageDef.h"
#include "ImageSubArea.h"
#include "ImageSequenceDef.h"
#include "ImageRW.h"

static const int SaveBufSize  = 8192;
static const int ReadBufSize  = 8192;
static const int WindowSize_8 = 8;
static const int DEF_DC       = 0;
static const int DEF_AC       = 1;
static const int DEF_DC_SIZE  = 16;
static const int DEF_AC_SIZE  = 256;
static const int BLOCKWIDTH   = 8;
static const int BLOCKHEIGHT  = 8;
static const double PI        = 3.14159265359;
static const int DCT_BOUND    = 1023;
static const int IDCT_BOUND   = 255;

/*
 * Quantization table for luminance coefficients.
 */
static unsigned char qu_table[8][8] = {{ 16,  11,  10,  16,  24,  40,  51,  61},
                                       { 12,  12,  14,  19,  26,  58,  60,  55},
                                       { 14,  13,  16,  24,  40,  57,  69,  56},
                                       { 14,  17,  22,  29,  51,  87,  80,  62},
                                       { 18,  22,  37,  58,  68, 109, 103,  77},
                                       { 24,  35,  55,  64,  81, 104, 113,  92},
                                       { 49,  64,  78,  87, 103, 121, 120, 101},
                                       { 72,  92,  95,  98, 112, 100, 103,  99}};
/*
 * Table used to indicate the zig-zag sequence.
 */
static int zz_index[64] = { 0,  1,  5,  6, 14, 15, 27, 28,
                            2,  4,  7, 13, 16, 26, 29, 42,
                            3,  8, 12, 17, 25, 30, 41, 43,
                            9, 11, 18, 24, 31, 40, 44, 53,
                           10, 19, 23, 32, 39, 45, 52, 54,
                           20, 22, 33, 38, 46, 51, 55, 60,
                           21, 34, 37, 47, 50, 56, 59, 61,
                           35, 36, 48, 49, 57, 58, 62, 63};

/*
 * Table used to calculate the code length of DCT
 * DC_coefficients for luminance block.
 */
static unsigned char DC_bits[17] = {0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01,
                                    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*
 * Table used to calculate the code length of DCT
 * AC_coefficients for luminance block.
 */
static unsigned char AC_bits[17] = {0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
                                    0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d, 0x00};

/*
 * Parametres used to generate the DC_coefficient
 *   huffman table for luminance block.
 */
static unsigned char DC_huffval[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                                       0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};

/*
 * Parametres used to generate the AC_coefficient
 *   huffman table for luminance block.
 */
static unsigned char AC_huffval[162] = {0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31,
                                        0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32,
                                        0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52,
                                        0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16,
                                        0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A,
                                        0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45,
                                        0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57,
                                        0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
                                        0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83,
                                        0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94,
                                        0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5,
                                        0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
                                        0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
                                        0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8,
                                        0xD9, 0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8,
                                        0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
                                        0xF9, 0xFA};

static int IMark[16] = {0x0001, 0x0003, 0x0007, 0x000f,
                        0x001f, 0x003f, 0x007f, 0x00ff,
                        0x01ff, 0x03ff, 0x07ff, 0x0fff,
                        0x1fff, 0x3fff, 0x7fff, 0xffff};

static int sizeofcode[256] = {0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5,
                              5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                              6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7,
                              7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                              7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                              7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                              8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                              8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                              8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                              8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                              8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                              8, 8, 8};

static unsigned char BOL[8] = {8, 1, 2, 3, 4, 5, 6, 7};

static struct {unsigned char FileType;
               short         ImageSizeP;
               short         ImageSizeL;
               long          RDataSize;
               long          GDataSize;
               long          BDataSize;
               short         QFactor;
              } CMPInfoHeader;


static short int Q;
static unsigned int BitsOfLeft;
static unsigned char CurByte;
static int preDC, zz[64];
static int WRC;
static unsigned char WRbuf[SaveBufSize];
static unsigned char *BufOffset;
static long int CmpTotalSize;
static unsigned char *BufHead;

static int ORI_ImageSizeL;
static int ORI_ImageSizeP;

static long int JPG_DataSize = 0;
static unsigned char Huffsize[165] = {0}, EncodeHuffsize[165] = {0};
static unsigned int  Huffcode[257] = {0}, EncodeHuffcode[257] = {0};

static double DctCoef[WindowSize_8][WindowSize_8]  = {0};
static double IDctCoef[WindowSize_8][WindowSize_8] = {0};
static int    Qtable[WindowSize_8][WindowSize_8];
static int    Result[WindowSize_8][WindowSize_8];

static unsigned char DC_huffsize[16], AC_huffsize[256];
static unsigned int  DC_huffcode[16], AC_huffcode[256];
static unsigned int  DC_maxcode[17], DC_mincode[17], DC_start_pos[17], DC_maxbitlen;
static unsigned int  AC_maxcode[17], AC_mincode[17], AC_start_pos[17], AC_maxbitlen;

static unsigned int  qtable[WindowSize_8][WindowSize_8];
static unsigned char dc_bits[17], ac_bits[17];
static unsigned int  dc_huffval[13], ac_huffval[163] = {0};

static int Restore[WindowSize_8][WindowSize_8];
static int ReadPos = 0;

static unsigned char ReadBuf[ReadBufSize]; // Buffer For Write Restoring Data.
static int ReadBufCount = ReadBufSize;

/*
 * 下列函数均为压缩和解压缩算法内部调用。
 */
static void LoadDctCoef(void);
static void FastDctTran(void);
static void GetQTable(unsigned int q, unsigned char table[WindowSize_8][WindowSize_8]);
static void Quant(void);
static void Zigzag(void);
static void ClearTables(void);
static void HuffmanSizeTable(int *pos, unsigned char *Huffbits);
static void HuffmanCodeTable(void);
static void BoundDctResult(void);
static void OrderCodes(int pos, unsigned char *Huffvalue);
static void DefaultHuffman(unsigned char *bitsp, unsigned char *vluep);
static void AssignCodeSize(int DCorAC);
static void MakeDefaultHuffman(void);
static void WriteBitsToStream(int codebitlen, unsigned int code);
static void EncodeDC(void);
static void EncodeAC(int BlockSize);
static void WriteMarkofMainend(void);
static void Do_Compress_VRAM(unsigned char *lpImage );
static void LoadIdctCoef(void );
static void IGetQTable(unsigned int q, unsigned char table[WindowSize_8][WindowSize_8]);
static void FastIDctTran(void);
static void InitHuffmanTable(void);
static unsigned char GetBitFromStream(void);
static unsigned int ReadBitsFromStream(int nbits);
static unsigned int GetDecode(int DCorAC);
static unsigned int POW2(int i);
static void DecodeDC(void);
static void DecodeAC(int BlockSize);
static void IZigzag(void);
static void IQuantize(void);
static void WriteBlock(void);
static void Data_Restoring_VRAM(unsigned char *lpImage );

namespace MBL
{
  namespace Image2D
  {
    /// 压缩一幅图像到一块内存中。
    /**
     * CMP压缩方法是北京航空航天大学图象中心早年研制的图像压缩算法。它的方法类似JPEG，但没有进行YUV变换，直接分别对
     * R、G、B通道进行压缩，压缩速度比较快。该函数将输入的一幅图像压缩到内存中。如果输入的内存指针有效，则该函数将压
     * 缩数据填入其中，否则如果为0则函数内部分配内存，调用者使用完毕后，应用delete将其删除。
     *
     * 由于压缩算法是按8×8的块进行压缩的，所以压缩后的图像数据其尺寸是按8对齐的，解压后不能恢复原尺寸。
     *
     * @param image 待处理的图像对象，目前只支持unsigned char的存储类型。
     * @param buf 一个内存指针的指针，如果其不为0，则表示其是已经分配好足够内存的指针（最大内存数量不会超过原始图像大
     *            小），则函数将直接使用它。否则如果其为0，则函数内部新分配一块内存（尺寸为原始图像大小）。从该地址开
     *            始将要存入压缩数据。
     * @param q_factor 压缩比，一般为70，值越大则压缩越多。
     * @return 函数执行成功则返回压缩数据的字节长度，否则返回0。
     *
     * @author 刘莉
     *
     * <PRE>
     * ImageDef<unsigned char> *image = 0;
     * unsigned char *buf = 0;
     * image = LoadImageAsBmp("图像所在路径");
     * EncodeImageAsCMP(image, &buf, 70);
     *        ...
     * ImageDef<unsigned char> *image2 = DecodeImageAsCMP(buf);
     * ...
     * delete image2;
     * delete  buf; //用完后，将内存释放。
     * delete image;
     * </PRE>
     */
    int EncodeImageAsCMP(ImageDef<unsigned char> *image, unsigned char **buf, short int q_factor)
    {
      int RCMPFile_Size = 0, GCMPFile_Size = 0, BCMPFile_Size = 0;

      int bytes = 0; //每个象素所占的存储字节数。
      int real_bytes = 0; //除去Alpha通道后每个象素所占的字节数。
      switch (image->Format)
      {
        case IMAGE_FORMAT_INDEX:
          bytes = 1;
          real_bytes = 1;
          break;
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
          bytes = 3;
          real_bytes = 3;
          break;
        case IMAGE_FORMAT_RGBA:
          bytes = 4;
          real_bytes = 3;
          break;
        case IMAGE_FORMAT_INDEX_ALPHA:
          bytes = 2;
          real_bytes = 1;
          break;
        default :
          throw UnsupportedFormatException();
          break;
      }

      //压缩是按8×8的块进行的，所以需要将图像数据按8补齐。
      ORI_ImageSizeL = image->Height;
      if (image->Height % 8 != 0)
      {
        ORI_ImageSizeL += 8 - (image->Height % 8);
      }
      ORI_ImageSizeP = image->Width;
      if (image->Width % 8 != 0)
      {
        ORI_ImageSizeP += 8 - (image->Width % 8);
      }

      //分配压缩需要的缓冲区内存。
      unsigned char *lpDIBBitsG = new unsigned char[ORI_ImageSizeL * ORI_ImageSizeP];
      if (lpDIBBitsG == 0)
      {
        throw OutOfMemoryException();
      }
      unsigned char *lpG = lpDIBBitsG;
      if (*buf == 0)
      {
        *buf = new unsigned char[ORI_ImageSizeP * ORI_ImageSizeL * real_bytes]; //开辟的内存空间大于实际需要的空间
        if (*buf == 0)
        {
          if (lpDIBBitsG != 0) delete [] lpDIBBitsG;
          throw OutOfMemoryException();
        }
      }


      Q = q_factor;
      GetQTable(Q, qu_table);
      if (DctCoef[0][0] == 0)  LoadDctCoef();
      if (Huffsize[0] == 0)  MakeDefaultHuffman();

      //压缩缓冲区指针归零。
      BufOffset = *buf;
      BufOffset += sizeof(CMPInfoHeader);

      //从图像对象中提取各个通道的数据，按8补齐后压缩。
      unsigned char *lpDIBBits = 0;
      for (int band = 0; band < real_bytes; band++)
      {
        lpDIBBits = image->Pixels + band;
        for (int ii = 0; ii < ORI_ImageSizeL; ii++)
        {
          if (ii < image->Height)
          {
            for (int jj = 0; jj < ORI_ImageSizeP; jj++)
            {
              if (jj < image->Width)
              {
                *lpDIBBitsG++ = *lpDIBBits;
                lpDIBBits += bytes;
              }
              else
              {
                *lpDIBBitsG = *(lpDIBBitsG - 1);
                lpDIBBitsG++;
              }
            }
          }
          else
          {
            for (int jj = 0; jj < ORI_ImageSizeP; jj++)
            {
              *lpDIBBitsG = *(lpDIBBitsG - ORI_ImageSizeP);
               lpDIBBitsG++;
            }
          }
        }
        lpDIBBitsG = lpG;

        //压缩数据。
        BitsOfLeft = 8;
        CurByte = 0;
        preDC = 0;
        WRC = 0;
        JPG_DataSize = 0;
        Do_Compress_VRAM(lpDIBBitsG);

        switch (band)
        {
          case 0:
            RCMPFile_Size = JPG_DataSize;
            break;
          case 1:
            GCMPFile_Size = JPG_DataSize;
            break;
          case 2:
            BCMPFile_Size = JPG_DataSize;
            break;
          default:
            break;
        }
        JPG_DataSize = 0;
      }
      delete [] lpDIBBitsG;


      //填充压缩数据头信息。
      CMPInfoHeader.FileType = image->Format == IMAGE_FORMAT_INDEX ? 0x00 : 0xFF;
      CMPInfoHeader.ImageSizeL = (short)ORI_ImageSizeL;
      CMPInfoHeader.ImageSizeP = (short)ORI_ImageSizeP;
      CMPInfoHeader.RDataSize = RCMPFile_Size;
      CMPInfoHeader.GDataSize = GCMPFile_Size;
      CMPInfoHeader.BDataSize = BCMPFile_Size;
      CMPInfoHeader.QFactor = q_factor;
      memcpy(*buf, &CMPInfoHeader, sizeof(CMPInfoHeader));

      //返回压缩数据长度。
      return sizeof(CMPInfoHeader) + CMPInfoHeader.RDataSize + CMPInfoHeader.GDataSize + CMPInfoHeader.BDataSize;
    }

    /// 把一块内存缓冲区中的压缩图像数据解压缩到一个ImageDef对象中。
    /**
     * 该函数使用CMP方法解压缩图像数据，该数据必须是EncodeImageAsCMP函数生成的。
     *
     * @param buf 存放压缩数据的内存指针。
     * @return 函数执行成功则返回解压缩后得到的图象对象，否则返回0。
     *
     * @author 刘莉
     *
     * @see EncodeImageAsCMP
     */
    ImageDef<unsigned char> * DecodeImageAsCMP(unsigned char *buf)
    {
      memcpy (&CMPInfoHeader, buf, sizeof(CMPInfoHeader));
      int bands = 0;

      ImageDef<unsigned char> *image = 0;
      if (CMPInfoHeader.FileType == 0xFF)
      {
        image = ImageDef<unsigned char>::CreateInstance(IMAGE_FORMAT_RGB, CMPInfoHeader.ImageSizeP, CMPInfoHeader.ImageSizeL, 0);
        bands = 3;
      }
      else
      {
        image = ImageDef<unsigned char>::CreateInstance(IMAGE_FORMAT_INDEX, CMPInfoHeader.ImageSizeP, CMPInfoHeader.ImageSizeL, 0);
        bands = 1;
      }

      unsigned char *dcmp_buf = new unsigned char[CMPInfoHeader.ImageSizeP * CMPInfoHeader.ImageSizeL];
      if (dcmp_buf == 0)
      {
        delete image;
        throw OutOfMemoryException();
      }

      Q = CMPInfoHeader.QFactor;
      ORI_ImageSizeL = CMPInfoHeader.ImageSizeL;
      ORI_ImageSizeP = CMPInfoHeader.ImageSizeP;

      if (IDctCoef[0][0] == 0)  LoadIdctCoef();
      IGetQTable(Q, qu_table);
      if (ac_huffval[0] == 0)  InitHuffmanTable();

      static unsigned char *offset[3];
      offset[0] = buf + sizeof(CMPInfoHeader);
      offset[1] = offset[0] + CMPInfoHeader.RDataSize;
      offset[2] = offset[1] + CMPInfoHeader.GDataSize;

      BufHead = buf;
      CmpTotalSize = sizeof(CMPInfoHeader) + CMPInfoHeader.RDataSize + CMPInfoHeader.GDataSize + CMPInfoHeader.BDataSize;

      for (int i = 0; i < bands; i++)
      {
        BufOffset = offset[i];

        ReadPos = 0;
        preDC = 0;
        ReadBufCount = ReadBufSize;
        CurByte = 0;

        Data_Restoring_VRAM(dcmp_buf);
        FillBand(image, i, dcmp_buf);
      }

      delete [] dcmp_buf;
      return image;
    }
  }
}

void Data_Restoring_VRAM(unsigned char *lpImage)
{
  int Pixel, Line,i,j;

  for (Line=0; Line<ORI_ImageSizeL; Line+=WindowSize_8)
  {
    for(Pixel=0; Pixel<ORI_ImageSizeP; Pixel+=WindowSize_8)
    {
       DecodeDC();
       DecodeAC(64);
       IZigzag();  // Reorder Decoded data
       IQuantize(); // To zz[] data interse quantize
       FastIDctTran();  // Fast interse DCT transform
       WriteBlock();
       for(i=0; i<WindowSize_8; i++)
       for(j=0; j<WindowSize_8; j++)
        *(lpImage +(int)(Line + i) * (int)ORI_ImageSizeP + j + Pixel)
              =(unsigned char)Restore[i][j];
    }
  }
}

void LoadIdctCoef(void )
{
   int i,j;
   for(j=0;j<WindowSize_8;j++)
      IDctCoef[0][j]=0.5/sqrt((double)2.0);

   for(i=1;i<WindowSize_8;i++)
     for(j=0;j<WindowSize_8;j++)
     IDctCoef[i][j]=0.5*cos((2.0*(double)j+1.0)*(double)i*PI/16.0);
}

void IGetQTable(unsigned int q,unsigned char table[WindowSize_8][WindowSize_8])
{
  unsigned int i,j,tmp;
  for(i=0;i<WindowSize_8;i++)
    for(j=0;j<WindowSize_8;j++)
     {
        tmp=(unsigned int)table[i][j]*q;
        qtable[i][j]=(unsigned int)(tmp/50.0+0.5);
     }
}

void InitHuffmanTable(void)
{
  int i,j,dc_position=0,ac_position=0;
  unsigned int dc_start_code=0,ac_start_code=0;

  for(j=0;j<16;j++)
     dc_bits[j]=(unsigned char)DC_bits[j];
          /* from file fetch DC_bits table */
  for(j=0;j<12;j++)
     dc_huffval[j]=DC_huffval[j];
          /* from file fetch DC_huffman value table */
  for(j=0;j<16;j++)
     ac_bits[j]=(unsigned char)AC_bits[j];
              /* from file fetch AC_bits table */
  for(j=0;j<162;j++)
     ac_huffval[j]=AC_huffval[j];
              /* from file fetch AC_huffman value table */

  for(i=0;i<16;i++)
  {
    DC_maxcode[i]=0;
    AC_maxcode[i]=0;
    DC_mincode[i]=0xffff;
    AC_mincode[i]=0xffff; /* init. these array */

    dc_start_code<<=1;
    if(dc_bits[i]!=0)
    {
       DC_mincode[i]=dc_start_code;
       dc_start_code+=dc_bits[i];
       DC_maxcode[i]=dc_start_code-1;
       DC_start_pos[i]=dc_position;
       dc_position+=dc_bits[i];
       DC_maxbitlen=i;
    }

    ac_start_code<<=1;
    if(ac_bits[i]!=0)
    {
      AC_mincode[i]=ac_start_code;
      ac_start_code+=ac_bits[i];
      AC_maxcode[i]=ac_start_code-1;
      AC_start_pos[i]=ac_position;
      ac_position+=ac_bits[i];
      AC_maxbitlen=i;
    }
  }
}

void DecodeDC(void)
{
  unsigned int diff;
  unsigned int nbits,temp;

  nbits = GetDecode(DEF_DC);  /* from huffman table get value */
  if (nbits)
  {
    diff =ReadBitsFromStream(nbits);
    temp=POW2(nbits);
    if(diff<(temp/2)) diff=diff-temp+1;
    preDC += diff;            /* Change the previous DC */
  }
  zz[0]=preDC;
}

unsigned int GetDecode(int DCorAC)
{
  unsigned int code,i,index;

  code =GetBitFromStream();
  if(DCorAC==DEF_DC)
  {
   i=0;
   while((code<DC_mincode[i]) || (code>DC_maxcode[i]))
   {
    i++;
    code=(code<<1)+GetBitFromStream();
   }
   if(code<(DC_maxcode[DC_maxbitlen]+1))
   {
    index= DC_start_pos[i] + code - DC_mincode[i];
    return(dc_huffval[index]);
   }
   else
   {
    //TRACE("DC--Huffman read error.");
    return 0;
   }
  }
  else
  {
    if(DCorAC==DEF_AC)
    {
      i=0;
      while((code<AC_mincode[i]) || (code>AC_maxcode[i]))
      {
        i++;
        code=(code<<1)+GetBitFromStream();
      }
      if(code< (AC_maxcode[AC_maxbitlen]+1)){
        index= AC_start_pos[i] + code - AC_mincode[i];
       return(ac_huffval[index]);
      }
      else
      {
        //TRACE("AC--Huffman read error.");
        return 0;
      }
    }
    else
    {
      return 0;
    }
  }

  return(0);
}

unsigned char GetBitFromStream(void)
{
   unsigned char tmp;
   int size = sizeof(unsigned char);

   if (ReadPos==0)
   {
      if(ReadBufCount==ReadBufSize)
      {
        int RealReadSize = ReadBufSize;
        if ( (BufHead + CmpTotalSize - BufOffset) < ReadBufSize )
        {
           RealReadSize = BufHead + CmpTotalSize - BufOffset;
        }
        memcpy(ReadBuf,BufOffset,RealReadSize*size);
        BufOffset+=RealReadSize*size;
        ReadBufCount=0;
      }
      CurByte=ReadBuf[ReadBufCount++];
   }
   tmp=CurByte << ReadPos;
   ReadPos++;
   if(ReadPos==8) ReadPos=0;
   return (tmp>>7);

}

unsigned int ReadBitsFromStream(int nbits)
{
  int i,coef=0;
  for(i=0;i<nbits;i++)
  {
    coef=(coef<<1) + GetBitFromStream();
  }
  return(coef & IMark[nbits-1]);
}

unsigned int POW2(int i)     /* return value of i bits */
{
   int j,k;
   for(j=0,k=1;j<i;j++,k*=2)
   {
     ;
   }
   return k;
}

void DecodeAC(int BlockSize)
{
  int k,bits,nbits,run_length,temp;
  int *mptr;

  for(mptr=zz+1;mptr< zz+BlockSize;mptr++) /* Set all values to zero */
  *mptr=0;

  for(k=1;k<BlockSize;)
  {
    bits= GetDecode(DEF_AC);               /* Decode Huffman */
    if(!bits) return;
    nbits = bits & 0x0f;            /* Find significant bits */
    run_length= (bits>> 4) & 0xf;
    if (nbits)
    {
      if ((k+=run_length)>=BlockSize) break;
      zz[k]=ReadBitsFromStream(nbits);

      temp=POW2(nbits);
      if(zz[k]<(temp/2)) zz[k]=zz[k]-temp+1;
      k++;                     /* Goto next element */
    }
    else if (run_length==15)  k+=16;       /* Zero run length code extnd */
       else break;
  }
}

void IZigzag(void)
{
   int i,j;
   for(i=0;i<WindowSize_8;i++)
     for(j=0;j<WindowSize_8;j++)
       Restore[i][j]=zz[zz_index[i*WindowSize_8+j]];
           /* reorder the coefficients in 8x8 block */
}

void IQuantize(void)
{
   int i,j;
   for(i=0;i<WindowSize_8;i++)
    for(j=0;j<WindowSize_8;j++)
     Restore[i][j] *= qtable[i][j];
}

void FastIDctTran(void)
{
  long temp0,temp1,temp2,temp3;
  long temp10,temp13,temp11,temp12;
  long z1,z2,z3,z4,z5,dcval;
  int row,col;

  for(row=0;row<WindowSize_8;row++)
  {
     if((Restore[row][1] | Restore[row][2] | Restore[row][3] | Restore[row][4] |
       Restore[row][5] | Restore[row][6] | Restore[row][7] ) == 0)
     {
         dcval=Restore[row][0] << 2;

         Restore[row][0]=dcval;
         Restore[row][1]=dcval;
         Restore[row][2]=dcval;
         Restore[row][3]=dcval;
         Restore[row][4]=dcval;
         Restore[row][5]=dcval;
         Restore[row][6]=dcval;
         Restore[row][7]=dcval;

         continue;
     }

     z2=(long)Restore[row][2];
     z3=(long)Restore[row][6];
     z1=(z2 + z3) * 4433L;

     temp2=z1 + (z3 * (-15137L));
     temp3=z1 + (z2 * 6270L);
     temp0=((long)Restore[row][0] + (long)Restore[row][4]) << 13;
     temp1=((long)Restore[row][0] - (long)Restore[row][4]) << 13;

     temp10=temp0 + temp3;
     temp13=temp0 - temp3;
     temp11=temp1 + temp2;
     temp12=temp1 - temp2;

     temp0=(long)Restore[row][7];
     temp1=(long)Restore[row][5];
     temp2=(long)Restore[row][3];
     temp3=(long)Restore[row][1];

     z1=temp0 + temp3;
     z2=temp1 + temp2;
     z3=temp0 + temp2;
     z4=temp1 + temp3;
     z5=(z3 + z4) * 9633L;

     temp0 *=2446L;
     temp1 *=16819L;
     temp2 *=25172L;
     temp3 *=12299L;

     z1 *=(-7373L);
     z2 *=(-20995L);
     z3 *=(-16069L);
     z4 *=(-3196L);

     z3 +=z5;
     z4 +=z5;

     temp0 +=(z1 + z3);
     temp1 +=(z2 + z4);
     temp2 +=(z2 + z3);
     temp3 +=(z1 + z4);

     Restore[row][0]=(temp10 + temp3 + 1024) >> 11;
     Restore[row][7]=(temp10 - temp3 + 1024) >> 11;
     Restore[row][1]=(temp11 + temp2 + 1024) >> 11;
     Restore[row][6]=(temp11 - temp2 + 1024) >> 11;
     Restore[row][2]=(temp12 + temp1 + 1024) >> 11;
     Restore[row][5]=(temp12 - temp1 + 1024) >> 11;
     Restore[row][3]=(temp13 + temp0 + 1024) >> 11;
     Restore[row][4]=(temp13 - temp0 + 1024) >> 11;
   } /* end of row_transform */

   for(col=0;col<WindowSize_8;col++)
   {
     if((Restore[1][col] | Restore[2][col] | Restore[3][col] | Restore[4][col] |
       Restore[5][col] | Restore[6][col] | Restore[7][col] ) == 0)
     {
         dcval=((long)Restore[0][col] + 16) >> 5;

         Restore[0][col]=dcval;
         Restore[1][col]=dcval;
         Restore[2][col]=dcval;
         Restore[3][col]=dcval;
         Restore[4][col]=dcval;
         Restore[5][col]=dcval;
         Restore[6][col]=dcval;
         Restore[7][col]=dcval;

         continue;
     }

     z2=(long)Restore[2][col];
     z3=(long)Restore[6][col];
     z1=(z2 + z3) * 4433L;

     temp2=z1 + (z3 * (-15137L));
     temp3=z1 + (z2 * 6270L);
     temp0=((long)Restore[0][col] + (long)Restore[4][col]) << 13;
     temp1=((long)Restore[0][col] - (long)Restore[4][col]) << 13;

     temp10=temp0 + temp3;
     temp13=temp0 - temp3;
     temp11=temp1 + temp2;
     temp12=temp1 - temp2;

     temp0=(long)Restore[7][col];
     temp1=(long)Restore[5][col];
     temp2=(long)Restore[3][col];
     temp3=(long)Restore[1][col];

     z1=temp0 + temp3;
     z2=temp1 + temp2;
     z3=temp0 + temp2;
     z4=temp1 + temp3;
     z5=(z3 + z4) * 9633L;

     temp0 *=2446L;
     temp1 *=16819L;
     temp2 *=25172L;
     temp3 *=12299L;

     z1 *=(-7373L);
     z2 *=(-20995L);
     z3 *=(-16069L);
     z4 *=(-3196L);

     z3 +=z5;
     z4 +=z5;

     temp0 +=(z1 + z3);
     temp1 +=(z2 + z4);
     temp2 +=(z2 + z3);
     temp3 +=(z1 + z4);

     Restore[0][col]=(temp10 + temp3 + 131072L) >> 18;
     Restore[7][col]=(temp10 - temp3 + 131072L) >> 18;
     Restore[1][col]=(temp11 + temp2 + 131072L) >> 18;
     Restore[6][col]=(temp11 - temp2 + 131072L) >> 18;
     Restore[2][col]=(temp12 + temp1 + 131072L) >> 18;
     Restore[5][col]=(temp12 - temp1 + 131072L) >> 18;
     Restore[3][col]=(temp13 + temp0 + 131072L) >> 18;
     Restore[4][col]=(temp13 - temp0 + 131072L) >> 18;
   } /* end of col_transform */
}

void WriteBlock(void)
{
  int i,j;

  for(i=0;i<BLOCKWIDTH;i++)
    for(j=0;j<BLOCKHEIGHT;j++)
    {
      Restore[i][j]+=128;

      if(Restore[i][j] > IDCT_BOUND ) Restore[i][j] = IDCT_BOUND;
      else if(Restore[i][j] < 0) Restore[i][j] = 0;
    }
}

void Do_Compress_VRAM( unsigned char *lpImage )
{
  int Line, Pixel, i, j;

  for(Line=0; Line<ORI_ImageSizeL; Line+=WindowSize_8)
  {
    for(Pixel=0; Pixel<ORI_ImageSizeP; Pixel+=WindowSize_8)
    {
      for(i=0; i<WindowSize_8; i++)
        for(j=0; j<WindowSize_8; j++)
          Result[i][j] = (int)(*(lpImage +
                   (unsigned long)(Line + i) * (unsigned long)ORI_ImageSizeP + j + Pixel)
                   -128);
      FastDctTran();     // Fast DCT transform
      BoundDctResult();
      Quant();       // To Result Quantize
      Zigzag();      // Reorder quantized data,store in zz[]
      EncodeDC();
      EncodeAC(64);    // To DC_coeffcient and AC_coeffcient encode,and code
    }
  }
  WriteMarkofMainend();   // Set End Of File Mark
}

void GetQTable(unsigned int q, unsigned char table[WindowSize_8][WindowSize_8])
{
  unsigned int i,j,tmp;

  for(i=0;i<WindowSize_8;i++)
    for(j=0;j<WindowSize_8;j++)
    {
      tmp=(int)table[i][j]*q;
      Qtable[i][j]=(int)(tmp/50.0+0.5);
    }
}

void LoadDctCoef(void)
{
  int i,j;

  for (i=0;i<WindowSize_8;i++)
    DctCoef[i][0]=0.5/sqrt((double)2.0);
    for(j=1;j<WindowSize_8;j++)
      for(i=0;i<WindowSize_8;i++)
        DctCoef[i][j]=0.5*cos((2.0*(double)i+1.0)*(double)j*PI/16.0);
}

void MakeDefaultHuffman(void)
{
  ClearTables();
  DefaultHuffman(DC_bits,DC_huffval);
  AssignCodeSize(DEF_DC);
  ClearTables();
  DefaultHuffman(AC_bits,AC_huffval);
  AssignCodeSize(DEF_AC);
}

void ClearTables(void)
{
  int i;

  for(i=0;i<257;i++)
  {
    EncodeHuffcode[i]=0;
    EncodeHuffsize[i]=0;
  }
  for(i=0;i<165;i++)
  {
    Huffcode[i]=0;
    Huffsize[i]=0;
  }
}

void DefaultHuffman(unsigned char *bitsp, unsigned char *valuep)
{
  int pos;

  HuffmanSizeTable(&pos,bitsp);
  HuffmanCodeTable();
  OrderCodes(pos,valuep);
}

static void HuffmanSizeTable(int *pos, unsigned char *Huffbits)
{
  int i,j,p;
  for(p=0,i=0;i<16;i++)
    for(j=1;j<=Huffbits[i];j++)
      Huffsize[p++]=i+1;
  Huffsize[p]=0;
  *pos=p;
}

static void HuffmanCodeTable(void)
{
  int p=0,code=0,size;

  size=Huffsize[0];
  while(1)
  {
    do
    {
      Huffcode[p++]=code;
      code++;
    }
    while(Huffsize[p]==size && p<257);

    if(!Huffsize[p])  break;
    else
    {
      do
      {
        code<<=1;
        size++;
      }
      while(Huffsize[p]!=size);
    }
  }
}

static void OrderCodes(int pos, unsigned char *Huffvalue)
{
  int index,i;
  for(i=0;i<pos;i++)
  {
     index=Huffvalue[i];
     EncodeHuffcode[index]=Huffcode[i];
     EncodeHuffsize[index]=Huffsize[i];
  }
}

void AssignCodeSize(int DCorAC)
{
  int i;

  if(DCorAC==DEF_DC)
  {
    for(i=0;i<DEF_DC_SIZE;i++)
    {
      DC_huffsize[i]=EncodeHuffsize[i];
      DC_huffcode[i]=EncodeHuffcode[i];
    }
  }
  else
  {
    for(i=0;i<DEF_AC_SIZE;i++)
    {
      AC_huffsize[i]=EncodeHuffsize[i];
      AC_huffcode[i]=EncodeHuffcode[i];
    }
  }
}

void FastDctTran(void)
{
  int row,col;
  long temp0,temp1,temp2,temp3,temp4,temp5,temp6,temp7;
  long temp10,temp13,temp11,temp12;
  long z1,z2,z3,z4,z5;

  for(row=0;row<WindowSize_8;row++)
  {
    temp0=Result[row][0] + Result[row][7];
    temp7=Result[row][0] - Result[row][7];
    temp1=Result[row][1] + Result[row][6];
    temp6=Result[row][1] - Result[row][6];
    temp2=Result[row][2] + Result[row][5];
    temp5=Result[row][2] - Result[row][5];
    temp3=Result[row][3] + Result[row][4];
    temp4=Result[row][3] - Result[row][4];

    temp10=temp0 + temp3;
    temp13=temp0 - temp3;
    temp11=temp1 + temp2;
    temp12=temp1 - temp2;
    Result[row][0]=(temp10 + temp11) << 2;
    Result[row][4]=(temp10 - temp11) << 2;
    z1=(temp12+temp13) * 4433L;

    Result[row][2]=((z1+(temp13 * 6270L)) + 1024 ) >> 11;
    Result[row][6]=((z1+(temp12 * (-15137L)))+ 1024 ) >> 11;

    z1=temp4 + temp7;
    z2=temp5 + temp6;
    z3=temp4 + temp6;
    z4=temp5 + temp7;
    z5=(z3 + z4) * 9633L;

    temp4 *= 2446L;
    temp5 *= 16819L;
    temp6 *= 25172L;
    temp7 *= 12299L;
    z1    *= -7373L;
    z2    *= -20995L;
    z3    *= -16069L;
    z4    *= -3196L;

    z3    += z5;
    z4    += z5;

    Result[row][7]=((temp4 + z1 + z3)+ 1024) >> 11;
    Result[row][5]=((temp5 + z2 + z4)+ 1024) >> 11;
    Result[row][3]=((temp6 + z2 + z3)+ 1024) >> 11;
    Result[row][1]=((temp7 + z1 + z4)+ 1024) >> 11;

  } /* end of row_transform */

  for(col=0;col<WindowSize_8;col++)
  {
    temp0=Result[0][col] + Result[7][col];
    temp7=Result[0][col] - Result[7][col];
    temp1=Result[1][col] + Result[6][col];
    temp6=Result[1][col] - Result[6][col];
    temp2=Result[2][col] + Result[5][col];
    temp5=Result[2][col] - Result[5][col];
    temp3=Result[3][col] + Result[4][col];
    temp4=Result[3][col] - Result[4][col];

    temp10=temp0 + temp3;
    temp13=temp0 - temp3;
    temp11=temp1 + temp2;
    temp12=temp1 - temp2;

    Result[0][col]=((temp10 + temp11)+ 16) >> 5;
    Result[4][col]=((temp10 - temp11)+ 16) >> 5;
    z1=(temp12+temp13) * 4433L;

    Result[2][col]=((z1+(temp13 * 6270L))+ 131072L ) >>18;
    Result[6][col]=((z1+(temp12 * (-15137L)))+ 131072L) >>18;

    z1=temp4 + temp7;
    z2=temp5 + temp6;
    z3=temp4 + temp6;
    z4=temp5 + temp7;
    z5=(z3 + z4) * 9633L;

    temp4 *= 2446L;
    temp5 *= 16819L;
    temp6 *= 25172L;
    temp7 *= 12299L;
    z1    *= -7373L;
    z2    *= -20995L;
    z3    *= -16069L;
    z4    *= -3196L;

    z3    += z5;
    z4    += z5;

    Result[7][col]=((temp4 + z1 + z3)+ 131072L)>> 18;
    Result[5][col]=((temp5 + z2 + z4)+ 131072L)>> 18;
    Result[3][col]=((temp6 + z2 + z3)+ 131072L)>> 18;
    Result[1][col]=((temp7 + z1 + z4)+ 131072L)>> 18;

  } /* end of col_transform */
}

void BoundDctResult(void)
{
  int i,j;

  for(i=0;i<BLOCKHEIGHT;i++)
    for(j=0;j<BLOCKWIDTH;j++)
    {
      if (Result[i][j] + DCT_BOUND < 0) Result[i][j] = -DCT_BOUND;
        else if (Result[i][j] - DCT_BOUND > 0) Result[i][j] = DCT_BOUND;
    }
}

void Quant(void)
{
  int i,j;
  for(i=0;i< WindowSize_8;i++)
    for(j=0;j< WindowSize_8;j++)
    {
      if(Result[i][j]>=0)
         Result[i][j]=(int)((Result[i][j]+Qtable[i][j]/2)/(float)Qtable[i][j]);
       else
        Result[i][j]=(int)((Result[i][j]-Qtable[i][j]/2)/(float)Qtable[i][j]);
    }
}

void Zigzag(void)
{
  int i,j;

  for(i=0;i< BLOCKHEIGHT;i++)
    for(j=0;j< BLOCKWIDTH;j++)
      zz[zz_index[i * BLOCKWIDTH+j]]=Result[i][j];
}

void EncodeDC(void)
{
  int diff;
  unsigned int  dc_coef,bit_nums,temp;
  diff=zz[0]-preDC;
  preDC=zz[0];
  dc_coef=abs(diff);
  if(dc_coef < 256) bit_nums=sizeofcode[dc_coef];
  else
  {
    dc_coef>>=8;
    bit_nums=sizeofcode[dc_coef]+8;
  }

  WriteBitsToStream((int)DC_huffsize[bit_nums],DC_huffcode[bit_nums]);

  if(diff==0) return;
  if(diff<0)
  {
    diff--;
      temp=0xffff;
    temp>>=16-bit_nums;
    temp &=diff;
  }
  else temp=diff;

  WriteBitsToStream(bit_nums,temp);
}

void EncodeAC(int BlockSize)
{
  int  tbits,k,run_length=0,bit_nums=0;
  unsigned int  temp,ac_coef;
  for(k=1;k<BlockSize;k++)
  {
    ac_coef=abs(zz[k]);
    if(ac_coef>0)
    {
      if(ac_coef<256) bit_nums=sizeofcode[ac_coef];
      else
      {
        ac_coef>>=8;
        bit_nums=sizeofcode[ac_coef]+8;
      }
    }

    if(ac_coef==0)
    {
      if(k==(BlockSize-1))
      {
        WriteBitsToStream((int)AC_huffsize[0],AC_huffcode[0]);
        return;
      }
      run_length++;
    }
    else
    {
      while(run_length>15)
      {
         WriteBitsToStream((int)AC_huffsize[240],AC_huffcode[240]);
         run_length-=16;
      }
      tbits=run_length*16+bit_nums;
      run_length=0;

      WriteBitsToStream((int)AC_huffsize[tbits],AC_huffcode[tbits]);
      if(zz[k]<0)
      {
        zz[k]--;
        temp=0xffff;
        temp>>=16-bit_nums;
        temp &=zz[k];
      }
      else temp=zz[k];

      WriteBitsToStream(bit_nums,temp);
    }
  }
}

void WriteBitsToStream(int codebitlen, unsigned int code)
{
   int j, k=0, bits,size;
   bits=codebitlen-BitsOfLeft;
   size=sizeof(unsigned char);

   for(j=-8 ;j < bits ;j+=8)
   {
    if((k=codebitlen-BitsOfLeft)>=0)
    {
      WRbuf[WRC++]=CurByte|code>>k;
      if(WRC==SaveBufSize)
      {
         memcpy(BufOffset,WRbuf,SaveBufSize*size);
         BufOffset+=SaveBufSize*size;
         WRC=0;
         JPG_DataSize+=SaveBufSize*size;
      }
      CurByte=0;
    }
    else  CurByte |=code<<(-k);
    codebitlen-=8;
   }
   BitsOfLeft=BOL[-k];
}

void WriteMarkofMainend(void)
{
  unsigned char pad;
  int size;

  if(WRC!=0)
  {
    size=sizeof(unsigned char);
    JPG_DataSize+=WRC*size;
    memcpy(BufOffset,WRbuf,WRC*size);
    BufOffset+=WRC*size;
  }
  if(BitsOfLeft!=8)
  {
    pad=0xff;
    pad>>=(8-BitsOfLeft);
    CurByte |=pad;
    size=sizeof(char);
    memcpy(BufOffset,&CurByte,size);
    BufOffset+=size;
    JPG_DataSize+=size;
  }
}
