#include <gd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>


#define IMG_TYPE_GIF      1
#define IMG_TYPE_PNG      2
#define IMG_TYPE_JPG      3
#define IMG_TYPE_WBM      4
#define IMG_TYPE_XBM      5
#define IMG_TYPE_XPM      6
#define IMG_CONVERT_WBM   7
#define IMG_TYPE_GD       8
#define IMG_TYPE_GD2      9
#define IMG_TYPE_GD2PART  10
#define IMG_TYPE_WEBP     11

static const char sig_gd2[3] = {'g', 'd', '2'};
static const char sig_gif[3] = {'G', 'I', 'F'};
static const char sig_psd[4] = {'8', 'B', 'P', 'S'};
static const char sig_bmp[2] = {'B', 'M'};
static const char sig_swf[3] = {'F', 'W', 'S'};
static const char sig_swc[3] = {'C', 'W', 'S'};
static const char sig_jpg[3] = {(char) 0xff, (char) 0xd8, (char) 0xff};
static const char sig_png[8] = {(char) 0x89, (char) 0x50, (char) 0x4e, (char) 0x47, (char) 0x0d, (char) 0x0a, (char) 0x1a, (char) 0x0a};
static const char sig_tif_ii[4] = {'I','I', (char)0x2A, (char)0x00};
static const char sig_tif_mm[4] = {'M','M', (char)0x00, (char)0x2A};
static const char sig_jpc[3]  = {(char)0xff, (char)0x4f, (char)0xff};
static const char sig_jp2[12] = {(char)0x00, (char)0x00, (char)0x00, (char)0x0c, (char)0x6a, (char)0x50, (char)0x20, (char)0x20, (char)0x0d, (char)0x0a, (char)0x87, (char)0x0a};
static const char sig_iff[4] = {'F','O','R','M'};
static const char sig_ico[4] = {(char)0x00, (char)0x00, (char)0x01, (char)0x00};

static int ctx_getmbi(gdIOCtx *ctx)
{
    int i, mbi = 0;

    do {
        i = (ctx->getC)(ctx);
        if (i < 0) {
            return -1;
        }
        mbi = (mbi << 7) | (i & 0x7f);
    } while (i & 0x80);

    return mbi;
}

static int image_type(char data[8])
{
    if (data == NULL) {
        return -1;
    }

    if (!memcmp(data, sig_gd2, 3)) {
        return IMG_TYPE_GD2;
    } else if (!memcmp(data, sig_jpg, 3)) {
        return IMG_TYPE_JPG;
    } else if (!memcmp(data, sig_png, 3)) {
        if (!memcmp(data, sig_png, 8)) {
            return IMG_TYPE_PNG;
        }
    } else if (!memcmp(data, sig_gif, 3)) {
        return IMG_TYPE_GIF;
    } else {
        gdIOCtx *io_ctx;
        io_ctx = gdNewDynamicCtxEx(8, data, 0);
        if (io_ctx) {
            if (ctx_getmbi(io_ctx) == 0 && ctx_getmbi(io_ctx) >= 0) {
                io_ctx->gd_free(io_ctx);
            return IMG_TYPE_WBM;
            } else {
                io_ctx->gd_free(io_ctx);
            }
        }
    }

    return -1;
}

gdImage* createImage(const char* filename)
{
    FILE* fp;
    printf("filename = '%s'\n", filename);
    fp = fopen(filename, "rb");
    if(!fp)
    {
        fprintf(stderr, "can't read %s\n", filename);
        return NULL;
    }

    gdImage* im;
    int imtype;
    char sig[8];
    int i;

    fread(sig, 1, 8, fp);
    fseek(fp, 0, SEEK_SET);
    imtype = image_type(sig);

    printf("image_type = %d\n", imtype);

    switch (imtype) {
        case IMG_TYPE_JPG:
            printf("JPG\n");
            im = gdImageCreateFromJpeg(fp);
            break;

        case IMG_TYPE_PNG:
            printf("PNG\n");
            im = gdImageCreateFromPng(fp);
            break;

        case IMG_TYPE_GIF:
            printf("GIF\n");
            im = gdImageCreateFromGif(fp);
            break;

        case IMG_TYPE_WBM:
            printf("WBM\n");
            im = gdImageCreateFromWBMP(fp);
            break;

        case IMG_TYPE_GD2:
            printf("GD2\n");
            im = gdImageCreateFromGd2(fp);
            break;

        default:
            im = NULL;
    }

    fclose(fp);

    if (!im) {
        fprintf(stderr, "[create image] not created\n");
        return NULL;
    }

    return im;
}

int do_watermark(gdImage* watermark, gdImage* picture, int offset_x, int offset_y, int opacity)
{
    assert(watermark);
    assert(picture);

    gdImageSaveAlpha(watermark, 1);
    int watermarkX = gdImageSX(watermark);
    int watermarkY = gdImageSX(watermark);

    //opacity
    gdImageAlphaBlending(picture, 1);
    gdImageCopy(picture, watermark, offset_x, offset_y, 0, 0, watermarkX, watermarkY);
}

int main(int argc, char** arg)
{
    gdImage* watermark = createImage("./images/watermark.png");
    gdImage* picture   = createImage("images/picture.png");

    if(!watermark)
    {
        fprintf(stderr, "watermark have not been read");
        return 1;
    }

    if(!picture)
    {
        fprintf(stderr, "picture have not been read");
        return 1;
    }

    do_watermark(watermark, picture, 5, 6, 50);

    FILE* fp = fopen("images/result.png", "wb");
    if(!fp)
    {
        fprintf(stderr, "can't open file for result");
        return 1;
    }

    gdImagePng(picture, fp);
    fclose(fp);

    return 0;
}
