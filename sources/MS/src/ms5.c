/**********************************************************************
*
*    ms5.c
*    =====
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     msintr();   Handles interrupt
*     mslset();   Sets alarm signal
*     mslinc();   Catch alarm signal
*     mswait();   Handles Wait....
*
*     mswlma();   Write line in message area
*     msrpma();   Repaint message area
*     msupwb();   Update window borders
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


static int drwait(); /* Visar v�nta... */

static char messtr[V3STRLEN+1];
static HWND meswin = NULL;

/* meswin �r meddelandef�nstret och messtr �r aktivt meddelande.
   F�nstret skapas av mswlma() och "repaintas" av msrpma() */

#define WP_INTTIME 2000       /* Antal sekunder mellan alarm */

static int     n_larm;
static int     n_secs;
static WPGWIN *wt_win;


/* n_larm s�tt till noll av wpwait(xxx,TRUE) och
   r�knas upp av wplinc() samt ner av wpintr(). 
   n_secs s�tt ocks� till noll men r�knas inte ner.
   wt_win �r en pekare till det f�nster d�r drwait()
   skall visa v�ntameddelandet. */


extern char        pidnam[],jobnam[];
extern V3MDAT      sydata;


#ifdef senare
/*!*******************************************************/

       bool msintr()

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
#ifdef SENARE
     while ( XCheckMaskEvent(xdisp,KeyPressMask,keyev) )
       {
       if ( keyev->state & ControlMask  &&
            XLookupString(keyev,tknbuf,10,&keysym,&costat) == 1   &&
            tknbuf[0] == *smbind[11].str )
         {
         XBell(xdisp,100);
         return(TRUE);
         }
       }
#endif
     }

   return(FALSE);
 }

/*********************************************************/
/*!*******************************************************/

       int  mslset(set)
       bool set;

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
     if ( n_secs == 0 )
       {
       n_secs = WP_INTTIME;
       SetTimer(ms_main,NULL,2*WP_INTTIME,mslinc);
       }
     else SetTimer(ms_main,NULL,WP_INTTIME,mslinc);
     }

   return(0);
 }

/*********************************************************/
/*!*******************************************************/

 VOID CALLBACK mslinc()

/*     F�ngar en alarmsignal best�lld av mslset()
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

   return(0);
 }

/*********************************************************/
/*!*******************************************************/

       int   mswait(win_id,wait)
       v2int win_id;
       bool  wait;

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
 *******************************************************!*/

 {
   WPWIN  *winptr;

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
       winptr = mswgwp((wpw_id)GWIN_MAIN);
       wt_win = (WPGWIN *)winptr->ptr;
       }
/*
***Om det handlar om att v�nta i ett visst f�nster fixar vi
***en C-pekare till just det f�nstret.
*/
     else
       {
       winptr = mswgwp((wpw_id)win_id);
       wt_win = (WPGWIN *)winptr->ptr;
       }
/*
***Eftersom v�nta skall sl�s p� nollst�ller vi lite
***och startar best�llning av alarmsignaler.
*/
     n_larm = n_secs = 0;
     mslset(TRUE);
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
     mslset(FALSE);
     }

   return(0);
 }

/*********************************************************/

/*!*******************************************************/

static int   drwait(draw)
       bool  draw;

/*     Ritar v�nta....
 *
 *     In: draw => TRUE = Sudda+rita. FALSE = Bara sudda.
 *
 *     Ut: Inget.
 *
 *     FV: 0
 *
 *     (C)microform ab 25/11/95 J. Kjellander
 *
 *******************************************************!*/

 {
   int     x,y,dx,dy;
   char    wt_str[V3STRLEN+1];
   char    timbuf[V3STRLEN+1];

static  WPBUTT *buttpt = NULL;

/*
***Om buttpt == NULL �r det f�rsta g�ngen och d� m�ste vi skapa
***sj�lva v�nta-f�nstret.
*
   if ( draw  &&  buttpt == NULL )
     {
     dx = msstrl("V�nta...XXXXX") + 15;
     dy = (int)(1.6*(double)msstrh());
     x  = (int)(wt_win->vy.scrwin.xmax - dx - 10);
     y  = (int)(wt_win->geo.dy - wt_win->vy.scrwin.ymin - dy - 10);

     mswcbu(wt_win->id.ms_id,x,y,dx,dy,1,"V�nta...","V�nta...","",
                                            WP_BGND,WP_FGND,&buttpt);
     }
/*
***Skall vi rita eller sudda ?
*/
   if ( draw )
     {
     strcpy(wt_str,"V�nta...");
     sprintf(timbuf,"%d",n_secs);
     strcat(wt_str,timbuf);

     strcpy(buttpt->stron,wt_str);
     strcpy(buttpt->stroff,wt_str);
     }
/*
***Om sudda beg�rts d�dar vi f�nstret.
*/
   else if ( buttpt != NULL )
     {
     msdlbu(buttpt);
     buttpt = NULL;
     }

   return(0);
 }

/*********************************************************/
#endif
/*!*******************************************************/

       int     msupwb(gwinpt)
       WPGWIN *gwinpt;

