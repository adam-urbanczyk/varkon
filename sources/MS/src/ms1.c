/**********************************************************************
*
*    ms1.c
*    =====
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     msinit();   Opens mspac
*     msexit();   Closes mspac
*     mscdgw();   Creates default grapfical window
*     msclrg();   Closes all graphical windows
*     mssvjb();   Writes graphical windows to jofile
*     msldjb();   Reads graphical windows from jobfile
*     msmbox();   Windows message box
*     msmwsx();   Main window size in X-direction
*     msmwsy();   Main window size in Y-direction
*     msggeo();   Window size and position
*     msngws();   How many graphical windows are there ?
*     msprtf();   Prints file
*     msgtwi();   Returns window data
*     msmest();   Translates message -> text
*     msf1hk();   F1-filter
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
#include "../../../sources/EX/include/EX.h"
#include "../../../sources/GP/include/GP.h"
#include "../../../sources/WP/include/WP.h"



LRESULT CALLBACK msf1hk(int,WPARAM,LPARAM);

POINT    ms_buf[PLYMXV];
int      ms_ncrd;
HDC      ms_dc,ms_bmdc;

static HHOOK f1hook;

/* Buffert, r�knare och dc f�r gp3.c */

MNUALT  *actmeny;
MNUALT  *ms_cmdtab[MALMAX];

/* ms_cmdtab mappar WM_COMMAND:s till menyalternativ */

HCURSOR  ms_grcu;

/* Cursor f�r grafiska f�nster */


extern short  gpsnpx,gpsnpy,gpgorx,gpgory,gpgnpx,gpgnpy;
extern double gpgszx,gpgszy;
extern VY     actvy;

/*!******************************************************/

        int msinit(inifil)
        char *inifil;

/*      �ppnar mspac. Skapar applikationens huvudf�nster.
 *      Anropas av WinMain vid uppstart. Denna rutin f�r/kan 
 *      bara anropas en g�ng. D�refter m�ste V3 avslutas
 *      innan msinit() kan anv�ndas igen.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 18/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
   int   status;

/*
***Ladda/initiera resurser.
*/
   if ( (status=msmrdb(inifil,TRUE)) != 0 ) return(status);
/*
***Initiera och skapa huvudf�nster.
*/
   if ( (status=mswini()) != 0 ) return(status);
/*
***Initiera f�rger och fonter.
*/
   mscini();
   msfini();
/*
***Nollst�ll grapac/gp3:s koordinatr�knare.
*/
   ms_ncrd = 0;
/*
***Initiera menyer.
*/
   if ( (status=msmini()) < 0 ) return(status);
/*
***Installera F1-filter.
*
   if ( (f1hook=SetWindowsHookEx(WH_KEYBOARD,msf1hk,NULL,NULL)) == NULL )
     return(-2);
/*
***Slut.
*/
   return(0);
  }

/********************************************************/
/*!******************************************************/

        int msexit()

/*      Avslutar mspac. Anropas av v3exit() n�r VARKON
 *      avsutas.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 18/10/95 J. Kjellander
 *
 ******************************************************!*/

  {

/*
***D�da alla grafiska f�nster.
*/
   msclrg();
/*
***Avinstallera F1-filtret.
*
   UnhookWindowsHookEx(f1hook);
/*
***D�da menyer.
*/
   msmexi();
/*
***St�ng mspac.
*/
   mswexi();
/*
***L�mna tillbaks font- och f�rg-resurser.
*/
   msfexi();
   mscexi();
/*
***L�mna tillbaks minne allokerat f�r resursdatabasen.
*/
   msmrdb("",FALSE);
/*
***Slut.
*/
   return(0);
  }

/********************************************************/
/*!******************************************************/

LRESULT CALLBACK msf1hk(code,wp,lp)
        int    code;
        WPARAM wp;
        LPARAM lp;

/*      F1-filter avsett f�r contextk�nslig hj�lp.
 *
 *      In: code = Hur processa ?
 *          wp   = Virtual keycode.
 *          lp   = Keystroke-message information.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 18/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
   int mask;

/*
***Om code < 0 skall vi inte bry oss s�ger manualen.
*/
   if ( code < 0 ) return(CallNextHookEx(f1hook,code,wp,lp));
/*
***Om code �r HC_ACTION och det �r F1 och knappen �r p� v�g ned
***skickar vi ut hj�lp.
*/
   else if ( code == HC_ACTION  &&  wp == VK_F1  &&  (lp&(mask=1<<31)) == 0 )
     {
     ighelp();
     return(1);
     }
/*
***Annars returnerar vi en nolla som inneb�r att tecknet skickas
***vidare till f�nstret det ska till.
*/
   return(0);
  }

/********************************************************/
/*!******************************************************/

        int  mscdgw(create)
        bool create;

