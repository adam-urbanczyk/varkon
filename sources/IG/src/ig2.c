/*!******************************************************************/
/*  File: ig2.c                                                     */
/*  ===========                                                     */
/*                                                                  */
/*  This file includes:                                             */
/*                                                                  */
/*  igwstr();     Write string, low level                           */
/*  igmvac();     Move VT100 cursor, rough                          */
/*  igpstr();     Write horisontal string                           */
/*  igvstr();     Write vertical string                             */
/*  igdfld();     Define field                                      */
/*  igfstr();     Write to field                                    */
/*  iggtts();     Get C-ptr to t-string                             */
/*  igerar();     Erase subarea of VT100 screen                     */
/*  igersc();     Erase entire VT100 screen                         */
/*  igflsh();     Flush stdout                                      */
/*  igbell();     Ring bell                                         */
/*  iggtsm();     Read symbol from stdin                            */
/*  iglned();     Read with editing                                 */
/*  igglin();     MBS-read                                          */
/*  iggtch();     Read one character, low level                     */
/*  iggtds();     Read from digitizer port                          */
/*                                                                  */
/*  This file is part of the VARKON IG Library.                     */
/*  URL:  http://www.varkon.com                                     */
/*                                                                  */
/*  This library is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU Library General Public     */
/*  License as published by the Free Software Foundation; either    */
/*  version 2 of the License, or (at your option) any later         */
/*  version.                                                        */
/*                                                                  */
/*  This library is distributed in the hope that it will be         */
/*  useful, but WITHOUT ANY WARRANTY; without even the implied      */
/*  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR         */
/*  PURPOSE.  See the GNU Library General Public License for more   */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Library General      */
/*  Public License along with this library; if not, write to the    */
/*  Free Software Foundation, Inc., 675 Mass Ave, Cambridge,        */
/*  MA 02139, USA.                                                  */
/*                                                                  */
/*  (C)Microform AB 1984-1999, Johan Kjellander, johan@microform.se */
/*                                                                  */
/********************************************************************/

#include "../../DB/include/DB.h"
#include "../include/IG.h"
#include "../include/screen.h"
#include "../../GP/include/GP.h"
#include "../../WP/include/WP.h"
#include <string.h>

#ifdef UNIX
#undef VSTART
#include "termio.h"
#endif

extern MNUALT smbind[];      /* Symboler */
extern DIGDAT digdes;        /* Digitizer */
extern char   txtmem[];      /* Alla t-str�ngar i en "ragged" array */
extern char  *txtind[];      /* Pekare till alla textstr�ngar */
extern short  igtrty;        /* Terminaltyp */
extern short  rmarg,bmarg;   /* Sk�rmens storlek */
extern short  mant;          /* Antal aktiva menyer */
extern short  gptnpx,gptnpy; /* Grafisk texts storlek i pixels */
extern short  gpsnpy;        /* Sk�rmens storlek i pixels */
extern char   hstack[][40];
extern char   jobdir[],jobnam[];

static short vtx;           /* vt100-mark�rens aktuella l�ge i x-led */
static short vty;           /* vt100-mark�rens aktuella l�ge i y-led */
static short vtfx=1;        /* f�rsta x-pos i aktivt f�lt */
static short vtfy=1;        /* f�ltets y-l�ge */
static short vtfln=80;      /* f�ltets l�ngd i antal tecken */

/*!******************************************************/

       void igwstr(char *s)

/*      Low level write string. Used by all other
 *      routines to write to stdout or LOG-file.
 *
 *      In: s = Pointer to output string.
 *
 *      (C)microform ab 18/4/87 J. Kjellander
 *
 *      1999-04-23 BATCH, J. Kjellander
 *
 ******************************************************!*/

  {
   char  errfil[V3PTHLEN];
   FILE *lf;

    switch ( igtrty )
      {
      case X11:
      case MSWIN:
      break;

      case BATCH:
      sprintf(errfil,"%s%s.LOG",jobdir,jobnam);
      lf = fopen(errfil,"a");
      fprintf(lf,"%s\n",s);
      fflush(lf);
      fclose(lf);
      break;

      default:
      fputs(s,stdout);
      break;
      }
  }

/*!******************************************************/
/*!******************************************************/

       void igmvac( 
       short x,
       short y)

