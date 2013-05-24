/**********************************************************************
*
*    wp5.c
*    ====
*
*    This file is part of the VARKON WindowPac Library.
*    URL: http://www.varkon.com
*
*    This file includes:
*
*    wpintr();   Interrupt routine
*    wplset();   Sets alarm signal
*    wplinc();   Catches alarm signal
*    wpwait();   Handles Wait...
*    wpwton();   Returns wait status On/Off
*    wpupwb();   Updates WPGWIN title text
*    wpposw();   Positions windows
*    wpwlma();   Dsiplays message text
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
*    (C)Microform AB 1984-1999, Johan Kjellander, johan@microform.se
*
***********************************************************************/

#include "../../DB/include/DB.h"
#include "../../IG/include/IG.h"
#include "../include/WP.h"
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define WP_INTTIME 2       /* Antal sekunder mellan alarm */

static int     n_larm = 0;
static int     n_secs = 0;
static bool    wt_on  = FALSE;
static WPGWIN *wt_win = NULL;

/* n_larm s�tt till noll av wpwait(xxx,TRUE) och
   r�knas upp av wplinc() samt ner av wpintr(). 
   n_secs s�tt ocks� till noll men r�knas inte ner.
   wt_on �r TRUE om v�nta �r p�slaget av wpwait() annars FALSE.
   wt_win �r en pekare till det f�nster d�r drwait()
   skall visa v�ntameddelandet. */


extern char        pidnam[],jobnam[];
extern short       gptrty;
extern MNUALT      smbind[];
extern V3MDAT      sydata;

/*!*******************************************************/

       bool wpintr()

/*     wp:s interrupt-rutin. Anropas med j�mna mellanrum
 *     av alla rutiner som kan ta l�ng tid p� sig. Sk�ter
 *     uppdatering av v�nta-meddelandet om v�nta �r p� samt
 *     pollar event-k�n efter <CTRL>c.
 *
 *     In: Inget.
 *
 *     Ut: Inget.
 *
 *     FV: TRUE  = V3 skall avbrytas.
 *         FALSE = Forts�tt exekveringen.
 *
 *     (C)microform ab 17/1/95 J. Kjellander
 *
 *     1998-02-26 event, J.Kjellander
 *
 *******************************************************!*/

 {
   XEvent          event;
   XKeyEvent      *keyev = (XKeyEvent *) &event;
   char            tknbuf[11];
   KeySym          keysym;
   XComposeStatus  costat;

/*
***Om n_larm > 0 �r v�nta-hanteringen p�slagen och det har
***g�tt tillr�ckligt med tid sen vi sist uppdaterade v�nta-
***meddelandet. Allts� uppdaterar vi igen. Om n_larm == 0
***har det antingen inte g�tt tillr�ckligt l�ng tid eller
***ocks� �r inte v�nta-hanteringen p�slagen �verhuvudtaget.
*/
   if ( n_larm > 0 )
     {
     drwait(TRUE);
   --n_larm;
/*
***Samtidigt som vi uppdaterar v�nta-meddelandet passar vi
***p� att polla eventk�n efter <CTRL>c.
*/
     while ( XCheckMaskEvent(xdisp,KeyPressMask,&event) )
       {
       if ( keyev->state & ControlMask  &&
            XLookupString(keyev,tknbuf,10,&keysym,&costat) == 1   &&
            tknbuf[0] == *smbind[11].str )
         {
         XBell(xdisp,100);
         XFlush(xdisp);
         return(TRUE);
         }
       }
     }

   return(FALSE);
 }

/*********************************************************/
/*!*******************************************************/

       short wplset(
       bool set)

/*     Best�ller en alarmsignal om en liten stund eller
 *     d�dar en redan best�lld.
 *
 *     In: set => TRUE  = Best�ll signal.
 *                FALSE = D�da best�lld.
 *
 *     Ut: Inget.
 *
 *     FV: 0
 *
 *     (C)microform ab 17/1/95 J. Kjellander
 *
 *******************************************************!*/

 {
/*
***Om det �r f�rsta g�ngen v�ntar vi lite l�ngre sen
***k�r vi p� som vanligt.
*/
   if ( set )
     {
     signal(SIGALRM,wplinc);
     if ( n_secs == 0 )
       {
       n_secs = WP_INTTIME;
       alarm(2*WP_INTTIME);
       }
     else alarm(WP_INTTIME);
     }
   else
     {
     alarm(0);
     signal(SIGALRM,SIG_DFL);
     }

   return(0);
 }

