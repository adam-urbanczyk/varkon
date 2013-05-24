/**********************************************************************
*
*    ms16.c
*    ======
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     msgtmp();   Poll mouse position
*     msgtmc();   Return mouse coordinates in R2
*     msgmc3();   Return mouse coordinates in R3
*     msgtsc();   Return mouse position in screen coordinates
*     msiaut();   AutoZOOM
*     mschvi();   Set view
*     msgtsw();   Rubberband rectangle
*     mszoom();   ZOOM
*     msneww();   Create new window
*     msmaut();   Autozoom in MBS
*     msgtvi();   GET_VIEW in MBS
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
#include "../../../sources/GE/include/GE.h"

static void drwbox(); /* Ritar/suddar gummibandsbox */

extern VY       vytab[];
extern HCURSOR  ms_grcu;
extern DBTmat  *lsyspk;

/*!******************************************************/

        int  msgtmp(px,py)
        int *px;
        int *py;

/*      Returnerar (pollar) aktuell musposition relativt sk�rmens
 *      �vre v�nstra h�rn.
 *
 *      In: px     = Pekare till utdata.
 *          py     = Pekare till utdata.
 *
 *      Ut: *px    = X-koordinat i 2D modellkoordinater.
 *          *py    = Y-koordinat i 2D modellkoordinater.
 *
 *      FV:      0 = Ok.
 *
 *      (C)microform ab 1998-03-24 J. Kjellander
 *
 ******************************************************!*/

 {
   POINT p;
 
   GetCursorPos(&p);

  *px = (int)p.x;
  *py = (int)p.y;
/*
***Slut.
*/
   return(0);
 }

/********************************************************/
/*!******************************************************/

        int     msgtmc(pektkn,px,py,mark)
        char   *pektkn;
        double *px;
        double *py;
        bool    mark;

/*      Returnerar pek-tecken samt 2D-koordinat i modell-
 *      f�nster koordinatsystemet.
 *
 *      In: pektkn => Pekare till pek-tecknet.
 *          px     => Pekare till X-koordinaten.
 *          py     => Pekare till Y-koordinaten.
 *          mark   => Pek-m�rke TRUE/FALSE
 *
 *      Ut:*pektkn => Pek-tecken.
 *         *px     => X-koordinat i 2D modellkoordinater.
 *         *py     => Y-koordinat i 2D modellkoordinater.
 *
 *      FV:      0 = Ok.
 *
 *      (C)microform ab 3/11/95 J. Kjellander
 *
 ******************************************************!*/

 {
   short   ix,iy;
   v2int   win_id;
   WPWIN  *winptr;
   WPGWIN *gwinpt;

   msgtsc(mark,pektkn,&ix,&iy,&win_id);

   winptr = mswgwp((wpw_id)win_id);
   gwinpt = (WPGWIN *)winptr->ptr;

  *px = gwinpt->vy.modwin.xmin + (ix - gwinpt->vy.scrwin.xmin)*
                   (gwinpt->vy.modwin.xmax - gwinpt->vy.modwin.xmin)/
                   (gwinpt->vy.scrwin.xmax - gwinpt->vy.scrwin.xmin);
  *py = gwinpt->vy.modwin.ymin + (iy - gwinpt->vy.scrwin.ymin)*
                   (gwinpt->vy.modwin.ymax - gwinpt->vy.modwin.ymin)/
                   (gwinpt->vy.scrwin.ymax - gwinpt->vy.scrwin.ymin);

   return(0);
 }

/********************************************************/
/*!******************************************************/

        int     msgmc3(pektkn,pout,mark)
        char   *pektkn;
        DBVector  *pout;
        bool    mark;

