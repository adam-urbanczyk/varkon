/**********************************************************************
*
*    wpinit.c
*    ========
*
*    This file is part of the VARKON WindowPac Library.
*    URL: http://www.tech.oru.se/cad/varkon
*
*    This file includes:
*
*    WPinit();   Init WP
*    WPexit();   Close WP
*    WPclrg();   Close graphical windows
*    WPsvjb();   Write graphical window data to JOB-file
*    WPldjb();   Read graphical window data from jobfile
*    WPngws();   Return current number of graphical windows
*    WPcgws();   Create graphical windows
*    WPmrdb();   Merge X resource databases
*    WPgtwi();   Return window data
*    WPgtwp();   Return window position 
*    WPgwsz();   Return window size
*    WPsdpr();   Set window PROTOCOL
*    WPgrst();   Returns X resource value
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
***********************************************************************/

#include "../../DB/include/DB.h"
#include "../../IG/include/IG.h"
#include "../../EX/include/EX.h"
#include "../include/WP.h"
#include <string.h>


Window       xgwin;     /* F�r grapac */
Pixmap       xgmap;     /* F�r grapac */
GC           gpxgc;     /* F�r grapac */

Display     *xdisp;
GC           xgc;
XPoint       xbuf[PLYMXV];
int          xscr;
MNUDAT      *actmeny;
Cursor       xgcur1,xgcur2,xwcur,xtcur;
XrmDatabase  xresDB;

/* xgwin �r window ID f�r det grafiska huvud-f�nstret.
   xdisp �r display ID.
   xgc �r Graphic Context ID.
   xbuf �r grapac:s polyline-buffert
   xscr �r Screen,
   actmeny �r en pekare till aktiv meny. Om meny saknas �r actmeny = NULL.
   xgcur �r den cursor som aktiveras vid h�rkors-pekning.
   xtcur �r I-balk cursorn vid textinmatning
   xwcur �r v�nta-klockan
   xresDB �r den resursdatabas som skapas vid uppstart.
*/

extern short  gpsnpx,gpsnpy,gpgorx,gpgory,gpgnpx,gpgnpy;
extern double gpgszx,gpgszy;
extern VY     actvy;

/*!******************************************************/

        short WPinit(
        char *inifil_1,
        char *inifil_2)

