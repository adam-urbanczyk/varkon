/*!******************************************************************/
/*  File: ig14.c                                                    */
/*  ============                                                    */
/*                                                                  */
/*  This file includes:                                             */
/*                                                                  */
/*  getidt();      Returns  ID + type by crosshairs & mask          */
/*  getmid();      Returns many ID                                  */
/*  gtpcrh();      Position with crosshairs or grid                 */
/*  gtpend();      Position in end of entity                        */
/*  gtpon();       Position on entity                               */
/*  gtpint();      Position by intersect                            */
/*  gtpcen();      Position in centre                               */
/*  gtpdig();      Position by digitizer                            */
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
#include "../../GE/include/GE.h"
#include "../include/IG.h"
#include "../../WP/include/WP.h"
#include "../../GP/include/GP.h"
#include "../../EX/include/EX.h"
#include <math.h>

extern MNUALT  smbind[];
extern DBTmat *lsyspk;
extern DIGDAT  digdes;
extern GMDATA  v3dbuf;
extern short   modtyp,gptrty;
extern gmflt   rstrox,rstroy,rstrdx,rstrdy;

static short getwid(DBId idmat1[][MXINIV], DBId idmat2[][MXINIV],
                    short *idant1, short idant2, DBetype typvek[]);
static short gtpcrh_3D(DBVector *vecptr);
static short gtpcrh_2D(DBVector *vecptr);

/*!******************************************************/

        short getidt(
        DBId    *idvek,
        DBetype *typ,
        bool    *end,
        bool    *right,
        short   utstat)

