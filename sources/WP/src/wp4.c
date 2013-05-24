/**********************************************************************
*
*    wp4.c
*    ====
*
*    This file is part of the VARKON WindowPac Library.
*    URL: http://www.varkon.com
*
*    This file includes:
*
*    wpfini();   Init font handling
*    wpgfnr();   Map font name -> number
*    wpgfnt();   Returns XFont pointer
*    wpsfnt();   Sets active font
*
*    wpcini();   Init color handling 
*    wpccol();   Allocate/create color
*    wpgcol();   Map color number -> pixel value
*    wpgrgb();   Returns XColor-structure
*
*    wpstrl();   Length of string with active font
*    wpgtsl();   Length of string with specified font
*    wpstrh();   Height of active font
*    wpgtsh();   Height of any font
*    wpftpy();   Center text vertically
*
*    wpwstr();   Display text in window
*    wpmaps();   Maps 7-bit ASII to ISO8859 (obsolete)
*    wpdivs();   Split long line of text into two lines
*
*    wpd3db();   Draw window 3D border
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

#define FNTTBSIZ 20

typedef struct
{
char        namn[V3STRLEN+1];
XFontStruct *xfs;
}WPFONT;

static short  actfnt;
static WPFONT fnttab[FNTTBSIZ];

/* fnttab �r en tabell med information om laddade fonter.
   Alla namn initieras till "" och alla XFS:s till NULL.
   actfnt h�ller reda p� vilken font i fnttab som �r aktiv
   dvs. ing�r i aktiv GC. Font nummer 0 laddas som default
   font av wpfini() vid uppstart. �vriga fonter laddas d�
   olika typer av f�nster, knappar, ikoner etc. skapas av V3
   sj�lvt eller via MBS.
*/

static XColor coltab[WP_NPENS+WP_SPENS];

/* coltab �r en tabell med XColor-structures f�r V3:s olika
   f�rger. Varje entry i coltab svarar mot en f�rg med
   samma nummer. coltab initieras av wpcini() vid uppstart.
   De 256 f�rsta f�rgerna motsvarar penna 0 tom. 255 och de
   5 sista f�rgerna anv�nds av systemet till �vriga f�nster.
*/

XFontStruct *xfs;             /* Tills vidare bara !!!! */

/*!******************************************************/

        short wpfini()

/*      Init-rutin f�r font-hanteringen. Anropas av
 *      wpinit() vid uppstart. Laddar default font
 *      och g�r den till font nr: 0.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      Felkod: WP1002 = Kan ej ladda fonten %s.
 *
 *      (C)microform ab 13/12/93 J. Kjellander
 *    
 *      1/11-94  VAX-defaltfont, J. Kjellander
 *
 ******************************************************!*/

  {
    short    i;
    char     fntnam[V3STRLEN+1],*type[20];
    XrmValue value;

/*
***Nollst�ll fnttab.
*/
    for ( i=0; i<FNTTBSIZ; ++i )
      {
      fnttab[i].namn[0] = '\0';
      fnttab[i].xfs     = NULL;
      }
/*
***H�mta fontnamn fr�n resursdatabasen.
*/
    if ( XrmGetResource(xresDB,"varkon.font","Varkon.Font",
                        type,&value) ) strcpy(fntnam,value.addr);
#ifdef UNIX
    else strcpy(fntnam,"fixed");
#endif
#ifdef VMS
    else strcpy(fntnam,"*courier-bold-*14*75-m-iso8859-1");
#endif

/*
***Prova att ladda.
*/
    if ( wpgfnr(fntnam) < 0 ) return(erpush("WP1002",fntnam));
/*
***Allt gick bra, d� g�r vi den aktiv ocks�. actfnt = -1
***tvingar wpsfnt() att aktivera.
*/
    actfnt = -1;
    wpsfnt((short)0);
/*
***Ladda fonten "cursor" som font 1.
*/
    strcpy(fntnam,"cursor");
    if ( wpgfnr(fntnam) < 0 ) return(erpush("WP1002",fntnam));
/*
***Slut.
*/
    return(0);
  }

