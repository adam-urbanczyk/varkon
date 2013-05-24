/**********************************************************************
*
*    ms18.c
*    ======
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     msupgp();   Update grapac
*     msfixg();   Sets grapac for drawing
*     msspen();   Sets active pen
*     msclin();   Clip line
*     mscply();   Clip polyline
*
*     mscpmk();   Create hit marker
*     msepmk();   Delete hit marker
*     msdpmk();   Draw/erase hit marker
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
#include "../../../sources/GP/include/GP.h"
#include "../../../sources/WP/include/WP.h"

#ifdef senare
int     msscur();    /* S�tter grafisk cursor */
#endif

extern short   actpen;

/********************************************************/

        int     msupgp(gwinpt)
        WPGWIN *gwinpt;

/*      Uppdaterar grapac:s actwin och actvym mm.
 *
 *      In: gwinpt => Pekare till f�nster.
 *
 *      Ut: Inget.
 *
 *      FV: Inget.
 *
 *      (C)microform ab 1/11/95 J. Kjellander
 *
 ********************************************************/

 {
   double viewpt[4];

extern bool   gpgenz;
extern short  gpgorx,gpgory,gpsnpx,gpsnpy;
extern double k1x,k2x,k1y,k2y;
extern VY     actvy;
extern DBTmat actvym;

/*
***Vi b�rjar med viewport. Denna kan �ndras tex. av wpcogw().
***ConfigureNotify-handlern.
*/
   viewpt[0] = gwinpt->vy.scrwin.xmin;
   viewpt[1] = gwinpt->vy.scrwin.ymin;
   viewpt[2] = gwinpt->vy.scrwin.xmax;
   viewpt[3] = gwinpt->vy.scrwin.ymax;
   gpsvpt(viewpt);
/*
***grapac-variablerna gpgnpx och gpgnpy s�tts av gpsvpt()
***men inte gpgorx och gpgory.
*/
   gpgorx = (short)gwinpt->vy.scrwin.xmin;
   gpgory = (short)gwinpt->vy.scrwin.ymin;
/*
***Av samma anledning m�ste globala variablerna gpsnpx och
***gpsnpy uppdateras.
*/
   gpsnpx = gwinpt->geo.dx;
   gpsnpy = gwinpt->geo.dy;
/*
***Sen tar vi aktiv vy.
*/
   strcpy(actvy.vynamn,gwinpt->vy.vynamn);
   actvy.vywin[0] = gwinpt->vy.modwin.xmin;
   actvy.vywin[2] = gwinpt->vy.modwin.xmax;
   actvy.vywin[1] = gwinpt->vy.modwin.ymin;
   actvy.vywin[3] = gwinpt->vy.modwin.ymax;
   actvy.vydist   = gwinpt->vy.vydist;
   actvy.vy3d     = (tbool)gwinpt->vy.vy_3D;
/*
***Om perspektiv �r p� skall Z-flaggan s�ttas.
*/
   if ( gwinpt->vy.vydist == 0.0 ) gpgenz = FALSE;
   else                            gpgenz = TRUE;
/*
***2D Transformationskonstanter.
*/
   k1x = gwinpt->vy.k1x;
   k1y = gwinpt->vy.k1y;
   k2x = gwinpt->vy.k2x;
   k2y = gwinpt->vy.k2y;
/*
***Aktiv 3D transformationsmatris.
*/
   actvym.g11 = gwinpt->vy.vymat.k11;
   actvym.g12 = gwinpt->vy.vymat.k12;
   actvym.g13 = gwinpt->vy.vymat.k13;
   actvym.g14 = 0.0;
  
   actvym.g21 = gwinpt->vy.vymat.k21;
   actvym.g22 = gwinpt->vy.vymat.k22;
   actvym.g23 = gwinpt->vy.vymat.k23;
   actvym.g24 = 0.0;
  
   actvym.g31 = gwinpt->vy.vymat.k31;
   actvym.g32 = gwinpt->vy.vymat.k32;
   actvym.g33 = gwinpt->vy.vymat.k33;
   actvym.g34 = 0.0;
  
   actvym.g41 = 0.0;
   actvym.g42 = 0.0;
   actvym.g43 = 0.0;
   actvym.g44 = 1.0;

   return(0);
 }

/********************************************************/
/********************************************************/

        int     msfixg(gwinpt)
        WPGWIN *gwinpt;