/*      T�nder h�rkors. L�ser in koordinat + pektecken.
 *      G�r s�kning i displayfil via gpgtla() med mask
 *      som beror av pek-tecknet. Med utstat <> 0 till�ts
 *      rutinen avsluta med status = utstat f�rutom de
 *      vanliga REJECT och GOMAIN.
 *
 *      In: idvek  => Pekare till array av DBId   .
 *                    Arrayen beh�ver ej vara l�nkad.
 *                    M�ste finnas minst MXINIV element.
 *          typ    => �nskad typ.
 *          end    => Slut�nde, TRUE/FALSE.
 *          right  => Sida, TRUE/FALSE.
 *          utstat => Till�ten extra exit-status.
 *
 *      Ut: *idvek => L�nkad lista med identitet.
 *          *typ   => GM-typ.
 *
 *      FV:       0 => Ok.
 *           REJECT => Operationen avbruten.
 *           GOMAIN => Huvudmenyn
 *           IG3532 => Storheten ing�r ej i en part
 *           IG2242 => Syntaxfel i id-str�ng = %s
 *           IG2272 => Storheten %s finns ej
 *
 *      (C)microform ab 4/2/85 J. Kjellander
 *
 *      28/10/85 �nde och sida, J. Kjellander
 *      30/12/85 Pekm�rke, J. Kjellander
 *      30/12/85 Symbol, J. Kjellander
 *      10/3/86  Part, J. Kjellander
 *      13/3/86  Pektecken "i", J. Kjellander
 *      6/10/86  GOMAIN, J. Kjellander
 *      20/10/86 HELP, J. Kjellander
 *      23/12/86 Global ref, J. Kjellander
 *      27/8/87  B_plan, J. Kjellander
 *      17/11/88 utstat, J. Kjellander
 *      10/1-95  Multif�nster, J. Kjellander
 *      1996-04-30 Pekning p� part, J. Kjellander
 *      1997-04-10 WIN32, J.Kjellander
 *
 ******************************************************!*/

  {
    char    pektkn;
    short   ix,iy,status;
    DBetype typmsk,pektyp;
    DBptr   la;
    char    idstr[V3STRLEN+1];
    v2int   win_id;

/*
***L�s h�rkors.
*/
loop:
#ifdef V3_X11
    if ( gptrty == X11 ) wpgtsc(FALSE,&pektkn,&ix,&iy,&win_id);
    else                 gpgtsc(&pektkn,&ix,&iy,FALSE);
#else
#ifdef WIN32
    msgtsc(FALSE,&pektkn,&ix,&iy,&win_id);
#else
    gpgtsc(&pektkn,&ix,&iy,FALSE);
#endif
#endif
    if ( pektkn == *smbind[1].str ) return(REJECT);
    if ( pektkn == *smbind[7].str ) return(GOMAIN);
    if ( pektkn == *smbind[8].str )
      {
      if ( ighelp() == GOMAIN ) return(GOMAIN);
      else goto loop;
      }
    if ( utstat == POSMEN  &&  pektkn == *smbind[9].str ) return(POSMEN);
    if ( utstat == WINDOW  &&  pektkn == *iggtts(98) ) return(WINDOW);
/*
***Avg�r vilka storheter som f�r utpekas.
*/
    if ( *typ == PRTTYP ) pektyp = ALLTYP;
    else pektyp = *typ;
/*
***Kolla pek-tecknet och g�r iordning typmasken.
*/
    if ( pektkn == *iggtts(80) )
      {
      if ( (pektyp & POITYP) == 0 ) goto typerr;
      typmsk = POITYP;
      }

    else if ( pektkn == *iggtts(81) )
      {
      if ( (pektyp & LINTYP) == 0 ) goto typerr;
      typmsk = LINTYP;
      }

    else if ( pektkn == *iggtts(82) )
      {
      if ( (pektyp & ARCTYP) == 0 ) goto typerr;
      typmsk = ARCTYP;
      }

    else if ( pektkn == *iggtts(83) )
      {
      if ( (pektyp & CURTYP) == 0 ) goto typerr;
      typmsk = CURTYP;
      }

    else if ( pektkn == *iggtts(84) )
      {
      if ( (pektyp & SURTYP) == 0 ) goto typerr;
      typmsk = SURTYP;
      }

    else if ( pektkn == *iggtts(85) )
      {
      if ( (pektyp & CSYTYP) == 0 ) goto typerr;
      typmsk = CSYTYP;
      }

    else if ( pektkn == *iggtts(86) )
      {
      if ( (pektyp & TXTTYP) == 0 ) goto typerr;
      typmsk = TXTTYP;
      }

    else if ( pektkn == *iggtts(87) )
      {
      if ( (pektyp & LDMTYP) == 0 ) goto typerr;
      typmsk = LDMTYP;
      }

    else if ( pektkn == *iggtts(88) )
      {
      if ( (pektyp & CDMTYP) == 0 ) goto typerr;
      typmsk = CDMTYP;
      }

    else if ( pektkn == *iggtts(89) )
      {
      if ( (pektyp & RDMTYP) == 0 ) goto typerr;
      typmsk = RDMTYP;
      }

    else if ( pektkn == *iggtts(90) )
      {
      if ( (pektyp & ADMTYP) == 0 ) goto typerr;
      typmsk = ADMTYP;
      }

    else if ( pektkn == *iggtts(91) )
      {
      if ( (pektyp & XHTTYP) == 0 ) goto typerr;
      typmsk = XHTTYP;
      }

    else if ( pektkn == *iggtts(95) )
      {
      if ( (pektyp & PRTTYP) == 0 ) goto typerr;
      typmsk = ALLTYP;
      }

    else if ( pektkn == *iggtts(96) )
      {
      if ( (pektyp & BPLTYP) == 0 ) goto typerr;
      typmsk = BPLTYP;
      }

    else if ( pektkn == *iggtts(79) )
      {
      if ( (pektyp & MSHTYP) == 0 ) goto typerr;
      typmsk = MSHTYP;
      }

    else if ( pektkn == ' ' ) typmsk = pektyp;
/*
***Inmatning av ID fr�n tangentbordet. Om global identitet
***matas in (##id) returnerar vi �nd� lokal eftersom m�nga
***rutiner tex. pmlges() kr�ver detta.
*/
    else if ( pektkn == *iggtts(97) )
      {
inid:
      status = igssip(iggtts(283),idstr,"",V3STRLEN);
      if ( status == REJECT ) return(REJECT);
      if ( status == GOMAIN ) return(GOMAIN);
      if ( igstid(idstr,idvek) < 0 )
        {
        erpush("IG2242",idstr);
        errmes();
        goto inid;
        }
      if ( DBget_pointer('I',idvek,&la,&pektyp) < 0 )
        {
        erpush("IG2272",idstr);
        errmes();
        goto loop;
        }
      if ( idvek->seq_val < 0 ) idvek->seq_val = -idvek->seq_val;
      goto finish;
      }
/*
***Otill�tet pektecken.
*/
    else goto typerr;
/*
***G�r s�kning i df och skapa lokal ID.
*/
#ifdef V3_X11
    if ( gptrty == X11 )   
      {
      if ( (la=wpgtla(win_id,typmsk,ix,iy,&pektyp,end,right)) < 0 )
        goto typerr;
      }
    else
      {
      if ( (la=gpgtla(typmsk,ix,iy,&pektyp,end,right)) < 0 )
        goto typerr;
      }
#else
#ifdef WIN32
      if ( (la=wpgtla(win_id,typmsk,ix,iy,&pektyp,end,right)) < 0 )
        goto typerr;
#else
    if ( (la=gpgtla(typmsk,ix,iy,&pektyp,end,right)) < 0 ) goto typerr;
#endif
#endif
    DBget_id(la,idvek);
    idvek->seq_val = abs(idvek->seq_val);
/*
***Storheten hittad, highlighting.
*/
finish:
#ifdef V3_X11
    if ( gptrty == X11 ) wphgen(GWIN_ALL,la,TRUE);
    else
      {
      gphgen(la,1);
      igflsh();
      }
#else
#ifdef WIN32
    wphgen(GWIN_ALL,la,TRUE);
#else
    gphgen(la,1);
    igflsh();
#endif
#endif
/*
***Om best�lld typ = part eller pektkn f�r part
***anv�nts ser vi till att returnera identiteten f�r en
***part och inte storheten som ing�r i parten.
*/
    if ( *typ == PRTTYP || pektkn == *iggtts(95) )
      {
/*
***Om id bara har en niv� kan det vara en enkel storhet i
***aktiv modul. Detta �r inte till�tet. Vi kollar det med
***DBget_pointer().
*/
      if ( idvek->p_nextre == NULL )
        {
        if ( DBget_pointer('I',idvek,&la,&pektyp) < 0 ) goto loop;
    
        if ( pektyp != PRTTYP )
          {
          erpush("IG3532","");
          errmes();
          gphgal(0);
          goto loop;
          }
        }
      idvek->p_nextre = NULL;
      *typ = PRTTYP;
      }
/*
***Returnera r�tt typ.
*/
    else *typ = pektyp;

    return(0);
/*
***Fel pektecken.
*/
typerr:
    igbell();
    goto loop;
  }