/********************************************************/
/*!******************************************************/

        short wpgfnr(
        char *fntnam)

/*      Laddar fonten fntnam om den inte redan �r
 *      laddad och returnerar en siffra som anger
 *      dess plats i fnttab.
 *
 *      In: fntnam = Fontnamn.
 *
 *      Ut: Inget.
 *
 *      FV: Giltigt entry i fnttab eller -1.
 *
 *      Felkoder : WP1012 = fnttab full, font = %s.
 *                 WP1022 = Fonten %s finns ej.
 *
 *      (C)microform ab 12/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    short        i;
    XFontStruct *f;

/*
***B�rja med att kolla om fonten redan �r laddad.
*/
    for ( i=0; i<FNTTBSIZ; ++i )
      if ( strcmp(fnttab[i].namn,fntnam) == 0  ) return(i);
/*
***Ej laddad, leta upp ledig plats i fnttab.
*/
    for ( i=0; i<FNTTBSIZ; ++i ) if ( fnttab[i].xfs == NULL ) break;
    if ( i == FNTTBSIZ ) return(erpush("WP1012",fntnam));
/*
***Prova att ladda fonten.
*/
    if ( (f=XLoadQueryFont(xdisp,fntnam)) == NULL )
      return(erpush("WP1022",fntnam));
/*
***Allt v�l, lagra fontnam och pekare i tabellen.
*/
    strcpy(fnttab[i].namn,fntnam);
    fnttab[i].xfs = f;
/*
***Slut, returnera nytt fontnummer.
*/
    return(i);
  }

/********************************************************/
/*!******************************************************/

        XFontStruct *wpgfnt(
        short fntnum)

/*      Returnerar X-fontpekare till font med visst
 *      nummer.
 *
 *      In: fntnum = Fontnummer.
 *
 *      Ut: Inget.
 *
 *      FV: X-Fontpekare.
 *
 *      (C)microform ab 12/12/93 J. Kjellander
 *
 *
 ******************************************************!*/

  {
    return(fnttab[fntnum].xfs);
  }

/********************************************************/
/*!******************************************************/

        short wpsfnt(
        short fntnum)