/*      S�tter om globala variabler i grapac s� att
 *      grapac:s vektorgenereringsrutiner kan anv�ndas
 *      av MSPAC f�r WPGWIN-f�nster.
 *
 *      In: gwinpt => Pekare till f�nster eller NULL f�r
 *                    �terst�llning av ursprungliga data.
 *
 *      Ut: Inget.
 *
 *      FV: Inget.
 *
 *      (C)microform ab 1/11/95 J. Kjellander
 *
 ********************************************************/

 {
extern double k2x,k2y,gpgszx;
extern VY     actvy;
extern DBTmat actvym;

static   double orgszx,orgk2x,orgk2y;
static   VY     orgvy;
static   DBTmat orgvym;

/*
***G�ller det "tillst�llning" eller "�terst�llning" ?
*/
   if ( gwinpt != NULL )
     {
     orgszx = gpgszx;
     orgk2x = k2x;
     orgk2y = k2y;
     V3MOME(&actvy,&orgvy,sizeof(VY));
     V3MOME(&actvym,&orgvym,sizeof(DBTmat));

     actvy.vywin[0] = gwinpt->vy.modwin.xmin;
     actvy.vywin[1] = gwinpt->vy.modwin.ymin;
     actvy.vywin[2] = gwinpt->vy.modwin.xmax;
     actvy.vywin[3] = gwinpt->vy.modwin.ymax;
     actvy.vydist   = gwinpt->vy.vydist;
     actvy.vy3d     = (tbool)gwinpt->vy.vy_3D;
     gpgszx         = gwinpt->geo.dx * gwinpt->geo.psiz_x;
     k2x            = gwinpt->vy.k2x;
     k2y            = gwinpt->vy.k2y;

     actvym.g11 = gwinpt->vy.vymat.k11;
     actvym.g12 = gwinpt->vy.vymat.k12;
     actvym.g13 = gwinpt->vy.vymat.k13;
     actvym.g14 = 0.0;
  
     actvym.g21 = gwinpt->vy.vymat.k21;
     actvym.g22 = gwinpt->vy.vymat.k22;
     actvym.g23 = gwinpt->vy.vymat.k23;
     actvym.g24 = 0.0;
  
     actvym.g31 = gwinpt->vy.vymat.k31;
     actvym.g32 = gwinpt->vy.vymat.k32;
     actvym.g33 = gwinpt->vy.vymat.k33;
     actvym.g34 = 0.0;
  
     actvym.g41 = 0.0;
     actvym.g42 = 0.0;
     actvym.g43 = 0.0;
     actvym.g44 = 1.0;
     }
/*
***Vid �terst�llning kopierar vi tillbaks orginalen.
*/
   else
     {
     gpgszx         = orgszx;
     k2x            = orgk2x;
     k2y            = orgk2y;
     V3MOME(&orgvy,&actvy,sizeof(VY));
     V3MOME(&orgvym,&actvym,sizeof(DBTmat));
     }

   return(0);
 }

/********************************************************/
/********************************************************/

        int   msspen(pen)
        short pen;

/*      S�tter om aktiv penna, dvs. f�rg.
 *
 *      In: pen = Pennummer.
 *
 *      Ut: Inget.
 *
 *      FV: Inget.
 *
 *      (C)microform ab 1/11/95 J. Kjellander
 *
 ********************************************************/

{
   WPWIN  *winptr;
   WPGWIN *gwinpt;

/*
***�ndra aktiv penna (f�rgrundsf�rg) i WPGWIN-f�nstrets GC.
*/
   if ( pen != actpen  &&
       (winptr=mswgwp((wpw_id)GWIN_MAIN)) != NULL  &&
        winptr->typ == TYP_GWIN )
     {
     gwinpt = (WPGWIN *)winptr->ptr;
     SelectObject(gwinpt->dc,msgcol(pen));
     SelectObject(gwinpt->bmdc,msgcol(pen));
     actpen = pen;
     }

    return(0);
}

/********************************************************/
/*!******************************************************/

        int     msclin(gwinpt,pt1,pt2)
        WPGWIN *gwinpt;
        WPGRPT *pt1,*pt2;