/*      Flyttar VT100-mark�ren. "koordinaten" (1,1)
 *      �r sk�rmens �vre v�nstra h�rn.
 *
 *      In: x,y => Teckenposition att flytta mark�ren till.
 *
 *      Ut: Inget.
 *
 *      FV: Inget.
 *
 *      (C)microform ab 4/3/85 M. Nelson
 *
 *      3/2/86   gpansi(), R. Svedin
 *      18/4/87  igwstr(), J. Kjellander
 *      21/10/88 CGI, J. Kjellander
 *      3/11/88  N220G, R. Svedin
 *
 ******************************************************!*/

  {

    if ( igtrty == X11   ||
         igtrty == MSWIN ||
         igtrty == BATCH ) return;
/*
***Flytta mark�ren p� sk�rmen.
*/
     if ( igtrty == N220G  ||  igtrty == MSCOLOUR  ||  igtrty == MSMONO )
      {
      gpmvsc((x-1)*gptnpx+2,gpsnpy-2-y*gptnpy);
      gpansi();
      }
/*
***Kom ih�g aktuell mark�rposition.
*/
    vtx = x;
    vty = y;
  }

/*!******************************************************/
/*!******************************************************/

       void igpstr(
       char *s,
       short font)

/*      Skriver ut horisontell textstr�ng vid alfa-mark�ren.
 *
 *      In: s    -  pekare till str�ng
 *          font -  NORMAL (0)
 *               -  UNDLIN (4)
 *               -  REVERS (7)
 *
 *      Ut: Uppdaterar vtx och vty
 *
 *      FV: Inget.
 *
 *      (C)microform ab 21/3/85 M. Nelson
 *
 *      3/2/86   Test av terminalmode R. Svedin
 *      18/4/87  igwstr(), J. Kjellander
 *      26/10/88 CGI, J. Kjellander
 *      3/11/88  N220G, R. Svedin
 *      1999-06-13 Batch, J.Kjellander
 *
 ******************************************************!*/

  {
   char outbuf[180];

/*
***Nokia mfl.
*/
   if ( igtrty == N220G  ||  igtrty == MSCOLOUR  ||  igtrty == MSMONO )
     {
     gpansi();
     if ( igtrty == MSCOLOUR  &&  font == REVERS ) gpspen(4);
     igwstr(s);
     if ( igtrty == MSCOLOUR  &&  font == REVERS ) gpspen(1);
     }
/*
***Batch.
*/
   else if ( igtrty == BATCH ) igwstr(s);
/*
***VT100-terminaler.
*/
   else
     {
     gpansi();
     if ( font != NORMAL )
       {
       sprintf(outbuf,"\033[%dm%s\033[0m",font,s);
       igwstr(outbuf);
       }
     else igwstr(s);
     }

   if ( (vtx+=strlen(s)) > rmarg ) vtx = rmarg;
  }

/*!******************************************************/
/*!******************************************************/

       void igvstr(
       char  *s,
       short font)

/*      Skriver ut vertikal textstr�ng vid VT100-mark�ren.
 *
 *      In: s    -  pekare till str�ng
 *          font -  NORMAL (0)
 *               -  UNDLIN (4)
 *               -  REVERS (7)
 *
 *      Ut:
 *
 *      FV: Inget.
 *
 *      (C)microform ab 21/3/85 M. Nelson
 *
 *      3/2/86  Test av terminalmode R. Svedin
 *      18/4/87 igwstr(), J. Kjellander
 *
 ******************************************************!*/

   {
   register short i;
   char  buf[10];


   if ( igtrty == BATCH )
     {
     igwstr(s);
     return;
     }

   gpansi();

   if ( font != NORMAL )
     {
     sprintf(buf,"\033[%dm",font); /* set font */
     igwstr(buf);
     }

   for ( i=0; s[i] != '\0'; ++i)
     {
     sprintf(buf,"%c\010\033[B",s[i]);
     igwstr(buf);
     }

   if ( font != NORMAL ) igwstr("\033[0m"); /* reset font */
   }

/*!******************************************************/
/*!******************************************************/

       void igdfld(                    /* define field */
       short x,                        /* field first pos. x-dir. */
       short fw,                       /* field width */
       short y)                        /* field line */

/*      Definiera in/utmatningsf�lt - anv�nds tillsammans med
 *      igfstr() f�r utmatning och
 *      igistr() f�r inmatning.
 *
 *           fw  -  positivt - antal tecken i f�ltat
 *               -  negativt - f�ltats sista position
 *               -  0        - illegal no action
 *
 *      med #define kan "f�rdefinierade" f�lt anv�ndas
 *
 *      ex:  #define FIELD1 10,10,20
 *
 *           igdfld(FIELD1);
 *
 *      cursorns l�ge p�verkas ej
 *
 *      FV: Inget.
 *
 *      (C)microform ab 21/3/85 M. Nelson
 *
 ******************************************************!*/

   {
   vtfx=x;
   vtfy=y;

   if ( fw >= 1 ) vtfln = fw;
   else if ( fw <= -1 )
     {
     vtfln = (-fw)-x+1;
     if ( vtfln < 1 ) vtfln=1;                    /* is�fall vtfln = 1 */
     }
   }