/********************************************************/
/*!******************************************************/

        short getmid(
        DBId     idmat[][MXINIV],
        DBetype  typv[],
        short   *idant)

/*      Returnerar idant identiteter mha. getidt.
 *      Inparametern idmat �r en pekare till en
 *      matris av structures och deklareras som:
 *
 *         DBId    idmat�idant��MXINIV�;
 *
 *      In: idmat = Pekare till resultat.
 *          typv  = Pekare till �nskad typ.
 *          idant = Pekare till max antal ID.
 *
 *      Ut: *idmat = Identiteter.
 *          *typv  = Typer.
 *          *idant = Antal ID.
 *
 *      FV:      0 => Ok.
 *          GOMAIN => Huvudmenyn.
 *
 *      (C)microform ab 16/3/88 J. Kjellander
 *
 ******************************************************!*/

  {
    short   status,nref,nmax,nleft,pekmod;
    DBetype   orgtyp;
    bool    end,right;

/*
***Initiering.
*/
   nref = 0;
   nmax = *idant;
   orgtyp = typv[0];
   if ( nmax > 1 ) pekmod = WINDOW; else pekmod = 0;
/*
***Loopa och l�s identiteter.
*/
   while ( nref <  nmax )
     {
     typv[nref] = orgtyp;
/*
***Prova f�rst att f� en storhet utpekad i taget.
*/
     status = getidt(&idmat[nref][0],&typv[nref],&end,&right,pekmod);
/*
***REJECT �r normal avslutning och GOMAIN avbryter.
*/
     if ( status == REJECT ) break;
     else if ( status == GOMAIN) return(GOMAIN);
/*
***WINDOW medf�r utpekning med f�nstermetoden ist�llet.
*/
     else if ( status == WINDOW )
       {
       nleft = nmax - nref;
 /*JK990331 status = getwid(&idmat[nref][0],idmat,&nleft,nref,&typv[nref]);*/
       status = getwid(&idmat[nref],idmat,&nleft,nref,&typv[nref]);
       if ( status == GOMAIN) return(GOMAIN);
       nref += nleft;
       }
/*
***Varken REJECT, GOMAIN eller WINDOW, allts� har en enstaka
***storhet utpekats. Kolla att den inte �r utpekad tidigare.
*/
     else
       {
       if ( igcmid(&idmat[nref][0],idmat,nref) )
         { 
         erpush("IG5162","");
         errmes();
         }
       else ++nref;
       }
     }
/*
***Slut.
*/
   *idant = nref;
   if ( nref == 0 ) return(REJECT);
   else return(0);

  }