/*     Uppdaterar texten i ett WPGWIN-f�nsters ram.
 *
 *     In: gwinpt => C-pekare till grafiskt f�nster
 *                   eller NULL f�r huvudf�nstret.
 *
 *     Ut: Inget.
 *
 *     FV: 0
 *
 *     (C)microform ab 7/11/95 J. Kjellander
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
     if ( (winptr=mswgwp((wpw_id)GWIN_MAIN)) != NULL  &&
         winptr->typ == TYP_GWIN ) grawin = (WPGWIN *)winptr->ptr;
     else return(0);
     }
   else grawin = gwinpt;
/*
***Applikationsf�nstrets titel. Finns resurs, anv�nd den annars
***default titel med versionsnummer mm.
*/
   if ( !msgrst("APPWIN.TITLE",title) )
     {
     if ( sydata.opmode == BAS_MOD )
       sprintf(title,"VARKON %d.%d%c",sydata.vernr,sydata.revnr,
                                           sydata.level);
     else
       sprintf(title,"VARKON-R %d.%d%c",sydata.vernr,sydata.revnr,
                                           sydata.level);
     }
/*
***Skall projektnamn skrivas ut ?
*/
   if ( msgrst("APPWIN.TITLE.PROJECT",tmpbuf)  &&
        strcmp(tmpbuf,"True") == 0 )
     {
     strcat(title," - ");
     strcat(title,pidnam);
     }
/*
***Samma med jobnamn.
*/
   if ( msgrst("APPWIN.TITLE.JOBNAME",tmpbuf)  &&
        strcmp(tmpbuf,"True") == 0 )
     {
     strcat(title," - ");
     strcat(title,jobnam);
     }

   SetWindowText(ms_main,title);
/*
***WPGWIN-f�nstrets titel.
*/
   if ( grawin->vy.vynamn[0] == '\0' ) strcpy(title,"*****");
   else                                strcpy(title,grawin->vy.vynamn);

   SetWindowText(grawin->id.ms_id,title);

   return(0);
 }

/*********************************************************/
/*!*******************************************************/

     int   mswlma(s)
     char *s;

/*   Skriver ut meddelande i meddelandef�nster.
 *
 *   In: s  = Str�ng
 *
 *   Ut: Inget.
 *
 *   FV: 0
 *
 *   (C)microform ab 7/11/95 J. Kjellander
 *
 *   1996-12-15 Lite uppfr�schning, J.Kjellander
 *
 *******************************************************!*/

 {
   int    x,y,dx,dy;
   RECT   cr;

/*
***Om meswin != NULL finns gammalt meddelandef�nster att d�da.
*/
   if ( meswin != NULL )
     {
     DestroyWindow(meswin);
     meswin = NULL;
     }
/*
***Spara texten f�r repaint.
*/
   strcpy(messtr,s);
/*
***Om tom str�ng g�r det fort. Detta �r detsamma som sudda.
*/
   if ( strlen(s) == 0 ) return(0);
/*
***Skapa f�nster f�r det nya meddelandet. Placering i Y-led = 15 + 1
***pixels upp fr�n underkanten. H�jd = 15, Skugga = 1.
***Hur stort �r f�nstret ? Vid uppstart finns ingen meny !!!!
*/
   GetClientRect(ms_main,&cr);

   x  = 5;
   y  = cr.bottom - 16;

   dx = msstrl(s);
   dy = 15;

   meswin = CreateWindow(VARKON_IWIN_CLASS,"",WS_CHILD,
		      				 x,y,dx,dy,ms_main,NULL,ms_inst,NULL);
   ShowWindow(meswin,SW_SHOWNORMAL);
   UpdateWindow(meswin);
/*
***Slut.
*/
   return(0);
 }

/*********************************************************/
/*!*******************************************************/

     bool msrpma(win32_id)
     HWND win32_id;

/*   Repaintrutin f�r meddelandef�nster.
 *
 *   In: win32_id = F�nster id.
 *
 *   Ut: Inget.
 *
 *   FV: TRUE  = win32_id �r detta f�nster.
 *       FALSE = Det g�llde inte meddelandef�nstret.
 *
 *   (C)microform ab 7/11/95 J. Kjellander
 *
 *   1996-12-15 Lite uppfr�schning, J.Kjellander
 *
 *******************************************************!*/

 {
   int         x,y;
   HDC         dc;
   PAINTSTRUCT ps;
   TEXTMETRIC  tm;

/*
***G�ller det detta f�nster ?
*/
   if ( win32_id != meswin ) return(FALSE);
/*
***F�nstrets DC.
*/
   dc = BeginPaint(meswin,&ps);
/*
***Ber�kna textens l�ge. Tills vidare anv�nder vi font 0.
*/
   SelectObject(dc,msgfnt(0));
   GetTextMetrics(dc,&tm);

   x = 0;  
   y = 15 - tm.tmDescent - 2;
/*
***F�rg 7 �r ok.
*/
   SetBkColor(dc,PALETTEINDEX(7));             /* 7 = Gr� */
   SetTextColor(dc,PALETTEINDEX(0));           /* 1 = Svart */
/*
***Skriv ut.
*/
   mswstr(dc,x,y,messtr);
/*
***Sl�pp DC't.
*/
   EndPaint(meswin,&ps);
/*
***Slut.
*/
   return(TRUE);
 }

/*********************************************************/