/*!******************************************************/
/*!******************************************************/

       void igfstr(                /* put string in field */
       char *s,                    /* str�ng */
       short just,                 /* string just */
       short font)                 /* text font */


/*      Skriver ut textstr�ng i aktuellt f�lt. Tomma positioner
 *      i f�ltet fylls i med blanka. vid behov trunkeras str�ngen.
 *
 *
 *          just -  JULEFT         str�ng v�nsterjusteras
 *               -  JURIGHT        str�ng h�gerjusteras
 *               -  JUMID          str�ng centreras
 *
 *          font -  NORMAL (0)
 *               -  UNDLIN (4)
 *               -  REVERS (7)
 *
 *      Ut: vtx och vty uppdateras
 *
 *      FV: Inget.
 *
 *      (C)microform ab 21/3/85 M. Nelson
 *
 *      REVIDERAD:
 *
 *      18/10-85 Ulf johansson
 *
 ******************************************************!*/

{
   char sbuf[ 80 + MAXTXT + 1] ;  /* lokal arbetskopia av str�ngen */
   short i,slen;
   char *fchar;


   for (i = 0; i <= (rmarg + MAXTXT); i++)   /* blanka hela bufferten */
        sbuf[ i ] = ' ';

   fchar = sbuf + rmarg;          /* first character in output string */
   slen = strlen(s);                /* original string lenght */
   strcpy(fchar,s);                 /* kopiera till lokal buffert */

   *(fchar + slen) = ' ';           /* blanka NULL-term */
   if (just == JULEFT)              /* v�nsterjusterad str�ng !! */
        ;  
   else if(just == JURIGHT)         /* h�gerjusterad str�ng !! */
        fchar += (slen-vtfln);
   else if(just == JUMID)           /* centerjusterad */
        fchar += (slen-vtfln)/2;
   *(fchar + vtfln) = '\0';         /* hugg av str�ng till f�ltl�ngd */

   igmvac(vtfx,vtfy);               /* positionera cursor */
   igpstr(fchar,font);              /* ut med str�ngen */
}

/*!******************************************************/
/*!******************************************************/

       char *iggtts(short tnr)

/*      Returnerar en c-pekare till den med tnr angivna
 *      t-str�ngen.
 *
 *      In: tnr = T-str�ngpekarens index i txtind.
 *
 *      FV: C-pekare till str�ng.
 *
 *      (C)microform ab 1996-05-30 J.Kjellander
 *
 ******************************************************!*/


  {
    char  *tp; 
    static char notdef[80];

/*
***Har tnr ett rimligt v�rde ?
*/
    if ( tnr < 0  ||  tnr >= TXTMAX )
      {
      sprintf(notdef,"<invalid t%d>",tnr);
      return(notdef);
      }
/*
***Fixa fram motsvarande pekare.
*/
    tp = txtind[tnr];
/*
***�r den definierad ?
*/
    if ( tp == NULL )
      {
      sprintf(notdef,"<no t%d>",tnr);
      return(notdef);
      }

    return(tp);
  }

/*!******************************************************/
/*!******************************************************/

       void igerar(
       short x,
       short y)