/*********************************************************/
/*!*******************************************************/

       void wplinc()

/*     F�ngar en alarmsignal best�lld av wplset()
 *     samt best�ller en ny.
 *
 *     In: Inget.
 *
 *     Ut: Inget.
 *
 *     FV: 0
 *
 *     (C)microform ab 17/1/95 J. Kjellander
 *
 *     1998-02-26 void, J.Kjellander
 *
 *******************************************************!*/

 {

/*
***R�kna upp larm- och sekund-r�knarna.
*/
   ++n_larm;
   n_secs += WP_INTTIME;
/*
***Best�ll ett nytt larm om en stund igen.
*/
   wplset(TRUE);

   return;
 }

/*********************************************************/
/*!*******************************************************/

       bool wpwton()

/*     Returnerar v�rdet av wt_on. Anv�nds av rutiner
 *     som tillf�lligt vill st�nga av interruptgenereringen.
 *     Tex. exos(). Om wpwton() == TRUE g�r man d� f�rst
 *     wplset(FALSE) och senare wplset(TRUE) igen.
 *
 *     FV: wt_on.
 *
 *     (C)microform ab 2/1/96 J. Kjellander
 *
 *******************************************************!*/

 {
   return(wt_on);
 }

/*********************************************************/
/*!*******************************************************/

       short wpwait(
       v2int win_id,
       bool  wait)

/*     Sl�r p� eller av v�nta-hanteringen.
 *
 *     In: win_id => F�nster-ID eller GWIN_ALL.
 *         wait   => TRUE = sl� p�, FALSE = st�ng av.
 *
 *     Ut: Inget.
 *
 *     FV: 0
 *
 *     (C)microform ab 17/1/95 J. Kjellander
 *
 *     1998-05-13 Batch, J.Kjellander
 *
 *******************************************************!*/

 {
   WPWIN  *winptr;

/*
***Om inte X11 �r initierat kan vi inget g�ra.
*/
   if ( gptrty != X11 ) return(0);
/*
***Om det �r GWIN_ALL som g�ller handlar det om att v�nta
***p� n�got som inte h�nder i ett s�rskilt f�nster utan
***i alla f�nster eller inte i n�got f�nster alls. Is�fall
***visar vi v�nta... i huvudf�nstret.
*/
   if ( wait )
     {
     if ( win_id == GWIN_ALL )
       {
       winptr = wpwgwp((wpw_id)GWIN_MAIN);
       wt_win = (WPGWIN *)winptr->ptr;
       }
/*
***Om det handlar om att v�nta i ett visst f�nster fixar vi
***en C-pekare till just det f�nstret.
*/
     else
       {
       winptr = wpwgwp((wpw_id)win_id);
       wt_win = (WPGWIN *)winptr->ptr;
       }
/*
***Eftersom v�nta skall sl�s p� nollst�ller vi lite
***och startar best�llning av alarmsignaler.
*/
     n_larm = n_secs = 0;
     wplset(TRUE);
     }
/*
***Om v�nta skall sl�s av st�nger vi av best�llningen av
***alarmsignaler och nollst�ller larmr�knaren igen s� att
***wpintr() inte servar interrupt i on�dan om alla inte
***har hunnit servas medan v�nta var p�.
*/
   else
     {
     drwait(FALSE);
     n_larm = 0;
     wplset(FALSE);
     }
/*
***Kom ih�g om vi �r p� eller av. Anv�nds av wpwton().
*/
   wt_on = wait;
/*
***Slut.
*/
   return(0);
 }

/*********************************************************/
/*!*******************************************************/

       short drwait(
       bool  draw)

