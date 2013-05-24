/*!******************************************************************/
/*  File: ig34.c                                                    */
/*  ============                                                    */
/*                                                                  */
/*  This file includes:                                             */
/*                                                                  */
/*  igedst();    Edit MBS statement                                 */
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
#include "../../AN/include/AN.h"
#include "../../EX/include/EX.h"
#include "../../GP/include/GP.h"
#include <string.h>

extern short  modtyp,ialx,ialy;
extern DBptr  lsysla;
extern char   actcnm[];
extern struct ANSYREC sy;

/*!******************************************************/

        short igedst()

/*      Varkonfunktion f�r att editera en geometri-
 *      eller part-sats p� MBS-format.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: 0 = OK, REJECT = avsluta, GOMAIN = huvudmenyn
 *
 *      (C)microform ab 22/1/92 J. Kjellander
 *
 ******************************************************!*/

  {
    char     oldstr[V3STRLEN+1],newstr[V3STRLEN+1];
    char     oldpar[V3STRLEN+1],newpar[V3STRLEN+1];
    char     promt[V3STRLEN+1];

    short    status,nid,i,ntkn;
    DBetype  typv[1];
    DBptr    csyla;
    pm_ptr   slstla,lstla,nextla,statla,retla;
    PMMONO  *np;
    DBId     idmat[1][MXINIV];
    ANFSET   set;
    GMCSY    csy;
    V2NAPA   oldnap;

/*
***H�mta storhetens id.
*/
    nid = 1; igptma(268,IG_MESS); typv[0] = ALLTYP;
    status = getmid(idmat,typv,&nid);
    igrsma();
    if ( status < 0 )
      {
      gphgal(0);
      return(status);
      }
    else
      {
      idmat[0][0].ord_val = 1;
      idmat[0][0].p_nextre = NULL;
      }
/*
***Ta reda p� det koordinatsystem
***som var aktivt n�r den skapades.
*/
    status = EXgatt((DBId *)idmat,&oldnap,&csyla);
    if ( status < 0 ) goto exit;
/*
***Om csyla =  DBNULL var inget lokalt system aktivt.
***Om lsysla = DBNULL �r inget lokalt system aktivt.
***J�mf�r dom med varandra och meddela vilket som g�ller
***under �ndringen.
*/
    strcpy(promt,iggtts(116));

    if ( csyla != lsysla )
      {
      if ( csyla != DBNULL )
        {
        DBread_csys(&csy,NULL,csyla);
        strcat(promt,csy.name_pl);
        }
      else strcat(promt,iggtts(223));
      }
    else strcat(promt,actcnm);

    igplma(promt,IG_INP);
/*
***Var i PM ligger satsen ? F�rst en C-pekare till aktiv modul.
***Sen en PM-pekare till list-noden f�r utpekad sats.
***Sen en PM-pekare till sj�lva satsen.
*/
    np = (PMMONO *)pmgadr((pm_ptr)0);
    slstla = np->pstl_;
    if ( (status=pmlges((DBId *)idmat,&slstla,&lstla)) < 0 ) goto exit;
    if ( lstla == (pm_ptr)NULL ) goto exit;
    if ( (status=pmglin(lstla,&nextla,&statla)) < 0 ) goto exit;
/*
***De-kompilera dito till en str�ng.
*/
    if ( (status=pprsts(statla,modtyp,oldstr,V3STRLEN)) < 0 ) goto exit;
/*
***Klipp ut allt fram till 1:a parametern och g�r det
***till promt, resten blir default-v�rde.
*/
edit:
    for ( i=0; i<(int)strlen(oldstr); ++i ) if ( oldstr[i] == ',' ) break;
    i++;

    strcpy(promt,oldstr); promt[i] = '\0';
    strcpy(oldpar,oldstr+i);
/*
***L�t anv�ndaren editera.
*/
    ntkn = V3STRLEN;
    igmvac(ialx,ialy);
    status = igglin(promt,oldpar,&ntkn,newpar);
    if ( status < 0 )
      {
      igrsma();
      gphgal(0);
      return(status);
      }
/*
***Kopiera ihop promt och newpar till newstr.
*/
   strcpy(newstr,promt);
   strcat(newstr,newpar);
/*
***Notera aktuellt l�ge i PM och initiera scannern.
***Skapa tomt set.
***H�mta f�rsta token. anascan() returnerar ingen status.
***Analysera. anunst() �r en void.
***St�ng scannern.
*/
    pmmark(); anlogi();
    if ( (status=asinit(newstr,ANRDSTR)) < 0 ) goto exit;
    ancset(&set,NULL,0,0,0,0,0,0,0,0,0);
    anascan(&sy);
    anunst(&retla,&set);
    if ( (status=asexit()) < 0 ) goto exit;
/*
***Blev det n�gra fel ?
*/
    if ( anyerr() )
      {
      pmrele();
      erpush("IG3892","");
      errmes();
      strcpy(oldstr,newstr);
      goto edit;
      }
/*
***Finns det n�gra fram�t-referenser i den nya satsen.
*/
    if ( pmargs(retla) == TRUE )
      {
      pmrele();
      erpush("IG3882","");
      errmes();
      strcpy(oldstr,newstr);
      goto edit;
      }
/*
***Byt ut satsen i modulen. lstla = Den gamla satsens listnod
***som returnerats av pmlges(). retla = Den nya satsen som den
***kommer fr�n anunst().
*/
    status = pmrgps(lstla,retla);
    if ( status < 0 ) goto exit;
/*
***Oavsett om storheten �r refererad eller ej provar
***vi att reinterpretera f�r att se om det g�r bra.
*/
    status = EXrist((DBId *)idmat);
/*
***Om det inte gick bra att reinterpretera m�ste vi
***l�nka in den gamla satsen i PM igen s� att allt blir
***som det var fr�n b�rjan.
*/
    if ( status < 0 ) 
      {
      erpush("IG5222","MBS");
      errmes();
      status = pmrgps(lstla,statla);
      if ( status < 0 ) goto exit;
      goto edit;
      }
/*
***Om storheten �r refererad kanske hela modulen skall
***k�ras om.
*/
    if ( pmamir((DBId *)idmat)  &&  igialt(175,67,68,FALSE) )
      {
      status = igramo();
      if ( status < 0 )
        { 
        pmrgps(lstla,statla);
        goto edit;
        }
      }
/*
***Slut.
*/
exit:
    igrsma();
    gphgal(0);
    if ( status < 0 ) errmes();
    return(0);
  }

/********************************************************/