/*      Suddar area p� vt100-sk�rmen med bredden x tecken och 
 *      h�jden y rader fr.o.m. mark�rens l�ge.
 *
 *      Ut: Inget.
 *
 *      FV: Inget.
 *
 *      (C)microform ab Mats Nelson
 *
 *      3/2/86   Test av terminalmode R. Svedin
 *      18/4/87  igwstr(), J. Kjellander
 *      4/8/87   Hela sk�rmen, J. Kjellander
 *      26/10/88 CGI, J. Kjellander
 *      2/11/88  N220G, R. Svedin
 *      1999-06-13 Batch, J.Kjellander
 *
 ******************************************************!*/

 {
   register short i,ylim;
   char  sbuf[4*MAXTXT+1];

/*
***Initiering.
*/
   ylim = vty+y;
/*
***N220G.
*/
   if ( igtrty == N220G )
     {
     for ( i=0; i<x; ++i ) sbuf[i]=' '; sbuf[i] = '\0';
     for ( i=vty; i<ylim; ++i)
       {
       igmvac(vtx,i);
       igwstr("\033/1d");
       igwstr(sbuf);
       }
     igwstr("\033/0d");
     }
/*
***MSKERMIT.
*/
   else if ( igtrty == MSCOLOUR  ||  igtrty == MSMONO )
     {
     gpspen(0);
     for ( i=0; i<x; ++i ) sbuf[i]=' '; sbuf[i] = '\0';
     for ( i=vty; i<ylim; ++i)
       {
       igmvac(vtx,i);
       igwstr(sbuf);
       }
     gpspen(1);
     }
/*
***Batch.
*/
   else if ( igtrty == BATCH ) return;
/*
***VT100.
*/
   else
     {
     gpansi();
/*
***Hela sk�rmen.
*/
     if ( vtx==1  &&  vty==1  &&  x==rmarg  &&  y==bmarg ) igersc();
/*
***En eller flera hela rader.
*/
     else if ( vtx == 1  &&  x == rmarg )
       for ( i=vty; i<ylim; ++i) igwstr("\033[2K\033[B");
/*
***Fr�n b�rjan av raden.
*/
     else if (vtx == 1)
       {
       igmvac(x,vty);
       for ( i=vty; i<ylim; ++i) igwstr("\033[1K\033[B");
       }
/*
***Till slutet av raden.
*/
     else if (x == rmarg)
       for ( i=vty; i<ylim; ++i) igwstr("\033[0K\033[B");
/*
***Inne i en rad.
*/
     else
       {
       for ( i=0; i<x; ++i ) sbuf[i]=' '; sbuf[i] = '\0';
       for ( i=vty; i<ylim; ++i)
         {
         igmvac(vtx,i);
         igwstr(sbuf);
         }
       }
    }
  }

/*!******************************************************/
/*!******************************************************/

       void igersc()

/*      Suddar hela VT100-sk�rmen. 
 *
 *      Ut: Inget.
 *
 *      FV: Inget.
 *
 *      (C)microform ab J. Kjellander
 *
 ******************************************************!*/

  {
    if ( igtrty != BATCH ) igwstr("\033[2J");
  }

/*!******************************************************/
/*!******************************************************/

       void igflsh()

/*     T�mmer stdout-buffert
 *
 *     (C)microform ab Mats Nelson
 *
 *     31/10/88 CGI, J. Kjellander
 *
 ******************************************************!*/

  {
  if ( igtrty != BATCH ) fflush(stdout);
  }

/*!******************************************************/
/*!******************************************************/

       void igbell()

/*     S�ger pip!
 *
 *      (C)microform ab J. Kjellander
 *
 *      1998-03-13 Bug x11->X11, J.Kjellander
 *
 ******************************************************!*/

  {

#ifdef V3_X11
    if ( igtrty == X11 ) XBell(xdisp,100);
    else
#endif
     {
     if ( igtrty != BATCH )
       {
       igwstr("\007");
       igflsh();
       }
     }
  }

/*!******************************************************/
/*!******************************************************/

       short iggtsm(
       char   *cp,
       MNUALT **altptr)

/*     L�ser symbol fr�n standard input.
 *
 *      In: cp     = Pekare till utdata.
 *          altptr = Pekare till utdata.
 *
 *      Ut: *altptr = Ev. menyalternativ-pekare.
 *          *cp     = Sist inmatade tecken
 *
 *      FV:
 *          SMBNONE    - ok�nd symbol
 *          SMBCHAR    - ett tecken
 *          SMBRETURN  - return
 *          SMBBACKSP  - backspace
 *          SMBLEFT    - �t v�nster
 *          SMBRIGHT   - �t h�ger
 *          SMBUP      - upp�t
 *          SMBDOWN    - ned�t
 *          SMBESCAPE  - escape
 *          SMBMAIN    - huvudmenyn
 *          SMBHELP    - hj�lp
 *          SMBPOSM    - positionsmenyn
 *          SMBALT     - menyalternativ
 *
 *
 *      (C)microform ab 16/10-85 Ulf Johansson
 *
 *      29/10-85 Bug, Ulf johansson
 *      8/10/86  SMBMAIN, J, Kjellander
 *      13/10/86 SMBHELP, J, Kjellander
 *      8/11/88  snabb, J. Kjellander
 *      15/11/88 SMBPOSM J. Kjellander
 *      15/7/92  X11, J. Kjellander
 *      1/11/95  WIN32, J. Kjellander
 *
 ******************************************************!*/