/*      �ppnar winpac. F�ruts�tter att resursdatabasen
 *      �r skapad av andra rutiner tidigare.
 *
 *      In: inifil_1 = Ev. ytterligare resursfil.
 *          inifil_2 = Ev. ytterligare resursfil.
 *
 *      Ut: Inget.
 *
 *      Felkod: WP1042 = Kan ej �ppna dislay.
 *              WP1032 = Kan ej initiera fonter p� display %s
 *              WP1252 = Kan ej skapa meny-f�nster p� %s
 *              WP1442 = Hittar ej resursfilen %s
 *
 *      (C)microform ab 23/6/92 U.Andersson
 *
 *      24/1/94  Omarbetad, J. Kjellander
 *      7/11/94  Mer resurser, J. Kjellander
 *      8/1/95   Multif�nster, J. Kjellander
 *      21/12/95 Ytterligare resursfil, J. Kjellander
 *      1998-03-12 Ytterligare resursfil igen, J. Kjellander
 *
 ******************************************************!*/

  {
    short       status;
    XGCValues   values;
    XrmDatabase xdfDB;

/*
***Innan n�t annat g�rs mergar vi ev. initfiler med den redan
***skapade resursdatabasen.
*/
    if ( strlen(inifil_1) > 0  )
      {
      if ( v3ftst(inifil_1) )
        {
        xdfDB = XrmGetFileDatabase(inifil_1);
        XrmMergeDatabases(xdfDB,&xresDB);
        }
      else return(erpush("WP1442",inifil_1));
      }

    if ( strlen(inifil_2) > 0  )
      {
      if ( v3ftst(inifil_2) )
        {
        xdfDB = XrmGetFileDatabase(inifil_2);
        XrmMergeDatabases(xdfDB,&xresDB);
        }
      else return(erpush("WP1442",inifil_2));
      }
/*
***�ppna display.
*/
    if ( (xdisp=XOpenDisplay(NULL)) == NULL )
      return(erpush("WP1042",XDisplayName(NULL)));
/*
***Initiera V3:s f�nsterhanterare.
*/
    WPwini();
/*
***Ta reda p� sk�rm och antal bitplan.
*/
    xscr   = DefaultScreen(xdisp);
/*
***Initiera f�rger.
*/
    WPcini();
/*
***Skapa graphic context GC f�r winpac-f�nster.
***S�tt graphics_exposures till False s� att inte
***on�diga event typ GraphicsExpose eller NoExpose
***genereras i on�dan.
*/
    xgc =  DefaultGC(xdisp,xscr);
    values.graphics_exposures = False;
    XChangeGC(xdisp,xgc,GCGraphicsExposures,&values);
    XSetBackground(xdisp,xgc,WPgcol(WP_BGND));
    XSetForeground(xdisp,xgc,WPgcol(WP_FGND));
/*
***Initiera text-fonter.
*/
    status = WPfini();
    if ( status < 0 ) return(erpush("WP1032",XDisplayName(NULL)));
/*
***Skapa grafik-cursor.
*/
    xgcur1 = XCreateFontCursor(xdisp,34);
    xgcur2 = XCreateFontCursor(xdisp,52);
    xwcur  = XCreateFontCursor(xdisp,150);
/*
***Initiera menyhanteraren.
*/
    if ( (status=WPmini()) < 0 ) return(status);
/*
***Init fonts for graphical TEXT.
*/
   WPinfn();
/*
***Slut.
*/
    return(0);
  }

/********************************************************/
/*!******************************************************/

        short WPexit()

/*      Avslutar winpac.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 23/6/92 U.Andersson
 *
 ******************************************************!*/

  {
/*
***Nollst�ll allt som har med grafiska f�nster att g�ra.
*/
   WPclrg();
/*
***Nollst�ll grafiska fonter.
*/
    WPexfn();
/*
***Nollst�ll resten ocks�.
*/
   WPwexi();
/*
***Inga menyer nu aktiva.
*/
   actmeny = NULL;
/*
***St�ng display.
*/
   XCloseDisplay(xdisp);
/*
***Slut.
*/
   return(0);
  }

/********************************************************/
/*!******************************************************/

        short WPclrg()

/*      D�dar alla grafiska f�nster och nollst�ller
 *      det som har med dem att g�ra.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 27/9/95 J. Kjellander
 *
 ******************************************************!*/

  {
   int     i;
   WPWIN  *winptr;

/*
***Sudda meddelandef�nstret s� att dess button-pekare
***nollst�lls. Annars funkar inte lagra/nytt jobb.
*/
   WPwlma("");
/*
***D�da alla grafiska f�nster.
*/
   for ( i=0; i<WTABSIZ; ++i )
     if ( (winptr=WPwgwp((wpw_id)i)) != NULL  &&
           winptr->typ == TYP_GWIN ) WPwdel((DBint)i);
/*
***Slut.
*/
   return(0);
  }

/********************************************************/
/*!******************************************************/

        short WPsvjb(
        FILE *f)

