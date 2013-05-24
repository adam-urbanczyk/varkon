/**********************************************************************
*
*    gp28.c
*    ======
*
*    This file is part of the VARKON Graphics  Library.
*    URL: http://www.varkon.com
*
*    plycud();    Make dashed curve polyline
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
#include "../../GE/include/GE.h"
#include "../include/GP.h"
#include <math.h>

/*
***Ber�knar n�sta u-v�rde
*/
static short nextu(double ds);

/*
***Ber�knar n�sta position
*/
static short nextp(double ug, double x[], double y[], double z[]);

/*
***Ber�knar n�sta antal vektorer
*/
static short nextnv(double ug);

extern DBTmat actvym;
extern VY     actvy;
extern double curnog,gpgszx,k2x;
extern int    ncrdxy;
extern bool   pltflg,gpgenz;

/*
***Static-variabler.
*/

static double actl,actu,prevu,sl,sskala;
static int    k;
static int    nvec;
static GMSEG *segpek;
static GMCUR *curpek;

/*!******************************************************/

        short plycud(
        GMCUR *gmpost,
        GMSEG *segmnt,
        int   *n,
        double x[],
        double y[],
        double z[],
        char   a[])

/*      Bygger en streckad kurva i form av en 2D/3D-polyline.
 *
 *      In: 
 *         gmpost:      Pekare till curve-structure.
 *         segmnt:      Array med segment.
 *         n+1:         Offset till polylinjestart.
 *         x,y,z,a:     Arrayer f�r resultat.
 *
 *      Ut:
 *         n:           Offset till sista vektorn i polylinjen
 *         x,y,z,a:     Koordinater och status hos vektorerna
 *                      i polylinjen.
 *
 *      FV:   0 = Ok.
 *           -1 = F�r liten eller f�r stor streckl�ngd f�r att synas.
 *
 *      (C)microform ab 9/1/92 J. Kjellander
 *
 *      7/3/92   Uppsnabbning, J. Kjellander
 *      16/6/93  Dynamiska segment, J. Kjellander
 *      31/1/94  k = *n+1, J. Kjellander
 *      5/12/94  Font=2, J. Kjellander
 *      9/1/96   Bytt vydist->gpgenz, J. Kjellander
 *      19961104 nsgr, J. Kjellander
 *
 ******************************************************!*/

  {
    short   status;
    int     i;
    double  ltot,ds,mfakt,uinc,stopl;
    bool    offseg;
    GMSEG  *prjseg;
    DBTmat  m1,m2;

/*
***Div initiering.
*/
    curpek = gmpost;           /* Pekare till kurv-posten */
    k      = *n+1;             /* Offset i polylinjen */
    sl     = curpek->lgt_cu;   /* Streckl�ngd */
    actu   = 0.0;              /* Aktuellt globalt parameterv�rde */
    prevu  = 0.0;              /* F�reg�ende globala parameterv�rde */
    actl   = 0.0;              /* Aktuell relativ l�ngd */
    mfakt  = DASHRATIO;        /* Mellanrumsfaktor=0.25 */
    offseg = FALSE;            /* TRUE om n�got segm. har offset */
/*
***Sk�rmskala.
*/
    sskala = gpgszx/(actvy.vywin[2] - actvy.vywin[0]);
/*
***Om n�got av segmenten har offset �r det en offset-kurva 
***och d� skall kurvplanet transformeras till aktiv vy.
*/
    for ( i=0; i<curpek->nsgr_cu; ++i )
      {
      if ( (segmnt+i)->ofs != 0.0 )
        {
        if ( actvy.vy3d )
          {
          V3MOME(&curpek->csy_cu,&m1,sizeof(DBTmat));
          GEtform_inv(&actvym,&m2);
          GEtform_mult(&m1,&m2,&curpek->csy_cu);
          }
        offseg = TRUE;
        break;
        }
      }
/*
***Om 3D-vy, transformera segmenten till r�tt vy och ber�kna
***nya l�ngder. Om inte, kolla om l�ngd beh�vs och ber�kna is�fall.
***S� �r tex. fallet f�r en 3D-cirkel i 2D-vy d� vi anropas av gpplar().
***Om projicering beh�vs g�rs detta p� en kopia av segmenten s� att
***om det visar att det r�cker med heldraget (f�r stor eller liten
***streck-l�ngd) s� skall inte de ursprungliga segmenten projiceras
***en g�ng till av gpplcu(). F�r att segmentl�ngder skall bli r�tt
***och d�rmed streckl�ngder m�ste alla Z-koefficienter s�ttas till
***0 dvs. �kta projicering. F�r en offset-kurva kan man dock inte
***g�ra p� det viset eftersom man d� f�r�ndrar kurvans form.
*/
    if ( actvy.vy3d )
      {
      prjseg = DBcreate_segments(curpek->nsgr_cu);
      for ( i=0; i < curpek->nsgr_cu; ++i )
        {
        GEtfseg_to_local(&segmnt[i],&actvym,&prjseg[i]);
        if ( !offseg  &&  !gpgenz )
          prjseg[i].c0z = prjseg[i].c1z = prjseg[i].c2z = prjseg[i].c3z = 0.0;
        }
      segpek = prjseg;
      }
    else
      {
      prjseg = NULL;
      segpek = segmnt;
      }
/*
***Ber�kna hela kurvans l�ngd. Om ingen projektion skett ovan och
***kurvan har samma grafiska som geometriska representation �r detta
***viserligen redan gjort men hur ska vi veta det ???
*/
    GEarclength((DBAny *)curpek,segpek,&ltot);
/*
***Kommer strecken att synas p� sk�rmen ? Om inte rita heldraget.
*/
    if ( ltot < (1.5+mfakt)*sl ) { status = -1; goto error; }
    if ( mfakt*sl*k2x < 1.0  &&  !pltflg ) { status = -1; goto error; }
/*
***Ber�kna relativ stegl�ngd. 0 > ds > 1.
*/
    ds = sl/ltot;
/*
***Finns det plats ?
*/
    if ( k+2 > PLYMXV )
      {
      status = -1;
      goto error;
      }
/*
***B�rja med en sl�ckt f�rflyttning till kurvans startposition.
***actu = 0.
*/
    nextp(actu,x,y,z);
    a[k++] = INVISIBLE;
/*
***Sedan ett f�rsta t�nt delstreck. Ber�kna f�rst nytt actu
***och sedan antal vektorer genom ett nextnv()-anrop mitt p�.
*/
    if ( (status=nextu(ds)) < 0 )
      {
      status = -1;
      goto error;
      }
    nextnv(actu/2.0);
    uinc = actu/nvec;
/*
***Finns det plats ?
*/
    if ( k+nvec+2 > PLYMXV )
      {
      --k;
      status = -1;
      goto error;
      }
/*
***Generera.
*/
    for ( i=0; i<nvec; ++i )
      {
      prevu += uinc;
      nextp(prevu,x,y,z);
      a[k++] = VISIBLE;
      }
/*
***Vi st�r nu i slutet p� det f�rsta del-strecket med actl och actu.
***Generera resten utom sista. Detta forts�tter vi med s� l�nge vi
***befinner oss p� tryggt avst�nd fr�n slutet. Vad detta �r beror p�
***aktuell font. Med streckat �r det (1 delstreck + 2 mellanrum)
***och med streckprickat �r det (1 l�ngt streck + 1 mellanrum + 1 kort
***streck + 2 mellanrum).
*/
    if ( curpek->fnt_cu == 1 ) stopl = 1.0 - (1.0+2*mfakt)*ds;
    else                       stopl = 1.0 - (1.0+4*mfakt)*ds;

    while ( actl < stopl )
      {
/*
***Om det �r streckprickat l�gger vi nu in ett mellanrum + ett
***kort streck.
*/
      if ( curpek->fnt_cu != 1 )
        {
        if ( (status=nextu((2.0*mfakt)*ds)) < 0 )
          {
          status = -1;
          goto error;
          }
/*
***Vi approximerar mellanrummet med ett motsvarande steg i u.
*/
        prevu += (actu-prevu)/2.0;
        nextp(prevu,x,y,z);
        a[k++] = INVISIBLE;
/*
***Vi st�r nu i b�rjan av en eller flera t�nda f�rflyttningar.
***Ber�kna erfoderligt antal vektorer f�r delstrecket.
***Om de inte f�r plats i polylinjen avslutar vi utan att rita
***klart.
*/
        nextnv((actu+prevu)/2.0);

        if ( k+nvec+2 >= PLYMXV )
         {
       --k;
         status = -1;
         goto error;
         }
/*
***Generera nvec t�nda f�rflyttningar.
*/
        uinc = (actu-prevu)/nvec;

        for ( i=0; i<nvec; ++i )
          {
          prevu += uinc;
          nextp(prevu,x,y,z);
          a[k++] = VISIBLE;
          }
        }
/*
***Oavsett om det �r streckat eller streckprickat �r det nu dags att
***r�kna upp actl och actu med (ett mellanrum + ett l�ngt streck).
*/
      if ( (status=nextu((1.0+mfakt)*ds)) < 0 )
        {
        status = -1;
        goto error;
        }
/*
***Vi approximerar mellanrummet med ett motsvarande steg i u.
*/
      prevu = actu - 1.0/(1.0+mfakt)*(actu-prevu);
      nextp(prevu,x,y,z);
      a[k++] = INVISIBLE;
/*
***Vi st�r nu i b�rjan av en eller flera t�nda f�rflyttningar.
***Ber�kna erfoderligt antal vektorer f�r delstrecket.
***Om de inte f�r plats i polylinjen avslutar vi utan att rita
***klart.
*/
      nextnv((actu+prevu)/2.0);

      if ( k+nvec+2 >= PLYMXV )
       {
     --k;
       status = -1;
       goto error;
       }
/*
***Generera nvec t�nda f�rflyttningar.
*/
      uinc = (actu-prevu)/nvec;

      for ( i=0; i<nvec; ++i )
        {
        prevu += uinc;
        nextp(prevu,x,y,z);
        a[k++] = VISIBLE;
        }
      }
/*
***Ett sista mellanrum.
*/
    if ( (status=nextu(ds*mfakt)) < 0 )
      {
      status = -1;
      goto error;
      }
    nextp(actu,x,y,z);
    a[k++] = INVISIBLE;
/*
***Nu g�ller det att avsluta p� ett vettigt s�tt.
***Finns det tillr�ckligt med plats kvar g�r vi ett
***till streck, annars f�rl�nger vi f�reg�ende.
*/
    if ( 1.0 - actl < ds*mfakt )
      {
      k -= 1;
      actu = prevu;
      }
/*
***Hur m�nga vektorer beh�vs f�r det som �r kvar ?
*/
    nextnv((curpek->nsgr_cu + actu)/2.0);
    if ( k+nvec+2 >= PLYMXV )
      {
      --k;
      status = -1;
      goto error;
      }
/*
***Sista delstrecket.
*/
    uinc = (curpek->nsgr_cu - actu)/nvec;

    for ( i=0; i<nvec; ++i )
      {
      actu += uinc;
      nextp(actu,x,y,z);
      a[k++] = VISIBLE;
      }
/*
***N�r allt �r klart har k r�knats upp en g�ng f�r mycket.
*/
    k--;
/*
***Allt verkar ha g�tt bra.
*/
    status = 0;
/*
***Hur m�nga vektorer blev det ?
*/
   *n = k;
    ncrdxy = k+1;
/*
***Endside.
*/
    for ( i=k/2+1; i<=k; ++i ) a[i] = a[i] | ENDSIDE;
/*
***Ev. perspektiv.
*/
    if ( actvy.vydist != 0.0 ) gppstr(x,y,z);
/*
***Felutg�ng.
*/
error:
    if ( prjseg != NULL ) DBfree_segments(prjseg);

    return(status);
 }

