/** \file
 * \brief Haiku Double Buffer Driver
 *
 * See Copyright Notice in cd.h
 */

#include "cd.h"
#include "cd_private.h"
#include "cdhaiku.h"

#include <stdio.h>
#include <stdlib.h>

#include <Bitmap.h>
#include <View.h>

#define UNIMPLEMENTED printf("%s (%s %d) UNIMPLEMENTED\n",__func__,__FILE__,__LINE__);

static void cdkillcanvas (cdCtxCanvas* ctxcanvas)
{
  cdKillImage(ctxcanvas->image_dbuffer);
  cdhaikuKillCanvas(ctxcanvas);
}

static void cddeactivate(cdCtxCanvas* ctxcanvas)
{
  cdCanvas* canvas_dbuffer = ctxcanvas->canvas_dbuffer;
  /* this is done in the canvas_dbuffer context */
  cdCanvasDeactivate(canvas_dbuffer);
}

static void cdflush(cdCtxCanvas* ctxcanvas)
{
  int old_writemode;
  cdImage* image_dbuffer = ctxcanvas->image_dbuffer;
  cdCanvas* canvas_dbuffer = ctxcanvas->canvas_dbuffer;

  /* flush the writing in the image */
  ctxcanvas->view->LockLooper();
  ctxcanvas->view->Sync();
  ctxcanvas->view->UnlockLooper();

  /* this is done in the canvas_dbuffer context */
  /* Flush can be affected by Origin and Clipping, but not WriteMode */
  old_writemode = cdCanvasWriteMode(canvas_dbuffer, CD_REPLACE);
  cdCanvasPutImageRect(canvas_dbuffer, image_dbuffer, 0, 0, 0, 0, 0, 0);
  cdCanvasWriteMode(canvas_dbuffer, old_writemode);
}

static void cdcreatecanvas(cdCanvas* canvas, void* backbuffer)
{
  cdCanvas* canvas_dbuffer = (cdCanvas*)backbuffer;
  int w, h;
  cdCtxCanvas* ctxcanvas;
  cdImage* image_dbuffer;
  cdCtxImage* ctximage;

  cdCanvasActivate(canvas_dbuffer);
  w = canvas_dbuffer->w;
  h = canvas_dbuffer->h;
  if (w==0) w=1;
  if (h==0) h=1;

  /* this is done in the canvas_dbuffer context */
  image_dbuffer = cdCanvasCreateImage(canvas_dbuffer, w, h);
  if (!image_dbuffer) { 
    return;
  }

  ctximage = image_dbuffer->ctximage;

  /* Init the driver DBuffer */
  BView* view = new BView(ctximage->bitmap->Bounds(), "backbuffer", B_FOLLOW_NONE, B_WILL_DRAW);
  ctximage->bitmap->AddChild(view);
  ctxcanvas = cdhaikuCreateCanvas(canvas, view);

  if (!ctxcanvas)
    return;

  ctxcanvas->image_dbuffer = image_dbuffer;
  ctxcanvas->canvas_dbuffer = canvas_dbuffer;
}

static int cdactivate(cdCtxCanvas* ctxcanvas)
{
  int w, h;
  cdCanvas* canvas_dbuffer = ctxcanvas->canvas_dbuffer;

  /* this is done in the canvas_dbuffer context */
  /* this will update canvas size */
  cdCanvasActivate(canvas_dbuffer);
  w = canvas_dbuffer->w;
  h = canvas_dbuffer->h;
  if (w==0) w=1;
  if (h==0) h=1;

  /* check if the size changed */
  if (w != ctxcanvas->image_dbuffer->w ||
      h != ctxcanvas->image_dbuffer->h)
  {
    cdCanvas* canvas = ctxcanvas->canvas;
    /* save the current, if the rebuild fail */
    cdImage* old_image_dbuffer = ctxcanvas->image_dbuffer;
    cdCtxCanvas* old_ctxcanvas = ctxcanvas;

    /* if the image is rebuild, the canvas that uses the image must be also rebuild */

    /* rebuild the image and the canvas */
    canvas->ctxcanvas = NULL;
    canvas->context->cxCreateCanvas(canvas, canvas_dbuffer);
    if (!canvas->ctxcanvas)
    {
      canvas->ctxcanvas = old_ctxcanvas;
      return CD_ERROR;
    }

    /* remove the old image and canvas */
    cdKillImage(old_image_dbuffer);
    cdhaikuKillCanvas(old_ctxcanvas);

    ctxcanvas = canvas->ctxcanvas;

    /* update canvas attributes */
    cdUpdateAttributes(canvas);
  }

  return CD_OK;
}

static void cdinittable(cdCanvas* canvas)
{
  cdhaikuInitTable(canvas);

  canvas->cxActivate = cdactivate;
  canvas->cxDeactivate = cddeactivate;
  canvas->cxFlush = cdflush;
  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdDBufferContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_PATH | CD_CAP_BEZIER | CD_CAP_FPRIMTIVES ),
  CD_CTX_IMAGE,
  cdcreatecanvas,  
  cdinittable,
  NULL,             
  NULL, 
};

extern "C" cdContext* cdContextDBuffer(void)
{
  return &cdDBufferContext;
}