/********************************************************/
/*!******************************************************/

        short getwid(
        DBId     idmat1[][MXINIV],
        DBId     idmat2[][MXINIV],
        short   *idant1,
        short    idant2,
        DBetype  typvek[])

/*      Returnerar idant identiteter mha. gpgmla().
 *      Inparametern idmat �r en pekare till en
 *      matris av structures och deklareras som:
 *
 *         DBId    idmat[idant][MXINIV];
 *
 *      In: idmat1 = Pekare till resultat.
 *          idmat2 = ID:n att testa mot.
 *         *idant1 = Max antal ID:n i idmat1.
 *          idant2 = Antal ID:n i idmat2.
 *          typvek = Pekare till �nskad typ.
 *
 *      Ut: *idmat1 = Identiteter.
 *          *typvek = Typer.
 *          *idant1 = Antal ID.
 *
 *      Felkoder: IG3082 = Punkt 2 = Punkt 1
 *
 *      FV:      0 => Ok.
 *          GOMAIN => Huvudmenyn.
 *
 *      (C)microform ab 17/11/88 J. Kjellander
 *
 *      1997-04-10 WIN32, J.Kjellander
 *
 ******************************************************!*/

  {
    char    pektkn;
    short   nref,nok,ix1,ix2,iy1,iy2,i,tmp,mode;
    DBetype   orgtyp;
    DBptr   lavek[IGMAXID];
    v2int   win_id;

/*
***Initiering.
*/
   nref = *idant1;
   nok = 0;
   orgtyp = typvek[0];
/*
***T�nd h�rkorset 2 ggr och l�s in f�nsterkoordinater.
*/
loop1:
    igptma(142,IG_MESS);
#ifdef V3_X11
    if ( gptrty == X11 ) wpgtsc(TRUE,&pektkn,&ix1,&iy1,&win_id);
    else                 gpgtsc(&pektkn,&ix1,&iy1,TRUE);
#else
#ifdef WIN32
    msgtsc(TRUE,&pektkn,&ix1,&iy1,&win_id);
#else
    gpgtsc(&pektkn,&ix1,&iy1,TRUE);
#endif
#endif
    igrsma();
    if ( pektkn == *smbind[1].str ) goto end;
    if ( pektkn == *smbind[7].str ) return(GOMAIN);
    if ( pektkn == *smbind[8].str ) { ighelp(); goto loop1; }

    if ( pektkn == *iggtts(99) ) mode = 0;
    else if ( pektkn == *iggtts(102) ) mode = 2;
    else { igbell(); goto loop1; }

loop2:
    if ( mode == 0 ) igptma(143,IG_MESS); else igptma(144,IG_MESS);
#ifdef V3_X11
    if ( gptrty == X11 ) wpgtsc(TRUE,&pektkn,&ix2,&iy2,&win_id);
    else                 gpgtsc(&pektkn,&ix2,&iy2,TRUE);
#else
#ifdef WIN32
    msgtsc(TRUE,&pektkn,&ix2,&iy2,&win_id);
#else
    gpgtsc(&pektkn,&ix2,&iy2,TRUE);
#endif
#endif
    igrsma();
    if ( pektkn == *smbind[1].str ) goto loop1;
    if ( pektkn == *smbind[7].str ) return(GOMAIN);
    if ( pektkn == *smbind[8].str ) { ighelp(); goto loop2; }

    if ( pektkn != *iggtts(103)  &&  pektkn != *iggtts(104) )
      { igbell(); goto loop2; }

    if ( pektkn == *iggtts(104) ) ++mode;
/*
***Sortera.
*/
    if ( ix1 > ix2 ) { tmp=ix1; ix1=ix2; ix2=tmp; }
    if ( iy1 > iy2 ) { tmp=iy1; iy1=iy2; iy2=tmp; }
/*
***Felkontroll.
*/
    if ( (ix2-ix1 < 1)  || (iy2-iy1 < 1) )
      {
      erpush("IG3082","");
      errmes();
      goto loop2;
      }
/*
***Anropa gpgmla() och "highligta".
*/
#ifdef V3_X11
    wpgmla(win_id,ix1,iy1,ix2,iy2,mode,TRUE,&nref,typvek,lavek);
#else
#ifdef WIN32
    wpgmla(win_id,ix1,iy1,ix2,iy2,mode,TRUE,&nref,typvek,lavek);
#else
    gpgmla(ix1,iy1,ix2,iy2,mode,TRUE,&nref,typvek,lavek);
#endif
#endif
/*
***�vers�tt LA till lokalt ID och j�mf�r samtidigt varje
***storhet med alla storheter i idmat2. Om n�gon redan
***finns i idmat2, lagra den ej i idmat1.
*/
   for ( i=0; i<nref; ++i)
     {
     DBget_id(lavek[i],idmat1[nok]);
     idmat1[nok][0].seq_val = abs(idmat1[nok][0].seq_val);
     if ( !igcmid(idmat1[nok],idmat2,idant2) ) ++nok;
     }
/*
***Slut.
*/
end:
   *idant1 = nok;

   return(0);

  }