/*      Anv�nds av igepac/iginjb och igldjb f�r att skapa
 *      default grafiskt f�nster.
 *      Om create = TRUE skapas och aktiveras �ven default
 *      vy.
 *
 *      In: create => TRUE  = Skapa �ven default vy.
 *                    FALSE = Skapa bara f�nstren.
 *
 *      Ut: Inget.   
 *
 *      Felkod: WP1242 = Kan ej skapa grafiskt f�nster p� %s
 *
 *      (C)microform ab 3/11/95 J. Kjellander
 *
 *      1996-12-15 Lite sm�puts, J.Kjellander
 *
 ******************************************************!*/

  {
   int       x,y,dx,dy;
   double    dx_mm,dy_mm;
   v2int     id;
   char      avynam[GPVNLN+1],buf[20];
   WPGWIN   *gwinpt;
   VYVEC     bpos;

/*
***Eftersom mswcgw() anropar msupgp() kommer actvy att skrivas �ver.
***Spara namnet p� aktiv vy s� att denna kan aktiveras om jobb har
***laddats fr�n jobfil utan f�nster.
*/
   strcpy(avynam,actvy.vynamn);
/*
***V3:s grafiska huvud-f�nster, GWIN_MAIN.
***Hur stort skall det vara ? Om resurser finns tar vi dom annars
***g�r vi det s� stort som m�jligt.
*/
   msggeo(ms_main,&x,&y,&dx,&dy,&dx_mm,&dy_mm);
   x   = 0;
   y   = 2;
   dy -= (1 + 15 + 1 + 2 + 2 + 2);          /* Promtraden */

   if ( msgrst("GRAWIN_1.X",buf) )  sscanf(buf,"%d",&x);
   if ( msgrst("GRAWIN_1.Y",buf) )  sscanf(buf,"%d",&y);
   if ( msgrst("GRAWIN_1.DX",buf) ) sscanf(buf,"%d",&dx);
   if ( msgrst("GRAWIN_1.DY",buf) ) sscanf(buf,"%d",&dy);

   mswcgw(x,y,dx,dy,"",TRUE,&id);  
/*
***Fixa en C-pekare till det nu skapade f�nstret.
*/
   gwinpt  = (WPGWIN *)wpwtab[(wpw_id)id].ptr;
/*
***Underr�tta grapac om resultatet.
*/
   gpsnpx = gwinpt->geo.dx;
   gpsnpy = gwinpt->geo.dy;
   gpgorx = (short)gwinpt->vy.scrwin.xmin;
   gpgory = (short)gwinpt->vy.scrwin.ymin;
   gpgnpx = (short)(gwinpt->vy.scrwin.xmax - gwinpt->vy.scrwin.xmin);
   gpgnpy = (short)(gwinpt->vy.scrwin.ymax - gwinpt->vy.scrwin.ymin);
   gpgszx = gpgnpx*gwinpt->geo.psiz_x;
   gpgszy = gpgnpy*gwinpt->geo.psiz_y;
/*
***Huvudf�nstrets DC �r en global variabel som anv�nds i grapac.
*/
   ms_dc   = gwinpt->dc;
   ms_bmdc = gwinpt->bmdc;
/*
***Kanske skall vi ocks� skapa default vy. Genom att f�rst s�tta
***aktiv vy:s modellf�nster = Huvudf�nstrets ser vi till att vyn
***"xy" f�r samma modellf�nster som GWIN_MAIN. excrvp() kopierar
***n�mligen sitt modellf�nster fr�n aktiv vy.
*/
   if ( create )
     {
     actvy.vywin[0] = gwinpt->vy.modwin.xmin;
     actvy.vywin[1] = gwinpt->vy.modwin.ymin;
     actvy.vywin[2] = gwinpt->vy.modwin.xmax;
     actvy.vywin[3] = gwinpt->vy.modwin.ymax;
     bpos.x_vy = 0.0;
     bpos.y_vy = 0.0;
     bpos.z_vy = 1.0;
     EXcrvp("xy",&bpos);
     msacvi("xy",GWIN_MAIN);
     }
   else msacvi(avynam,GWIN_MAIN);

   return(0);
  }

/********************************************************/
/*!******************************************************/

        int msclrg()

/*      D�dar alla grafiska f�nster och nollst�ller
 *      det som har med dem att g�ra.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 3/11/95 J. Kjellander
 *
 ******************************************************!*/

  {
   int     i;
   WPWIN  *winptr;

/*
***D�da alla grafiska f�nster.
*/
   for ( i=0; i<WTABSIZ; ++i )
     if ( (winptr=mswgwp((wpw_id)i)) != NULL  &&
           winptr->typ == TYP_GWIN ) mswdel((v2int)i);
/*
***Slut.
*/
   return(0);
  }

/********************************************************/
/*!******************************************************/

        short mssvjb(f)
        FILE *f;