/*********************************************************/
/*!******************************************************/

 static short nextu(
        double ds)

/*      Ber�knar n�sta u-v�rde.
 *
 *      In: ds = Storleken p� n�sta steg, i relativ b�gl�ngd.
 *
 *      Ut: prevu, actu, actl.
 *
 *      FV:   0 = Ok.
 *
 *      (C)microform ab 10/3/92 J. Kjellander
 *
 ******************************************************!*/

 {
   short status;

/*
***S�tt f�reg�ende u = aktuellt u.
*/
   prevu = actu;
/*
***R�kna upp actl.
*/
   actl += ds;
/*
***Ber�kna ett nytt actu.
*/
   status = GE717((DBAny *)curpek,segpek,NULL,actl,&actu);

   if ( status < 0 ) return(status);

   actu -= 1.0;

   return(0);
 }

 /*******************************************************/
/*!******************************************************/

 static short nextp(
        double ug,
        double x[],
        double y[],
        double z[])

/*      Ber�knar n�sta position.
 *
 *      In: ug    = Globalt u-v�rde.
 *          x,y,z = Arrayer f�r resultat.
 *
 *      Ut: x,y,z.
 *
 *      FV:   0 = Ok.
 *
 *      (C)microform ab 10/3/92 J. Kjellander
 *
 ******************************************************!*/

 {
   int   intu;
   gmflt out[16];

/*
***V�lj r�tt segment.
*/
    if ( (intu=(int)HEL(ug)) == curpek->nsgr_cu ) intu -= 1;
/*
***Ber�kna koordinater.
*/
    GE107((GMUNON *)curpek,segpek+intu,ug-intu,(short)0,out);
    x[k] = out[0]; y[k] = out[1]; z[k] = out[2];
/*
***Slut.
*/
   return(0);

 }

 /*******************************************************/