/*      Returnerar pek-tecken samt 3D modellkoordinat
 *      i aktivt korrdinatsystems XY-plan (lokala
 *      koordinater, Z=0).
 *
 *      In: pektkn => Pekare till utdata.
 *          pout   => Pekare till utdata.
 *          mark   => Pek-m�rke TRUE/FALSE
 *
 *      Ut:*pektkn => Pek-tecken.
 *         *pout   => 3D position.
 *
 *      FV:      0 = Ok.
 *
 *      (C)microform ab 1998-10-20 J. Kjellander
 *
 ******************************************************!*/

 {
   short   ix,iy;
   v2int   win_id;
   gmflt   tt;
   WPWIN  *winptr;
   WPGWIN *gwinpt;
   DBVector   p,t;
   DBTmat  vm;

/*
***H�mta sk�rmkoordinat. msgtsc() returnerar �ven ID
***f�r det f�nster som det pekats i.
*/
   msgtsc(mark,pektkn,&ix,&iy,&win_id);
/*
***Fixa pekare till f�nstret.
*/
   winptr = mswgwp((wpw_id)win_id);
   gwinpt = (WPGWIN *)winptr->ptr;
/*
***Transformera till 2D modellkoordinat.
*/
   p.x_gm = gwinpt->vy.modwin.xmin + (ix - gwinpt->vy.scrwin.xmin)*
                   (gwinpt->vy.modwin.xmax - gwinpt->vy.modwin.xmin)/
                   (gwinpt->vy.scrwin.xmax - gwinpt->vy.scrwin.xmin);
   p.y_gm = gwinpt->vy.modwin.ymin + (iy - gwinpt->vy.scrwin.ymin)*
                   (gwinpt->vy.modwin.ymax - gwinpt->vy.modwin.ymin)/
                   (gwinpt->vy.scrwin.ymax - gwinpt->vy.scrwin.ymin);
   p.z_gm = 0.0;
/*
***Bilda 4X4 transformationsmatris. F�nstrets vymatris �r
***bara 3X3.
*/
   vm.g11 = gwinpt->vy.vymat.k11; vm.g12 = gwinpt->vy.vymat.k12;
   vm.g13 = gwinpt->vy.vymat.k13; vm.g14 = 0.0;
   vm.g21 = gwinpt->vy.vymat.k21; vm.g22 = gwinpt->vy.vymat.k22;
   vm.g23 = gwinpt->vy.vymat.k23; vm.g24 = 0.0;
   vm.g31 = gwinpt->vy.vymat.k31; vm.g32 = gwinpt->vy.vymat.k32;
   vm.g33 = gwinpt->vy.vymat.k33; vm.g34 = 0.0;
   vm.g41 = vm.g42 = vm.g43 = 0.0; vm.g44 = 1.0;
/*
***Transformera p till 3D modellkoordinat i BASIC.
*/
   GEtfpos_to_basic(&p,&vm,&p);
   GEtfpos_to_basic(&p,&vm,&p);
/*
***Skjutriktning, dvs. vyn:s Z-axel uttryckt i BASIC.
*/
   t.x_gm = gwinpt->vy.vymat.k31;
   t.y_gm = gwinpt->vy.vymat.k32;
   t.z_gm = gwinpt->vy.vymat.k33;
/*
***Transformera p och t till LOCAL.
*/
   if ( lsyspk != NULL )
     {
     GEtfpos_to_local(&p,lsyspk,&p);
     GEtfvec_to_local(&t,lsyspk,&t);
     }
/*
***Lokala Z-komponenten m�ste vara <> 0 annars �r XY-planet
***parallellt med skjutriktningen.
*/
   if ( ABS(t.z_gm) < 1E-10 ) return(-1);
/*
***Ber�kna sk�rning linje/XY-plan.
*/
   tt = -p.z_gm/t.z_gm;

   pout->x_gm = p.x_gm + tt*t.x_gm;
   pout->y_gm = p.y_gm + tt*t.y_gm;
   pout->z_gm = 0.0;
/*
***Slut.
*/
   return(0);
 }

/********************************************************/
/*!******************************************************/

        int      msgtsc(mark,pektkn,pix,piy,win_id)
        bool     mark;
        char    *pektkn;
        short   *pix;
        short   *piy;
        wpw_id  *win_id;

/*      Returnerar f�nster-ID, pek-tecken samt 2D-koordinat i
 *      ett koordinatsystem med origo i grafiska areans nedre
 *      v�nstra h�rn.
 *
 *      In: mark    => Pekm�rke TRUE/FALSE.
 *          pektkn  => Pekare till utdata.
 *          pix     => Pekare till utdata.
 *          piy     => Pekare till utdata.
 *          win_id  => Pekare till utdata.
 *
 *      Ut: *pektkn = Pek-tecken.
 *          *pix    = X-koordinat.
 *          *piy    = Y-koordinat.
 *          *win_id = ID f�r f�nster i vilket pekningen skett.
 *
 *      FV:      0 = Ok.
 *
 *      (C)microform ab 3/11/95 J. Kjellander
 *
 *       1996-01-30 Tangentbord, J. Kjellander
 *       1996-12-18 Multipla f�nster, J.Kjellander
 *
 ******************************************************!*/

 {
    int     x,y,dx,dy,xrk,yrk;
    bool    chrflg;
    WPGWIN *gwinpt;
    MSG     message;
    POINT   curpos;
    RECT    cr;
 
 /*
 ***Initiering.
 */
   chrflg = FALSE;
/*
***Se till att musen inte r�r sig utanf�r huvudf�nstrets applikationsarea.
*/
   msggeo(ms_main,&x,&y,&dx,&dy,NULL,NULL);
   cr.left   = x + GetSystemMetrics(SM_CXFRAME) + 2;
   cr.top    = y + GetSystemMetrics(SM_CYFRAME) +
                   GetSystemMetrics(SM_CYCAPTION) +
                   GetSystemMetrics(SM_CYMENU) + 2;
   cr.right  = cr.left + dx - 4;
   cr.bottom = cr.top  + dy - (1 + 15 + 1 + 2 + 2);
   ClipCursor(&cr);
/*
***Byt utseende p� muspekaren.
*/
   ms_grcu = LoadCursor(NULL,IDC_CROSS);
/*
***Meddelandeloop.
*/
loop:
   if ( GetMessage(&message,NULL,0,0) == TRUE )
     {
/*
***Om ett meddelande fanns att h�mta skickar vi det vidare
***till f�nstrets callback-rutin. Vissa meddelanden servas d�r
***men n�gra l�mnas kvar till l�ngre ner.
*/
     TranslateMessage(&message);
     DispatchMessage(&message);

     switch ( ms_lstmes.msg )
	   {
/*
***V�nster musknapp. Musknapptryckning ger koordinater relativt
***�vre v�nstra h�rnet p� client-arean, dvs. med h�nsyn tagen till
***storlek p� rubrikbalk.
*/
	   case WM_LBUTTONDOWN:
       if ( (gwinpt=msggwp(ms_lstmes.wh)) != NULL )
         {
         if ( !chrflg ) *pektkn = ' ';
         chrflg = FALSE;
         xrk = LOWORD(ms_lstmes.lp);
         yrk = HIWORD(ms_lstmes.lp);
         if ( mark ) wpcpmk((v2int)gwinpt->id.w_id,xrk,yrk);
        *pix    = (short)xrk;
        *piy    = (short)(gwinpt->geo.dy - yrk);
        *win_id = gwinpt->id.w_id;
         goto exit;
         }
       else
         {
         Beep(1000,1000);
         goto loop;
         }
/*
***H�ger musknapp.
*/
	   case WM_RBUTTONDOWN:
       if ( (gwinpt=msggwp(ms_lstmes.wh)) != NULL )
         {
        *pektkn = '\n';
        *pix    = (short)LOWORD(ms_lstmes.lp);
        *piy    = (short)(gwinpt->geo.dy - HIWORD(ms_lstmes.lp));
        *win_id = gwinpt->id.w_id;
         }
       else
         {
        *pektkn = '\n';
        *pix = 0;
        *piy = 0;
        *win_id = GWIN_MAIN;
         }
       goto exit;
/*
***Knapp p� tangentbordet. F�r att f� veta vilket f�nster musen befann
***sig i n�r knappen trycktes ned simulerar vi en mustryckning och s�tter
***teckenflaggan. Dett f�r till resultat att ett WM_LBUTTONDOWN genereras
***och vi tar hand om det hela h�gre upp.
*/
       case WM_CHAR:
       GetCursorPos(&curpos);
       mouse_event(MOUSEEVENTF_ABSOLUTE	|MOUSEEVENTF_LEFTDOWN,
                   curpos.x,curpos.y,NULL,NULL);
      *pektkn = ms_lstmes.wp;
       chrflg = TRUE;
       goto loop;
/*
***Andra meddelanden som WM_PAINT och s�nt skiter vi i.
*/
       default:
       goto loop;
	   }
	 }
/*
***Slut.
*/
exit:
   ms_grcu = LoadCursor(NULL,IDC_ARROW);
   ClipCursor(NULL);
/*
***Slut.
*/
   return(0);
 }