/*      Lagrar grafiska f�nster i jobfilen.
 *
 *      In: f => Pekare till �ppen jobbfil.
 *
 *      Ut: Inget.
 *
 *      Felkod: WP1422 = Fel vid skrivning till jobfil.
 *
 *      (C)microform ab 5/12/95 J. Kjellander
 *
 *      1996-12-15 Placering och storlek, J.Kjellander
 *      1997-03-20 Bug x,y,dx,dy, J.Kjellander
 *      1997-09-24 wpl.length, J.Kjellander
 *      1998-04-09 �ven ms_main, J.Kjellander
 *
 ******************************************************!*/

 {
   short           x,y,dx,dy;
   int             i,winant;
   bool            max;
   WPWIN          *winptr;
   WPGWIN         *gwinpt;
   RECT            rect;
   WINDOWPLACEMENT wpl;
/*
***F�rst skriver vi ut ms_main:s storlek.
*/
   GetWindowRect(ms_main,&rect);
   dx = (short)(rect.right - rect.left);
   dy = (short)(rect.bottom - rect.top);
   if ( fwrite((char *)&dx,sizeof(short),1,f) == 0 ) goto error;
   if ( fwrite((char *)&dy,sizeof(short),1,f) == 0 ) goto error;
   wpl.length = sizeof(WINDOWPLACEMENT);
   GetWindowPlacement(ms_main,&wpl);
   if ( (wpl.showCmd & SW_SHOWMAXIMIZED) == SW_SHOWMAXIMIZED )
     max = TRUE;
   else
     max = FALSE;
   if ( fwrite((char *)&max,sizeof(bool),1,f) == 0 ) goto error;
/*
***Hur m�nga WPGWIN har vi ?
*/
   winant = msngws();
/*
***Skriv ut antal f�nster.
*/
   if ( fwrite((char *)&winant,sizeof(int),1,f) == 0 ) goto error;
/*
***F�r varje f�nster, skriv ut placering, storlek, vynamn, modell-
***f�nster och niv�er.
*/
   for ( i=0; i<WTABSIZ; ++i )
     {
     if ( (winptr=mswgwp((wpw_id)i)) != NULL  &&
           winptr->typ == TYP_GWIN )
       {
       gwinpt = (WPGWIN *)winptr->ptr;
	   wpl.length = sizeof(WINDOWPLACEMENT);
       GetWindowPlacement(gwinpt->id.ms_id,&wpl);
       if ( (wpl.showCmd & SW_SHOWMAXIMIZED) == SW_SHOWMAXIMIZED )
         {
         x = -2;
         y = 2;
         }
       else
         {
         x = (short)wpl.rcNormalPosition.left;
         y = (short)wpl.rcNormalPosition.top;
         }

       GetWindowRect(gwinpt->id.ms_id,&rect);
       dx = (short)(rect.right - rect.left);
       dy = (short)(rect.bottom - rect.top);

       if ( fwrite((char *)&x,sizeof(short),1,f) == 0 )
         goto error;
       if ( fwrite((char *)&y,sizeof(short),1,f) == 0 )
         goto error;
    
       if ( fwrite((char *)&dx,sizeof(short),1,f) == 0 )
         goto error;
       if ( fwrite((char *)&dy,sizeof(short),1,f) == 0 )
         goto error;
       if ( fwrite((char *)&gwinpt->vy.vynamn,sizeof(char),GPVNLN+1,f) == 0 )
         goto error;
       if ( fwrite((char *)&gwinpt->vy.modwin,sizeof(VYWIN),1,f) == 0 )
         goto error;
       if ( fwrite((char *)&gwinpt->nivtab,sizeof(char),WP_NTSIZ,f) == 0 )
         goto error;
       }
     }
/*
***Slut.
*/
   return(0);
/*
***Felutg�ng.
*/
error:
   return(erpush("WP1422",""));
 }

/********************************************************/
/*!******************************************************/

        short   msldjb(f,psyd)
        FILE   *f;
        V3MDAT *psyd;