/*!******************************************************/

 static short nextnv(
        double ug)

/*      Ber�knar n�sta antal vektorer.
 *
 *      In: ug = Globalt u-v�rde.
 *
 *      Ut: x,y,z.
 *
 *      FV:   0 = Ok.
 *
 *      (C)microform ab 10/3/92 J. Kjellander
 *
 ******************************************************!*/

 {
   int   intu;
   gmflt kappa,out[16];

/*
***V�lj r�tt segment.
*/
    if ( (intu=(int)HEL(ug)) == curpek->nsgr_cu ) intu -= 1;
/*
***Ber�kna kr�kning.
*/
    GE107((GMUNON *)curpek,segpek+intu,ug-intu,(short)3,out);
    kappa = out[15]/sskala;
/*
***Antal vektorer.
***Som m�tt p� dsdu anv�nds sl. Se gp8.c.
***I gp8() anv�nds 0.2*SQRT(..... f�r en tredjedel av ett segment.
***H�r �r det d� logiskt att ta 3 ggr. s� mycket f�r ett streck.
*/
    nvec = (int)(0.6*SQRT(kappa)*sl*sskala*curnog + 0.5);
    if (nvec < 1 ) nvec = 1;
/*
***Slut.
*/
   return(0);

 }

 /*******************************************************/