/********************************************************/
/*!******************************************************/

        int      msgtsw(gwinptp,pix1,piy1,pix2,piy2,rubmod,prompt)WPGWIN **gwinptp;
        int     *pix1,*piy1,*pix2,*piy2;
        int      rubmod;
        bool     prompt
        ;

/*      Interaktiv rutin f�r sk�rmf�nster med gummibands-
 *      rektangel.
 *
 *      In:  gwinptp = Pekare till utdata.
 *           pix1    = Pekare till utdata.
 *           piy1    = Pekare till utdata.
 *           pix2    = Pekare till utdata.
 *           piy2    = Pekare till utdata.
 *           rubmod  = Typ av gummiband.
 *           prompt  = Autoprompt.
 *
 *      Ut: *gwinptp = Pekare till f�nster d�r det h�nde.
 *          *pix1    = �vre v�nstra h�rnets X-kordinat.
 *          *piy1    = �vre v�nstra h�rnets Y-koordinat.
 *          *pix2    = Nedre h�gra h�rnets X-koordinat.
 *          *piy2    = Nedre h�gra h�rnets Y-koordinat.
 *
 *      FV:      0 = OK.
 *          REJECT = Operationen avbruten.
 *          GOMAIN = Huvudmenyn.
 *
 *      (C)microform ab 7/11/95 J. Kjellander
 *
 *      1998-03-19 gwinptp, J.Kjellander
 *      1998-03-24 rubmod, J.Kjellander
 *      1998-04-03 prompt, J.Kjellander
 *
 ******************************************************!*/

  {
    bool    down;
    int     org_mode,status,x1,y1,curx2,cury2,lastx2,lasty2,
            minh,minw;
    MSG     message;
    HPEN    dash_pen,old_pen;
    WPGWIN *gwinpt;

/*
***Lite initiering.
*/
   if ( prompt ) igptma(322,IG_MESS);
   down = FALSE;
   ms_grcu = LoadCursor(NULL,IDC_CROSS); 
/*
***Meddelandeloop.
*/
loop:
   if ( GetMessage(&message,NULL,0,0) == TRUE )
     {
/*
***Om ett meddelande fanns att h�mta skickar vi det vidare
***till f�nstrets callback-rutin. Vissa meddelanden servas d�r
***men n�gra l�mnas kvar till l�ngre ner.
*/
     TranslateMessage(&message);
     DispatchMessage(&message);

     switch ( ms_lstmes.msg )
	    {
/*
***V�nster musknapp trycks ned. Ta reda p� vilket
***f�nster det �r och initiera f�r gummiband mm.
*/
       case WM_LBUTTONDOWN:
       if ( !down && ((gwinpt=msggwp(ms_lstmes.wh)) != NULL) )
         {
        *gwinptp = gwinpt;
         org_mode = GetROP2(gwinpt->dc);
         SetROP2(gwinpt->dc,R2_XORPEN);
         dash_pen = CreatePen(PS_DOT,1,RGB(0,0,0));
         old_pen = SelectObject(gwinpt->dc,dash_pen);
         x1 = curx2 = lastx2 = LOWORD(ms_lstmes.lp);
         y1 = cury2 = lasty2 = HIWORD(ms_lstmes.lp);
         if ( prompt )
           {
           igrsma();
           igptma(323,IG_MESS);
           }
         down = TRUE;
         if ( rubmod == WP_RUB_ARROW )
           {
           Arc(gwinpt->dc,x1-7,y1-7,x1+7,y1+7,x1+7,y1+7,x1+7,y1+7);
           drwbox(gwinpt,x1,y1,x1,y1,rubmod);
           }
         }
       else
         {
         status = REJECT;
         goto exit;
         }
       break;
/*
***H�ger musknapp trycks ned = REJECT.
*/
       case WM_RBUTTONDOWN:
       if ( prompt ) igrsma();
       status = REJECT;
       goto exit;

/*
***Nu flyttar sig musen.
*/
       case WM_MOUSEMOVE:
       if ( down )
         {
         curx2 = LOWORD(ms_lstmes.lp);
         cury2 = HIWORD(ms_lstmes.lp);
/*
***Det b�r inte vara till�tet att g� ut utanf�r f�nstret.
*/
         if ( curx2 < (int)gwinpt->vy.scrwin.xmin )
              curx2 = (int)gwinpt->vy.scrwin.xmin;

         if ( curx2 > (int)gwinpt->vy.scrwin.xmax )
              curx2 = (int)gwinpt->vy.scrwin.xmax;

         if ( cury2 < (int)(gwinpt->geo.dy-gwinpt->vy.scrwin.ymax) )
              cury2 = (int)(gwinpt->geo.dy-gwinpt->vy.scrwin.ymax);

         if ( cury2 > (int)(gwinpt->geo.dy-gwinpt->vy.scrwin.ymin) )
              cury2 = (int)(gwinpt->geo.dy-gwinpt->vy.scrwin.ymin);
/*
***Vi �r fortfarande kvar inuti f�nstret. Rita om boxen.
*/
         if ( curx2 != lastx2  || cury2 != lasty2 )
           {
           drwbox(gwinpt,x1,y1,lastx2,lasty2,rubmod);
           if ( curx2 != x1  || cury2 != y1 )
             drwbox(gwinpt,x1,y1,curx2,cury2,rubmod);
           lastx2 = curx2;
           lasty2 = cury2;
           }
         }
       break;
/*
***N�r knappen sl�pps igen �r det slut.
*/
       case WM_LBUTTONUP:
       if ( down )
         {
         curx2 = LOWORD(ms_lstmes.lp);
         cury2 = HIWORD(ms_lstmes.lp);

         if ( curx2 < (int)gwinpt->vy.scrwin.xmin )
              curx2 = (int)gwinpt->vy.scrwin.xmin;

         if ( curx2 > (int)gwinpt->vy.scrwin.xmax )
              curx2 = (int)gwinpt->vy.scrwin.xmax;

         if ( cury2 < (int)(gwinpt->geo.dy-gwinpt->vy.scrwin.ymax) )
              cury2 = (int)(gwinpt->geo.dy-gwinpt->vy.scrwin.ymax);

         if ( cury2 > (int)(gwinpt->geo.dy-gwinpt->vy.scrwin.ymin) )
              cury2 = (int)(gwinpt->geo.dy-gwinpt->vy.scrwin.ymin);

         if ( prompt ) igrsma();

         if ( lastx2 != x1  || lasty2 != y1 )
           drwbox(gwinpt,x1,y1,lastx2,lasty2,rubmod);
         if ( rubmod == WP_RUB_ARROW )
           Arc(gwinpt->dc,x1-7,y1-7,x1+7,y1+7,x1+7,y1+7,x1+7,y1+7);
/*
***Varkons y-axel �r motsatt X11.
*/
         y1    = gwinpt->geo.dy - y1;
         cury2 = gwinpt->geo.dy - cury2;
/*
***Utdata, ej sorterade.
*/
        *pix1 = x1;    *pix2 = curx2;
        *piy1 = y1;    *piy2 = cury2;
/*
***Minsta f�nster f�r att betraktas som ok = 1% av sk�rmen.
*/
         minh = 5;
         minw = 5;

         if ( abs(*pix2 - *pix1) < minw  ||
              abs(*piy2 - *piy1) < minh ) status = -1;
         else                             status = 0;

         goto exit;
         }
       break;
       }
     goto loop;
     }
/*
***Utg�ngar.
*/
exit:
   if ( down )
     {
     SetROP2(gwinpt->dc,org_mode);
     SelectObject(gwinpt->dc,old_pen);
     DeleteObject(dash_pen);
     }

   ms_grcu = LoadCursor(NULL,IDC_ARROW);

   return(status);
  }