/*      Laddar grafiska f�nster fr�n jobfilen.
 *
 *      In: f    = Pekare till �ppen jobbfil.
 *          psyd = Pekare till systemdata
 *
 *      Ut: Inget.
 *
 *      Felkoder: WP1432 = Kan ej l�sa jobfil.
 *                WP1242 = Kan ej skapa f�nster
 *
 *      (C)microform ab 5/12/95 J. Kjellander
 *
 *      1996-12-15 Placering och storlek, J.Kjellander
 *      1998-04-09 ms_main:s storlek, J.Kjellander
 *
 ******************************************************!*/

  {
   int       i,winant,mx,my,mdx,mdy,ymax;
   short     status,x,y,dx,dy;
   bool      max;
   v2int     w_id;
   WPGWIN   *gwinpt,tmpgwi;
   RECT      rect;
   HDC       dc;

/*
***Fr.o.m. version 1.15D b�rjar det med ms_main:s storlek.
*/
   if ( psyd->revnr > 15 || (psyd->revnr == 15 && psyd->level > 'C') )
     {
     if ( fread(&dx,sizeof(short),1,f) == 0 ) goto error;
     if ( fread(&dy,sizeof(short),1,f) == 0 ) goto error;
     if ( fread(&max,sizeof(bool),1,f) == 0 ) goto error;
     GetWindowRect(ms_main,&rect);
     if ( dx != (short)(rect.right - rect.left)  ||
          dy != (short)(rect.bottom - rect.top) )
       {
       if ( max )
         {
         ShowWindow(ms_main,SW_MAXIMIZE);
         }
       else
         { 
         dc = GetDC(ms_main);
         if ( rect.left+dx > GetDeviceCaps(dc,HORZRES) )
           rect.left = 1;
         if ( rect.left+dx > GetDeviceCaps(dc,HORZRES) )
           dx = GetDeviceCaps(dc,HORZRES)-2;
         if ( rect.top+dy > GetDeviceCaps(dc,VERTRES) )
           rect.top = 1;
         if ( rect.left+dy > GetDeviceCaps(dc,VERTRES) )
           dy = GetDeviceCaps(dc,VERTRES)-2;
         SetWindowPos(ms_main,NULL,rect.left,rect.top,
                     (int)dx,(int)dy,SWP_NOZORDER);
         }
       }
     }
/*
***Hur m�nga grafiska f�nster finns det ?
*/
   if ( fread(&winant,sizeof(int),1,f) == 0 ) goto error;
/*
***Om winant > 0 b�rjar vi med huvudf�nstret.
*/
   if ( winant > 0 )
     {
     if ( fread(&x,sizeof(short),1,f) == 0 ) goto error;
     if ( fread(&y,sizeof(short),1,f) == 0 ) goto error;
     if ( fread(&dx,sizeof(short),1,f) == 0 ) goto error;
     if ( fread(&dy,sizeof(short),1,f) == 0 ) goto error;
/*
***Kolla att det f�r plats i applikationsf�nstret utan
***att �verlappa promtraden.
*/
     msggeo(ms_main,&mx,&my,&mdx,&mdy,NULL,NULL);

     ymax = mdy - (1 + 15 + 1 + 2 + 2);

     if ( y > ymax ) y = 2;

     if ( y+dy > ymax )
       {
       y = ymax - dy;
       if ( y < 2 )
         {
         y = 2;
         dy = ymax - 2;
         }
       }
/*
***�vriga data.
*/
     if ( fread(&tmpgwi.vy.vynamn,sizeof(char),GPVNLN+1,f) == 0 ) goto error;
     if ( fread(&tmpgwi.vy.modwin,sizeof(VYWIN),1,f) == 0 ) goto error;
     if ( fread(&tmpgwi.nivtab,sizeof(char),WP_NTSIZ,f) == 0 ) goto error;
/*
***Skapa sj�lva huvudf�nstret.
*/
     status = mswcgw((int)x,(int)y,(int)dx,(int)dy,"",TRUE,&w_id);
     if ( status < 0 ) return(erpush("WP1242","WIN95-Display"));
/*
***Fixa en C-pekare till det nu skapade f�nstret.
*/
     gwinpt  = (WPGWIN *)wpwtab[(wpw_id)w_id].ptr;
/*
***Underr�tta grapac om resultatet.
*/
     gpsnpx = gwinpt->geo.dx;
     gpsnpy = gwinpt->geo.dy;
     gpgorx = (short)gwinpt->vy.scrwin.xmin;
     gpgory = (short)gwinpt->vy.scrwin.ymin;
     gpgnpx = (short)(gwinpt->vy.scrwin.xmax - gwinpt->vy.scrwin.xmin);
     gpgnpy = (short)(gwinpt->vy.scrwin.ymax - gwinpt->vy.scrwin.ymin);
     gpgszx = gpgnpx*gwinpt->geo.psiz_x;
     gpgszy = gpgnpy*gwinpt->geo.psiz_y;
/*
***Huvudf�nstrets DC �r en global variabel som anv�nds i grapac.
*/
     ms_dc   = gwinpt->dc;
     ms_bmdc = gwinpt->bmdc;
/*
***Aktivera f�nstrets vy. Detta g�r vi genom att f�rst aktivera
***den version av vyn som finns i vytab och sedan �ndra modell-
***f�nstret s� att det st�mmer med jobfilen. Normalisering beh�vs
***f�r att ber�kna nya 2D-konstanter och f�r att kompensera f�r
***olika storlek p� pixels i X- och Y-led p� olika sk�rmar. Slutligen
***aktiverar vi vyn igen varvid s�v�l vytab som actvy f�r r�tt
***utseende.
*/
     msacvi(tmpgwi.vy.vynamn,GWIN_MAIN);
     V3MOME(&tmpgwi.vy.modwin,&gwinpt->vy.modwin,sizeof(VYWIN));
     msnrgw(gwinpt);
     msacvi(tmpgwi.vy.vynamn,GWIN_MAIN);
/*
***Kopiera den l�sta niv�tabellen till det skapade f�nstret.
*/
     V3MOME(&tmpgwi.nivtab,&gwinpt->nivtab,WP_NTSIZ);
     }
/*
***Eventuella ytterligare f�nster.
*/
   for ( i=1; i<winant; ++i )
     {
     if ( fread(&x,               sizeof(short),      1,f) == 0 ) goto error;
     if ( fread(&y,               sizeof(short),      1,f) == 0 ) goto error;
     if ( fread(&dx,              sizeof(short),      1,f) == 0 ) goto error;
     if ( fread(&dy,              sizeof(short),      1,f) == 0 ) goto error;
     if ( fread(&tmpgwi.vy.vynamn,sizeof(char),GPVNLN+1,f) == 0 ) goto error;
     if ( fread(&tmpgwi.vy.modwin,sizeof(VYWIN),      1,f) == 0 ) goto error;
     if ( fread(&tmpgwi.nivtab,   sizeof(char),WP_NTSIZ,f) == 0 ) goto error;

     status = mswcgw((int)x,(int)y,(int)dx,(int)dy,"",FALSE,&w_id);
     if ( status < 0 ) return(erpush("WP1242","Windows 32"));

     gwinpt = (WPGWIN *)wpwtab[(wpw_id)w_id].ptr;
     msacvi(tmpgwi.vy.vynamn,w_id);
     V3MOME(&tmpgwi.vy.modwin,&gwinpt->vy.modwin,sizeof(VYWIN));
     msnrgw(gwinpt);
     V3MOME(&tmpgwi.nivtab,&gwinpt->nivtab,WP_NTSIZ);
     }
/*
***Applikationsf�nstrets storlek.
*/

/*
***Slut.
*/
    return(0);
/*
***Felutg�ng.
*/
error:
   return(erpush("WP1432",""));
  }

/********************************************************/
/*!******************************************************/

        int msmbox(rubrik,text,typ)
        char *rubrik;
        char *text;
        int   typ;


/*      Skickar ut en meddelandebox. Meddelandeboxen �r
 *      helt frist�ende fr�n MSPAC och kan visas utan
 *      att MSPAC �r initierat, tex. unde uppstart.
 *
 *      In: 
 *          rubrik = Text i ramen.
 *          text   = Text i boxen.
 *          typ    = Typ av box.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 1/11/95 J. Kjellander
 *
 ******************************************************!*/

  {
#ifdef DEBUG
    if ( dbglev(MSPAC) == 1 )
      {
      fprintf(dbgfil(MSPAC),"***Start-msmbox***\n");
      fprintf(dbgfil(MSPAC),"rubrik=%s\n",rubrik);
      fprintf(dbgfil(MSPAC),"text=%s\n",text);
      fprintf(dbgfil(MSPAC),"typ=%d\n",typ);
      fflush(dbgfil(MSPAC));
      }
#endif


   MessageBox(NULL,text,rubrik,MB_ICONSTOP | MB_OK);


#ifdef DEBUG
    if ( dbglev(MSPAC) == 1 )
      {
      fprintf(dbgfil(MSPAC),"***Slut-msmbox***\n\n");
      fflush(dbgfil(MSPAC));
      }
#endif
   return(0);
  }

/********************************************************/
/*!******************************************************/

        int msmwsx()