{
   short   i,j;
   char    tbuf[MAXTXT+1],cbuf[MAXTXT+1];
   short   symbol;
   char    c;
   bool    hit,exit;

/*
***H�r b�rjar det hela.
*/     
   symbol = SMBNONE;
   i = 0;
   exit = FALSE;

   do
     {
/*
***Med X11/WIN32 anv�nder vi XXgtch() som returnerar ett tecken eller
***en symbol/menyalternativ. Om tecken returneras g�r vi som vanligt.
***Om menyval returneras avslutar vi och returnerar menyvalet.
***Om REJECT returneras avslutar vi och returnerar dito.
*/
#ifdef V3_X11
     if ( igtrty == X11 )
       {
       c = wpgtch(altptr,&symbol,TRUE);
       if      ( symbol == SMBCHAR )   goto tecken;
       else if ( symbol == SMBALT )    return(SMBALT);
       else if ( symbol == SMBRETURN ) return(SMBRETURN);
       else if ( symbol == SMBUP )     return(SMBUP);
       else if ( symbol == SMBMAIN )   return(SMBMAIN);
       }
#endif
#ifdef WIN32
     c = msgtch(altptr,&symbol,TRUE);
     if      ( symbol == SMBCHAR )   goto tecken;
     else if ( symbol == SMBALT )    return(SMBALT);
     else if ( symbol == SMBRETURN ) return(SMBRETURN);
     else if ( symbol == SMBUP )     return(SMBUP);
     else if ( symbol == SMBMAIN )   return(SMBMAIN);
#endif
/*
***Utan X11/WIN32 anv�nder vi gamla iggtch() som bara
***returnerar ett tecken.
*/
     c = iggtch();                             /* L�s 1 tecken */     
tecken:
     cbuf[i++] = c;                            /* Buffra tecknet */
     if ( i >= MAXTXT ) break;
     cbuf[i] = '\0';                           /* null terminate */
/*
***�r det fr�gan om ett snabbval ?
*/
     hit = FALSE;
     for ( j=0; j < SMBMAX; j++ )
       {
       if ( smbind[j].str != NULL )
         {
         if ( strncmp(smbind[j].str,cbuf,i) == 0 )
           {
           hit = TRUE;
           break;
           }
         }
       }
/*
***Ja det �r ett snabbval.
*/
     if ( hit )
       {
       if ( strlen(smbind[j].str) == i )
         {
/*
***�r det snabbval av typen ALT, dvs. CR, BS etc.
*/
         if ( smbind[j].acttyp == ALT )
           {
           symbol = smbind[j].actnum;

           switch (symbol)
             {
             case SMBRETURN:       /* return */
             case SMBBACKSP:       /* backspace */
             case SMBLEFT:         /* �t v�nster */
             case SMBRIGHT:        /* �t h�ger */
             case SMBUP:           /* upp�t */
             case SMBDOWN:         /* ned�t */
             case SMBHELP:         /* hj�lp */
             case SMBMAIN:         /* huvudmenyn */
             case SMBPOSM:         /* pos-menyn */
             break;

             default:
             symbol = SMBNONE;     /* ok�nd symbol */
             break;
             }
           }
/*
***Snabbval av typen funktionstangent, dvs. ESC-sekvens.
*/
         else
           {
           symbol = SMBESCAPE;
           strcpy(tbuf,igqema());
           if ( *tbuf != '\0' ) igrsma();
           strcpy(hstack[mant],iggtts(50));
           if ( igdofu(smbind[j].acttyp,smbind[j].actnum) == GOMAIN )
              symbol = SMBMAIN;
           if ( *tbuf != '\0' ) igplma(tbuf,IG_MESS);
           }
         exit = TRUE;
         }
       }
/*
***Nej ingen tr�ff.
*/
     else
       {
       if ( i <= 1 )
         if ( (c >= ' ') && (c <= '~') ) symbol = SMBCHAR;
           exit = TRUE;
       }

     } while (!exit);

  *cp = c;
   return(symbol);
}

/*!******************************************************/
/*!******************************************************/

       short iglned(
       char  *s,
       short *cursor,
       short *scroll,
       bool  *dfuse,
       char  *ds,
       short maxlen,
       short font,
       bool  snabb)