/********************************************************/
/*!******************************************************/

        short msneww()

/*      Interaktiv funktion f�r att skapa nytt WPGWIN.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV:      0 = OK.
 *          REJECT = Operationen avbruten.
 *          GOMAIN = Huvudmenyn.
 *
 *      (C)microform ab 1996-12-15 J. Kjellander
 *
 ******************************************************!*/

  {
    bool    down;
    int     org_mode,status,x1,y1,x2,y2,curx2,cury2,lastx2,lasty2,
            x,y,dx,dy,minh,minw;
    wpw_id  id;
    MSG     message;
    HPEN    dash_pen,old_pen;
    RECT    cr;
    HDC     main_dc;
    WPWIN  *winptr;
    WPGWIN *gwinpt,drwin;

/*
***Lite initiering.
*/
   igptma(322,IG_MESS);

   down = FALSE;
/*
***L�t huvudf�nstret �ga musen en stund s� att klickningar och r�resler
***i ev. grafiska f�nster skickas dit ist�llet.
*/
   SetCapture(ms_main);
/*
***F�r att kunna anv�nda drwbox() anv�nder vi en tillf�llig WPGWIN-
***structure d�r vi s�tter dc-medlemmen till huvudf�nstrets DC.
*/
   main_dc = GetDC(ms_main);
   drwin.dc = main_dc;
/*
/***Rastermode och penna.
*/
   org_mode = GetROP2(main_dc);
   SetROP2(main_dc,R2_XORPEN);
   dash_pen = CreatePen(PS_DOT,1,RGB(0,0,0));
   old_pen = SelectObject(main_dc,dash_pen);
/*
***Se till att musen inte r�r sig utanf�r huvudf�nstrets applikationsarea.
*/
   msggeo(ms_main,&x,&y,&dx,&dy,NULL,NULL);
   cr.left   = x + GetSystemMetrics(SM_CXFRAME) + 2;
   cr.top    = y + GetSystemMetrics(SM_CYFRAME) +
                   GetSystemMetrics(SM_CYCAPTION) +
                   GetSystemMetrics(SM_CYMENU) + 2;
   cr.right  = cr.left + dx - 4;
   cr.bottom = cr.top  + dy - (1 + 15 + 1 + 2 + 2);
   ClipCursor(&cr);
/*
***Meddelandeloop.
*/
loop:
   if ( GetMessage(&message,NULL,0,0) == TRUE )
     {
/*
***Om ett meddelande fanns att h�mta skickar vi det vidare
***till f�nstrets callback-rutin. Vissa meddelanden servas d�r
***men n�gra l�mnas kvar till l�ngre ner.
*/
     TranslateMessage(&message);
     DispatchMessage(&message);

     switch ( ms_lstmes.msg )
	    {
/*
***V�nster musknapp trycks ned.
*/
       case WM_LBUTTONDOWN:
       x1 = curx2 = lastx2 = LOWORD(ms_lstmes.lp);
       y1 = cury2 = lasty2 = HIWORD(ms_lstmes.lp);
       igrsma();
       igptma(323,IG_MESS);
       down = TRUE;
       break;
/*
***H�ger musknapp trycks ned = REJECT.
*/
       case WM_RBUTTONDOWN:
       igrsma();
       status = REJECT;
       goto exit;

/*
***Nu flyttar sig musen.
*/
       case WM_MOUSEMOVE:
       if ( down )
         {
         curx2 = LOWORD(ms_lstmes.lp);
         cury2 = HIWORD(ms_lstmes.lp);
/*
***Rita om boxen.
*/
         if ( curx2 != lastx2  || cury2 != lasty2 )
           {
           drwbox(&drwin,x1,y1,lastx2,lasty2,WP_RUB_RECT);
           if ( curx2 != x1  || cury2 != y1 )
             drwbox(&drwin,x1,y1,curx2,cury2,WP_RUB_RECT);
           lastx2 = curx2;
           lasty2 = cury2;
           }
         }
       break;
/*
***N�r knappen sl�pps igen �r det slut.
*/
       case WM_LBUTTONUP:
       if ( down )
         {
         curx2 = LOWORD(ms_lstmes.lp);
         cury2 = HIWORD(ms_lstmes.lp);

         igrsma();

         if ( lastx2 != x1  || lasty2 != y1 )
           drwbox(&drwin,x1,y1,lastx2,lasty2,WP_RUB_RECT);
/*
***Sortera koordinaterna.
*/
         if ( x1 < curx2 ) { x1 = x1; x2 = curx2; }
         else              { x2 = x1; x1 = curx2; }
         if ( y1 < cury2 ) { y1 = y1; y2 = cury2; }
         else              { y2 = y1; y1 = cury2; }
/*
***Minsta f�nster f�r att betraktas som ok.
*/
         minh = 25;
         minw = 15;

         if ( abs(x2 - x1) < minw  ||
              abs(y2 - y1) < minh ) status = REJECT;
         else                       status = 0;

         goto exit;
         }
       break;
       }
     goto loop;
     }
/*
***Slut p� inmatning..
*/
exit:
   ClipCursor(NULL);
   ReleaseCapture();
   SetROP2(main_dc,org_mode);
   SelectObject(main_dc,old_pen);
   DeleteObject(dash_pen);
   ReleaseDC(ms_main,main_dc);

   if ( status < 0 ) return(status);
/*
***Skapa nytt WPGWIN.
*/
   if ( mswcgw(x1,y1,x2-x1,y2-y1,"",FALSE,&id) < 0 )
     {
     errmes();
     return(0);
     }
/*
***Aktivera samma vy som i huvudf�nstret.
*/
   winptr = mswgwp(GWIN_MAIN);
   gwinpt = (WPGWIN *)winptr->ptr;
   msacvi(gwinpt->vy.vynamn,id);
/*
***Rita.
*/
   msrepa(id);

   return(0);
  }