/*      S�tter aktiv font.
 *
 *      In: fntnum = Fontnummer.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 12/12/93 J. Kjellander
 *
 *
 ******************************************************!*/

  {
    if ( fntnum != actfnt )
      {
      actfnt = fntnum;
      XSetFont(xdisp,xgc,fnttab[fntnum].xfs->fid);
      xfs = fnttab[fntnum].xfs;
      }

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short wpcini()

/*      Init-rutin f�r f�rg-hanteringen. Anropas av
 *      wpinit() vid uppstart.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      Felkod:
 *
 *      (C)microform ab 13/12/93 J. Kjellander
 *
 *      1997-02-18 Omarbetad, J.Kjellander
 *      1999-03-24 Bug vid wpgrst, R. Svedin
 *
 ******************************************************!*/

  {
    int       i;
    char      colnam[80],resurs[80];
    XColor    rgb,hardw;
    Colormap  colmap; 

/*
***Fixa default colormap.
*/
    colmap = DefaultColormap(xdisp,xscr);
/*
***Penna 0 skall vara vit.
*/
    XLookupColor(xdisp,colmap,"White",&rgb,&hardw);
    XAllocColor(xdisp,colmap,&hardw);
    V3MOME(&hardw,&coltab[0],sizeof(XColor));
/*
***Och resten svart till att b�rja med.
*/
    XLookupColor(xdisp,colmap,"Black",&rgb,&hardw);
    XAllocColor(xdisp,colmap,&hardw);

    for ( i=1; i<WP_NPENS+WP_SPENS; ++i )
      V3MOME(&hardw,&coltab[i],sizeof(XColor));
/*
***Om vi k�r monokromt m�ste �ven varkon.background bli vit.
*/
    if ( DefaultDepth(xdisp,xscr) == 1 )
      {
      XLookupColor(xdisp,colmap,"White",&rgb,&hardw);
      XAllocColor(xdisp,colmap,&hardw);
      V3MOME(&hardw,&coltab[WP_BGND],sizeof(XColor));
      return(0);
      }
/*
***K�r vi f�rg initierar vi f�rst alla 5 systemf�rgerna.
*/
    if ( !wpgrst("varkon.background",colnam) ) strcpy(colnam,"Gray");
    XLookupColor(xdisp,colmap,colnam,&rgb,&hardw);
    if ( XAllocColor(xdisp,colmap,&hardw) )
      V3MOME(&hardw,&coltab[WP_BGND],sizeof(XColor));

    if ( !wpgrst("varkon.foreground",colnam) ) strcpy(colnam,"Black");
    XLookupColor(xdisp,colmap,colnam,&rgb,&hardw);
    if ( XAllocColor(xdisp,colmap,&hardw) )
      V3MOME(&hardw,&coltab[WP_FGND],sizeof(XColor));

    if ( !wpgrst("varkon.topShadowColor",colnam) ) strcpy(colnam,"White");
    XLookupColor(xdisp,colmap,colnam,&rgb,&hardw);
    if ( XAllocColor(xdisp,colmap,&hardw) )
      V3MOME(&hardw,&coltab[WP_TOPS],sizeof(XColor));

    if ( !wpgrst("varkon.bottomShadowColor",colnam) ) strcpy(colnam,"Black");

    XLookupColor(xdisp,colmap,colnam,&rgb,&hardw);
    if ( XAllocColor(xdisp,colmap,&hardw) )
      V3MOME(&hardw,&coltab[WP_BOTS],sizeof(XColor));

    if ( !wpgrst("varkon.highlightColor",colnam) ) strcpy(colnam,"Blue");
    XLookupColor(xdisp,colmap,colnam,&rgb,&hardw);
    if ( XAllocColor(xdisp,colmap,&hardw) )
      V3MOME(&hardw,&coltab[WP_NOTI],sizeof(XColor));
/*
***Och sen de normala f�rger som finns definierade som pen-resurser.
*/
    for ( i=0; i<WP_NPENS; ++i )
      {
      sprintf(resurs,"varkon.color_%d",i);
      if ( wpgrst(resurs,colnam) )
        {
        XLookupColor(xdisp,colmap,colnam,&rgb,&hardw);
        if ( XAllocColor(xdisp,colmap,&hardw) )
          V3MOME(&hardw,&coltab[i],sizeof(XColor));
        }
      }
/*
***Slut.
*/
    return(0);
  }

/********************************************************/
/*!******************************************************/

        short wpccol(
        int pen,
        int red,
        int green,
        int blue)

/*      Allokerar en f�rg med givna RGB-v�rden till
 *      viss penna.
 *
 *      In: pen = Pennummer (f�rgnummer).
 *          red,green,blu = RGB-v�rden 0 -> 255.
 *
 *      Ut: Inget.
 *
 *      Felkoder:
 *
 *      (C)microform ab 1997-02-18 J. Kjellander
 *
 ******************************************************!*/

  {
    XColor    rgb;
    Colormap  colmap; 

/*
***Fixa default colormap.
*/
    colmap = DefaultColormap(xdisp,xscr);
/*
***Lite felkontroll.
*/
   if ( pen < 0 ) pen = 0;
   if ( pen > WP_NPENS-1 ) pen = WP_NPENS-1;

   if ( red   < 0 ) red   = 0;
   if ( green < 0 ) green = 0;
   if ( blue  < 0 ) blue  = 0;

   if ( red   > 255 ) red   = 255;
   if ( green > 255 ) green = 255;
   if ( blue  > 255 ) blue  = 255;
/*
***Initiera en XColor. RGB-v�rdena skalas till 2**16.
*/
   rgb.red   = (unsigned short)((red*65535.0)/255.0);
   rgb.green = (unsigned short)((green*65535.0)/255.0);
   rgb.blue  = (unsigned short)((blue*65535.0)/255.0);
/*
***Skapa/allokera f�rgen.
*/
   if ( XAllocColor(xdisp,colmap,&rgb) )
     V3MOME(&rgb,&coltab[pen],sizeof(XColor));

   return(0);
  }

/********************************************************/
/*!******************************************************/

        unsigned long wpgcol(
        short colnum)

/*      Returnerar X-pixelv�rde till f�rg med visst
 *      nummer.
 *
 *      In: colnum = F�rgnummer.
 *
 *      Ut: Inget.
 *
 *      FV: Pixelv�rde.
 *
 *      (C)microform ab 12/12/93 J. Kjellander
 *
 *      1997-02-18 Omarbetatd, J.Kjellander
 *
 ******************************************************!*/

  {
    if ( colnum < 0 ) colnum = 0;
    if ( colnum > WP_NPENS+WP_SPENS-1 ) colnum = WP_NPENS+WP_SPENS-1;

    return(coltab[colnum].pixel);
  }

/********************************************************/
/*!******************************************************/

        short  wpgrgb(
        short   colnum,
        XColor *rgb)

/*      Returnerar pekare till f�rg med visst
 *      nummer.
 *
 *      In: colnum = F�rgnummer.
 *
 *      Ut: *rgb = XColor-structure ifylld.
 *
 *      (C)microform ab 1997-02-17 J. Kjellander
 *
 ******************************************************!*/

  {
    if ( colnum < 0 ) colnum = 0;
    if ( colnum > WP_NPENS+WP_SPENS-1 ) colnum = WP_NPENS+WP_SPENS-1;

    V3MOME(&coltab[colnum],rgb,sizeof(XColor));

    return(0);
  }

/********************************************************/
/*!******************************************************/

        int wpstrl(
        char *fstring)

/*      Returnerar l�ngden p� en str�ng i pixels med aktiv font.
 *
 *      In: fontstring.
 *
 *      Ut: Inget.
 *
 *      FV: L�ngden p� str�ngen i pixels.
 *
 *      (C)microform ab 23/6/92 U.Andersson
 *
 *
 ******************************************************!*/

  {
    return(XTextWidth(xfs,fstring,strlen(fstring)));
  }

/********************************************************/
/*!******************************************************/

        short wpgtsl(
        char *str,
        char *font,
        int  *plen)

/*      R�knar ut l�ngden p� en str�ng i pixels om den
 *      skulle skrivas ut med viss font.
 *
 *      In: str  = Text att skriva ut.
 *          font = Fontens namn.
 *
 *      Ut: *plen = L�ngd i pixels.
 *
 *      (C)microform ab 1996-05-21 J.Kjellander
 *
 ******************************************************!*/

  {
   short        fntnum;
   XFontStruct *fs;
/*
***Om det g�ller aktiv font tar vi den globala variablen xfs.
*/
   if ( *font == '\0' ) fs = xfs;
/*
***Annars tar vi den fr�n fonttabellen. Finns den inte d�r
***laddas den av wpgfnr() automatisk. Om fonten inte finns
***�verhuvudtaget eller om fonttabellen �r full returneras
***negativt status.
*/
   else
     {
     if ( (fntnum=wpgfnr(font)) < 0 ) return(fntnum);
     else                             fs = wpgfnt(fntnum);
     }
/*
***Nu har vi en fontpekare och kan ber�kna textl�ngden.
*/
   *plen = XTextWidth(fs,str,strlen(str));

    return(0);
  }

/********************************************************/
/*!******************************************************/

        int wpstrh()

/*      R�knar ut h�jden p� en str�ng i pixels.
 *
 *      In: fontstring.
 *
 *      Ut: Inget.
 *
 *      FV: H�jden p� str�ngen i pixels.
 *
 *      (C)microform ab 23/6/92 U.Andersson
 *
 *
 ******************************************************!*/

  {
    return(xfs->ascent + xfs->descent);
  }

/********************************************************/
/*!******************************************************/

        short wpgtsh(
        char *font,
        int  *phgt)

/*      Ber�knar h�jden p� en viss font.
 *
 *      In: font = Fontens namn.
 *
 *      Ut: *phgt = H�jd i pixels.
 *
 *      (C)microform ab 1996-05-21 J.Kjellander
 *
 ******************************************************!*/

  {
   short        fntnum;
   XFontStruct *fs;
/*
***Om det g�ller aktiv font tar vi den globala variablen xfs.
*/
   if ( *font == '\0' ) fs = xfs;
/*
***Annars tar vi den fr�n fonttabellen. Finns den inte d�r
***laddas den av wpgfnr() automatisk. Om fonten inte finns
***�verhuvudtaget eller om fonttabellen �r full returneras
***negativt status.
*/
   else
     {
     if ( (fntnum=wpgfnr(font)) < 0 ) return(fntnum);
     else                             fs = wpgfnt(fntnum);
     }
/*
***Nu har vi en fontpekare och kan ber�kna h�jden.
*/
   *phgt = fs->ascent + fs->descent;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        int wpftpy(
        int dy)

/*      R�knar ut y-koordinaten f�r en text i pixels.
 *
 *      In: dy = h�jden f�r textf�nstret.
 *
 *      Ut: Inget.
 *
 *      FV: Y-koordinaten i pixels.
 *
 *      (C)microform ab 23/6/92 U.Andersson
 *
 *
 ******************************************************!*/

  {
    return((dy - xfs->descent + xfs->ascent)/2);
  }

/********************************************************/
/*!******************************************************/

        short wpwstr(
        Window wid,
        int    x,
        int    y,
        char  *s)

/*      L�gniv�-rutin f�r utskrift av text i f�nster.
 *      Textfont, f�rg mm. st�lls in innan denna rutin
 *      anropas.
 *
 *      In: wid = F�nster att skriva texten i.
 *          x,y = Position relativt f�nstret.
 *          s   = Pekare till NULL-terminerad str�ng.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 17/8/92 J. Kjellander
 *
 *
 ******************************************************!*/

  {

/*
***Mappa till 8-bitars ASCII, ISO8859.
*/
    wpmaps(s);
/*
***Skriv ut.
*/
    XDrawImageString(xdisp,wid,xgc,x,y,s,strlen(s));

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short wpmaps(
        char *s)

/*      Mappar 7-bitars ASCII till ISO8859.
 *
 *      In: s = Pekare till NULL-terminerad str�ng.
 *
 *      Ut: *s = Mappad str�ng.
 *
 *      FV: 0.
 *
 *      (C)microform ab 4/1/94 J. Kjellander
 *
 ******************************************************!*/

  {
/* Not used after 1999-03-20
    short i;

    i = 0;
    do
      {
      switch ( s[i] )
        {
        case '}':  s[i] = 229; break;
        case '{':  s[i] = 228; break;
        case '|':  s[i] = 246; break;
        case ']':  s[i] = 197; break;
        case '[':  s[i] = 196; break;
        case '\\': s[i] = 214; break;
        }
      } while ( s[i++] != '\0' );
*/

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short wpdivs(
        char text[],
        int  maxpix,
        int  *tdx,
        int  *tdy,
        char str1[],
        char str2[])

/*      Delar en textstr�ng i tv� delar.
 *      Ber�knar platsbehov i pixels (x-och y-led)
 *
 *      In: text   = textstr�ng som ska anv�ndas.
 *          maxpix = max antal pixels som det aktiva f�nstret inneh�ller.
 *
 *      Ut: tdx  = textbredd i pixels.
 *          tdy  = texth�jd i pixels.
 *          str1 = textstr�ng1.
 *          str2 = textstr�ng2. 
 *
 *      FV: 0.
 *
 *      (C)microform ab 23/6/92 U.Andersson
 *
 *      15/2/95 Bug, J. Kjellander
 *
 ******************************************************!*/

  {
    int sl,slm;

/*
***Ber�kna antalet tecken som texten inneh�ller 
***och hur m�nga pixels detta �r.
*/
    sl = strlen(text);   
   *tdx = wpstrl(text); 
/*
***textstr�ng <= max antal pixels som texten 
***f�r ta med h�nsyn till aktuell sk�rmbredd.
*/
    if ( *tdx <= maxpix )    
      {
      strcpy(str1,text);
      strcpy(str2,"");
      *tdy = wpstrh();
      }
/*
***textstr�ng > max antal pixels som texten 
***f�r ta med h�nsyn till aktuell sk�rmbredd.
*/
    else                  
      {
/*
***B�rja mitt i str�ngen, g� �t h�ger och leta upp f�rsta mellanslag.
*/
      slm = sl/2;

      while( slm < sl  &&  text[slm] != ' ') ++slm; 
/*
***Om det fanns ett delar vi str�ngen d�r ?
*/ 
      if ( slm < sl )
        {
        text[slm] = '\0';
        strcpy(str1,text);
        strcpy(str2,&text[slm+1]);
       *tdx = wpstrl(str1);
       *tdy = 2*wpstrh();
        }
/*
***Om inte b�rjar vi om i mitten och letar �t v�nster ist�llet.
*/
      else
        {
        slm = sl/2;
        while( slm > 0  &&  text[slm] != ' ') --slm; 

        if ( slm > 0 )
          {
          text[slm] = '\0';
          strcpy(str1,text);
          strcpy(str2,&text[slm+1]);
         *tdx = wpstrl(str2);
         *tdy = 2*wpstrh();
          }
/*
***Om inget mellanslag hittas d�r heller delar vi str�ngen mitt av.
*/
        else
          {
          slm = sl/2;
          text[slm] = '\0';
          strcpy(str1,text);
          strcpy(str2,&text[slm+1]);
         *tdx = wpstrl(str1);
         *tdy = 2*wpstrh();
          }
        }
      }

    return(0); 
  }

/********************************************************/
/*!******************************************************/

        short wpd3db(
        char  *winptr,
        int    wintyp)

/*      L�gniv�-rutin f�r att f�rse ett f�nster med 
 *      en 3D-ram.
 *
 *      In: winptr = Pekare till WPBUTT, WPEDIT eller WPICON.
 *          wintyp = Typ av f�nster, TYP_EDIT, TYP_BUTTON
 *                                   TYP_ICON.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 19/1/94 J. Kjellander
 *
 *      1998-03-29 OpenGL f�r AIX, J.Kjellander
 *
 ******************************************************!*/

  {
    int     dx,dy,bw,i;
    short   bc,upleft,dnrgth;
    Window  xid;
    GC      act_gc;
    WPBUTT *butptr;
    WPEDIT *edtptr;
    WPICON *icoptr;
    WPRWIN *rwinpt;
    unsigned long bgpix,fgpix,ulpix,drpix;


/*
***Vilken sorts f�nster �r det ? Ta reda p� storlek etc.
*/
    switch ( wintyp )
      {
      case TYP_BUTTON:
      butptr = (WPBUTT *)winptr;
      if ( wpwtab[butptr->id.p_id].typ  ==  TYP_RWIN )
        {
        rwinpt = (WPRWIN *)wpwtab[butptr->id.p_id].ptr;
        act_gc = rwinpt->win_gc;
        }
      else act_gc = xgc;
      xid    = butptr->id.x_id;
      dx     = butptr->geo.dx;
      dy     = butptr->geo.dy;
      bw     = butptr->geo.bw;
      bc     = butptr->color.bckgnd;
      upleft = WP_TOPS;
      dnrgth = WP_BOTS;
      ulpix  = wpgcbu(butptr->id.p_id,WP_TOPS);
      drpix  = wpgcbu(butptr->id.p_id,WP_BOTS);
      bgpix  = wpgcbu(butptr->id.p_id,WP_BGND);
      fgpix  = wpgcbu(butptr->id.p_id,WP_FGND);
      break;

      case TYP_EDIT:
      edtptr = (WPEDIT *)winptr;
      act_gc = xgc;
      xid    = edtptr->id.x_id;
      dx     = edtptr->geo.dx;
      dy     = edtptr->geo.dy;
      bw     = edtptr->geo.bw;
      bc     = WP_BGND;
      upleft = WP_BOTS;
      dnrgth = WP_TOPS;
      ulpix  = wpgcol(WP_BOTS);
      drpix  = wpgcol(WP_TOPS);
      bgpix  = wpgcol(WP_BGND);
      fgpix  = wpgcol(WP_FGND);
      break;

      case TYP_ICON:
      icoptr = (WPICON *)winptr;
      act_gc = xgc;
      xid    = icoptr->id.x_id;
      dx     = icoptr->geo.dx;
      dy     = icoptr->geo.dy;
      bw     = icoptr->geo.bw;
      bc     = icoptr->color.bckgnd;
      upleft = WP_TOPS;
      dnrgth = WP_BOTS;
      ulpix  = wpgcol(WP_TOPS);
      drpix  = wpgcol(WP_BOTS);
      bgpix  = wpgcol(WP_BGND);
      fgpix  = wpgcol(WP_FGND);
      break;

      default:
      return(0);
      }
/*
***Rita. Antal pixels = dx,dy men adresserna = 0 till dx-1.
*/
    dx -= 1;
    dy -= 1;
/*
***Skugga �ver och till v�nster.
*/
    if ( upleft != WP_FGND )
      XSetForeground(xdisp,act_gc,ulpix);

    for ( i=1; i<bw+1; ++i )
      {
      XDrawLine(xdisp,xid,act_gc,i,i,dx-i,i);
      XDrawLine(xdisp,xid,act_gc,i,i,i,dy-i);
      }
/*
***Om knappen har annorlunda bakgrundsf�rg �n huvud-
***f�nstret m�ste en ram p� 1 pixel med bakgrundsf�rg
***skapas f�rst. ( Luften mellan 3D-ramen och Notify-ramen ).
***Detta g�rs h�r (mellan upleft och dnrgth) f�r att minimera
***antalet anrop till XSetForeground().
*/
    if ( bc != WP_BGND )
      {
      XSetForeground(xdisp,act_gc,bgpix);
      XDrawLine(xdisp,xid,act_gc,0,0,dx,0);
      XDrawLine(xdisp,xid,act_gc,dx,0,dx,dy);
      XDrawLine(xdisp,xid,act_gc,dx,dy,0,dy);
      XDrawLine(xdisp,xid,act_gc,0,dy,0,0);
      }
/*
***Skugga under och till h�ger.
*/
    XSetForeground(xdisp,act_gc,drpix);

    for ( i=1; i<bw+1; ++i )
      {
      XDrawLine(xdisp,xid,act_gc,dx-i,i,dx-i,dy-i);
      XDrawLine(xdisp,xid,act_gc,i,dy-i,dx-i,dy-i);
      }
/*
***Slut.
*/
    if ( dnrgth != WP_FGND )
      XSetForeground(xdisp,act_gc,fgpix);

    return(0);
  }

/********************************************************/
