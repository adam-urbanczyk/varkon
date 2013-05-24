/**********************************************************************
*
*    ms35.c
*    ======
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     msinsh();   Init OpenGL shader
*     msexsh();   Exit shader
*
*    This library is free software; you can redistribute it and/or
*    modify it under the terms of the GNU Library General Public
*    License as published by the Free Software Foundation; either
*    version 2 of the License, or (at your option) any later version.
*
*    This library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    Library General Public License for more details.
*
*    You should have received a copy of the GNU Library General Public
*    License along with this library; if not, write to the Free
*    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*    (C) 1984-1999, Johan Kjellander, Microform AB
*    (C) 200-2004, Johan Kjellander, �rebro university
*
***********************************************************************/

#include "../../../sources/DB/include/DB.h"
#include "../../../sources/IG/include/IG.h"
#include "../../../sources/WP/include/WP.h"

static bool msspfm(); /* S�tter upp pixelformat */

static WPGWIN *gwinpt; /* F�nsterpekare */
static HDC     dc;     /* F�nstrets DC */
static HGLRC   rc;     /* Motsvarande Rendering Context */

/*!******************************************************/

        int    msinsh(wid,zmin,zmax,smooth)
        int    wid;
        double zmin,zmax;
        bool   smooth;

/*      Initrutin f�r OpenGL-shading med WIN32.
 *
 *      In: wid       = F�nster-id.
 *          zmin,zmax = Scenes djup.
 *          smooth    = FLAT/SMOOTH.
 *
 *      Ut: Inget.
 *
 *      Felkoder: WP1633 = Ingen "visual"
 *                WP1643 = Ingen context
 *
 *      (C)microform ab 1997-02-24 J. Kjellander
 *
 ******************************************************!*/

  {

/*
***F�nsterpekare..
*/
   if ( (gwinpt=(WPGWIN *)wpwtab[(wpw_id)wid].ptr) == NULL ) return(-2);
/*
***Tror att det �r b�st att g�ra GetDC() och inte anv�nda
***gwinpt->dc eftersom detta DC �r till f�r normal grafik.
*/
   dc = GetDC(gwinpt->id.ms_id);
/*
***Initiera OpenGL. Skapa rendering context.
*/
   if ( !msspfm(dc) ) return(erpush("WP1633",""));

   if ( (rc=wglCreateContext(dc)) == NULL ) return(erpush("WP1643",""));
/*
***Aktivera Rendering Contextet.
*/
   wglMakeCurrent(dc,rc);
/*
***Aktivera djuptest.
*/
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);
   glClearDepth((GLclampd)1.0);
/* 
***Transformation. I varkon kan zmin = zmax men OpenGL gillar
***det inte.
*/
   if ( zmax - zmin < 1e-3 )
     {
     zmax = zmax + 0.01;
     zmin = zmin - 0.01;
     }

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(gwinpt->vy.modwin.xmin,gwinpt->vy.modwin.xmax,
              gwinpt->vy.modwin.ymin,gwinpt->vy.modwin.ymax,
              -zmax,-zmin);
/*
***Viewport. Den del av det grafiska f�nstret som f�r anv�ndas.
***Eventuella marginaler f�r knappar skall ej skrivas �ver.
*/
   glScissor((GLint)gwinpt->vy.scrwin.xmin,(GLint)gwinpt->vy.scrwin.ymin,
             (GLint)(gwinpt->vy.scrwin.xmax - gwinpt->vy.scrwin.xmin),
             (GLint)(gwinpt->vy.scrwin.ymax - gwinpt->vy.scrwin.ymin));
   glEnable(GL_SCISSOR_TEST);

   glViewport((GLint)gwinpt->vy.scrwin.xmin,(GLint)gwinpt->vy.scrwin.ymin,
              (GLint)(gwinpt->vy.scrwin.xmax - gwinpt->vy.scrwin.xmin),
              (GLint)(gwinpt->vy.scrwin.ymax - gwinpt->vy.scrwin.ymin));
/*
***L�t positiv normal definieras av moturs riktning precis
***som i Varkon.
*/
   glFrontFace(GL_CCW);
/*
***Configurera aktiva ljusk�llor.
*/
   wpconl(gwinpt);
/*
***Normalisera ytnormaler s� att f�rgstyrka
***ber�knas p� r�tt s�tt.
*/
   glEnable(GL_NORMALIZE);
/*
***Smooth shading kan ha betydelse om man anv�nder
***spitlights.
*/
   if ( smooth ) glShadeModel(GL_SMOOTH);
   else          glShadeModel(GL_FLAT);
/*
***Sudda f�rgbuffert och djupbuffert.
*/ 
   glClearColor((GLclampf)1.0,(GLclampf)1.0,(GLclampf)1.0,(GLclampf)1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   return(0);
  }

/********************************************************/
/*!******************************************************/

        int   msexsh()

/*      Exitrutin f�r OpenGL-shading.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 1997-02-07 J. Kjellander
 *
 *      1997-04-09 Sve_under, J.Kjellander
 *
 ******************************************************!*/

  {

/*
***Flusha OpenGL.
*/
   glFlush();
/*
***Kopiera resultatet (hela f�nstret) till "save_under"-bitmappen.
*/
   BitBlt(gwinpt->bmdc,0,0,gwinpt->geo.dx,gwinpt->geo.dy,gwinpt->dc,0,0,SRCCOPY);
/*
***St�ng OpenGL.
*/  
   wglMakeCurrent(dc,NULL); 
   wglDeleteContext(rc);
   ReleaseDC(gwinpt->id.ms_id,dc);


   return(0);
  }

/********************************************************/
/*!******************************************************/

 static bool msspfm(dc)
        HDC  dc;

/*      S�tter upp pixelformat.
 *
 *      In: F�nstrets WIN32-DC.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 1997-02-07 J. Kjellander
 *
 ******************************************************!*/

  {
    int pixelformat;

/*
***�nskade egenskaper.
*/
    static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),		// size of this pfd
	1,									// version number
	PFD_DRAW_TO_WINDOW |				// support window
	PFD_SUPPORT_OPENGL, 				// support OpenGL
	PFD_TYPE_RGBA,                      // F�rgsystem
	24,									// 24-bit color depth
	0, 0, 0, 0, 0, 0,					// color bits ignored
	0,									// no alpha buffer
	0,									// shift bit ignored
	0,									// no accumulation buffer
	0, 0, 0, 0, 						// accum bits ignored
    32,									// 32-bit z-buffer	
	0,									// no stencil buffer
	0,									// no auxiliary buffer
	PFD_MAIN_PLANE,						// main layer
	0,									// reserved
	0, 0, 0								// layer masks ignored
    };
/*
***Vad �r n�rmast m�jliga?
*/
    if ( (pixelformat=ChoosePixelFormat(dc,&pfd)) == 0 )
    {
        MessageBox(NULL,"ChoosePixelFormat failed","Error",MB_OK);
        return(FALSE);
    }
/*
***Aktivera det.
*/
    if (SetPixelFormat(dc,pixelformat,&pfd) == FALSE)
    {
        MessageBox(NULL,"SetPixelFormat failed","Error",MB_OK);
        return(FALSE);
    }
/*
***Hur gick det.
*/
    pixelformat = GetPixelFormat(dc);
    DescribePixelFormat(dc,pixelformat,sizeof(PIXELFORMATDESCRIPTOR),&pfd);

    if ( pfd.dwFlags & PFD_NEED_PALETTE )
      {
      return(TRUE);
      }
 


    return(TRUE);
  }

/********************************************************/