/*      L�ser in textstr�ng med editering.
 *
 *      Startstr�ngens v�rde matas ut i inmatningsf�nstret.
 *      Om anv�ndaren b�rjar mata in tecken f�rsvinner startv�rdet.
 *      Om anv tar bort alla tecken matas defaultstr�ngen ut.
 *
 *      IN:
 *        s        - Startstr�ng / inf�lt
 *        cursor   - Mark�rens position i inf�ltet.
 *        scroll   - Inf�nstrets position relativt inf�ltet.
 *        dfuse    - Default-s�kring
 *        ds       - Default str�ng
 *        maxlen   - Str�ngens maximala l�ngd
 *        font     - Textfont
 *                    - NORMAL (0)
 *                    - UNDLIN (4)
 *                    - REVERS (7)
 *        snabb    - Snabbval till�tet ja/nej
 *
 *      UT:
 *        s        - Inl�st str�ng
 *        cursor   - Mark�rens position i inf�ltet.
 *        scroll   - Inf�nstrets position relativt inf�ltet.
 *        dfuse    - Default-s�kring
 *
 *      FV:
 *          SMBRETURN  - return
 *          SMBESCAPE  - escape
 *          SMBUP      - upp�t
 *          SMBMAIN    - huvudmenyn
 *          SMBHELP    - hj�lp
 *          SMBPOSM    - positionsmenyn
 *
 *
 *      (C)microform ab 25/9-85 Ulf Johansson
 *
 *
 *      REVIDERAD:
 *
 *      28/10-85 Bug, Ulf johansson
 *      6/10/86  SMBMAIN, J. Kjellander
 *      13/10/86 SMBHELP, J. Kjellander
 *      3/11/88  N220G, R. Svedin
 *      8/11/88  snabb, J. Kjellander
 *      15/11/88 SMBPOSM, J. Kjellander
 *      10/9/91  CGI-cursor, J. Kjellander
 *      20/8/92  X11, J. Kjellander
 *
 ******************************************************!*/


{
   char sbuf[ MAXTXT + 1 ];      /* lokal arbetskopia av str�ngen */
   char c;                       /* inl�st tecken */
   bool exit,fuse;
   short cpos,cscr,cnum; 
   short i,symbol;
   MNUALT *altptr;

   strcpy(sbuf,s);                           /* kopiera startstr�ngen */
   cpos = *cursor;                           /* mark�rens position */
   cscr = *scroll;                           /* f�nstrets position */
   fuse = *dfuse;                            /* default fuse */


   cnum = strlen(sbuf);

   exit = FALSE;
   do { 

/*
***Scrollning.
*/
        if ( cpos > cscr+vtfln ) cscr++; 
        else if ( cpos < cscr ) cscr--;
/*
***N220G.
*/
        if ( igtrty == N220G )
          {
          gpansi();
          igwstr("\033/1d");
          igfstr(&sbuf[ cscr ],JULEFT,font);      /* erase field */
          igmvac(vtfx + cpos - cscr,vtfy);        /* cursor position */
          gpansi();
          igwstr("\033/0d");
          }
/*
***Skriv ut str�ngen. X11 anv�nder winpac-rutin �vriga sk�rmar
***anv�nder den gamla teckenbaserade igfstr().
*/
#ifdef V3_X11
        if ( igtrty == X11 )
          ; /*wpfstr(&sbuf�cscr�,cpos); */
        else
#endif
          igfstr(&sbuf[cscr],JULEFT,font);
/*
***Positionera cursorn.
*/
        if ( igtrty != X11 )
          {
          igmvac(vtfx + cpos - cscr,vtfy);
          igflsh();
          }
/*
***L�s tecken fr�n tangentbordet.
*/
        switch (symbol = iggtsm(&c,&altptr))
          {
          case SMBCHAR:                  /* insert */
          if ( !fuse && (cnum>=maxlen) ) igbell();
          else
            {          
            if ( fuse )
              {          /* ta bort default */
              *sbuf = '\0';
              cnum = 0;
              fuse = FALSE;
              }
            for ( i=cnum++; i>=cpos; i--) sbuf[i+1] = sbuf[i];
            sbuf[cpos++] = c;
            }
          break;

           case SMBBACKSP:
           if ( cpos > 0 )
             {
             for ( i= --cpos; i<cnum; i++) sbuf[i] = sbuf[i+1];
             cnum--;
             }
           else if ( cnum > 0 )
             {
             for ( i=cpos; i<cnum; i++) sbuf[i] = sbuf[i+1];
             cnum--;
             }
           else
             {
             strcpy(sbuf,ds);  /* kopiera defaultstr�ngen */
             cnum = strlen(sbuf);
             if ( cnum == 0 ) igbell();
             }
           fuse = FALSE;
           break;

           case SMBLEFT:              /* �t v�nster */
           if ( cpos > 0 ) cpos--;
           else igbell();
           fuse = FALSE;
           break;

           case SMBRIGHT:             /* �t h�ger */
           if ( cpos < cnum ) cpos++;
           else igbell();
           fuse = FALSE;
           break;

           case SMBRETURN:                    /* return */
           case SMBUP:                        /* upp�t */
           case SMBDOWN:                      /* ned�t */
           case SMBESCAPE:                    /* escape */
           case SMBMAIN:                      /* huvudmenyn */
           case SMBHELP:                      /* hj�lp */
           case SMBPOSM:                      /* pos-menyn */
           exit = TRUE;
           break;

           default:                  /* ok�nd eller ej till�ten symbol */
           igbell();
           break;
         }

   } while (!exit);

   strcpy(s,sbuf);                           /* kopiera res till anropare */

   *cursor = cpos;                           /* mark�rens position */
   *scroll = cscr;                           /* f�nstrets pos */
   *dfuse = fuse;
/*
***Slut.
*/
   return(symbol);
}
/*!******************************************************/
/*!******************************************************/

       short igglin(
       char  *pmt,
       char  *dstr,
       short *ntkn,
       char  *istr)