/*     Ritar v�nta....
 *
 *     In: draw => TRUE = Sudda+rita. FALSE = Bara sudda.
 *
 *     Ut: Inget.
 *
 *     FV: 0
 *
 *     (C)microform ab 17/1/95 J. Kjellander
 *
 *     1998-09-08 iggtts(), J.Kjellander
 *
 *******************************************************!*/

 {
   int     x,y,dx,dy;
   char    wt_str[V3STRLEN+1];
   char    timbuf[V3STRLEN+1];

static  WPBUTT *buttpt = 0;

/*
***Om buttpt == NULL �r det f�rsta g�ngen och d� m�ste vi skapa
***sj�lva v�nta-f�nstret.
*/
   if ( draw  &&  buttpt == NULL )
     {
     dx = wpstrl("V�nta...XXXXX") + 15;
     dy = (int)(1.6*(double)wpstrh());
     x  = (int)(wt_win->vy.scrwin.xmax - dx - 10);
     y  = (int)(wt_win->geo.dy - wt_win->vy.scrwin.ymin - dy - 10);

     wpwcbu(wt_win->id.x_id,
           (short)x,(short)y,(short)dx,(short)dy,
           (short)1,"V{nta...","V{nta...","",
           (short)WP_BGND,(short)WP_FGND,&buttpt);

     XMapRaised(xdisp,buttpt->id.x_id);
     }
/*
***Skall vi rita eller sudda ?
*/
   if ( draw )
     {
     strcpy(wt_str,iggtts(164));
     sprintf(timbuf,"%d",n_secs);
     strcat(wt_str,timbuf);

     wpmaps(wt_str);
     strcpy(buttpt->stron,wt_str);
     strcpy(buttpt->stroff,wt_str);
     wpxpbu(buttpt);
     XFlush(xdisp);
     }
/*
***Om sudda beg�rts d�dar vi f�nstret.
*/
   else if ( buttpt != NULL )
     {
     XDestroyWindow(xdisp,buttpt->id.x_id);
     wpdlbu(buttpt);
     buttpt = NULL;
     }

   return(0);
 }

/*********************************************************/
/*!*******************************************************/

       short wpupwb(
       WPGWIN *gwinpt)

/*     Uppdaterar texten i ett WPGWIN-f�nsters ram.
 *
 *     In: gwinpt => C-pekare till grafiskt f�nster
 *                   eller NULL f�r huvudf�nstret.
 *
 *     Ut: Inget.
 *
 *     FV: 0
 *
 *     (C)microform ab 28/1/95 J. Kjellander
 *
 *     2/9/95  varkon.title.project, J. Kjellander
 *     28/9/95 NULL = GWIN_MAIN, J. Kjellander
 *
 *******************************************************!*/

 {
   char      title[V3STRLEN+1],tmpbuf[V3STRLEN+1];
   WPGWIN   *grawin;
   WPWIN    *winptr;
/*
***Initiering.
*/
   title[0] = '\0';
/*
***Fixa fram f�nstrets C-pekare.
*/
   if ( gwinpt == NULL )
     {
     if ( (winptr=wpwgwp((wpw_id)GWIN_MAIN)) != NULL  &&
         winptr->typ == TYP_GWIN ) grawin = (WPGWIN *)winptr->ptr;
     else return(0);
     }
   else grawin = gwinpt;
/*
***Om det �r huvudf�nstret inleder vi med en s�rskild titel-resurs.
*/
   if ( grawin->id.w_id == GWIN_MAIN )
     {
     if ( !wpgrst("varkon.title",title) )
       {
       if ( sydata.opmode == BAS_MOD )
         sprintf(title,"VARKON-3D/B %d.%d%c",sydata.vernr,sydata.revnr,
                                             sydata.level);
       else
         sprintf(title,"VARKON-3D/R %d.%d%c",sydata.vernr,sydata.revnr,
                                             sydata.level);
       }
/*
***Om varkon.title.project �r satt skriver vi �ven ut projektnamn i 
***huvudf�rnstret.
*/
     if ( wpgrst("varkon.title.project",tmpbuf) &&
          strcmp(tmpbuf,"True") == 0 )
         {
         strcat(title," - ");
         strcat(title,pidnam);
         }
     }
/*
***Alla f�nster kan ha jobnamn och/eller vynamn.
*/  
   if ( wpgrst("varkon.title.jobname",tmpbuf) &&
        strcmp(tmpbuf,"True") == 0 )
       {
       if ( grawin->id.w_id == GWIN_MAIN )
         {
         strcat(title," - ");
         strcat(title,jobnam);
         }
         else strcpy(title,jobnam);
       }

   if ( wpgrst("varkon.title.viewname",tmpbuf) &&
        strcmp(tmpbuf,"True") == 0 )
       {
       if ( title[0] != '\0' )             strcat(title," - ");
       if ( grawin->vy.vynamn[0] == '\0' ) strcat(title,"*****");
       else                                strcat(title,grawin->vy.vynamn);
       }
/*
***Mappa till svenska tecken.
*/
   wpmaps(title);
/*
***Uppdatera f�nsterramen.
*/
   XStoreName(xdisp,grawin->id.x_id,title);

   return(0);
 }