/*      Returnerar huvudf�nstrets applikationsareas 
 *      storlek i X-led.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 24/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
  int  dx;
  RECT rect;

/*
***Ev. DEBUG.
*/
#ifdef DEBUG
    if ( dbglev(MSPAC) == 15)
      {
      fprintf(dbgfil(MSPAC),"***Start-msmwsx***\n");
      fflush(dbgfil(MSPAC));
      }
#endif

/*
***Tar reda p� huvudf�nstrets storlek.
*/
   GetWindowRect(ms_main,&rect);

   dx = rect.right - rect.left;
/*
***Slut.
*/
#ifdef DEBUG
    if ( dbglev(MSPAC) == 15 )
      {
      fprintf(dbgfil(MSPAC),"dx=%d\n",dx);
      fprintf(dbgfil(MSPAC),"***Slut-msmwsx***\n\n");
      fflush(dbgfil(MSPAC));
      }
#endif

    return(dx);
  }

/********************************************************/
/*!******************************************************/

        int msmwsy()

/*      Returnerar huvudf�nstrets applikationsareas 
 *      storlek i Y-led.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 24/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
  int  dy;
  RECT rect;

/*
***Tar reda p� huvudf�nstrets storlek.
*
   GetWindowRect(ms_main,&rect);
*/
  GetClientRect(ms_main,&rect);
  dy = rect.bottom - rect.top;

  return(dy);
  }

/********************************************************/
/*!******************************************************/

        int     msggeo(win32_id,px,py,pxp,pyp,pxm,pym)
        HWND    win32_id;
        int    *px,*py,*pxp,*pyp;
        double *pxm,*pym;

/*      Returnerar f�nstrets nuvarande l�ge och storlek i antal
 *      pixels och i mm.
 *
 *      In: win32_id = MS f�nster-ID.
 *          px       = Pekare till l�ge i X-led.
 *          py       = Pekare till l�ge i Y-led.
 *          pxp      = Pekare till antal pixels i X-led.
 *          pyp      = Pekare till antal pixels i Y-led.
 *          pxm      = Pekare till antal millimeter i X-led.
 *          pym      = Pekare till antal millimeter i Y-led.
 *
 *      Ut: *px  = L�ge i X-led.
 *          *py  = L�ge i Y-led.
 *          *pxp = Antal pixels i X-led.
 *          *pyp = Antal pixels i Y-led.
 *          *pxm = Antal millimeter i X-led.
 *          *pym = Antal millimeter i Y-led.
 *
 *      FV:  0.
 *
 *      (C)microform ab 28/10/95 J. Kjellander
 *
 *      1997-09-24 wpl.length, J.Kjellander
 *
 ******************************************************!*/

  {
  int             dx,dy;
  double          dx_mm,dy_mm;
  HDC             dc;
  RECT            rect;
  WINDOWPLACEMENT wpl;
 
/*
***F�nstrets placering p� sk�rmen.
*/
   wpl.length = sizeof(WINDOWPLACEMENT);
   GetWindowPlacement(win32_id,&wpl);

  *px = wpl.rcNormalPosition.left;
  *py = wpl.rcNormalPosition.top;
/*
***Klientareans storlek i pixels. Under uppstart finns ingen menybalk
***och GetClientRect() returnerar d�rf�r ett f�r stort v�rde i Y-led.
***Om s� �r fallet kompenserar vi med detta h�r.
*/
   GetClientRect(win32_id,&rect);
  *pxp = rect.right  - rect.left;
  *pyp = rect.bottom - rect.top;
/*
***Om det g�ller huvudf�nstret och vi �nnu inte f�tt ig�ng menyraden
***kompenserar vi f�r det.
*/
   if ( win32_id == ms_main  &&  GetMenu(ms_main) == NULL )
     *pyp -= GetSystemMetrics(SM_CYMENU);
/*
***Sk�rmens storlek i pixels.
*/
   dc = GetDC(win32_id);
   dx = GetDeviceCaps(dc,HORZRES);
   dy = GetDeviceCaps(dc,VERTRES);
/*
***Vad blir det i millimeter ?
*/
   dx_mm = (double)GetDeviceCaps(dc,HORZSIZE);
   dy_mm = (double)GetDeviceCaps(dc,VERTSIZE);
   /* GetDeviceCaps ger fel storlek !!! */
   dx_mm = 320;
   dy_mm = 240;
   ReleaseDC(win32_id,dc);

   if ( pxm != NULL ) *pxm = *pxp/(double)dx*dx_mm;
   if ( pym != NULL ) *pym = *pyp/(double)dy*dy_mm;
/*
***Slut.
*/
    return(0);
  }

/********************************************************/
/*!******************************************************/

        int msngws()

/*      Talar om hur m�nga grafiska f�nster som finns f�r
 *      tillf�llet.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: Antal grafiska f�nster.
 *
 *      (C)microform ab 8/11/95 J. Kjellander
 *
 ******************************************************!*/

 {
   int     i,winant;
   WPWIN  *winptr;

/*
***Initiering.
*/
   winant = 0;
/*
***Hur m�nga WPGWIN har vi ?
*/
   for ( i=0; i<WTABSIZ; ++i )
     if ( (winptr=mswgwp((wpw_id)i)) != NULL  &&
           winptr->typ == TYP_GWIN ) winant++;

   return(winant);
  }

/********************************************************/
/*!******************************************************/

        int   msprtf(rubrik,filnam)
        char *rubrik;
        char *filnam;

