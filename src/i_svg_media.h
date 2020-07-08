#ifndef _SVG_MEDIA
#define _SVG_MEDIA

#include "nanosvg.h"
#include "nanosvgrast.h"
#include "bmp.h"

enum {
    SVG_ARC,
    SVG_BLOCK,
    SVG_BOOK,
    SVG_BRAZIL,
    SVG_BRICK,
    SVG_BRUSH,
    SVG_BUCKET,
    SVG_CIRCLE,
    SVG_CLOSE,
    SVG_COPY,
    SVG_CURSOR,
    SVG_CUT,
    SVG_CZ,
    SVG_DOWN,
    SVG_DUPLI,
    SVG_EDIT,
    SVG_ELIPSE,
    SVG_EXPLODE,
    SVG_EXPORT,
    SVG_EYE,
    SVG_FIND,
    SVG_FONT,
    SVG_FREEZE,
    SVG_GEAR,
    SVG_HAND,
    SVG_HATCH,
    SVG_HELP,
    SVG_IMAGE,
    SVG_IMPORT,
    SVG_INFO,
    SVG_I_SEL,
    SVG_I_TEXT,
    SVG_LAYERS,
    SVG_LEFT,
    SVG_LINE,
    SVG_LOCK,
    SVG_LTYPE,
    SVG_MAGNET,
    SVG_MIRROR,
    SVG_MOVE,
    SVG_NEW,
    SVG_NEXT,
    SVG_NO_EYE,
    SVG_OPEN,
    SVG_PASTE,
    SVG_PIECE,
    SVG_PIECE_P,
    SVG_PLINE,
    SVG_PREV,
    SVG_PRINT,
    SVG_PUZZLE,
    SVG_RECT,
    SVG_RECT_SEL,
    SVG_REDO,
    SVG_RIGTH,
    SVG_ROT,
    SVG_RULER,
    SVG_SAVE,
    SVG_SCALE,
    SVG_SCRIPT1,
    SVG_SCRIPT2,
    SVG_SINGLE_SEL,
    SVG_SPLINE,
    SVG_STYLE,
    SVG_SUN,
    SVG_SYMBOL,
    SVG_TAG,
    SVG_TAGS,
    SVG_TAG_E,
    SVG_TEXT,
    SVG_TEXT_E,
    SVG_TEXT_STY,
    SVG_TOOL,
    SVG_TRASH,
    SVG_UNDER_L,
    SVG_UNDO,
    SVG_UNLOCK,
    SVG_UP,
    SVG_WARNING,
    SVG_ZOOM_A,
    SVG_ZOOM_M,
    SVG_ZOOM_P,
    SVG_ZOOM_W,
    SVG_MEDIA_SIZE
} svg_list;

NSVGimage ** i_svg_all_curves(void);

void i_svg_free_curves(NSVGimage **curves);

bmp_img ** i_svg_all_bmp(NSVGimage **curves, int w, int h);

bmp_img * i_svg_bmp(NSVGimage *curve, int w, int h);

void i_svg_free_bmp(bmp_img **img);

#endif