/*      Klipper en grafisk linje mot ett f�nster.
 *
 *      In: gwinpt  => Pekare till f�nster.
 *          pt1,pt2 => Pekare till indata/utdata.
 *
 *      Ut: *pt1,*pt2 = �ndpunkter, ev. klippta.
 *
 *      FV: -1 => Linjen �r oklippt och ligger utanf�r f�nstret
 *           0 => Linjen �r oklippt och ligger innanf�r f�nstret.
 *           1 => Linjen �r klippt i pt1 (start).
 *           2 => Linjen �r klippt i pt2 (slut).
 *           3 => Linjen �r klippt i b�de pt1 och pt2.
 *
 *      (C) microform ab 1/11/95 J. Kjellander
 *
 ******************************************************!*/

 {
   short  sts1,sts2;
   double p1x,p1y,p2x,p2y,xmin,xmax,ymin,ymax;

   xmin = gwinpt->vy.modwin.xmin;
   ymin = gwinpt->vy.modwin.ymin;
   xmax = gwinpt->vy.modwin.xmax;
   ymax = gwinpt->vy.modwin.ymax;

   p1x = pt1->x;
   p1y = pt1->y;
   p2x = pt2->x;
   p2y = pt2->y;

/*
***Till att b�rja med antar vi att hela vektorn ligger
***innanf�r f�nstret.
*/
   sts1 = sts2 = 0;
/*
***Om punkt 1 ligger till v�nster om f�nstret kanske
***punkt 2 g�r det ocks�.
*/
   if ( p1x < xmin )
     {
     if ( p2x < xmin ) return(-1);
/*
***Om inte klipper vi.
*/
     p1y += (p2y - p1y)*(xmin - p1x)/(p2x - p1x);
     p1x  = xmin;
     sts1 = 1;
     }
/*
***Punkt 1 ligger till h�ger om f�nstret. Kanske punkt 2 ocks�.
*/
   else if ( p1x > xmax )
     {
     if ( p2x > xmax ) return(-1);
/*
***Ack nej, d� klipper vi mot h�gra kanten ist�llet.
*/
     p1y += (p2y - p1y)*(xmax - p1x)/(p2x - p1x);
     p1x  = xmax;
     sts1 = 1; 
     }
/*
***Om punkt 1 ligger nedanf�r kanske punkt 2 g�r det ocks�.
*/
   if ( p1y < ymin )
     {
     if ( p2y < ymin ) return(-1);
/*
***Nej d� klipper vi mot underkanten.
*/
     p1x += (p2x - p1x)*(ymin - p1y)/(p2y - p1y);
     p1y  = ymin;
     sts1 = 1;
     }
/*
***Om punkt 1 ligger ovanf�r kanske punkt 2 g�r det ocks�.
*/
   else if ( p1y > ymax )
     {
     if ( p2y > ymax ) return(-1);
/*
***Nej, d� klipper vi mot �verkanten.
*/
     p1x += (p2x - p1x)*(ymax - p1y)/(p2y - p1y);
     p1y  = ymax;
     sts1 = 1;
     }
/*
***Om punkt 1 �r klippt men fortfarande utanf�r m�ste �nd�
***hela vektorn ligga utanf�r.
*/
   if ( sts1 == 1 )
     if ( (p1x < xmin)  || (p1x > xmax)  ||
          (p1y < ymin)  || (p1y > ymax)) return(-1);
/*
***Punkt 1 ligger innanf�r f�nstret, klipp punkt 2 om utanf�r.
*/ 
   if ( p2x < xmin )
     {
     p2y += (p2y -  p1y)*(xmin - p2x)/(p2x - p1x);
     p2x  = xmin;
     sts2 = 2;
     }
   else if ( p2x > xmax )
     {
     p2y += (p2y -  p1y)*(xmax - p2x)/(p2x - p1x);
     p2x  = xmax;
     sts2 = 2;
     }

   if ( p2y < ymin )
     {
     p2x += (p2x - p1x)*(ymin - p2y)/(p2y - p1y);
     p2y  = ymin;
     sts2 = 2;
     }
   else if ( p2y > ymax )
     {
     p2x += (p2x - p1x)*(ymax - p2y)/(p2y - p1y);
     p2y  = ymax;
     sts2 = 2;
     }
/*
***Kopiera tillbaks klippta koordinater till pt1 och pt2.
*/
   pt1->x = p1x;
   pt1->y = p1y;
   pt2->x = p2x;
   pt2->y = p2y;
/*
***Slut.
*/
   return(sts1 + sts2);
 }

/********************************************************/
/*!******************************************************/

        bool     mscply(gwinpt,kmin,kmax,x,y,a)
        WPGWIN  *gwinpt;
        short    kmin;
        short   *kmax;
        double   x[];
        double   y[];
        char     a[];