/*      Skriver ut en textfil p� skrivaren.
 *
 *      In: rubrik = Printjobbets namn.
 *          filnam = Filnamn med v�gbeskrivning.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 23/11/95 J. Kjellander
 *
 *      1997-11-17 VC++ 5.0, J.Kjellander
 *
 ******************************************************!*/

  {
   int        i,ntkn,x,y,dy;
   char       buf[256];
   TEXTMETRIC tm;
   FILE      *fp;
   DOCINFO    di;

   static PRINTDLG pd;

/*
***Initiera Printdata.
*/
   pd.lStructSize         = sizeof(PRINTDLG);
   pd.hwndOwner           = ms_main;
   pd.hDevMode            = NULL;
   pd.hDevNames           = NULL;
   pd.nFromPage           = 1;
   pd.nToPage             = 1;
   pd.nMinPage            = 0;
   pd.nMaxPage            = 0;
   pd.nCopies             = 1;
   pd.lpfnSetupHook       = NULL;
   pd.lpSetupTemplateName = NULL;
   pd.lpfnPrintHook       = NULL;
   pd.lpPrintTemplateName = NULL;
   pd.lCustData           = NULL;
   pd.Flags               = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION;
/*
***Anropa PrintDlg och erh�ll p� s� vis ett DC.
*/
   if ( PrintDlg(&pd) )
     {
/*
***Hur stor �r default font i skrivaren, dvs. hur stort radspr�ng
***skall vi anv�nda och hur m�nga rader p� en sida ?
*/
     GetTextMetrics(pd.hDC,&tm);
     dy = GetDeviceCaps(pd.hDC,VERTRES) - tm.tmHeight;
/*
***Initiera skrivaren.
*/
     di.cbSize      = sizeof(DOCINFO);
     di.lpszDocName = rubrik;
     di.lpszOutput  = NULL;
     di.fwType      = 0;   /* 1997-11-17 JK*/

     StartDoc(pd.hDC,&di);

     for ( i=0; i<pd.nCopies; ++i )
       {
/*
***�ppna filen som skall skrivas ut.
*/
       if ( (fp=fopen(filnam,"r")) == NULL )
         {
         MessageBox(ms_main,filnam,
                   "Systemfel, kan ej �ppna fil",MB_OK | MB_ICONSTOP);
         EndDoc(pd.hDC);
         DeleteDC(pd.hDC);
         return(0);
         }
/*
***Ny sida.
*/
       StartPage(pd.hDC);
/*
***L�s en rad i taget fr�n filen som skall skrivas ut.
***fgets() strippar inte \newline s� det f�r
***vi g�ra.
*/
       x = (int)(0.2*GetDeviceCaps(pd.hDC,LOGPIXELSX));       /* V�nster marg. = 0.2 tum */
       y = tm.tmHeight;

       while ( fgets(buf,255,fp) != NULL )
         {
         buf[255] = '\0';
         ntkn = strlen(buf);
         if ( ntkn > 0 )
           {
           buf[ntkn-1] ='\0';
           --ntkn;
           if ( ntkn > 0 ) TextOut(pd.hDC,x,y,buf,ntkn);
           }

         y += tm.tmHeight;
/*
***Ny sida.
*/
         if ( y > dy )
           {
           EndPage(pd.hDC);
           StartPage(pd.hDC);
           y = tm.tmHeight;
           }
         }
/*
***Avsluta.
*/
       fclose(fp);
       EndPage(pd.hDC);
       }
     EndDoc(pd.hDC);
     DeleteDC(pd.hDC);
     }

   return(0);
  }

/********************************************************/
/*!******************************************************/

        int    msgtwi(id,px,py,pdx,pdy,ptyp,rubrik)
        v2int  id;
        int   *px,*py,*pdx,*pdy,*ptyp;
        char  *rubrik;