/********************************************************/
/*!******************************************************/

        short gtpcrh(DBVector *pos)

/*      L�s in position med h�rkors.
 *
 *      In: pos = Pekare till utdata.
 *
 *      Ut: *pos = Koordinat.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn.
 *
 *      (C)microform ab 1998-03-31 J.Kjellander
 *
 ******************************************************!*/

  {
   short status;

   switch ( modtyp )
     {
     case 2: status = gtpcrh_2D(pos); break;
     case 3: status = gtpcrh_3D(pos); break;
     default: status = REJECT; break;
     }

   return(status);
  }

/********************************************************/
/*!******************************************************/

static short gtpcrh_3D(DBVector *vecptr)

/*      L�s in 3D position med h�rkors.
 *
 *      In: vecptr => Pekare till utdata.
 *
 *      Ut: *vecptr => Vektor.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn.
 *
 *      (C)microform ab 1998-03-31, J.Kjellander
 *
 *      1998-10-20 WIN32, J.Kjellander
 *
 ******************************************************!*/

  {
    short  status;
    char   pektkn;

/*
***L�s in 3D modellkoordinater via h�rkors.
*/
loop:
#ifdef V3_X11
    status = wpgmc3(&pektkn,vecptr,TRUE);
#endif
#ifdef WIN32
    status = msgmc3(&pektkn,vecptr,TRUE);
#endif
/*
***B�rja med att kolla pektecknet.
*/
    if ( pektkn == *smbind[1].str) return(REJECT);
    if ( pektkn == *smbind[7].str) return(GOMAIN);
    if ( pektkn == *smbind[8].str)
      {
      if ( ighelp() == GOMAIN ) return(GOMAIN);
      goto loop;
      }
    if ( pektkn == *smbind[9].str) return(POSMEN);
/*
***Sen status. Negativ status inneb�r att XY-planet
***�r vinkelr�tt mot sk�rmplanet.
*/
    if ( status < 0 )
      {
      igbell();
      wpepmk(GWIN_ALL);
      goto loop;
      }
/*
***Slut.
*/ 
    return(0);
  }

/********************************************************/
/*!******************************************************/

static short gtpcrh_2D(DBVector *vecptr)