/*      Klipper en polylinje mot ett visst f�nster.
*
*       In: gwinpt => Pekare till grafiskt f�nster.
*           kmin+1 => Offset till polylinjestart.
*           kmax   => Offset till polylinjeslut.
*           x      => X-koordinater.
*           y      => Y-koordinater.
*           a      => T�nd/Sl�ck.
*
*           x�kmin+1�,y� min+1�,a�kmin+1� = Startpunkt med status
*            !
*           x�kmax�,y�kmax�,a�kmax� = Slutpunkt med status
* 
*       Ut: *x,*y  => Klippta vektorer.
*           *kmax  => Offset till polylinjeslut.
*           *a     => Status (Bit(ENDSIDE) modifierad) om mer �n 50%
*                    av polylinjen klippts bort.
* 
*       FV: TRUE = N�gon del av polylinjen �r synlig.
*
*       (C)microform ab 1/11/95 J. Kjellander
*
********************************************************!*/

{
   short  i,j,l;
   double sav0,sav1;
   WPGRPT pt1,pt2;

/*
***Initiering.
*/
   i = j = kmin + 1;
/*
***Leta upp 1:a synliga vektor. Om ingen synlig, returnera FALSE.
*/
   do
     { 
     if ( i >= *kmax )
       {
       *kmax = kmin;
       return(FALSE);
       } 
     pt1.x = x[i];
     pt1.y = y[i];
     pt2.x = sav0 = x[++i];
     pt2.y = sav1 = y[i];
     } while ( msclin(gwinpt,&pt1,&pt2) == -1 );
/*
***I vec ligger nu en vektor som efter klippning syns, helt
***eller delvis! B�rja med en sl�ckt f�rflyttning till
***startpunkten och en normal f�rflyttning till n�sta punkt.
*/
   x[j]  = pt1.x;
   y[j]  = pt1.y;
   a[j] &= a[i-1] & ~VISIBLE; /* Varf�r & framf�r = ???? */

   x[++j] = pt2.x;
   y[j]   = pt2.y;
   a[j]   = a[i];
/*
***Loopa igenom resten av polylinjen.
*/
   while ( ++i <= *kmax )
     {
     pt1.x = sav0;
     pt1.y = sav1;
     pt2.x = sav0 = x[i];
     pt2.y = sav1 = y[i];
/*
***Klipp aktuell vektor. 0 => Vektorn helt innanf�r.
***                      2 => �nde 2 utanf�r.
*/
     switch ( msclin(gwinpt,&pt1,&pt2) )
       {
       case 0:
       case 2:
       x[++j] = pt2.x;
       y[j]   = pt2.y;
       a[j]   = a[i];            /* Detta �r fel !!! */
       break;
/*
*** 1 => �nde 1 utanf�r.
*** 3 => B�da �ndarna utanf�r. Vektorn klippt p� tv� st�llen.
*/
       case 1: 
       case 3:
       if ( ++j == i )
         { /* En koordinat blir tv� */    
         if ( ++i >= PLYMXV )
           { /* Om det finns plats, */
           *kmax = j;
           return(TRUE);
           }
         for ( l = ++(*kmax); l >= j; l-- )
           { /* fixa utrymme */
           x[l] = x[l-1];
           y[l] = y[l-1];
           a[l] = a[l-1];
           }
         }
       x[j]   = pt1.x;
       y[j]   = pt1.y;
       a[j]   = a[i-1] & ~VISIBLE;    /* Osynlig */
       x[++j] = pt2.x;
       y[j]   = pt2.y;
       a[j]   = a[i];
       break;
       }
     }
/*
***Uppdatera polylinjeslut.
*/
     *kmax = j;

     return(TRUE);
}

/********************************************************/
/*!******************************************************/

        int   mscpmk(win_id,ix,iy)
        v2int win_id;
        int   ix,iy;

