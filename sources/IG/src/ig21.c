/*!******************************************************************/
/*  File: ig21.c                                                    */
/*  ============                                                    */
/*                                                                  */
/*  This file includes:                                             */
/*                                                                  */
/*  igssip();   Single string input                                 */
/*  igsfip();   Single float value input                            */
/*  igsiip();   Single int value input                              */
/*  igmsip();   Multiple string input                               */
/*  igialt();   Input alternative with t-strings                    */
/*  igials();   Input alternative with C-strings                    */
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
#include "../../WP/include/WP.h"
#include "../include/screen.h"

extern short igtrty,rmarg,ialy,ialx;

/*!*******************************************************/

     short igssip(
     char *ps,                     /* promptstr�ng */
     char *is,                     /* inputstr�ng */
     char *ds,                     /* defaultstr�ng */
     short ml)                     /* maximal str�ngl�ngd */

/*   L�s in en str�ng med redigering och snabbval.
 *
 *   FV:       0 = OK
 *        REJECT = Operationen avbruten.
 *        GOMAIN = �terv�nd till huvudmenyn.
 *
 *   REVIDERAD:
 *
 *   16/9-85 Snabbval, Ulf Johansson
 *   6/10/86 GOMAIN, J. Kjellander
 *
 *******************************************************!*/

{
    return(igmsip(&ps,&is,&ds,&ml,1));
}

/*********************************************************/
/*!*******************************************************/

     short igsfip(
     char    *ps,                   /* promptstr�ng */
     DBfloat *fval)                 /* flyttalet */

/*   L�s in ett flyttal med redigering och snabbval.
 *
 *   FV:       0 = OK
 *        REJECT = Operationen avbruten.
 *        GOMAIN = �terv�nd till huvudmenyn.
 *
 *   REVIDERAD:
 *
 *   16/9-85 Snabbval, Ulf Johansson
 *   6/10/86 GOMAIN, J. Kjellander
 *
 *******************************************************!*/

{
    char ch[ 1 + 1 ];
    char is[ MAXDTXT + 1 ];
    short retsmb;

    do
      {
      if ( (retsmb = igssip(ps,is,"",MAXDTXT)) == REJECT ||
              retsmb == GOMAIN || retsmb == POSMEN ) break;

      is[strlen(is)] = '\b';
      if ( sscanf(is,"%lf%1s",fval,ch) == 2 )
        {
        if (*ch == '\b') break;
        }

      else igbell();
    } while (TRUE);

    return(retsmb);
}

/*********************************************************/
/*!*******************************************************/

    short igsiip(
    char  *ps,                              /* promptstr�ng */
    int *ival)                              /* heltalet */

/*   L�s in ett heltal med redigering och snabbval.
 *
 *
 *   FV:       0 = OK
 *        REJECT = Operationen avbruten.
 *        GOMAIN = �terv�nd till huvudmenyn.
 *
 *   REVIDERAD:
 *
 *   16/9-85 Snabbval, Ulf Johansson
 *   6/10/86 GOMAIN, J. Kjellander
 *
 *******************************************************!*/

{
    char ch[ 1 + 1 ];
    char is[ MAXLITXT + 1 ];
    short retsmb;

    do
      {
      if ( (retsmb=igssip(ps,is,"",MAXLITXT)) == REJECT ||
              retsmb == GOMAIN || retsmb == POSMEN ) break;

      is[strlen(is)] = '\b';
      if ( sscanf(is,"%d%1s",ival,ch) == 2 )
        {
        if ( *ch == '\b' ) break;
        }

      else igbell();
      } while (TRUE);

    return(retsmb);
}

/*********************************************************/
/*!*******************************************************/

     short igmsip(
     char *ps[],                   /* promptstr�ngar */
     char *is[],                   /* inputstr�ngar */
     char *ds[],                   /* defaultstr�ngar */
     short ml[],                   /* maximala str�ngl�ngder */
     short as)                     /* antal str�ngar att l�sa in */