/*      Returnerar div. data om ett f�nster.
 *
 *      In: id      = F�nster-ID.
 *
 *      Ut: *px     = L�ge i X-led.
 *          *py     = L�ge i Y-led.
 *          *pdx    = Storlek i X-led.
 *          *pdy    = Storlek i Y-led.
 *          *ptyp   = 0=Inmatningsf�nster, 1=Grafiskt f�nster.
 *          *rubrik = F�nstertitel.
 *
 *      Felkoder:
 *                WP1562 = F�nster %s finns ej
 *                WP1572 = F�nstret ej IWIN/GWIN.
 *
 *      (C)microform ab 1996-05-21 J. Kjellander
 *
 ******************************************************!*/

  {
    char    errbuf[80];
    HWND    ms_id;
    HDC     dc;
    WPWIN  *winptr;
    WPIWIN *iwinpt;
    WPGWIN *gwinpt;

/*
***�r det m�jligen hela sk�rmen som avses ?
*/
    if ( id == -1 )
      {
     *px   = *py = 0;

      dc = GetDC(ms_main);
     *pdx = GetDeviceCaps(dc,HORZRES);
     *pdy = GetDeviceCaps(dc,VERTRES);
      ReleaseDC(ms_main,dc);

     *ptyp = -1;
      strcpy(rubrik,"WIN32");
      return(0);
      }
/*
***Fixa C-pekare till f�nstret.
*/
    if ( (winptr=mswgwp(id)) == NULL )
      {
      sprintf(errbuf,"%d",(int)id);
      return(erpush("WP1562",errbuf));
      }

    switch ( winptr->typ )
      {
      case TYP_IWIN:
      iwinpt = (WPIWIN *)winptr->ptr;
      ms_id  = iwinpt->id.ms_id;
     *ptyp   = 0;
      break;

      case TYP_GWIN:
      gwinpt = (WPGWIN *)winptr->ptr;
      ms_id   = gwinpt->id.ms_id;
     *ptyp   = 1;
      break;

      default:
      sprintf(errbuf,"%d",(int)id);
      return(erpush("WP1572",errbuf));
      }
/*
***Rubrik.
*/
   GetWindowText(ms_id,rubrik,V3STRLEN);
/*
***Placering och storlek.
*/
    msggeo(ms_id,px,py,pdx,pdy,NULL,NULL);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        char *msmest(UINT message)

/*      �vers�tter en messagecode till klartext.
 *
 *      In: message = mesaage code.
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 26/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
static char buf[80];

   switch ( message )
     {
     case WM_NULL:               return("WM_NULL");
     case WM_CREATE:             return("WM_CREATE");
     case WM_DESTROY:            return("WM_DESTROY");
     case WM_MOVE:               return("WM_MOVE");
     case WM_SIZE:               return("WM_SIZE");
     case WM_ACTIVATE:           return("WM_ACTIVATE");  
     case WM_SETFOCUS:           return("WM_SETFOCUS");
     case WM_KILLFOCUS:          return("WM_KILLFOCUS");
     case WM_ENABLE:             return("WM_ENABLE");
     case WM_SETREDRAW:          return("WM_SETREDRAW");
     case WM_SETTEXT:            return("WM_SETTEXT");
     case WM_GETTEXT:            return("WM_GETTEXT");
     case WM_GETTEXTLENGTH:      return("WM_GETTEXTLENGTH");
     case WM_PAINT:              return("WM_PAINT");
     case WM_CLOSE:              return("WM_CLOSE");
     case WM_QUERYENDSESSION:    return("WM_QUERYENDSESSION");
     case WM_QUIT:               return("WM_QUIT");
     case WM_QUERYOPEN:          return("WM_QUERYOPEN");
     case WM_ERASEBKGND:         return("WM_ERASEBKGND");
     case WM_SYSCOLORCHANGE:     return("WM_SYSCOLORCHANGE");
     case WM_ENDSESSION:         return("WM_ENDSESSION");
     case WM_SHOWWINDOW:         return("WM_SHOWWINDOW");
     case WM_WININICHANGE:       return("WM_WININICHANGE");
     case WM_ACTIVATEAPP:        return("WM_ACTIVATEAPP");
     case WM_FONTCHANGE:         return("WM_FONTCHANGE");
     case WM_TIMECHANGE:         return("WM_TIMECHANGE");
     case WM_CANCELMODE:         return("WM_CANCELMODE");
     case WM_SETCURSOR:          return("WM_SETCURSOR");
     case WM_MOUSEACTIVATE:      return("WM_MOUSEACTIVATE");
     case WM_CHILDACTIVATE:      return("WM_CHILDACTIVATE");
     case WM_QUEUESYNC:          return("WM_QUEUESYNC");
     case WM_GETMINMAXINFO:      return("WM_GETMINMAXINFO");
     case WM_PAINTICON:          return("WM_PAINTICON");
     case WM_ICONERASEBKGND:     return("WM_ICONERASEBKGND");
     case WM_NEXTDLGCTL:         return("WM_NEXTDLGCTL");
     case WM_SPOOLERSTATUS:      return("WM_SPOOLERSTATUS");
     case WM_DRAWITEM:           return("WM_DRAWITEM");
     case WM_MEASUREITEM:        return("WM_MEASUREITEM");
     case WM_VKEYTOITEM:         return("WM_KEYTOITEM");
     case WM_CHARTOITEM:         return("WM_CHARTOITEM");
     case WM_SETFONT:            return("WM_SETFONT");
     case WM_GETFONT:            return("WM_GETFONT");
     case WM_SETHOTKEY:          return("WM_SETHOTKEY");
     case WM_GETHOTKEY:          return("WM_GETHOTKEY");
     case WM_QUERYDRAGICON:      return("WM_QUERYDRAGICON");
     case WM_COMPAREITEM:        return("WM_COMPAREITEM");
     case WM_COMPACTING:         return("WM_COMPACTING");   
     case WM_WINDOWPOSCHANGING:  return("WM_WINDOWPOSCHANGING");
     case WM_WINDOWPOSCHANGED:   return("WM_WINDOWPOSCHANGED");
     case WM_POWER:              return("WM_POWER");
     case WM_COPYDATA:           return("WM_COPYDATA");
     case WM_CANCELJOURNAL:      return("WM_CANCELJOURNAL");
     case WM_NCCREATE:           return("WM_NCCREATE");
     case WM_NCDESTROY:          return("WM_NCDESTROY");
     case WM_NCCALCSIZE:         return("WM_NCCALCSIZE");
     case WM_NCHITTEST:          return("WM_NCHITTEST");
     case WM_NCPAINT:            return("WM_NCPAINT");
     case WM_NCACTIVATE:         return("WM_NCACTIVATE");
     case WM_GETDLGCODE:         return("WM_GETDIALOGCODE");
     case WM_NCMOUSEMOVE:        return("WM_NCMOUSEMOVE");
     case WM_NCLBUTTONDOWN:      return("WM_NCLBUTTONDOWN");
     case WM_NCLBUTTONUP:        return("WM_NCLBUTTONUP");
     case WM_NCLBUTTONDBLCLK:    return("WM_NCLBUTTONDBLCLK");
     case WM_NCRBUTTONDOWN:      return("WM_NCRBUTTONDOWN");
     case WM_NCRBUTTONUP:        return("WM_NCRBUTTONUP");
     case WM_NCRBUTTONDBLCLK:    return("WM_NCRBUTTONDBLCLK");
     case WM_NCMBUTTONDOWN:      return("WM_NCMBUTTONDOWN");
     case WM_NCMBUTTONUP:        return("WM_NCMBUTTONUP");
     case WM_NCMBUTTONDBLCLK:    return("WM_NCMBUTTONDBLCLK");
     case WM_KEYDOWN:            return("WM_KEYDOWN");
     case WM_KEYUP:              return("WM_KEYUP");
     case WM_CHAR:               return("WM_CHAR");
     case WM_DEADCHAR:           return("WM_DEADCHAR");
     case WM_SYSKEYDOWN:         return("WM_SYSKEYDOWN");
     case WM_SYSKEYUP:           return("WM_SYSKEYUP");
     case WM_SYSCHAR:            return("WM_SYSCHAR");
     case WM_SYSDEADCHAR:        return("WM_SYSDEADCHAR");
     case WM_KEYLAST:            return("WM_KEYLAST");
     case WM_INITDIALOG:         return("WM_INITDIALOG");
     case WM_COMMAND:            return("WM_COMMAND");
     case WM_SYSCOMMAND:         return("WM_SYSCOMMAND");
     case WM_TIMER:              return("WM_TIMER");
     case WM_HSCROLL:            return("WM_HSCROLL");
     case WM_VSCROLL:            return("WM_VSCROLL");
     case WM_INITMENU:           return("WM_INITMENU");
     case WM_INITMENUPOPUP:      return("WM_INITMENUPOPUP");
     case WM_MENUSELECT:         return("WM_MENUSELECT");
     case WM_MENUCHAR:           return("WM_MENUCHAR");
     case WM_ENTERIDLE:          return("WM_ENTERIDLE");
     case WM_CTLCOLORMSGBOX:     return("WM_CTLCOLORMSGBOX");
     case WM_CTLCOLOREDIT:       return("WM_CTLCOLOREDIT");
     case WM_CTLCOLORLISTBOX:    return("WM_CTLCOLORLISTBOX");
     case WM_CTLCOLORBTN:        return("WM_CTLCOLORBTN");
     case WM_CTLCOLORDLG:        return("WM_CTLCOLORDLG");
     case WM_CTLCOLORSCROLLBAR:  return("WM_CTLCOLORSCROLLBAR");
     case WM_CTLCOLORSTATIC:     return("WM_CTLCOLORSTATIC");
     case WM_MOUSEMOVE:          return("WM_MOUSEMOVE");    
     case WM_LBUTTONDOWN:        return("WM_LBUTTONDOWN");
     case WM_LBUTTONUP:          return("WM_LBUTTONUP");
     case WM_LBUTTONDBLCLK:      return("WM_LBUTTONDBLCLK");
     case WM_RBUTTONDOWN:        return("WM_RBUTTONDOWN");
     case WM_RBUTTONUP:          return("WM_RBUTTONUP");
     case WM_RBUTTONDBLCLK:      return("WM_RBUTTONDBLCLK");
     case WM_MBUTTONDOWN:        return("WM_MBUTTONDOWN");
     case WM_MBUTTONUP:          return("WM_MBUTTONUP");
     case WM_MBUTTONDBLCLK:      return("WM_MBUTTONDBLCLK");  
     case WM_PARENTNOTIFY:       return("WM_PARENTNOTIFY");
     case WM_ENTERMENULOOP:      return("WM_ENTERMENULOOP");
     case WM_EXITMENULOOP:       return("WM_EXITMENULOOP");
     case WM_MDICREATE:          return("WM_MDICREATE");
     case WM_MDIDESTROY:         return("WM_MDIDESTROY");
     case WM_MDIACTIVATE:        return("WM_MDIACTIVATE");
     case WM_MDIRESTORE:         return("WM_MDIRESTORE");
     case WM_MDINEXT:            return("WM_MDINEXT");
     case WM_MDIMAXIMIZE:        return("WM_MDIMAXIMIZE");
     case WM_MDITILE:            return("WM_MDITILE");
     case WM_MDIICONARRANGE:     return("WM_MDIICONARRANGE");
     case WM_MDIGETACTIVE:       return("WM_MDIGETACTIVE");
     case WM_MDISETMENU:         return("WM_MDISETMENU");
     case WM_DROPFILES:          return("WM_DROPFILES");
     case WM_MDIREFRESHMENU:     return("WM_MDIREFRESHMENU");
     case WM_CUT:                return("WM_CUT");
     case WM_COPY:               return("WM_COPY");
     case WM_PASTE:              return("WM_PASTE");
     case WM_CLEAR:              return("WM_CLEAR");
     case WM_UNDO:               return("WM_UNDO");
     case WM_RENDERFORMAT:       return("WM_RENDERFORMAT");
     case WM_DESTROYCLIPBOARD:   return("WM_DESTROYCLIPBOARD");
     case WM_DRAWCLIPBOARD:      return("WM_DRAWCLIPBOARD");
     case WM_PAINTCLIPBOARD:     return("WM_PAINTCLIPBOARD");
     case WM_VSCROLLCLIPBOARD:   return("WM_VSCROLLCLIPBOARD");
     case WM_SIZECLIPBOARD:      return("WM_VSIZECLIPBOARD");
     case WM_ASKCBFORMATNAME:    return("WM_VASKCBFORMATNAME");
     case WM_CHANGECBCHAIN:      return("WM_CHANGECBCHAIN");
     case WM_HSCROLLCLIPBOARD:   return("WM_HSCROLLCLIPBOARD");
     case WM_QUERYNEWPALETTE:    return("WM_QUERYNEWPALETTE");
     case WM_PALETTEISCHANGING:  return("WM_PALETTEISCHANGING");
     case WM_PALETTECHANGED:     return("WM_PALETTECHANGED");
     case WM_HOTKEY:             return("WM_HOTKEY");
     case WM_PENWINFIRST:        return("WM_PENWINFIRST");
     case WM_PENWINLAST:         return("WM_PENWINLAST");

     default:
     sprintf(buf,"Ok�nt message, kod = %d",message);
     return(buf);
     }
     
  }

/********************************************************/