/*********************************************************/
/*!*******************************************************/

       short wpposw(
       int  rx,
       int  ry,
       int  dx,
       int  dy,
       int *px,
       int *py)

/*     Placerar ett f�nster s� n�ra en �nskad punkt som
 *     m�jligt utan att det kommer ut utanf�r sk�rmen.
 *
 *     In: rx,ry = �nskad position relativt root-f�nstret.
 *         dx,dy = F�nstrets storlek.
 *         px,py = Pekare till utdata.
 *
 *     Ut: *px,*py = Ny position.
 *
 *     FV: 0
 *
 *     (C)microform ab 17/1/95 J. Kjellander
 *
 *******************************************************!*/

 {
   int x,y,width,height;

   width  = DisplayWidth(xdisp,xscr);
   height = DisplayHeight(xdisp,xscr);
/*
***I sidled.
*/
   x = rx;
   if ( x + dx > width ) x = width - dx;
   if ( x < 0 ) x = 0;
/*
***I h�jdled.
*/
   y = ry;
   if ( y + dy > height ) y = height - dy;
   if ( y < 0 ) y = 0;

  *px = x;
  *py = y;

   return(0);
 }

/*********************************************************/
/*!*******************************************************/

     short wpwlma(
     char *s)

/*   Skriver ut meddelande i meddelandef�nster.
 *
 *   In: s  = Str�ng
 *
 *   Ut: Inget.
 *
 *   FV: 0
 *
 *   (C)microform ab 31/1/95 J. Kjellander
 *
 *******************************************************!*/

 {
   int     x,y,dx,dy;
   WPWIN  *winptr;
   WPGWIN *mainpt;

/*
***Om det inte finns n�got grafiskt f�nster g�r vi intet.
***Annars en C-pekare till V3:s huvudf�nster.
*/
   if ( (winptr=wpwgwp((wpw_id)GWIN_MAIN)) != NULL  &&
         winptr->typ == TYP_GWIN ) mainpt = (WPGWIN *)winptr->ptr;
   else return(0);
/*
***Mappa till ISO-8859.
*/
   wpmaps(s);
/*
***Om mesbpt != NULL finns gammalt meddelande att d�da.
*/
   if ( mainpt->mesbpt != NULL )
     {
     XDestroyWindow(xdisp,(mainpt->mesbpt)->id.x_id);
     wpdlbu(mainpt->mesbpt);
     mainpt->mesbpt = NULL;
     }
/*
***Om tom str�ng g�r det fort. Detta �r detsamma som sudda.
*/
   if ( strlen(s) == 0 ) return(0);
/*
***Skapa knapp f�r nya meddelandet.
*/
   dx = wpstrl(s) + 15;
   dy = (int)(1.6*(double)wpstrh());
   x  = (int)(mainpt->vy.scrwin.xmin + 10);
   y  = (int)(mainpt->geo.dy - mainpt->vy.scrwin.ymin - dy - 10);

   wpwcbu(mainpt->id.x_id,
         (short)x,(short)y,(short)dx,(short)dy,
         (short)1,s,s,"",
         (short)WP_BGND,(short)WP_FGND,&mainpt->mesbpt);

   XMapRaised(xdisp,(mainpt->mesbpt)->id.x_id);
   wpxpbu(mainpt->mesbpt);
   XFlush(xdisp);
/*
***Slut.
*/
   return(0);
 }

/*********************************************************/