/********************************************************/
/*!******************************************************/

        int     mszoom()

/*      Varkon-funktion f�r ZOOM med Microsoft Windows.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV:      0 = OK.
 *          REJECT = Operationen avbruten.
 *
 *      Felkod: IG3042 = Kan ej minska skalan mera.
 *
 *      (C)microform ab 14/12/94 J. Kjellander
 *
 *      1998-04-03 Ny msgtsw(), J.Kjellander
 *
 ******************************************************!*/

  {
   short   status;
   int     ix1,iy1,ix2,iy2,tmp;
   double  x1,y1,x2,y2;
   double  dx,dy;
   WPGWIN *gwinpt;

/*
***Vi b�rjar med att h�mta ett sk�rmf�nster. wpgtsw()
***sorterar koordinaterna.
*/
   if ( (status=msgtsw(&gwinpt,&ix1,&iy1,&ix2,&iy2,WP_RUB_RECT,TRUE)) < 0 )
     return(status);
/*
***Sortera koordinaterna.
*/
   if ( ix2 < ix1 ) { tmp = ix1; ix1 = ix2; ix2 = tmp; }
   if ( iy2 < iy1 ) { tmp = iy1; iy1 = iy2; iy2 = tmp; }
/*
***Transformera till modellkoordinater.
*/
   x1 = gwinpt->vy.modwin.xmin + (ix1-gwinpt->vy.scrwin.xmin)*
       (gwinpt->vy.modwin.xmax-gwinpt->vy.modwin.xmin)/
       (gwinpt->vy.scrwin.xmax-gwinpt->vy.scrwin.xmin);

   y1 = gwinpt->vy.modwin.ymin + (iy1-gwinpt->vy.scrwin.ymin)*
       (gwinpt->vy.modwin.ymax-gwinpt->vy.modwin.ymin)/
       (gwinpt->vy.scrwin.ymax-gwinpt->vy.scrwin.ymin);

   x2 = gwinpt->vy.modwin.xmin + (ix2-gwinpt->vy.scrwin.xmin)*
       (gwinpt->vy.modwin.xmax-gwinpt->vy.modwin.xmin)/
       (gwinpt->vy.scrwin.xmax-gwinpt->vy.scrwin.xmin);

   y2 = gwinpt->vy.modwin.ymin + (iy2-gwinpt->vy.scrwin.ymin)*
       (gwinpt->vy.modwin.ymax-gwinpt->vy.modwin.ymin)/
       (gwinpt->vy.scrwin.ymax-gwinpt->vy.scrwin.ymin);
/*
***Felkontroll.
*/
   dx = x2 - x1;
   dy = y2 - y1;

   if ( dx < 1e-10 || dy < 1e-10 )
     {
     erpush("IG3042","mszoom");
     errmes();
     return(0);
     }
/*
***Lagra det nya modellf�nstret i WPGWIN-posten och normalisera.
*/
   V3MOME(&gwinpt->vy,&gwinpt->old_vy,sizeof(WPVY));

   gwinpt->vy.modwin.xmin = x1;
   gwinpt->vy.modwin.xmax = x2;
   gwinpt->vy.modwin.ymin = y1;
   gwinpt->vy.modwin.ymax = y2;

   msnrgw(gwinpt);
/*
***Om det �r V3:s huvudf�nster skall �ven grapac underr�ttas.
*/
   if ( gwinpt->id.w_id == GWIN_MAIN ) wpupgp(gwinpt);
/*
***Sudda f�nstret och rita om.
*/
   msrepa((v2int)gwinpt->id.w_id);

   return(0);
  }