/*      Ritar pekm�rke och uppdaterar pekm�rkes-listan.
 *
 *      In: win_id  => ID f�r f�nster att rita i.
 *          ix,iy   => Pekm�rkets position i f�nstret.
 *
 *      Ut: Inget.
 *
 *      FV: Inget.
 *
 *      (C) microform ab 1/11/95 J. Kjellander
 *
 ******************************************************!*/

 {
   int     i;
   WPWIN  *winptr;
   WPGWIN *gwinpt;

/*
***Loopa igenom alla WPGWIN-f�nster och rita d�r s�
***beg�rts.
*/
   for ( i=0; i<WTABSIZ; ++i )
     {
     if ( (winptr=mswgwp((wpw_id)i)) != NULL  &&
           winptr->typ == TYP_GWIN ) 
       {
       gwinpt = (WPGWIN *)winptr->ptr;

       if ( win_id == GWIN_ALL  ||  win_id == gwinpt->id.w_id )
         {
         msdpmk(gwinpt,ix,iy,TRUE);
         if ( gwinpt->pmkant < WP_PMKMAX-1 )
           {
           gwinpt->pmktab[gwinpt->pmkant].x = ix;
           gwinpt->pmktab[gwinpt->pmkant].y = iy;
         ++gwinpt->pmkant;
           }
         }
       }
     }

   return(0);
 }

/********************************************************/
/*!******************************************************/

        int   msepmk(win_id)
        v2int win_id;

/*      Suddar alla pekm�rken och nollst�ller pekm�rkes-
 *      listan i det/de beg�rda f�nstret/f�nstren.
 *
 *      In: win_id => F�nster-ID eller GWIN_ALL.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C) microform ab 1/11/95 J. Kjellander
 *
 ******************************************************!*/

 {
   int     i,j;
   WPWIN  *winptr;
   WPGWIN *gwinpt;

/*
***Loopa igenom alla WPGWIN-f�nster och sudda d�r s�
***beg�rts.
*/
   for ( i=0; i<WTABSIZ; ++i )
     {
     if ( (winptr=mswgwp((wpw_id)i)) != NULL  &&
           winptr->typ == TYP_GWIN ) 
       {
       gwinpt = (WPGWIN *)winptr->ptr;

       if ( win_id == GWIN_ALL  ||  win_id == gwinpt->id.w_id )
         {
         for ( j=0; j<gwinpt->pmkant; ++j )
           {
           msdpmk(gwinpt,(int)gwinpt->pmktab[j].x,
                         (int)gwinpt->pmktab[j].y,FALSE);
           }
         gwinpt->pmkant = 0;
         }
       }
     }

   return(0);
 }

/********************************************************/
/*!******************************************************/

        int     msdpmk(gwinpt,ix,iy,draw)
        WPGWIN *gwinpt;
        int     ix,iy;
        bool    draw;

/*      Ritar pekm�rke och uppdaterar pekm�rkes-listan.
 *
 *      In: gwinpt  => Pekare till f�nster att rita i.
 *          ix,iy   => Pekm�rkets position i f�nstret.
 *          draw    => TRUE = Rita, FALSE = Sudda.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C) microform ab 1/11/95 J. Kjellander
 *
 ******************************************************!*/

 {
   
   if      (  draw  &&  actpen != 1 ) msspen(1);
   else if ( !draw  &&  actpen != 0 ) msspen(0);

   MoveToEx(gwinpt->dc,ix,iy-1,NULL);
   LineTo(gwinpt->dc,ix,iy+1);

   MoveToEx(gwinpt->dc,ix-1,iy,NULL);
   LineTo(gwinpt->dc,ix+1,iy);

   return(0);
 }

/********************************************************/
#ifdef senare
/*!******************************************************/

        int     msscur(win_id,set,cursor)
        int     win_id;
        bool    set;
        Cursor  cursor;

/*      St�ller om aktiv cursor.
 *
 *      In: win_id => F�nster att byta cursor i.
 *          set    => TRUE = s�tt, FALSE = �terst�ll
 *          cursor => Om set, X-cursor.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C) microform ab 1/11/95 J. Kjellander
 *
 ******************************************************!*/

 {
   int     i;
   WPWIN  *winptr;
   WPGWIN *gwinpt;

/*
***Loopa igenom alla WPGWIN-f�nster.
*/
   for ( i=0; i<WTABSIZ; ++i )
     {
     if ( (winptr=mswgwp((wpw_id)i)) != NULL  &&  winptr->typ == TYP_GWIN ) 
       {
       gwinpt = (WPGWIN *)winptr->ptr;
/*
***Kanske skall vi byta i detta f�nster ?
*
       if ( win_id == GWIN_ALL  ||  win_id == gwinpt->id.w_id )
         {
         if ( set ) XDefineCursor(xdisp,gwinpt->id.x_id,cursor);
         else       XUndefineCursor(xdisp,gwinpt->id.x_id);
         }
         */
       }
     }

   return(0);
 }

/********************************************************/
#endif