/*      L�s in 2D position med h�rkors eller
 *      position p� en rasterpunkt. Positionen 
 *      transformeras till aktivt koordinatsystem.
 *      <SP> och h ger h�rkorsposition.
 *      r ger rasterposition.
 *
 *      In: vecptr => Pekare till DBVector.
 *
 *      Ut: *vecptr => Vektor.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn.
 *
 *      (C)microform ab
 *
 *      9/9/85   H�rkors/raster sammanslaget, R. Svedin
 *      30/12/85 Pekm�rke, J. Kjellander
 *      3/10/86  GOMAIN, J. Kjellander
 *      20/10/86 HELP, J. Kjellander
 *
 ******************************************************!*/

  {
    char   pektkn;
    double x,y;
    double xg,yg,ng;

/*
***L�s in 2D modellkoordinater via h�rkors.
*/
loop:
    gpgtmc(&pektkn,&x,&y,TRUE);
    if ( pektkn == *smbind[1].str) return(REJECT);
    if ( pektkn == *smbind[7].str) return(GOMAIN);
    if ( pektkn == *smbind[8].str)
      {
      if ( ighelp() == GOMAIN ) return(GOMAIN);
      goto loop;
      }
    if ( pektkn == *smbind[9].str) return(POSMEN);
/*
***Raster eller h�rkors
*/
    if ( pektkn == *iggtts(93) )     
      {
      ng = ABS((x-rstrox)/rstrdx);            /* Rasterber�kning */
      if ( DEC(ng) > 0.5 ) ++ng;

      if ( x > rstrox ) xg = rstrox + HEL(ng) * rstrdx;
      else xg = rstrox - HEL(ng) * rstrdx;

      ng = ABS((y-rstroy)/rstrdy);
      if ( DEC(ng) > 0.5 ) ++ng;

      if ( y > rstroy ) yg = rstroy + HEL(ng) * rstrdy;
      else yg = rstroy - HEL(ng) * rstrdy;
      }

    else if ( pektkn == *iggtts(92) || pektkn == 32 )
      {
      xg = x;                                 /* H�rkorsposition */
      yg = y;
      }

    else 
      {
      igbell();                             /* Signalera */
      goto loop;
      }
/*
***Lagra.
*/ 
    vecptr->x_gm = xg;
    vecptr->y_gm = yg;
    vecptr->z_gm = 0.0;
/*
***Ev. transformation till lokalt koordinatsystem.
*/
    if ( lsyspk != NULL ) GEtfpos_to_local(vecptr,lsyspk,vecptr);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short gtpend(DBVector *vecptr)

/*      Position i ena �nden av en storhet.
 *
 *      In: vecptr => Pekare till DBVector.
 *
 *      Ut: *vecptr => Vektor.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn.
 *
 *      (C)microform ab 28/10/85 J. Kjellander
 *
 *      26/3/86  Felutskrift fr�n EXon, B. Doverud
 *      6/10/86  GOMAIN, J. Kjellander
 *
 ******************************************************!*/

  {
    gmflt   t;
    DBetype   typ;
    DBptr   la;
    bool    end,right;
    short   status;
    DBId    idvek[MXINIV];
    GMUNON  gmpost;

/*
***H�mta id f�r den refererade storheten.
*/
loop:
    typ = LINTYP+ARCTYP+CURTYP;
    igptma(331,IG_MESS);
    if ( (status=getidt(idvek,&typ,&end,&right,POSMEN)) < 0 ) goto exit;
/*
***Kolla typ och �nde och v�lj r�tt parameterv�rde.
*/
    if ( end )
      {
      if ( typ == CURTYP )
        {
        DBget_pointer('I',idvek,&la,&typ);
        DBread_curve(&gmpost.cur_un,NULL,NULL,la);
        t = gmpost.cur_un.ns_cu;
        }
      else t = 1.0;
      }
    else t = 0.0;
/*
***Ber�kna positionen.
*/
    if ( EXon(idvek,t,(gmflt)0.0,vecptr) < 0 )
      {
      errmes();
      goto loop;
      }
/*
***Avslutning.
*/
exit:
    igrsma();
    return(status);
  }

/********************************************************/
/*!******************************************************/

        short gtpon(DBVector *vecptr)

/*      Position p� en storhet.
 *
 *      In: vecptr => Pekare till DBVector.
 *
 *      Ut: *vecptr => Vektor.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn.
 *
 *      (C)microform ab 20/1/85 J. Kjellander
 *
 *      28/10/85 �nde och sida, J. Kjellander
 *      29/12/85 Symbol, J. Kjellander
 *      26/3/86  Felutskrift fr�n EXon, B. Doverud
 *      6/10/86  GOMAIN, J. Kjellander
 *      22/2/93  Nytt anrop till EXon(), J. Kjellander
 *
 ******************************************************!*/

  {
    double  t;
    DBetype   typ;
    bool    end,right;
    short   status;
    DBId    idvek[MXINIV];

/*
***H�mta id f�r den refererade storheten.
*/
loop:
    typ = POITYP+LINTYP+ARCTYP+CURTYP+CSYTYP+
          TXTTYP+LDMTYP+CDMTYP+RDMTYP+ADMTYP;
    igptma(52,IG_MESS);
    if ( (status=getidt(idvek,&typ,&end,&right,POSMEN)) < 0 ) goto exit;
/*
***Kolla typ. Om punkt etc. s�tt parameterv�rdet = 0.
*/
    if ( typ == POITYP || typ == CSYTYP || typ == TXTTYP ||
         typ == LDMTYP || typ == CDMTYP || typ == RDMTYP ||
         typ == ADMTYP ) t = 0.0;
    else
       {
       igptma(208,IG_INP);
       status=igsfip(iggtts(320),&t);
       igrsma();
       if ( status < 0 ) goto exit;
       }
/*
***Ber�kna positionen.
*/
    if ( EXon (idvek,t,(gmflt)0.0,vecptr) < 0 )
      {
      igrsma();
      errmes();
      goto loop;
      }
/*
***Avslutning.
*/
exit:
    igrsma();
    return(status);
  }

/********************************************************/
/*!******************************************************/

       short gtpint(DBVector *vecptr)

/*      Position i sk�rningspunkt.
 *
 *      In: vecptr => Pekare till DBVector.
 *
 *      Ut: *vecptr => Vektor.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn.
 *
 *      (C)microform ab 4/2/85 J. Kjellander 
 *
 *      28/10/85 �nde och sida, J. Kjellander
 *      26/3/86  Felutskrift fr�n EXsect, B. Doverud
 *      6/10/86  GOMAIN, J. Kjellander
 *      26/11/89 Neg intnr, J. Kjellander
 *
 ******************************************************!*/

  {
    short   alt,status;
    int     ival;
    DBetype typ1,typ2;
    DBptr   la;
    bool    end,right,enkel=FALSE;
    DBId    idv1[MXINIV],idv2[MXINIV];
    GMUNON  gmpost;
    GMSEG   arcseg[4];

/*
***1:a storheten.
*/
loop:
    igptma(324,IG_MESS);
    typ1 = LINTYP+ARCTYP+CURTYP;
    if ( modtyp == 3 ) typ1 += BPLTYP+CSYTYP;
    if ( (status=getidt(idv1,&typ1,&end,&right,POSMEN)) < 0 ) goto exit;
    igrsma();
/*
***2:a storheten.
*/
    typ2 = LINTYP+ARCTYP+CURTYP;
    if ( modtyp == 3  &&  typ1 != BPLTYP  &&  typ1 != CSYTYP )
      typ2 += BPLTYP+CSYTYP;
    igptma(325,IG_MESS);
    if ( (status=getidt(idv2,&typ2,&end,&right,POSMEN)) < 0 ) goto exit;
    igrsma();
/*
***Om sk�rning linje/linje, alt = -1.
*/
    if ( typ1 == LINTYP  &&  typ2 == LINTYP ) alt = -1;
/*
***�r det en enkel 2D-sk�rning ?
*/
    else
      {
      if ( typ1 == LINTYP  &&  typ2 == ARCTYP )
        {
        DBget_pointer('I',idv2,&la,&typ2);
        DBread_arc(&gmpost.arc_un,arcseg,la);
        if ( gmpost.arc_un.ns_a == 0 ) enkel = TRUE;
        }
      else if ( typ1 == ARCTYP  &&  typ2 == LINTYP )
        {
        DBget_pointer('I',idv1,&la,&typ1);
        DBread_arc(&gmpost.arc_un,arcseg,la);
        if ( gmpost.arc_un.ns_a == 0 ) enkel = TRUE;
        }
      else if ( typ1 == ARCTYP  &&  typ2 == ARCTYP )
        {
        DBget_pointer('I',idv1,&la,&typ1);
        DBread_arc(&gmpost.arc_un,arcseg,la);
        if ( gmpost.arc_un.ns_a == 0 )
          {
          DBget_pointer('I',idv2,&la,&typ2);
          DBread_arc(&gmpost.arc_un,arcseg,la);
          if ( gmpost.arc_un.ns_a == 0 ) enkel = TRUE;
          } 
        }
      else enkel = FALSE;
/*
***Om det �r en enkel 2D-sk�rning, fr�ga efter 1:a eller
***2:a sk�rningen.
*/
      if ( enkel )
        {
        if ( igialt(160,161,162,FALSE) ) alt = -1;
        else alt = -2;
        }
/*
***Nej det �r inte en enkel 2D-sk�rning, l�s in alternativ.
*/
      else
        {
        igptma(327,IG_INP);
        if ( (status=igsiip(iggtts(46), &ival)) < 0 ) goto exit;
        alt = (short)ival;
        }
      }
/*
***Ber�kna sk�rningen.
*/
    if ( EXsect (idv1,idv2,alt,0,vecptr) < 0 )
      {
      errmes();
      goto loop;
      }
/*
***Utg�ng f�r avbruten operation.
*/
exit:
    igrsma();
    return(status);
  }

/********************************************************/
/*!******************************************************/

        short gtpcen(DBVector *vecptr)

/*      Position i kr�kningscentrum.
 *
 *      In: vecptr => Pekare till DBVector.
 *
 *      Ut: *vecptr => Vektor.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn.
 *
 *      (C)microform ab 26/4/87 J. Kjellander
 *
 ******************************************************!*/

  {
    double  t;
    DBetype   typ;
    bool    end,right;
    short   status;
    DBId    idvek[MXINIV];

/*
***H�mta id f�r den refererade storheten.
*/
loop:
    typ = ARCTYP+CURTYP;
    igptma(53,IG_MESS);
    if ( (status=getidt(idvek,&typ,&end,&right,POSMEN)) < 0 ) goto exit;
/*
***Kolla typ. Om arc s�tt parameterv�rdet = 0.
*/
    if ( typ == ARCTYP ) t = 0.0;
    else
       {
       igptma(208,IG_INP);
       status=igsfip(iggtts(320),&t);
       igrsma();
       if ( status < 0 ) goto exit;
       }
/*
***Ber�kna positionen.
*/
    if ( EXcen(idvek,t,lsyspk,vecptr) < 0 )
      {
      igrsma();
      errmes();
      goto loop;
      }
/*
***Avslutning.
*/
exit:
    igrsma();
    return(status);
  }

/********************************************************/
/*!******************************************************/

        short gtpdig(DBVector *vecptr)

/*      L�ser in position via digitizer.
 *
 *      In: vecptr => Pekare till DBVector.
 *
 *      Ut: *vecptr => Vektor.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn.
 *
 *      Felkod: IG0322 = Digitizer ej ansluten
 *              IG0332 = Formatfel fr�n digitizer
 *
 *      (C)microform ab 14/3/88 J. Kjellander
 *
 *      16/12/88 Ut�kad felkontroll, J. Kjellander
 *
 ******************************************************!*/

  {
   char   digbuf[81];
   int    ix,iy;
   gmflt  lx,ly,rx,ry,mx,my;
   short  nx,ny,status;

/*
***H�mta str�ng fr�n digitizer och tolka.
*/
   if ( digdes.def )
     {
     status = iggtds(digbuf);
     if ( status < 0 ) return(status);

     switch (digdes.typ )
       {
/*
***C91360P format 3 100 lines/mm.
*/
       case C91360P:
       nx = sscanf(&digbuf[2],"%d",&ix);
       ny = sscanf(&digbuf[8],"%d",&iy);
       if ( nx != 1  ||  ny != 1  || ix < 0  ||  iy < 0 )
         {
         digbuf[80] = '\0';
         erpush("IG0332",digbuf);
         errmes(); return(REJECT);
         }
       lx = ix/100.0; ly = iy/100.0;
       break;
/*
***C2200P format 1 10 lines/mm., XXXX,YYYY+pennstatysbyte+CR
*/
       case C2200P:
       nx = sscanf(&digbuf[0],"%d",&ix);
       ny = sscanf(&digbuf[5],"%d",&iy);
       if ( nx != 1  ||  ny != 1  || ix < 0  ||  iy < 0 )
         {
         digbuf[80] = '\0';
         erpush("IG0332",digbuf);
         errmes(); return(REJECT);
         }
       lx = ix/10.0; ly = iy/10.0;
       break;
/*
***MM1201/961 ASCII BCD 20 lines/mm., XXXX,YYYY+pennstatusbyte+CR+LF
*/
       case MM1200P:
       nx = sscanf(&digbuf[0],"%d",&ix);
       ny = sscanf(&digbuf[5],"%d",&iy);
       if ( nx != 1  ||  ny != 1  || ix < 0  ||  iy < 0 )
         {
         digbuf[80] = '\0';
         erpush("IG0332",digbuf);
         errmes(); return(REJECT);
         }
       lx = ix/20.0; ly = iy/20.0;
       break;
       }
     }
   else
     {
     erpush("IG0322","");
     errmes();
     return(REJECT);
     }
/*
***Transformation pga. kalibrering. Digitizerns koordinat lx,ly
***�r nu i millimeter.
*/
   lx = lx - digdes.ldx;
   ly = ly - digdes.ldy;
   
   if ( digdes.v != 0.0 )
     {
     rx = lx*COS(digdes.v) - ly*SIN(digdes.v);
     ry = ly*COS(digdes.v) + lx*SIN(digdes.v);
     }
   else
     {
     rx = lx;
     ry = ly;
     }

   mx = digdes.lmx + rx*digdes.s;
   my = digdes.lmy + ry*digdes.s;
/*
***Lagra koordinater.
*/
   vecptr->x_gm = mx;
   vecptr->y_gm = my;
   vecptr->z_gm = 0.0;
/*
***Ev. transformation till lokalt koordinatsystem.
*/
   if ( lsyspk != NULL ) GEtfpos_to_local(vecptr,lsyspk,vecptr);

   return(0);
  }

/********************************************************/