/*   L�s in flera str�ngar med redigering och snabbval.
 *
 *   FV:       0 = OK
 *        REJECT = Operationen avbruten.
 *        GOMAIN = �terv�nd till huvudmenyn.
 *
 *   16/9-85  Snabbval, Ulf Johansson
 *   6/10/86  GOMAIN, J. Kjellander
 *   13/10/86 SMBMAIN, J. Kjellander
 *   8/11/88  snabb till getsmb(), J. Kjellander
 *   1996-12-12 typarr, J.Kjellander
 *
 *******************************************************!*/
{
    int   typarr[V2MPMX];
    short prx[ MSMAX ];                     /* promptl�gen */
    short isx[ MSMAX ];                     /* inputf�ltl�gen */
    short fwdt[ MSMAX ];                    /* f�ltl�ngder */
    short ilen,plen;
    short tab,air,retsmb=0;
    short cursor[ MSMAX ];                  /* cursor pos */
    short scroll[ MSMAX ];                  /* f�nster pos */
    bool dfuse[ MSMAX ];                    /* defaults�kringar */
    short i,j;

/*
***X11.
*/
#ifdef V3_X11
    if ( igtrty == X11 )
      {
      for ( i=0; i<as; ++i ) typarr[i] = C_STR_VA;
      return(wpmsip(igqema(),ps,ds,is,ml,typarr,as));
      }
#endif
/*
***WIN32
*/
#ifdef WIN32
    for ( i=0; i<as; ++i ) typarr[i] = C_STR_VA;
    if ( as < 4 )return((short)msdl03(igqema(),ps,ds,is,ml,(int)as));
    else         return((short)msmsip(igqema(),ps,ds,is,ml,typarr,as));
#endif
/*
***G�r inmatning.
*/
    if ( as < 1 ) as = 1;
    else if ( as > MSMAX ) as = MSMAX;

    air = 1;                                   /* luft mellan f�lten */
    tab = ((rmarg-ialx)/as) - (air*(as - 1));  /* max utrymme per f�lt */

    j = ialx;                                /* aktuellt x-l�ge */

    for ( i=0; i < as; i++ )
      {
      prx[i] = j;                            /* promtens l�ge */
      plen = strlen(ps[i]) + 1;              /* promtl�ngd */
      isx[i] = j + plen;                     /* inf�ltets l�ge */
      ilen = (tab - 1) - plen;               /* inf�ltets max l�ngd */
      if (ilen > ml[i])                      /* beh�vs inte hela f�ltet ? */
           ilen = ml[i];
      fwdt[i] = ilen;                        /* inf�ltets l�ngd */
      j = isx[i] + fwdt[i] + 1 + air;        /* n�sta promt */ 
      }
/*
***Promt och defaultv�rde.
*/
    for (i = 0; i < as; i++)
      {
      igmvac(prx[ i ],ialy);     
      igpstr(ps[ i ],REVERS);           /* ut med prompten */

      igdfld(isx[ i ],fwdt[ i ],ialy);
      strcpy(is[ i ],ds[ i ]);            /* default -> inf�ltet */  
      dfuse[ i ] = TRUE;
      cursor[ i ] = 0;
      scroll[ i ] = 0;
      igfstr(is[ i ],JULEFT,NORMAL);      /* inf�ltet */ 
      }

    j = 0;
    while ((j >= 0) && (j < as)) {

        retsmb = 0;
        igdfld(isx[ j ],fwdt[ j ],ialy);
/*
***L�s in str�ng med redigering och snabbval.
*/
        switch (iglned(is[ j ],&cursor[ j ],&scroll[ j ],
                        &dfuse[ j ],ds[ j ],ml[ j ],NORMAL,TRUE)) {

             case SMBRETURN:
             case SMBDOWN:
                  if (strlen(is[ j ]) == 0) {
                       j--;
                       retsmb = REJECT;
                  } else
                       j++;
                  break;

             case SMBMAIN:
                  retsmb = GOMAIN;
                  goto end;

             case SMBPOSM:
                  retsmb = POSMEN;
                  goto end;

             case SMBUP:
                  j--;
                  retsmb = REJECT;
                  break;
/*
***Hj�lp.
*/
             case SMBHELP:
                  if ( ighelp() == GOMAIN )
                    {
                    retsmb = GOMAIN;
                    goto end;
                    }
/*
***Snabbval och hj�lp, �terst�ll inmatningsraden.
*/
             case SMBESCAPE:
                  igmvac(ialx,ialy);
                  igerar(rmarg,1);
                  for (i = 0; i < as; i++)
                     {
                     igmvac(prx[ i ],ialy);     
                     igpstr(ps[ i ],REVERS);
                     igdfld(isx[ i ],fwdt[ i ],ialy);
                     igfstr((is[ i ] + scroll[ i ]),JULEFT,NORMAL);
                     }
                  break;
        }
    }

/*
***Avsluta.
*/
end:
    igmvac(ialx,ialy);
    igerar(rmarg,1);

    return(retsmb);
}

/**************************************************************/
/*!*******************************************************/

     bool igialt(
     short psnum,
     short tsnum,
     short fsnum,
     bool  pip)

/*   L�ser in alternativ (1 tecken) typ j/n, +/- etc.
 *
 *   In: psnum = Promptstr�ngnummer
 *       tsnum = TRUE-str�ngnummer
 *       fsnum = FALSE-str�ngnummer
 *       pip   = Inledande pip, Ja/Nej
 *
 *   Ut: Inget.
 *
 *   FV: TRUE  om svar = 1:a tecknet i tsnum
 *       FALSE om svar = 1:a tecknet i fsnum
 *
 *   (C)microform ab 15/3/86 J. Kjellander
 *
 *   23/10/86   pip, J. Kjellander
 *   1998-09-17 igials(), J.Kjellander
 *
 *******************************************************!*/

 {
    if ( psnum > 0 )
      return(igials(iggtts(psnum),iggtts(tsnum),iggtts(fsnum),pip));
    else
      return(igials("",iggtts(tsnum),iggtts(fsnum),pip));
 }

/*********************************************************/
/*!*******************************************************/

     bool igials(
     char *ps,
     char *ts,
     char *fs,
     bool  pip)

/*   Samma som igialt men med C-str�nga som indata.
 *
 *   In: ps  = Prompt
 *       ts  = TRUE-str�ng
 *       fs  = FALSE-str�ng
 *       pip = Inledande pip, Ja/Nej
 *
 *   Ut: Inget.
 *
 *   FV: TRUE  om svar = 1:a tecknet i tsnum
 *       FALSE om svar = 1:a tecknet i fsnum
 *
 *   (C)microform ab 1998-09-17 J. Kjellander
 *
 *******************************************************!*/

 {
    char c;
/*
***X11.
*/
#ifdef V3_X11
      if ( igtrty == X11 )
        return(wpialt(ps,ts,fs,pip));
#endif
/*
***WIN32.
*/
#ifdef WIN32
      return(msialt(ps,ts,fs,pip));
#endif
/*
***Ev. ett litet pip.
*/
    if ( pip ) igbell();
/*
***Ev. prompt.
*/
loop:
    if ( strlen(ps) > 0 ) igplma(ps,IG_INP);
/*
***L�s in svar, max ett tecken och j�mf�r med ts och fs.
*/
    c = iggtch();

    if ( strlen(ps) > 0 ) igrsma();

    if ( c == *ts ) return(TRUE);
    else if ( c == *fs ) return(FALSE);
    else
     {
     igbell();
     goto loop;
     }
 }

/*********************************************************/