/********************************************************/
/*!******************************************************/

        int     msiaut(gwinpt)
        WPGWIN *gwinpt;

/*      Varkon-funktion f�r AutoZOOM med Microsoft Windows.
 *
 *      In: gwinpt = Pekare till f�nstret som skall zoom:as.
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 29/12/94 J. Kjellander
 *
 ******************************************************!*/

  {
   double dmx,dmy,dx,dy;
   VY     minvy;

/*
***Sudda f�nstret.
*/
   msergw((v2int)gwinpt->id.w_id);
/*
***Ber�kna nytt modellf�nster. F�r att detta skall ske
***med r�tt actvy, actvym osv. fixar vi grapac f�rst
***och �terst�ller efter�t.
*/
   if ( gwinpt->id.w_id == GWIN_MAIN )
     {
     wpfixg(gwinpt);
     igmsiz(&minvy);
     wpfixg(NULL);
     }
   else wpmsiz(gwinpt,&minvy);
/*
***Kolla att f�nstret inte blev o�ndligt litet. Kan tex.
***intr�ffa om modellen best�r av en enda punkt. Om s� �r
***fallet placerar vi punkten i mitten av f�nstret och s�tter
***skala = 1.
*/
   dx = minvy.vywin[2] - minvy.vywin[0];
   dy = minvy.vywin[3] - minvy.vywin[1];

   if ( dx == 0.0  &&  dy == 0.0 )
     {
     dmx = gwinpt->geo.psiz_x *
          (gwinpt->vy.scrwin.xmax - gwinpt->vy.scrwin.xmin);
     dmy = gwinpt->geo.psiz_y *
          (gwinpt->vy.scrwin.ymax - gwinpt->vy.scrwin.ymin);
     minvy.vywin[0] -= dmx/2.0;
     minvy.vywin[1] -= dmy/2.0;
     minvy.vywin[2] += dmx/2.0;
     minvy.vywin[3] += dmy/2.0;
     }
/*
***Om f�nstret inte blev o�ndligt litet men mindre �n 1e-10
***har vi inget f�nster alls. Inga storheter har gett upphov
***till n�gon grafik. I s�fall kan sk�rmen f�rbli suddad och 
***vi avslutar direkt.
*/
   else if ( dx < 1e-10  &&  dy < 1e-10 )
     {
     /*wpwait(gwinpt->id.w_id,FALSE); */
     return(0);
     }
/*
***Om f�nstret blev orimligt smalt som tex fallet med en
***horisontell eller vertikal linje g�r vi det lite bredare eller h�gre.
*/
loop:
   dx = minvy.vywin[2] - minvy.vywin[0];
   dy = minvy.vywin[3] - minvy.vywin[1];

   if ( dx < 1e-10 )
     {
     minvy.vywin[0] -= (0.05*dy);
     minvy.vywin[2] += (0.05*dy);
     goto loop;
     }

   if ( dy < 1e-10 )
     {
     minvy.vywin[1] -= (0.05*dx);
     minvy.vywin[3] += (0.05*dx);
     goto loop;
     }
/*
***G�r f�nstret 8% st�rre s� att allt syns ordentligt.
*/
   minvy.vywin[0] -= (0.04*dx);
   minvy.vywin[2] += (0.04*dx);
   minvy.vywin[1] -= (0.04*dy);
   minvy.vywin[3] += (0.04*dy);
/*
***Uppdatera modellf�nstret i WPGWIN-posten och normalisera.
*/
   V3MOME(&gwinpt->vy,&gwinpt->old_vy,sizeof(WPVY));

   gwinpt->vy.modwin.xmin = minvy.vywin[0];
   gwinpt->vy.modwin.xmax = minvy.vywin[2];
   gwinpt->vy.modwin.ymin = minvy.vywin[1];
   gwinpt->vy.modwin.ymax = minvy.vywin[3];

   msnrgw(gwinpt);
/*
***Om det �r V3:s huvudf�nster skall �ven grapac underr�ttas.
*/
   if ( gwinpt->id.w_id == GWIN_MAIN ) wpupgp(gwinpt);
/*
***Rita om f�nstret.
*/
   msrepa(gwinpt->id.w_id);
/*
***Slut p� v�ntan.
*
   wpwait(gwinpt->id.w_id,FALSE);
*/
   return(0);
  }