/*      Lagrar grafiska f�nster i jobfilen.
 *
 *      In: f => Pekare till �ppen jobbfil.
 *
 *      Ut: Inget.
 *
 *      Felkod: WP1422 = Fel vid skrivning till jobfil.
 *
 *      (C)microform ab 28/1/95 J. Kjellander
 *
 ******************************************************!*/

 {
   short   corrx,corry;
   int     i,winant;
   WPWIN  *winptr;
   WPGWIN *gwinpt;

/*
***Hur m�nga WPGWIN har vi ?
*/
   winant = WPngws();

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
     if ( (winptr=WPwgwp((wpw_id)i)) != NULL  &&
           winptr->typ == TYP_GWIN )
       {
       gwinpt = (WPGWIN *)winptr->ptr;

       corrx = gwinpt->geo.x - gwinpt->wmandx;
       corry = gwinpt->geo.y - gwinpt->wmandy;
       if ( fwrite((char *)&corrx,sizeof(short),1,f) == 0 )
         goto error;
       if ( fwrite((char *)&corry,sizeof(short),1,f) == 0 )
         goto error;

       if ( fwrite((char *)&gwinpt->geo.dx,sizeof(short),1,f) == 0 )
         goto error;
       if ( fwrite((char *)&gwinpt->geo.dy,sizeof(short),1,f) == 0 )
         goto error;
       if ( fwrite((char *)&gwinpt->vy.vynamn,sizeof(char),GPVNLN+1,f) == 0 )
         goto error;
       if ( fwrite((char *)&gwinpt->vy.modwin,sizeof(VYWIN),1,f) == 0 )
         goto error;
       if ( fwrite((char *)gwinpt->nivtab,sizeof(char),WP_NTSIZ,f) == 0 )
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

        short WPldjb(
        FILE *f)

/*      Laddar grafiska f�nster fr�n jobfilen.
 *
 *      In: f => Pekare till �ppen jobbfil.
 *
 *      Ut: Inget.
 *
 *      Felkoder: WP1432 = Kan ej l�sa jobfil.
 *                WP1242 = Kan ej skapa f�nster
 *
 *      (C)microform ab 28/1/95 J. Kjellander
 *
 ******************************************************!*/

  {
   int       i,winant,ix,iy,idx,idy; 
   short     status,x,y,dx,dy;
   DBint     w_id;
   WPGWIN   *gwinpt,tmpgwi;

/*
***Hur m�nga f�nster skall skapas ?
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
     if ( fread(&tmpgwi.vy.vynamn,sizeof(char),GPVNLN+1,f) == 0 ) goto error;
     if ( fread(&tmpgwi.vy.modwin,sizeof(VYWIN),1,f) == 0 ) goto error;
     if ( fread( tmpgwi.nivtab,sizeof(char),WP_NTSIZ,f) == 0 ) goto error;
/*
***Kolla att resultatet hamnar p� sk�rmen.
*/
     ix = x; iy = y; idx = dx; idy = dy;
     WPposw(ix,iy,idx+10,idy+25,&ix,&iy);
/*
***Skapa sj�lva huvudf�nstret.
*/
     status = WPwcgw((short)ix,(short)iy,(short)idx,(short)idy,"",TRUE,&w_id);
     if ( status < 0 ) return(erpush("WP1242",XDisplayName(NULL)));
/*
***Vilket X-id fick f�nstret och dess 'save_under' pixmap ?
*/
     gwinpt  = (WPGWIN *)wpwtab[(wpw_id)w_id].ptr;
     xgwin   = gwinpt->id.x_id;
     xgmap   = gwinpt->savmap;
     gpxgc   = gwinpt->win_gc;
/*
***Aktivera f�nstrets vy. Detta g�r vi genom att f�rst aktivera
***den version av vyn som finns i vytab och sedan �ndra modell-
***f�nstret s� att det st�mmer med jobfilen. Normalisering beh�vs
***f�r att ber�kna nya 2D-konstanter och f�r att kompensera f�r
***olika storlek p� pixels i X- och Y-led p� olika sk�rmar. Slutligen
***aktiverar vi vyn igen varvid s�v�l vytab som actvy f�r r�tt
***utseende.
*/
     WPacvi(tmpgwi.vy.vynamn,GWIN_MAIN);
     V3MOME(&tmpgwi.vy.modwin,&gwinpt->vy.modwin,sizeof(VYWIN));
     WPnrgw(gwinpt);
     WPacvi(tmpgwi.vy.vynamn,GWIN_MAIN);
/*
***Kopiera den l�sta niv�tabellen till det skapade f�nstret.
*/
     V3MOME(tmpgwi.nivtab,gwinpt->nivtab,WP_NTSIZ);
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
     if ( fread( tmpgwi.nivtab,   sizeof(char),WP_NTSIZ,f) == 0 ) goto error;

     ix = x; iy = y; idx = dx; idy = dy;
     WPposw(ix,iy,idx+10,idy+25,&ix,&iy);

     status = WPwcgw((short)ix,(short)iy,(short)idx,(short)idy,"",FALSE,&w_id);
     if ( status < 0 ) return(erpush("WP1242",XDisplayName(NULL)));

     gwinpt  = (WPGWIN *)wpwtab[(wpw_id)w_id].ptr;
     WPacvi(tmpgwi.vy.vynamn,w_id);
     V3MOME(&tmpgwi.vy.modwin,&gwinpt->vy.modwin,sizeof(VYWIN));
     WPnrgw(gwinpt);

     V3MOME(tmpgwi.nivtab,gwinpt->nivtab,WP_NTSIZ);
     }
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

        int WPngws()

/*      Talar om hur m�nga grafiska f�nster som finns f�r
 *      tillf�llet.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: Antal grafiska f�nster.
 *
 *      (C)microform ab 28/1/95 J. Kjellander
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
     if ( (winptr=WPwgwp((wpw_id)i)) != NULL  &&
           winptr->typ == TYP_GWIN ) winant++;

   return(winant);
  }

/********************************************************/
/*!******************************************************/

        short WPcgws(
        bool create)

/*      Skapar grafiska f�nster enligt resursdatabsen.
 *      Om create = TRUE skapas och aktiveras �ven default
 *      vy.
 *      S�tter div. grapac-variabler.
 *
 *      In: create => TRUE  = Skapa �ven default vy.
 *                    FALSE = Skapa bara f�nstren.
 *
 *      Ut: Inget.   
 *
 *      Felkod: WP1242 = Kan ej skapa grafiskt f�nster p� %s
 *
 *      (C)microform ab 18/1-95 J. Kjellander
 *
 ******************************************************!*/

  {
   int       height,width,nadd,i,x,y; 
   unsigned int dx,dy;
   long      flags;
   char     *type[20];
   char      xrmstr1[81],xrmstr2[81],numstr[81];
   short     status;
   DBint     w_id;
   char      avynam[GPVNLN+1];
   XrmValue  value;
   WPGWIN   *gwinpt;
   VYVEC     bpos;

/*
***Eftersom WPwcgw() anropar WPupgp() kommer actvy att skrivas �ver.
***Spara namnet p� aktiv vy s� att denna kan aktiveras om jobb har
***laddats fr�n jobfil utan f�nster.
*/
   strcpy(avynam,actvy.vynamn);
/*
***V3:s grafiska huvud-f�nster, GWIN_MAIN.
***H�rdprogrammerade defaultv�rden.
*/
   width  = DisplayWidth(xdisp,xscr);
   height = DisplayHeight(xdisp,xscr); 

   x  = 0.2*width - 12;
   y  = 5;
   dx = 0.8*width - 15;
   dy = 0.9*height;
/*
***V�rden fr�n resursdatabasen.
***Kolla att resultatet hamnar p� sk�rmen.
*/
   if ( XrmGetResource(xresDB,"varkon.grawin_1.geometry",
                              "Varkon.Grawin_1.Geometry",
                               type,&value) )
     {
     flags = XParseGeometry((char *)value.addr,&x,&y,&dx,&dy);
     if ( XValue & flags )
       if ( XNegative & flags ) x = width + x - dx;
     if ( YValue & flags )
       if ( YNegative & flags ) y = height + y - dy;
     }

   WPposw(x,y,dx+10,dy+25,&x,&y);
/*
***Skapa sj�lva f�nstret. Observera att detta �r det f�rsta 
***f�nstret som skapas och det f�r d�rmed ID = 0 = GWIN_MAIN.
***S� m�ste det ovillkorligen vara eftersom m�nga rutiner inte
***minst i MBS bygger p� detta.
*/
   status = WPwcgw((short)x,(short)y,(short)dx,(short)dy,"",TRUE,&w_id);
   if ( status < 0 ) return(erpush("WP1242",XDisplayName(NULL)));
/*
***Vilket X-id fick f�nstret och dess 'save_under' pixmap ?
*/
   gwinpt  = (WPGWIN *)wpwtab[(wpw_id)w_id].ptr;
   xgwin   = gwinpt->id.x_id;
   xgmap   = gwinpt->savmap;
   gpxgc   = gwinpt->win_gc;
/*
***Kanske skall vi ocks� skapa default vy. Genom att f�rst s�tta
***aktiv vy:s modellf�nster = Huvudf�nstrets ser vi till att vyn
***"xy" f�r samma modellf�nster som GWIN_MAIN. EXcrvp() kopierar
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
     WPacvi("xy",GWIN_MAIN);
     }
   else WPacvi(avynam,GWIN_MAIN);
/*
***Eventuella ytterligare grafiska f�nster.
***Antal ytterligare �r till att b�rja med = 0.
*/
   nadd = 0;

   for ( i=2; i<WP_GWMAX; ++i )
     {
     strcpy(xrmstr1,"varkon.grawin_");
     strcpy(xrmstr2,"Varkon.Grawin_");
     sprintf(numstr,"%d",i);
     strcat(xrmstr1,numstr);
     strcat(xrmstr2,numstr);
     strcat(xrmstr1,".geometry");
     strcat(xrmstr2,".Geometry");
/*
***Prova att h�mta .geometry-resursen.
*/
     if ( XrmGetResource(xresDB,xrmstr1,xrmstr2,type,&value) )
       {
       flags = XParseGeometry((char *)value.addr,&x,&y,&dx,&dy);
       if ( XValue & flags )
         if ( XNegative & flags ) x = gwinpt->geo.dx + x - dx;
       if ( YValue & flags )
         if ( YNegative & flags ) y = gwinpt->geo.dy + y - dy;

       WPposw(x,y,dx+10,dy+25,&x,&y);
/*
***Sen skapar vi ett ytterligare grafiskt f�nster.
*/
       status = WPwcgw((short)x,(short)y,(short)dx,(short)dy,"",FALSE,&w_id);
       if ( status < 0 ) return(erpush("WP1242",XDisplayName(NULL)));
/*
***Om create = TRUE skall vyn "xy" aktiveras i alla f�nster.
*/
       if ( create ) WPacvi("xy",w_id);
       else          WPacvi(avynam,w_id);
/*
***Slutligen r�knar vi upp nadd.
*/
       if ( ++nadd == WP_GWMAX ) return(0);
       }
     }
/*
***Slut.
*/
   return(0);
  }

/********************************************************/
/*!******************************************************/

        short WPmrdb(
        int      *ac,
        char     *av[])

/*      Skapar resursdatabas. Anropas av main()->igppar() och
 *      lagrar pekare till datbasen i den globala variabeln
 *      xresDB.
 *
 *      In:  ac = Pekare till antal parametrar p� kommandoraden.
 *           av = Kommandoradsparametrarna.
 *
 *      Ut:  Inget.
 *
 *      FV:  0.
 *
 *      (C)microform ab 8/2/94 J. Kjellander
 *
 *      940602 VMS, J. Kjellander
 *      1998-02-06 XPointer, J.Kjellander
 *
 ******************************************************!*/

  {
    char        path[256];
    XrmDatabase cmdDB,appDB,xdfDB;

#define OPTANT 10

static XrmOptionDescRec opttab[] = {
  {"-font",          ".font",        XrmoptionSepArg, (XPointer)NULL},
  {"-fn",            ".font",        XrmoptionSepArg, (XPointer)NULL},
  {"-geometry",      ".geometry",    XrmoptionSepArg, (XPointer)NULL},
  {"-title",         ".title",       XrmoptionSepArg, (XPointer)NULL},
  {"-bg",            ".background",  XrmoptionSepArg, (XPointer)NULL},
  {"-background",    ".background",  XrmoptionSepArg, (XPointer)NULL},
  {"-fg",            ".foreground",  XrmoptionSepArg, (XPointer)NULL},
  {"-foreground",    ".foreground",  XrmoptionSepArg, (XPointer)NULL},
  {"-iconic",        ".iconStartup", XrmoptionSepArg, (XPointer)"on"},
  {"-xrm",           NULL,           XrmoptionResArg, (XPointer)NULL},
 };

/*
***OPTANT anger hur m�nga options som finns i opttab. opttab
***beskriver de optioner som supportas f�r n�rvarande.
*/

/*
***Ev. DEBUG. eftersom denna rutin anropas innan kommandoraden
***tolkats av V3 kan inte debug vara p�slaget. En variant �r
***att sl� p� debug h�r ovillkorligt. Av n�gon orsak verkar
***�tminstone SCO/UNIX-versionen av XrmParseCommand() strippa
***parametrar med bara 1 bokstav samt �ven f�ljande parameter
***trots att de inte har n�got med X-resurser att g�ra. Ex. -g
***f�rsvinner liksom -x. L�sningen �r att ha dem sist eller att 
***inte anv�nda parametrar med bara 1 bokstav. M�jligen har det 
***med ett mellanslag mellan parametern och v�rdet att g�ra.
***X tror att -g -gm50 �r en tilldelning av parametern -g med
***v�rdet -gm50 eller n�t s�nt. F�rst�r �nd� inte riktigt.....
*/
/*
***Initiering.
*/
    XrmInitialize();
/*
***Skapa resursdatabas av ev. resursfil p� "app-defaults".
*/
#ifdef UNIX
    appDB = XrmGetFileDatabase("/usr/lib/X11/app-defaults/Varkon");
#endif

#ifdef VMS
    appDB = XrmGetFileDatabase("V3$APP-DEFAULTS:Varkon");
#endif

    XrmMergeDatabases(appDB,&xresDB);
/*
***Merga med ev. ".Xdefaults" p� i f�rsta hand "XENVIRONMENT"-
***directoryt och i andra hand "HOME"-directoryt.
*/
#ifdef UNIX
    if ( gtenv3("XENVIRONMENT") != NULL )
      strcpy(path,gtenv3("XENVIRONMENT"));
    else
      {
      strcpy(path,gtenv3("HOME"));
      strcat(path,"/.Xdefaults");
      }

    xdfDB = XrmGetFileDatabase(path);
#endif

#ifdef VMS
    xdfDB = XrmGetFileDatabase("SYS$LOGIN:.Xdefaults");
#endif

    XrmMergeDatabases(xdfDB,&xresDB);
/*
***Merga med ev. parametrar p� kommandoraden.
***Parametrar som inte svarar mot opttab l�mnas kvar och
***parsas av V3 sj�lvt. ac och av uppdateras is�fall av
***XrmParseCommand(). Som klassnamn anv�nder vi "varkon".
*/
    cmdDB = NULL;
    XrmParseCommand(&cmdDB,(XrmOptionDescList)opttab,
                                     OPTANT,"varkon",ac,av);
    XrmMergeDatabases(cmdDB,&xresDB);
/*
***Slut.
*/
    return(0);

  }

/********************************************************/
/*!******************************************************/

        short  WPgtwi(
        DBint  id,
        int   *px,
        int   *py,
        int   *pdx,
        int   *pdy,
        int   *ptyp,
        char  *rubrik)

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
    char    errbuf[80],*name;
    Window  xid;
    WPWIN  *winptr;
    WPIWIN *iwinpt;
    WPGWIN *gwinpt;

/*
***�r det m�jligen hela sk�rmen som avses ?
*/
    if ( id == -1 )
      {
     *px   = *py = 0;
     *pdx  = DisplayWidth(xdisp,xscr);
     *pdy  = DisplayHeight(xdisp,xscr);
     *ptyp = -1;
      sprintf(rubrik,"%s:%d:%d:%d",
              ServerVendor(xdisp),VendorRelease(xdisp),
              ProtocolVersion(xdisp),ProtocolRevision(xdisp)); 
      return(0);
      }
/*
***Fixa C-pekare till f�nstret.
*/
    if ( (winptr=WPwgwp(id)) == NULL )
      {
      sprintf(errbuf,"%d",(int)id);
      return(erpush("WP1562",errbuf));
      }

    switch ( winptr->typ )
      {
      case TYP_IWIN:
      iwinpt = (WPIWIN *)winptr->ptr;
      xid    = iwinpt->id.x_id;
     *ptyp   = 0;
      break;

      case TYP_GWIN:
      gwinpt = (WPGWIN *)winptr->ptr;
      xid    = gwinpt->id.x_id;
     *ptyp   = 1;
      break;

      default:
      sprintf(errbuf,"%d",(int)id);
      return(erpush("WP1572",errbuf));
      }
/*
***Rubrik.
*/
   XFetchName(xdisp,xid,&name);

   if ( name != NULL )
     {
     strcpy(rubrik,name);
     XFree(name);
     }
   else strcpy(rubrik,"");
/*
***Placering och storlek.
*/
    WPgwsz(xid,px,py,pdx,pdy,NULL,NULL);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short WPgtwp(
        Window  wid,
        int    *px,
        int    *py)

/*      Returnerar f�nstrets nuvarande l�ge.
 *
 *      In: wid = F�nster-ID.
 *          px  = Pekare till l�ge i X-led.
 *          py  = Pekare till l�ge i Y-led.
 *
 *      Ut: *px  = L�ge i X-led.
 *          *py  = L�ge i Y-led.
 *
 *      FV:  0.
 *
 *      (C)microform ab 11/1/94 J. Kjellander
 *
 ******************************************************!*/

  {
    Window root,child;
    int    x,y;
    unsigned int dx,dy,bw,d;

    XGetGeometry(xdisp,wid,&root,&x,&y,&dx,&dy,&bw,&d);
    root = DefaultRootWindow(xdisp);
    XTranslateCoordinates(xdisp,wid,root,x,y,px,py,&child);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short WPgwsz(
        Window  wid,
        int    *px,
        int    *py,
        int    *pxp,
        int    *pyp,
        double *pxm,
        double *pym)

/*      Returnerar f�nstrets nuvarande l�ge och storlek i antal
 *      pixels och i mm. Om f�nstret �r ett "Top Level"-f�nster
 *      och �gs av f�nsterhanteraren returneras x,y = 0.
 *
 *      In: wid = F�nster-ID.
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
 *      (C)microform ab 29/6/92 J. Kjellander
 *
 ******************************************************!*/

  {
    Window root,child;
    unsigned int bw,d;

/*
***Fr�ga X efter storlek och placering i pixels.
***Efter m�nga sv�ra stunder har det framkommit att
***f�nstrets l�ge inte �r relativt sk�rmen utan n�got
***annat, gud vet vad. px och py �r alltid 0 efter 
***XGetGeometry. XTranslateCoordinates fixar detta dock.
***F�nster som �r "top level" och �gs av motif verkar f�
***0,0 som position.
*/
    XGetGeometry(xdisp,wid,&root,px,py,
                (unsigned int *)pxp,(unsigned int *)pyp,&bw,&d);
    XTranslateCoordinates(xdisp,wid,root,0,0,px,py,&child);
/*
***Vad blir det i millimeter ?
*/
    if ( pxm != NULL )
      *pxm = *pxp/(double)DisplayWidth(xdisp,xscr)*
                          XDisplayWidthMM(xdisp,xscr);

    if ( pym != NULL )
      *pym = *pyp/(double)DisplayHeight(xdisp,xscr)*
                          XDisplayHeightMM(xdisp,xscr);
/*
***Slut.
*/
    return(0);
  }

/********************************************************/
/*!******************************************************/

        short WPsdpr(
        Window win)

/*      Aktiverar WM_PROTOCOLS med WM_DELETE_WINDOW
 *      f�r ett visst f�nster. Detta f�r till effekt
 *      att en f�nsterhanterare inte g�r destroy p�
 *      f�nstret eller d�dar processen n�r anv�ndaren
 *      v�ljer close i f�nster-menyn. Ist�llet
 *      skickas ett ClientMessageEvent till event-k�n.
 *      ClientMessageEvents beh�ver man inte be om med
 *      XSelectInput, dessa kommer �nd�!!!
 *
 *      Detta property skall bara anv�ndas p� top-level
 *      f�nster som �gs av f�nsterhanteraren, dvs. det
 *      grafiska f�nstret, listf�nster etc. Rutinen skall
 *      anropas direkt efter att f�nstret har skapats och
 *      innan det g�rs map p� det.
 *
 *      In: win => F�nster-id.   
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 3/8/92 J. Kjellander
 *
 ******************************************************!*/

  {
    Atom proto,delete,type,focus;

/*
***H�mta Atoms.
*/
    proto  = XInternAtom(xdisp,"WM_PROTOCOLS",False);
    delete = XInternAtom(xdisp,"WM_DELETE_WINDOW",False);
    type   = XInternAtom(xdisp,"ATOM",False);
/*
***Aktivera WM_PROTOCOLS-property med data = WM_DELETE_WINDOW.
*/
    XChangeProperty(xdisp,win,proto,type,32,
                    PropModeReplace,(unsigned char *)&delete,1);
/*
***Samma med FOCUS.
*/
    focus = XInternAtom(xdisp,"WM_TAKE_FOCUS",False);
/*
***Aktivera WM_PROTOCOLS-property med data = WM_DELETE_WINDOW.
*/
    XChangeProperty(xdisp,win,proto,type,32,
                    PropModeReplace,(unsigned char *)&focus,1);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        bool WPgrst(
        char *resnam,
        char *resval)

/*      Om resursen �r definierad returneras dess v�rde
 *      och TRUE annars FALSE.
 *
 *      In: resnam = Resurs.
 *
 *      Ut: resval = V�rde.
 *
 *      FV: TRUE = Resursen finns, v�rde returneras.
 *          FALSE = Resursen finns ej, "" returneras.
 *
 *      (C)microform ab 17/10/95 J. Kjellander
 *
 ******************************************************!*/

 {
   bool     hit;
   int      i,ntkn;
   char     buf[V3STRLEN+1];

   char    *type[20];
   XrmValue value;

/*
***Lite initiering.
*/
   hit    = FALSE;
  *resval = '\0';
/*
***H�r kommer X-Windows versionen. F�r att kunna anv�nda
***XrmGetResource() m�ste vi skapa tv� str�ngar, en med
***sm� bokst�ver och en med stora.
*/
   ntkn = strlen(resnam);
   if ( ntkn > 0  &&  ntkn < V3STRLEN )
     {
     strcpy(buf,resnam);

     buf[0] -= 32;
     for ( i=0; i<ntkn; ++i )
       if ( resnam[i] == '.' ) buf[i+1] -= 32;
     if ( XrmGetResource(xresDB,resnam,buf,type,&value) )
       {
       strcpy(resval,value.addr);
       hit = TRUE;
       }
     }

    return(hit); 
  }

/********************************************************/