/*     L�ser str�ng fr�n stdin med promt, defaultv�rde, hj�lp
 *     och editering men utan snabbval.
 *
 *     In: pmt   = Pekare till promttext.
 *         dstr  = Pekare till defaultstr�ng.
 *         ntkn  = Pekare till max antal tecken.
 *         istr  = Pekare till resultat.
 *
 *     Ut: *istr = Nollterminerad str�ng.
 *         *ntkn = Antal tecken l�sta.
 *
 *     FV: 0
 *
 *     (C)microform ab 7/11/86 J. Kjellander
 *
 *     28/4/87  Bug, J. Kjellander
 *     8/11/88  Anropar nu iglned(), J. Kjellander
 *     7/8/91   SMBMAIN, J. Kjellander
 *     27/1/92  status, J. Kjellander
 *     1996-03-08 WIN32, J.Kjelander
 *
 ******************************************************!*/

{
   int   typarr[1];
   char  is[MAXTXT+1],*isadr=is;
   char  ds[MAXTXT+1],*dsadr=ds;
   short symbol,status,cursor,scroll,ix,iy,pl,fwdth=0;
   bool  dfuse;
   
/*
***Initiering.
*/
   typarr[0] = C_STR_VA;
   is[0] = '\0';
   strcpy(ds,dstr);
   strcpy(is,dstr);
   cursor = scroll = 0;
   dfuse = TRUE;
   ix = vtx; iy = vty;
   if ( (pl=strlen(pmt)) > 0 ) ++pl;
/*
***Om det �r X11 anv�nder vi tills vidare XXmsip().
***Denna returnerar status typ REJECT och GOMAIN,
***ej symboler.
*/
#ifdef V3_X11
    if ( igtrty == X11 )
      {
      symbol = wpmsip(igqema(),&pmt,&dsadr,&isadr,ntkn,typarr,(short)1);
      if ( symbol == REJECT ) symbol = SMBUP;
      if ( symbol == GOMAIN ) symbol = SMBMAIN;
      goto slut;
      }
#endif
/*
***Med WIN32 anv�nder vi igssip() som i sin tur anv�nder
***msmsip() som anv�nder den ordinarie WIN32-dialogboxen.
*/
#ifdef WIN32
    symbol = igssip(pmt,isadr,dsadr,*ntkn);
    if ( symbol == REJECT ) symbol = SMBUP;
    if ( symbol == GOMAIN ) symbol = SMBMAIN;
    goto slut;
#endif 
/*
***Inmatningsf�ltets storlek.
*/
   if ( (fwdth=rmarg-ix-pl) > *ntkn ) fwdth = *ntkn;
/*
***Ev. promt.
*/
start:
   if ( igtrty == CGI  ||  igtrty == N220G )
     {
     igmvac(ix,iy);
     igerar(fwdth+pl,1);
     }

   igmvac(ix,iy);
   igpstr(pmt,REVERS);
   if ( pl > 0 ) igpstr(" ",NORMAL);
/*
***L�s in svar med iglned.
*/
   igdfld(ix+pl,fwdth,iy);
   symbol = iglned(is,&cursor,&scroll,&dfuse,ds,*ntkn,NORMAL,FALSE);

slut:
   switch ( symbol )
     {
     case SMBHELP:
     ighelp();
     strcpy(ds,is);
     goto start;

     case SMBUP:
     *ntkn = 0;
     is[0] = '\0';
     status = REJECT;
     break;

     case SMBMAIN:
     *ntkn = 0;
     is[0] = '\0';
     status = GOMAIN;
     break;

     default:
     status = 0;
     break;
     }

   *ntkn = strlen(is);
   strcpy(istr,is);
/*
***Sudda raden igen.
*/
   if ( igtrty != X11  &&  igtrty != MSWIN )
     {
     igmvac(ix,iy);
     igerar(fwdth+pl,1);
     }

   return(status);
}