/********************************************************/
/*!******************************************************/

        int   msmaut(win_id)
        v2int win_id;

/*      AutoZOOM fr�n MBS.
 *
 *      In: win_id = ID f�r f�nster som skall zoom:as.
 *
 *      Ut: Inget.
 *
 *      Felkod: WP1382 = F�nster %s finns ej
 *
 *      (C)microform ab 1996-12-19 J. Kjellander
 *
 ******************************************************!*/

  {
   char    errbuf[81];
   WPWIN  *winptr;
   WPGWIN *gwinpt;

/*
***Om f�nstret finns fixar vi en C-pekare till det
***och anropar den interaktiva versionen av AutoZOOM.
*/
   if ( (winptr=mswgwp((wpw_id)win_id)) != NULL  &&
         winptr->typ == TYP_GWIN )
     {
     gwinpt = (WPGWIN *)winptr->ptr;
     return(msiaut(gwinpt));
     }
   else
     {
     sprintf(errbuf,"%d",win_id);
     return(erpush("WP1382",errbuf));
     }
  }

/********************************************************/
/*!******************************************************/

         int     mschvi(gwinpt,x,y)
         WPGWIN *gwinpt;
         int     x,y;

/*      Byter vy i ett visst f�nster.
 *
 *      In:  gwinpt = C-pekare till grafiskt f�nster.
 *           x,y    = Aktiverande kanpps/ikons rootl�ge.
 *
 *      Ut:  Inget.
 *
 *      (C)microform ab 6/11/95 J. Kjellander
 *
 ******************************************************!*/

  {
   char    *namlst[GPMAXV],rubrik[V3STRLEN];
   int      i,nvy,alt,actalt;

/*
msview();
return(0);
*/
/*
***Fixa en array av pekare till vynamnen.
***Till att b�rja med �r inget alternativ aktivt
***men om n�gon av vyerna �r aktiv i detta f�nster
***noterar vi det som actalt.
*/
   actalt = -1;
   nvy    =  0;

   for ( i=0; i<GPMAXV; ++i )
     {
     if ( vytab[i].vynamn[0] != '\0' )
       {
       if ( strcmp(vytab[i].vynamn,gwinpt->vy.vynamn) == 0 ) actalt = nvy;
       namlst[nvy] = vytab[i].vynamn;
       ++nvy;
       }
     }
/*
***Vy-f�nstrets rubrik.
*/
   if ( !msgrst("VIEW.TITLE",rubrik) ) strcpy(rubrik,"Vyer");
/*
***Anropa wpilst().
*/
   if ( msilst(x,y,rubrik,namlst,actalt,nvy,&alt) == REJECT ) return(0);
/*
***Tr�ff i alternativ nummer "alt". Aktivera den.
*/
   V3MOME(&gwinpt->vy,&gwinpt->old_vy,sizeof(WPVY));

   msacvi(namlst[alt],(v2int)gwinpt->id.w_id);
/*
***Sudda f�nstret och rita om.
*/
   msrepa((v2int)gwinpt->id.w_id);

   return(0);
  }

/********************************************************/
/*!******************************************************/

        int     msgtvi(win_id,vynamn,skala,xmin,ymin,xmax,ymax,vymat,persp)
        v2int   win_id;
        char   *vynamn;
        double *skala,*xmin,*ymin,*xmax,*ymax,*persp;
        DBTmat *vymat;

/*      WIN32-versionen av MBS-GET_VIEW.
 *
 *      In: win_id = ID f�r aktuellt f�nster.
 *
 *      Ut: *vynamn = Vy:ns namn.
 *          *skala  = Aktuell skala.
 *          *xmin   = Aktuellt modellf�nster.
 *          *ymin   =         -""-
 *          *xmax   =         -""-
 *          *ymax   =         -""-
 *          *vymat  = Aktuell 3D-transformation.
 *          *persp  = Perspektivavst�nd.
 *
 *      Felkod: WP1402 = F�nster med id %s finns ej
 *
 *      (C)microform ab 30/11/95 J. Kjellander
 *
 ******************************************************!*/

  {
   char    errbuf[81];
   WPWIN  *winptr;
   WPGWIN *gwinpt;

/*
***Fixa C-pekare till WPGWIN-f�nstret.
*/
   if ( (winptr=mswgwp((wpw_id)win_id)) != NULL  &&
         winptr->typ == TYP_GWIN )
     {
     gwinpt = (WPGWIN *)winptr->ptr;
     }
   else
     {
     sprintf(errbuf,"%d",win_id);
     return(erpush("WP1402",errbuf));
     }
/*
***Returnera vydata.
*/
   strcpy(vynamn,gwinpt->vy.vynamn);

  *skala = gwinpt->geo.psiz_x*
          (gwinpt->vy.scrwin.xmax - gwinpt->vy.scrwin.xmin) /
          (gwinpt->vy.modwin.xmax - gwinpt->vy.modwin.xmin);

  *xmin  = gwinpt->vy.modwin.xmin;
  *ymin  = gwinpt->vy.modwin.ymin;
  *xmax  = gwinpt->vy.modwin.xmax;
  *ymax  = gwinpt->vy.modwin.ymax;

   vymat->g11 = gwinpt->vy.vymat.k11;
   vymat->g12 = gwinpt->vy.vymat.k12;
   vymat->g13 = gwinpt->vy.vymat.k13;
   vymat->g14 = 0.0;

   vymat->g21 = gwinpt->vy.vymat.k21;
   vymat->g22 = gwinpt->vy.vymat.k22;
   vymat->g23 = gwinpt->vy.vymat.k23;
   vymat->g24 = 0.0;

   vymat->g31 = gwinpt->vy.vymat.k31;
   vymat->g32 = gwinpt->vy.vymat.k32;
   vymat->g33 = gwinpt->vy.vymat.k33;
   vymat->g34 = 0.0;

   vymat->g41 = 0.0;
   vymat->g42 = 0.0;
   vymat->g43 = 0.0;
   vymat->g44 = 1.0;
  
  *persp = gwinpt->vy.vydist;

   return(0);
  }

/********************************************************/
/*!******************************************************/

  static void drwbox(gwinpt,ix1,iy1,ix2,iy2,rubmod)
         WPGWIN *gwinpt;
         int     ix1,iy1,ix2,iy2;
         int     rubmod;

/*      Ritar/suddar gummibands-rektangel.
 *
 *      In:  
 *           ix1    = H�rn 1 X-koordinat.
 *           iy1    = H�rn 1 Y-koordinat.
 *           ix2    = H�rn 2 X-koordinat.
 *           iy2    = H�rn 2 Y-koordinat.
 *           rubmod = Typ av gummiband
 *
 *      Ut:  Inget.
 *
 *      (C)microform ab 7/11/95 J. Kjellander
 *
 *      1998-03-24 rubmod, J.Kjellander
 *
 ******************************************************!*/

  {
   int    x1,y1,x2,y2,dx,dy;
   double alfa,cosalf,sinalf;
   POINT  p[5];

/*
***Vilken sorts figur ?
*/
   switch ( rubmod )
     {
     case WP_RUB_NONE:
     return;
/*
***Rektangel, hur stor ?
*/
     case WP_RUB_RECT:
     dx = ix2 - ix1;
     dy = iy2 - iy1;
/*
***Om dx eller dy �r negativ m�ste vi fixa lite eftersom
***alla X-servrar inte klarar detta.
*/
     if ( dx < 0 ) x1 = ix2, x2 = ix1;
     else          x1 = ix1, x2 = ix2;

     if ( dy < 0 ) y1 = iy2, y2 = iy1;
     else          y1 = iy1, y2 = iy2;

     p[0].x = x1; p[0].y = y1;
     p[1].x = x2; p[1].y = y1;
     p[2].x = x2; p[2].y = y2;
     p[3].x = x1; p[3].y = y2;
     p[4].x = x1; p[4].y = y1;
     Polyline(gwinpt->dc,p,5);
     break;
/*
***Linje �r enkelt.
*/
     case WP_RUB_LINE:
     MoveToEx(gwinpt->dc,ix1,iy1,NULL);
     LineTo(gwinpt->dc,ix2,iy2);
     break;
/*
***Pil, ber�kna vinkeln mellan gummibandslinjen och positiva
***X-axeln.
*/
     case WP_RUB_ARROW:
     dx = ix2 - ix1;
     dy = iy2 - iy1;

     if ( dx == 0  &&  dy > 0 ) alfa = PI05;
     else if ( dx == 0 ) alfa = -PI05;
     else
       {
       if ( dx > 0 )
         {
         if ( dy >= 0 ) alfa =  ATAN((double)dy/(double)dx);
         else           alfa = -ATAN((double)-dy/(double)dx);
         }
       else
         {
         if ( dy >= 0 ) alfa = PI - ATAN((double)dy/(double)-dx);
         else           alfa = ATAN((double)-dy/(double)-dx) - PI;
         }
       }
/*
***Rita sj�lva pilen.
*/
     MoveToEx(gwinpt->dc,ix1,iy1,NULL);
     LineTo(gwinpt->dc,ix2,iy2);
/*
***Pilspetsen
*/
     cosalf = cos(alfa);
     sinalf = sin(alfa);

     dx = (int)(-25.0*cosalf + 10.0*sinalf);
     dy = (int)(-10.0*cosalf - 25.0*sinalf);
     MoveToEx(gwinpt->dc,ix2,iy2,NULL);
     LineTo(gwinpt->dc,ix2+dx,iy2+dy);

     dx = (int)(-25.0*cosalf - 10.0*sinalf);
     dy = (int)( 10.0*cosalf - 25.0*sinalf);
     MoveToEx(gwinpt->dc,ix2,iy2,NULL);
     LineTo(gwinpt->dc,ix2+dx,iy2+dy);
     break;
     }
  }

/********************************************************/