/*!******************************************************/
/*!******************************************************/

       char iggtch()          

/*      L�ser in ett tecken fr�n standard input.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: Tecknet.
 *
 *      (C)microform ab 3/2/86 R. Svedin
 *
 *      13/2/86  Villkorlig kompilering R. Svedin
 *      18/4/87  BATCH, J. Kjellander
 *      21/10/88 CGI, J. Kjellander
 *      12/3/91  intrup, J. Kjellander
 *      16/7/92  X11, J. Kjellander
 *
 ******************************************************!*/

 {
/*
***L�s ett tecken, olika beroende p� milj�.
*/
   char  c;
   short alttyp;
   extern bool intrup;

/*
***B�rja med att s�tta intrup-flaggan till FALSE.
***Operat�ren kan ha tryckt p� <CTL>c sen sist.
***Genom att centralt s�tta intrup till FALSE h�r 
***�r det ingen risk att den �r TRUE n�r en tids�dande
***operation startar.
*/
   intrup = FALSE;
/*
***Oavsett OS m�ste vi b�rja med att kolla om det �r X11.
***Med X11 anv�nder vi wpgtch() och FALSE som inneb�r att
***bara tecken �nskas, inte andra typer av symboler.
*/
#ifdef V3_X11
     if ( igtrty == X11 )
       {
       c = wpgtch(NULL,&alttyp,FALSE);
       return(c);
       }
#endif

#ifdef UNIX
   c = getchar();
#endif
/*
***Oavsett operativsystem etc. skall alltid
***<CR> mappas till UNIX-newline.
*/
   if ( c == 13 ) c = '\n';

   return(c);

 }

/*!******************************************************/
/*!******************************************************/

       short iggtds(char digbuf[])

/*      L�ser str�ng fr�n digitizerport.
 *
 *      In:  digbuf = Pekare till resultat med plats
 *                    f�r upp till 80 tecken.
 *
 *      Ut:  *digbuf = L�st str�ng, "" om digitizer saknas.
 *
 *      (C)microform ab 14/3/88 J. Kjellander
 *
 *      5/12/88  Pollning, J. Kjellander
 *      30/5/94  �ndrade ifdef, J. Kjellander
 *
 ******************************************************!*/

  {
#ifdef SCO_UNIX
   char    c;
   short   ntkn,kbstat,dgstat;
   MNUALT *altptr;
#endif

/*
***Denna rutin fungerar endast under XENIX eller SCO/UNIX.
***�vriga implementeringar returnerar tom str�ng.
*/
   *digbuf = '\0';
/*
***I XENIX och SCO/UNIX anv�nder vi tv� icke standardiserade
***rutiner f�r att l�sa fr�n digitizer-porten. rdchk() och ioctl().
***I �vriga versioner av systemet st�ds digitizer ej.
*/
#ifdef SCO_UNIX
   if ( digdes.def )
     {
/*
***T�m inputbuffrar.
*/
     ioctl(0,TCFLSH,0);
     ioctl(digdes.fd,TCFLSH,0);
/*
***Polla b�da portarna.
*/
poll:
     kbstat = rdchk(0);
     dgstat = rdchk(digdes.fd);

     if ( kbstat == 1 )
       {
       switch ( iggtsm(&c,&altptr,TRUE) )
         {
         case SMBRETURN:
         return(REJECT);

         case SMBMAIN:
         return(GOMAIN);

         case SMBPOSM:
         return(POSMEN);

         default:
         goto poll;
         }
       }
     if ( dgstat == 1 )
       {
       ntkn = 0;
loop:
       read(digdes.fd,&c,1);
       if ( c == 13 || ntkn == 80 ) digbuf[ntkn] = '\0';
       else
         {
         digbuf[ntkn] = c;
         ntkn++;
         goto loop;
         }
       }
     else goto poll;
     }
/*
***Ingen digitizer aktiv.
*/
   else *digbuf = '\0';

#endif

      return(0);
  }

/*!******************************************************/
