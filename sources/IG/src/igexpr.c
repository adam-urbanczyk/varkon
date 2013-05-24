/*!******************************************************************/
/*  igexpr.c                                                        */
/*  ========                                                        */
/*                                                                  */
/*  This file includes:                                             */
/*                                                                  */
/*  genint();     Genererate integer expression                     */
/*  geninv();     Genererate integer literal value                  */
/*  genflt();     Genererate float expression                       */
/*  genflv();     Genererate float literal value                    */
/*  genstr();     Genererate string expression                      */
/*  genstv();     Genererate string literal value                   */
/*  genstm();     Genererate string litval by menu selection        */
/*  genref();     Genererate reference expression                   */
/*  genpos();     Genererate vector expression                      */
/*  genpov();     Genererate vector literal value                   */
/*  igpmon();     Set pos-mode on                                   */
/*  igpmof();     Set pos-mode off                                  */
/*  getidt();     Returns  ID + type by crosshairs & mask           */
/*  getmid();     Returns many ID                                   */
/*  gtpcrh();     Position with crosshairs or grid                  */
/*  gtpend();     Position in end of entity                         */
/*  gtpon();      Position on entity                                */
/*  gtpint();     Position by intersect                             */
/*  gtpcen();     Position in centre                                */
/*  gtpdig();     Position by digitizer                             */
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
#include "../../AN/include/AN.h"
#include "../../WP/include/WP.h"
#include "../../EX/include/EX.h"
#include <math.h>

extern bool    tmpref;
extern short   v3mode,modtyp,posmod;
extern MNUALT  smbind[];
extern DBfloat rstrox,rstroy,rstrdx,rstrdy;
extern DBTmat *lsyspk;

static short genuv(short pnr, pm_ptr *pexnpt);
static short genrfs(short pnr, DBetype *ptyp, pm_ptr *pexnpt, bool *pend,
                    bool *pright, short utstat);
static short gnpmen(pm_ptr *pexnpt);
static short gnpabs(pm_ptr *pexnpt);
static short gnprel(pm_ptr *pexnpt);
static short gnpcrh(pm_ptr *pexnpt);
static short gnpexp(pm_ptr *pexnpt);
static short gnpend(pm_ptr *pexnpt);
static short gnpon(pm_ptr *pexnpt);
static short gnpint(pm_ptr *pexnpt);
static short gnpcen(pm_ptr *pexnpt);
static short getwid(DBId idmat1[][MXINIV], DBId idmat2[][MXINIV],
                    short *idant1, short idant2, DBetype typvek[]);
static short gtpcrh_3D(DBVector *vecptr);
static short gtpcrh_2D(DBVector *vecptr);

/*!******************************************************/

        short genint(
        short  pnr,
        char   *dstr,
        char   *istr,
        pm_ptr *pexnpt)

/*      Huvudrutin f�r att skapa en uttrycksnod f�r
 *      ett heltals-v�rde.
 *
 *      In: pnr    => Promptstr�ng nr, om 0 ingen utskrift.
 *          dstr   => Pekare till defaultstr�ng.
 *          istr   => Pekare till inputstr�ng.
 *          pexnpt => Pekare till pm_ptr.
 *
 *      Ut: *pexnpt => PM-pekare till expression-nod.
 *
 *      FV:       0 => Ok.
 *           REJECT => Operationen avbruten.
 *           GOMAIN => Tillbaks till huvudmenyn
 *           IG2232 => Fel datatyp
 *
 *      (C)microform ab 17/1/85 M Nelson
 *
 *      30/10/85 Defaultstr�ng, J. Kjellander
 *      6/3/86   Ny defaulthantering, B. Doverud
 *      6/10/86  GOMAIN, J. Kjellander
 *
 ******************************************************!*/

  {
    short  status;
    sttycl type;

/*
***Skriv ut ev. promtstr�ng.
*/
    if (pnr > 0) igptma(pnr,IG_INP);
/*
***L�s in v�rde.
*/
loop:
    if ( (status=igssip(iggtts(46),istr,dstr,V3STRLEN)) < 0) goto exit;

    if (anaexp(istr,FALSE,pexnpt,&type) != 0)
      {
      errmes();
      goto loop;
      }

    if (type != ST_INT)
        {
        erpush("IG2232",iggtts(4));
        errmes();
        goto loop;
        }
exit:
    if ( pnr > 0 ) igrsma();
    return(status);
  }

/********************************************************/
/*!******************************************************/

        short geninv(
        short  pnr,
        char   *istr,
        char   *dstr,
        DBint  *ival)

/*      L�ser in ett heltalsuttryck och ber�knar v�rdet.
 *
 *      In: pnr    => Promptstr�ng nr, om 0 ingen utskrift.
 *          dstr   => Pekare till defaultstr�ng.
 *          istr   => Pekare till inputstr�ng.
 *          ival   => Pekare till resultat.
 *
 *      Ut: *ival  => Heltal.
 *
 *      FV:       0 => Ok.
 *           REJECT => Operationen avbruten.
 *           GOMAIN => Tillbaks till huvudmenyn
 *
 *      (C)microform ab 29/9/87 J. Kjellander
 *
 ******************************************************!*/

  {
    short  status;
    pm_ptr exnpt,valtyp;
    PMLITVA val;

/*
***Spara aktuell pm-stack pekare.
*/
    pmmark();
/*
***Generera uttryck.
*/
    if ( (status=genint(pnr,dstr,istr,&exnpt)) < 0 ) goto end;
/*
***Interpretera uttrycket.
*/
    inevev(exnpt,&val,&valtyp);

    *ival = val.lit.int_va;
/*
***Slut.
*/
end:
    pmrele();
    return(status);

  }
/********************************************************/
/*!******************************************************/

        short genflt(
        short  pnr,
        char   *dstr,
        char   *istr,
        pm_ptr *pexnpt)

/*      Huvudrutin f�r att skapa en uttrycksnod f�r
 *      ett double-v�rde.
 *
 *      In: pnr    => Promptstr�ng nr, om 0 ingen utskrift.
 *          dstr   => Pekare till defaultstr�ng.
 *          istr   => Pekare till inputstr�ng.
 *          pexnpt => Pekare till pm_ptr.
 *
 *      Ut: *pexnpt => PM-pekare till expression-nod.
 *
 *      FV:       0 => Ok.
 *           REJECT => Operationen avbruten.
 *           GOMAIN => Tillbaks till huvudmenyn
 *           IG2232 => Fel datatyp
 *
 *      (C)microform ab 17/1/85 J. Kjellander
 *
 *      28/10/85 Defaultstr�ng, J. Kjellander
 *      6/3/86   Ny defaulthantering, B. Doverud
 *      6/10/86  GOMAIN, J. Kjellander
 *
 ******************************************************!*/

  {
    short  status;
    sttycl type;

/*
***Skriv ut ev. promtstr�ng.
*/
    if (pnr > 0) igptma(pnr,IG_INP);
/*
***L�s in v�rde.
*/
loop:
    if ( (status=igssip(iggtts(204),istr,dstr,V3STRLEN)) < 0 ) goto exit;

    if (anaexp(istr,FALSE,pexnpt,&type) != 0)
       {
       errmes();
       goto loop;
       }

    if ((type != ST_FLOAT) && (type != ST_INT))
        {
        erpush("IG2232",iggtts(5));
        errmes();
        goto loop;
        }

exit:
    if ( pnr > 0 ) igrsma();
    return(status);

  }
/********************************************************/
/*!******************************************************/

        short genflv(
        short  pnr,
        char   *istr,
        char   *dstr,
        double *fval)

/*      L�ser in ett flyttalsuttryck och ber�knar v�rdet.
 *
 *      In: pnr    => Promptstr�ng nr, om 0 ingen utskrift.
 *          dstr   => Pekare till defaultstr�ng.
 *          istr   => Pekare till inputstr�ng.
 *          fval   => Pekare till resultat.
 *
 *      Ut: *fval  => Flyttal.
 *
 *      FV:       0 => Ok.
 *           REJECT => Operationen avbruten.
 *           GOMAIN => Tillbaks till huvudmenyn
 *
 *      (C)microform ab 29/9/87 J. Kjellander
 *
 ******************************************************!*/

  {
    short  status;
    pm_ptr exnpt,valtyp;
    PMLITVA val;

/*
***Spara aktuell pm-stack pekare.
*/
    pmmark();
/*
***Generera uttryck.
*/
    if ( (status=genflt(pnr,dstr,istr,&exnpt)) < 0 ) goto end;
/*
***Interpretera uttrycket.
*/
    inevev(exnpt,&val,&valtyp);

    if ( val.lit_type == C_INT_VA ) *fval = val.lit.int_va;
    else *fval = val.lit.float_va;
/*
***Slut.
*/
end:
    pmrele();
    return(status);

  }
/********************************************************/
/*!******************************************************/

 static short genuv(
        short   pnr,
        pm_ptr *pexnpt)

/*      Ett UV-v�rde. Skapar en uttrycksnod
 *      av typen vector.
 *
 *      In:
 *           pnr    => Promtnummer. 
 *           pexnpt => Pekare till utdata.
 *
 *      Ut: *pexnpt => PM-pekare till expression node.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *             < 0 => Fel fr�n PM.
 *
 *      (C)microform ab 6/9/95 J. Kjellander
 *
 ******************************************************!*/

  {
    char   ps[2][40];
    char   is[2][80];
    char  *psarr[2];
    char  *isarr[2];
    char  *dsarr[2];
    short  ml[2];
    sttycl type;
    char   expr[132];
    short  status;

    static char ds[2][80] = {"0","0"};

/*
***Initiering.
*/
    strcpy(ps[0],iggtts(182));
    strcpy(ps[1],iggtts(183));

    ml[0]=80;
    ml[1]=80;

    psarr[0] = ps[0];
    psarr[1] = ps[1];

    isarr[0] = is[0];
    isarr[1] = is[1];

    dsarr[0] = ds[0];
    dsarr[1] = ds[1];
/*
***Skriv ut ev. promtstr�ng.
*/
    if ( pnr > 0 ) igptma(pnr,IG_INP);
/*
***L�s in v�rden.
*/    
loop:
    if ( (status=igmsip(psarr,isarr,dsarr,ml,(short)2)) < 0 ) goto exit;
/*
*** Kopiera svaret till defaultstr�ngarna 
*/
    strcpy(ds[0],is[0]); 
    strcpy(ds[1],is[1]);
/*
***Skapa funktionsstr�ngen
*/
    sprintf(expr,"vec(%s,%s)",is[0],is[1]);

    if ( anaexp(expr,FALSE,pexnpt,&type) != 0 )
      {
      errmes();
      goto loop;
      }

    else status = 0;
/*
***Slut.
*/
exit:
    if ( pnr > 0 ) igrsma();
    return(status);
    }

/********************************************************/
/*!******************************************************/

        short genstr(
        short  pnr,
        char   *dstr,
        char   *istr,
        pm_ptr *pexnpt)

/*      Huvudrutin f�r att skapa en uttrycksnod f�r
 *      ett str�ng-v�rde. Provar f�rst att tolka
 *      den inl�sta str�ngen som en str�ngvariabel. Om
 *      ingen variabel av klassen parameter och typen str�ng
 *      finns antas str�ngen representera ett explicit
 *      v�rde.
 *
 *      In: pnr    => Promt-str�ng nr, om 0 ingen utskrift.
 *          dstr   => Pekare till defaultstr�ng.
 *          istr   => Pekare till inputstr�ng.
 *          pexnpt => Pekare till pm_ptr.
 *
 *      Ut: *pexnpt => PM-pekare till expression-nod.
 *
 *      FV:       0 => Ok.
 *           REJECT => Operationen avbruten.
 *           GOMAIN => Tillbaks till huvudmenyn
 *
 *      (C)microform ab 25/2/85  Mats Nelson
 *
 *      28/10/85 Ny def. av PMLITVA, J. Kjellander
 *      15/11/85 anaexp, J. Kjellander
 *      3/12/85  erinit, J. Kjellander
 *      6/3/86   Ny defaulthantering, B. Doverud
 *      23/3/86  Prompt-str�ng B. Doverud
 *      14/4/86  Bug, J. Kjellander
 *      6/10/86  GOMAIN, J. Kjellander
 *
 ******************************************************!*/

  {
    short   status;
    sttycl  type;
    PMLITVA litval;

/*
***Skriv ut ev. promtstr�ng.
*/
    if (pnr > 0) igptma(pnr,IG_INP);
/*
***L�s in v�rde.
*/
    if ( (status=igssip(iggtts(266),istr,dstr,V3STRLEN)) < 0 )
       goto exit;
/*
***Analysera. Om det var ett MBS-str�nguttryck �r allt ok.
*/
    if (anaexp(istr,FALSE,pexnpt,&type) == 0 && type == ST_STR)
      {
      status = 0;
      }
/*
***Inget MBS-str�nguttryck, t�m felstacken och skapa en literal.
*/
    else
      {
      erinit();
      litval.lit_type = C_STR_VA;
      strcpy(litval.lit.str_va,istr);
      pmclie( &litval, pexnpt);
      }

exit:
    if ( pnr > 0 ) igrsma();
    return(status);

  }

/********************************************************/
/*!******************************************************/

        short genstv(
        short  pnr,
        char   *istr,
        char   *dstr,
        char   *strval)

/*      L�ser in ett str�nguttryck och ber�knar v�rdet.
 *
 *      In: pnr    => Promptstr�ng nr, om 0 ingen utskrift.
 *          dstr   => Pekare till defaultstr�ng.
 *          istr   => Pekare till inputstr�ng.
 *          strval => Pekare till resultat.
 *
 *      Ut: *strval => Str�ng.
 *
 *      FV:       0 => Ok.
 *           REJECT => Operationen avbruten.
 *           GOMAIN => Tillbaks till huvudmenyn
 *
 *      (C)microform ab 29/9/87 J. Kjellander
 *
 ******************************************************!*/

  {
    short  status;
    pm_ptr exnpt,valtyp;
    PMLITVA val;

/*
***Spara aktuell pm-stack pekare.
*/
    pmmark();
/*
***Generera uttryck.
*/
    if ( (status=genstr(pnr,dstr,istr,&exnpt)) < 0 ) goto end;
/*
***Interpretera uttrycket.
*/
    inevev(exnpt,&val,&valtyp);

    strcpy(strval,val.lit.str_va);
/*
***Slut.
*/
end:
    pmrele();
    return(status);

  }
/********************************************************/
/*!******************************************************/

        short genstm(
        short   mnum,
        pm_ptr *pexnpt)

/*      Genererar ett uttryck best�ende av aktionskod
 *      f�r valt alternativ i en meny.
 *
 *      In: mnum   = Menynummer.
 *          pexnpt = Pekare till resultat.
 *
 *      Ut: *pexnpt = Str�nguttryck.
 *
 *      FV:       0 => Ok.
 *           REJECT => Operationen avbruten.
 *           GOMAIN => Tillbaks till huvudmenyn
 *
 *      (C)microform ab 11/11/88 J. Kjellander
 *
 *       1/3/94  Snabbval, J. Kjellander
 *
 ******************************************************!*/

  {
    short   alttyp;
    MNUALT *pmualt;
    PMLITVA litval;

/*
***Generera uttryck.
*/
#ifdef WIN32
   msshmu(mnum);
#else
   igaamu(mnum);
#endif

   iggalt(&pmualt,&alttyp);

#ifdef WIN32
   mshdmu();
#endif

    if ( pmualt == NULL )
      {
      switch ( alttyp )
        {
        case SMBRETURN:
        igsamu();
        return(REJECT);

        case SMBMAIN:
        return(GOMAIN);
        }
      }
    else
      {
      litval.lit_type = C_STR_VA;
      litval.lit.str_va[0] = pmualt->acttyp;
      sprintf(&litval.lit.str_va[1],"%d",pmualt->actnum);
      pmclie( &litval, pexnpt);
      igsamu();
      return(0);
      }

   return(0);
  }
/********************************************************/
/*!******************************************************/

        short genref(
        short    pnr,
        DBetype *ptyp,
        pm_ptr  *pexnpt,
        bool    *pend,
        bool    *pright)

/*      Huvudrutin f�r att skapa en uttrycksnod f�r
 *      en referens.
 *
 *      In: pnr    => Promt-str�ng nr, om 0 ingen utskrift.
 *          ptyp   => Pekare till DBetype, typmask.
 *          pexnpt => Pekare till pm_ptr.
 *          pend   => Pekare till bool.
 *          pright => Pekare till bool.
 *
 *      Ut: *pexnpt => PM-pekare till expression-nod.
 *          *pend   => TRUE om pekning i �nden.
 *          *pright => TRUE om pekning till h�ger.
 *          *typ    => Typ av storhet.
 *
 *      FV:       0 => Ok.
 *           REJECT => Operationen avbruten.
 *           GOMAIN => Tillbaks till huvudmenyn
 *
 *      (C)microform ab 17/1/85 J. Kjellander
 *
 *      28/10/85 Ny def. av PMLITVA, J. Kjellander
 *      31/10/85 �nde och sida, J. Kjellander
 *      23/3/86  Prompt-str�ng B. Doverud
 *      6/10/86  GOMAIN, J. Kjellander
 *
 ******************************************************!*/

  {
    short   status;
    PMREFVA idvek[MXINIV];
    PMLITVA litval;

/*
***Skriv ut ev. promtstr�ng.
*/
    if (pnr > 0) igptma(pnr,IG_MESS);
/*
***L�s in ID f�r refererad storhet.
*/
    if ((status=getidt(idvek,ptyp,pend,pright,(short)0)) < 0 ) goto exit;
/*
***Skapa en literal value node av typen referens.
*/
    litval.lit_type = C_REF_VA;
    litval.lit.ref_va[0].seq_val = idvek[0].seq_val;
    litval.lit.ref_va[0].ord_val = idvek[0].ord_val;
    litval.lit.ref_va[0].p_nextre = idvek[0].p_nextre;
    pmclie( &litval, pexnpt);

exit:
    if ( pnr > 0 ) igrsma();
    return(status);
   }

/********************************************************/
/*!******************************************************/

static  short genrfs(
        short    pnr,
        DBetype *ptyp,
        pm_ptr  *pexnpt,
        bool    *pend,
        bool    *pright,
        short    utstat)

/*      Huvudrutin f�r att skapa en uttrycksnod f�r
 *      en referens med m�jlighet att till�ta extra
 *      utstatus.
 *
 *      In: pnr    => Promt-str�ng nr, om 0 ingen utskrift.
 *          ptyp   => Pekare till DBetype, typmask.
 *          pexnpt => Pekare till pm_ptr.
 *          pend   => Pekare till bool.
 *          pright => Pekare till bool.
 *          utstat => Utstatus till getidt().
 *
 *      Ut: *pexnpt => PM-pekare till expression-nod.
 *          *pend   => TRUE om pekning i �nden.
 *          *pright => TRUE om pekning till h�ger.
 *          *typ    => Typ av storhet.
 *
 *      FV:       0 => Ok.
 *           REJECT => Operationen avbruten.
 *           GOMAIN => Tillbaks till huvudmenyn
 *
 *      (C)microform ab 17/11/88 J. Kjellander
 *
 ******************************************************!*/

  {
    short   status;
    PMREFVA idvek[MXINIV];
    PMLITVA litval;

/*
***Skriv ut ev. promtstr�ng.
*/
    if (pnr > 0) igptma(pnr,IG_MESS);
/*
***L�s in ID f�r refererad storhet.
*/
    if ((status=getidt(idvek,ptyp,pend,pright,utstat)) < 0 ) goto exit;
/*
***Skapa en literal value node av typen referens.
*/
    litval.lit_type = C_REF_VA;
    litval.lit.ref_va[0].seq_val = idvek[0].seq_val;
    litval.lit.ref_va[0].ord_val = idvek[0].ord_val;
    litval.lit.ref_va[0].p_nextre = idvek[0].p_nextre;
    pmclie( &litval, pexnpt);

exit:
    if ( pnr > 0 ) igrsma();
    return(status);
   }

/********************************************************/
/*!******************************************************/

       short genpos(
       short  pnr,
       pm_ptr *pexnpt)

/*      Huvudrutin f�r att skapa en uttrycks-nod f�r
 *      en position.
 *
 *      In: pnr    => Promt-str�ng nr, om 0 ingen utskrift.
 *          pexnpt => Pekare till pm_ptr variabel.
 *
 *      Ut: *pexnpt => PM-pekare till expression-node.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Tillbaks till huvudmenyn
 *             < 0 => Fel fr�n PM.
 *
 *      (C)microform ab 15/11/88 J. Kjellander
 *
 *      1998-03-31 default, J.Kjellander
 *
 ******************************************************!*/

  {
    short status;

/*
***Skriv ut ev. promtstr�ng.
*/
    if ( pnr > 0 ) igptma(pnr,IG_MESS);
/*
***Generera position.
*/
   if ( posmod == 0 ) status = gnpmen(pexnpt);
   else
     {
     switch ( posmod )
       {
       case 1:  status = gnpabs(pexnpt); break;
       case 2:  status = gnprel(pexnpt); break;
       case 3:  status = gnpcrh(pexnpt); break;
       case 4:  status = gnpexp(pexnpt); break;
       case 5:  status = gnpend(pexnpt); break;
       case 6:  status = gnpon(pexnpt); break;
       case 7:  status = gnpcen(pexnpt); break;
       case 8:  status = gnpint(pexnpt); break;
       default: status = REJECT; break;
       }
     if ( status == POSMEN ) status = gnpmen(pexnpt);
     }
/*
***Sudda promt.
*/
   if ( pnr > 0 ) igrsma();
/*
***Slut.
*/
   return(status);
  }

/********************************************************/
/*!******************************************************/

       short genpov(DBVector *pos)

/*      Skapar ett positions-v�rde. Ers�tter getpos().
 *
 *      In: pos    => Pekare till position.
 *
 *      Ut: *pos   => Position.
 *
 *      FV: Returnerar status fr�n genpos().
 *
 *      (C)microform ab 15/11/88 J. Kjellander
 *
 ******************************************************!*/

  {
   short   status;
   pm_ptr  exnpt,valtyp;
   PMLITVA val;

/*
***Spara pm-stack pekaren.
*/
   pmmark();
/*
***Skapa positions-uttryck.
*/
   if ( (status=genpos((short)0,&exnpt)) < 0 ) goto end;
/*
***Interpretera.
*/
   inevev(exnpt,&val,&valtyp);

   pos->x_gm = val.lit.vec_va.x_val;
   pos->y_gm = val.lit.vec_va.y_val;
   pos->z_gm = val.lit.vec_va.z_val;
/*
***Slut.
*/
end:
   pmrele();
   return(status);
  }

/********************************************************/
/*!******************************************************/

static short gnpmen(pm_ptr *pexnpt)

/*      Skapar en uttrycks-nod f�r en position via val
 *      i positionsmenyn.
 *
 *      In: pexnpt => Pekare till pm_ptr variabel.
 *
 *      Ut: *pexnpt => PM-pekare till expression-node.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Tillbaks till huvudmenyn
 *             < 0 => Fel fr�n PM.
 *
 *      (C)microform ab 15/11/88 J. Kjellander
 *
 *       1/3/94  Snabbval, J. Kjellander
 *       1998-03-31 H�rkors i 3D, J.Kjellander
 *       1998-10-20 �ven f�r WIN32, J.Kjellander
 *
 ******************************************************!*/

  {
    short   status,alttyp,alt=-1;
    MNUALT *pmualt;

/*
***Skriv ut meny och l�s in svar, snabbval ej till�tet.
*/
#ifdef WIN32
l1:
    msshmu(141);
#else
    igaamu(141);
l1:
#endif

    iggalt(&pmualt,&alttyp);

#ifdef WIN32
    mshdmu();
#endif

    if ( pmualt == NULL )
      {
      switch ( alttyp )
        {
        case SMBRETURN:
        igsamu();
        return(REJECT);

        case SMBMAIN:
        return(GOMAIN);

        case SMBPOSM:
        igbell();
        goto l1;
        }
      }
    else alt = pmualt->actnum;

    switch(alt)
      {
      case 1:                            /* Absolut */
      igptma(326,IG_INP);
      status=gnpabs(pexnpt);
      igrsma();
      break;

      case 2:                            /* Relativt */
      status=gnprel(pexnpt);
      break;

      case 3:                            /* H�rkors */
      status=gnpcrh(pexnpt);
      break;

      case 4:                            /* Uttryck */
      status=gnpexp(pexnpt);
      break;

      case 5:                            /* �nde av storhet */
      status=gnpend(pexnpt);
      break;

      case 6:                            /* P� en storhet */
      status=gnpon(pexnpt);
      break;

      case 7:                            /* Centrum av storhet */
      status=gnpcen(pexnpt);
      break;

      case 8:                            /* Sk�rning */
      status=gnpint(pexnpt);
      break;

      case 10:                            /* Byt referens */
      if ( tmpref ) tmpref = FALSE;
      else tmpref = TRUE;
      goto l1;

      default:                           /* Ok�nt alt. */
      erpush("IG0103","");
      errmes();
      goto l1;
      }

    if ( status == REJECT )
      {
      goto l1;
      }
/*
***Sudda positionsmenyn och �terv�nd.
*/
    if ( status != GOMAIN ) igsamu();

    return(status);
  }

/********************************************************/
/*!******************************************************/

static short gnpabs(pm_ptr *pexnpt)

/*      Position med X,Y,Z format. Skapar en uttrycksnod
 *      av typen vector.
 *
 *      In: pexnpt => Pekare till pm_ptr variabel.
 *
 *      Ut: *pexnpt => PM-pekare till expression node.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *             < 0 => Fel fr�n PM.
 *
 *      (C)microform ab 16/1/85 J. Kjellander
 *
 *      22/10/85 L�ngre inputstr�ngar, J. Kjellander
 *      6/10/86  GOMAIN, J. Kjellander
 *      8/10/86  Promt, J. Kjellander
 *
 ******************************************************!*/

  {
    char   ps[3][40];     /* promptstr�ng */
    char   is[3][80];     /* inputstr�ng */
    char  *psarr[3];
    char  *isarr[3];
    char  *dsarr[3];
    short  ml[3];         /* maxl�ngder */
    sttycl type;
    char   expr[132];
    short  status;

    static char ds[3][80] = {"0","0","0"};

/*
***Initiering.
*/
    strcpy(ps[0],iggtts(201));       /* kopiera promptar */
    strcpy(ps[1],iggtts(202));
    strcpy(ps[2],iggtts(203));

    ml[0]=80;
    ml[1]=80;
    ml[2]=80;

    psarr[0] = ps[0];                  /* initiera pekarna */
    psarr[1] = ps[1];
    psarr[2] = ps[2];

    isarr[0] = is[0];
    isarr[1] = is[1];
    isarr[2] = is[2];

    dsarr[0] = ds[0];
    dsarr[1] = ds[1];
    dsarr[2] = ds[2];
/*
***L�s in v�rden.
*/    
loop:
    if ( (status=igmsip(psarr,isarr,dsarr,ml,modtyp)) < 0 ) return(status);
    if ( modtyp == 2 ) strcpy(is[2],"0");
/*
*** Kopiera svaret till defaultstr�ngarna 
*/
    strcpy(ds[0],is[0]); 
    strcpy(ds[1],is[1]);
    strcpy(ds[2],is[2]);
/*
***Skapa funktionsstr�ngen
*/
    if ( modtyp == 2 )
      {
      sprintf(expr,"vec(%s,%s)",is[0],is[1]);
      }
    else
      {
      sprintf(expr,"vec(%s,%s,%s)",is[0],is[1],is[2]);
      }

    if (anaexp(expr,FALSE,pexnpt,&type) != 0)
      {
      errmes();
      goto loop;
      }

    if ( posmod > 0 ) posmod = 1;

    return(0);
    }

/********************************************************/
/*!******************************************************/

static short gnprel(pm_ptr *pexnpt)

/*      Position relativt position. Skapar en uttrycksnod
 *      av typen vector.
 *
 *      In: pexnpt => Pekare till pm_ptr variabel.
 *
 *      Ut: *pexnpt => PM-pekare till expression node.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => �ter till huvudmenyn
 *
 *      (C)microform ab 9/8/85 J. Kjellander
 *
 *      23/3/86  Anrop genpos(pnr,... B. Doverud
 *      6/10/86  GOMAIN, J. Kjellander
 *
 ******************************************************!*/

  {
    pm_ptr  exnpt1,exnpt2;
    short   status;

/*
***Skapa basposition.
*/
     igptma(51,IG_MESS);
     if ( (status=gnpmen(&exnpt1)) < 0 ) goto end;
     igrsma();
/*
***Skapa "vec(dx,dy,dz)".
*/
     igptma(287,IG_INP);
     if ( (status=gnpabs(&exnpt2)) < 0 ) goto end;
/*
***L�nka ihop till "pos + vec(dx,dy,dz)".
*/
    pmcbie(PM_PLUS,exnpt1,exnpt2,pexnpt);

end:
    if ( posmod > 0 ) posmod = 2;
    igrsma();
    return(status);
    }

/********************************************************/
/*!******************************************************/

static short gnpcrh(pm_ptr *pexnpt)

/*      Position med h�rkors. Skapar en literal
 *      av typen vector.
 *
 *      In: pexnpt => Pekare till pm_ptr variabel.
 *
 *      Ut: *pexnpt => PM-pekare till expression node.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn
 *
 *      (C)microform ab 3/7/85 J. Kjellander
 *
 *      3/10/86  GOMAIN, J. Kjellander
 *
 ******************************************************!*/

  {
    DBVector   pos;
    PMLITVA litstr;
    short   status;

/*
***L�s in h�rkorsposition.
*/
    if ( (status=gtpcrh(&pos)) < 0 ) return(status);
/*
***Skapa literal.
*/
    litstr.lit_type = C_VEC_VA;
    litstr.lit.vec_va.x_val = pos.x_gm;
    litstr.lit.vec_va.y_val = pos.y_gm;
    litstr.lit.vec_va.z_val = pos.z_gm;
    pmclie(&litstr,pexnpt);

    if ( posmod > 0 ) posmod = 3;

    return(0);
  }

/********************************************************/
/*!******************************************************/

static short gnpexp(pm_ptr *pexnpt)

/*      Position med uttryck. Skapar en identifierar-
 *      uttrycksnod.
 *
 *      In: pexnpt => Pekare till pm_ptr variabel.
 *
 *      Ut: *pexnpt => PM-pekare till expression node.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn
 *
 *      Felkoder: IG2063 = Variabeln har fel datatyp
 *
 *      (C)microform ab 16/1/85 J. Kjellander
 *
 *      6/10/86 GOMAIN, J. Kjellander
 *      7/10/86 errmes(), J. Kjellander
 *
 ******************************************************!*/

    {
    char    istr[V3STRLEN+1];
    sttycl  type;
    short   status;

/*
***L�s in uttryck.
*/
    igptma(209,IG_INP);
loop:
    if ( (status=igssip(iggtts(267),istr,"",V3STRLEN)) < 0 ) goto end;
/*
***Prova att analysera.
*/
    if ( anaexp(istr,FALSE,pexnpt,&type ) != 0 )
      {
      errmes();
      goto loop;
      }
/*
***Kolla att det var r�tt typ.
*/
    if (type != ST_VEC)
      {
      erpush("IG2232",iggtts(7));
      errmes();
      goto loop;
      }
/*
***Slut.
*/
end:
    if ( posmod > 0 ) posmod = 4;
    igrsma();
    return(status);
    }

/********************************************************/
/*!******************************************************/

static short gnpend(pm_ptr *pexnpt)

/*      Huvudrutin f�r position i �nden av storhet med fast
 *      eller tempor�r referens.
 *
 *      In: pexnpt => Pekare till pm_ptr variabel.
 *
 *      Ut: *pexnpt => PM-pekare till expression node.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn
 *
 *      (C)microform ab 1/11/85 J. Kjellander
 *
 *      20/3/86    Anrop pmtcon, pmclie B. Doverud
 *      6/10/86    GOMAIN, J. Kjellander
 *      1997-04-21 STARTP/ENDP, J.Kjellander
 *
 ******************************************************!*/

  {
    DBetype   typ;
    pm_ptr  arglst,ref,dummy;
    pm_ptr  exnpt;
    stidcl  kind;
    bool    end,right;
    short   status;
    PMLITVA litstr;
    DBVector   posvec;
    PMREFVA idvek[MXINIV];

/*
***Tempor�r referens.
*/
    if ( tmpref)
      {
      if ( (status=gtpend(&posvec)) < 0 ) goto exit;

      litstr.lit_type = C_VEC_VA;
      litstr.lit.vec_va.x_val = posvec.x_gm;
      litstr.lit.vec_va.y_val = posvec.y_gm;
      litstr.lit.vec_va.z_val = posvec.z_gm;
      pmclie( &litstr, pexnpt);
      }
/*
***Fast referens.
*/
    else
      {
/*
***H�mta id f�r den refererade storheten.
*/
      typ = LINTYP+ARCTYP+CURTYP;
      igptma(331,IG_MESS);
      if ( (status=getidt(idvek,&typ,&end,&right,POSMEN)) < 0 ) goto exit;
/*
***Skapa PM-referens.
*/
      litstr.lit_type = C_REF_VA;
      litstr.lit.ref_va[0].seq_val = idvek[0].seq_val;
      litstr.lit.ref_va[0].ord_val = idvek[0].ord_val;
      litstr.lit.ref_va[0].p_nextre = idvek[0].p_nextre;
      pmclie( &litstr, &exnpt);
/*
***Skapa argumentlistan.
*/
      pmtcon(exnpt,(pm_ptr)NULL,&arglst,&dummy);
/*
***Skapa funktionsuttrycksnoden.
*/
      if ( end ) stlook("ENDP",&kind,&ref);
      else       stlook("STARTP",&kind,&ref);

      pmcfue(ref,arglst,pexnpt);
      }
/*
***Avslutning.
*/
exit:
    if ( posmod > 0 ) posmod = 5;
    igrsma();
    return(status);
  }

/********************************************************/
/*!******************************************************/

static short gnpon(pm_ptr *pexnpt)

/*      Huvudrutin f�r position p� en storhet med fast
 *      eller tempor�r referens.
 *
 *      In: pexnpt => Pekare till pm_ptr variabel.
 *
 *      Ut: *pexnpt => PM-pekare till expression node.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn
 *
 *      (C)microform ab 16/1/85 J. Kjellander
 *
 *      28/10/85 Ny def. av PMLITVA, J. Kjellander
 *      28/10/85 �nde och sida, J. Kjellander
 *      20/3/86  Anrop pmtcon, pmclie B. Doverud
 *      24/3/86  Felutg�ng B. Doverud
 *      6/10/86  GOMAIN, J. Kjellander
 *      1998-09-24 b_plan, J.Kjellander
 *
 ******************************************************!*/

  {
    DBetype   typ;
    pm_ptr  retla,arglst,ref,dummy;
    pm_ptr  exnpt1,exnpt2;
    stidcl  kind;
    bool    end,right;
    char    istr[V3STRLEN+1];
    short   status;
    PMLITVA litstr;
    DBVector   posvec;

    static char dstr[V3STRLEN+1] = "0.5";

/*
***Tempor�r referens.
*/
    if ( tmpref)
      {
      if ( (status=gtpon(&posvec)) < 0 ) goto exit;

      litstr.lit_type = C_VEC_VA;
      litstr.lit.vec_va.x_val = posvec.x_gm;
      litstr.lit.vec_va.y_val = posvec.y_gm;
      litstr.lit.vec_va.z_val = posvec.z_gm;
      pmclie( &litstr, pexnpt);
      }
/*
***Fast referens.
*/
    else
      {
/*
***Skapa referens.
*/
      typ = POITYP+LINTYP+ARCTYP+CURTYP+CSYTYP+BPLTYP+
            TXTTYP+LDMTYP+CDMTYP+RDMTYP+ADMTYP+SURTYP;
      if ( (status=genrfs(52,&typ,&exnpt1,&end,&right,POSMEN)) < 0 ) goto exit;
/*
***Alla typer av storheter kr�ver en referens som argument.
*/
      pmtcon(exnpt1,(pm_ptr)NULL,&retla,&dummy);
/*
***Tr�dstorheter har en FLOAT som ytterligare parameter.
*/
      if ( typ == LINTYP  ||  typ == ARCTYP  ||  typ == CURTYP )
        {
        if ( (status=genflt((short)208,dstr,istr,&exnpt2)) < 0 ) goto exit;
        pmtcon(exnpt2,retla,&arglst,&dummy);
        strcpy(dstr,istr);
        }
/*
***Ytor har en VECTOR som ytterligare argument.
*/
      else if ( typ == SURTYP  ||  typ == BPLTYP )
        {
        if ( (status=genuv((short)208,&exnpt2)) < 0 ) goto exit;
        pmtcon(exnpt2,retla,&arglst,&dummy);
        }
/*
***�vriga storheter har ingen ytterligare parameter.
*/
      else arglst = retla;
/*
***Skapa funktionsuttrycksnoden.
*/
      stlook("ON",&kind,&ref);
      pmcfue(ref,arglst,pexnpt);
      }
/*
***Avslutning.
*/
exit:
    if ( posmod > 0 ) posmod = 6;
    return(status);
  }

/********************************************************/
/*!******************************************************/

static short gnpint(pm_ptr *pexnpt)

/*      Huvudrutin f�r position sk�rning mellan storheter
 *      med fast eller tempor�r referens.
 *
 *      In: pexnpt => Pekare till pm_ptr variabel.
 *
 *      Ut: *pexnpt => PM-pekare till expression-node.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn.
 *
 *      (C)microform ab 10/1/85 J. Kjellander
 *
 *      28/10/85 Ny def. av PMLITVA, J. Kjellander
 *      28/10/85 �nde och sida, J. Kjellander
 *      6/3/86   Ny defaulthantering, B. Doverud
 *      20/3/86  Anrop pmtcon, pmclie B. Doverud
 *      24/3/86  Felutg�ng B. Doverud
 *      7/10/86  GOMAIN, J. Kjellander
 *      20/11/89 Neg. intnr, J. Kjellander
 *      23/12/91 Bplan och koord.sys, J. Kjellander
 *      7/9/95   Ytor, J. Kjellander
 *
 ******************************************************!*/

  {
    DBetype   typ1,typ2;
    pm_ptr  retla,arglst,ref,dummy;
    stidcl  kind;
    pm_ptr  exnpt1,exnpt2,exnpt3;
    short   status;
    bool    end,right;
    char    istr[V3STRLEN+1];
    DBVector   posvec;
    PMLITVA litstr;

/*
***Tempor�r referens.
*/
    if ( tmpref || (v3mode == RIT_MOD) )
      {
      if ( (status=gtpint(&posvec)) < 0 ) goto exit;

      litstr.lit_type = C_VEC_VA;
      litstr.lit.vec_va.x_val = posvec.x_gm;
      litstr.lit.vec_va.y_val = posvec.y_gm;
      litstr.lit.vec_va.z_val = posvec.z_gm;
      pmclie( &litstr, pexnpt);
      }
/*
***Fast referens.
*/
    else
      {
/*
***L�s in 2 referenser.
*/
      typ1 = LINTYP+ARCTYP+CURTYP;
      if ( modtyp == 3 ) typ1 += BPLTYP+CSYTYP+SURTYP;
      if ( (status=genrfs(324,&typ1,&exnpt1,&end,
                          &right,POSMEN)) < 0 ) goto exit;

      typ2 = LINTYP+ARCTYP+CURTYP;
      if ( modtyp == 3  &&  typ1 != BPLTYP  &&
                            typ1 != CSYTYP  &&
                            typ1 != SURTYP )
        typ2 += BPLTYP+CSYTYP+SURTYP;
      if ( (status=genrfs(325,&typ2,&exnpt2,&end,
                          &right,POSMEN)) < 0 ) goto exit;
/*
***Om sk�rning linje/linje, alt = -1.
*/
      if ( typ1 == LINTYP && typ2 == LINTYP )
         {
         litstr.lit_type = C_INT_VA;
         litstr.lit.int_va = -1;
         pmclie( &litstr, &exnpt3);
         }
/*
***Annars l�s in alternativ.
*/
      else
         {
         if ( (status=genint(327,"1",istr,&exnpt3)) < 0 ) goto exit;
         }
/*
***Skapa argumentlistan.
*/
      pmtcon( exnpt1, (pm_ptr)NULL, &retla, &dummy);
      pmtcon( exnpt2, retla, &retla, &dummy);
      pmtcon( exnpt3, retla, &arglst, &dummy);
/*
***Skapa funktionsuttrycksnoden.
*/
      stlook("INTERSECT",&kind,&ref);
      pmcfue( ref, arglst, pexnpt);
      }
/*
***Avslutning.
*/
exit:
    if ( posmod > 0 ) posmod = 8;
    return(status);

  }

/********************************************************/
/*!******************************************************/

static short gnpcen(pm_ptr *pexnpt)

/*      Huvudrutin f�r position i kr�kningscentrum med fast
 *      eller tempor�r referens.
 *
 *      In: pexnpt => Pekare till pm_ptr variabel.
 *
 *      Ut: *pexnpt => PM-pekare till expression node.
 *
 *      FV:      0 => Ok.
 *          REJECT => Operationen avbruten.
 *          GOMAIN => Huvudmenyn
 *
 *      (C)microform ab 26/4/87 J. Kjellander
 *
 ******************************************************!*/

  {
    DBetype   typ;
    pm_ptr  retla,arglst,ref,dummy;
    pm_ptr  exnpt1,exnpt2;
    stidcl  kind;
    bool    end,right;
    char    istr[V3STRLEN+1];
    short   status;
    PMLITVA litstr;
    DBVector   posvec;

    static char dstr[V3STRLEN+1] = "0.0";

/*
***Tempor�r referens.
*/
    if ( tmpref)
      {
      if ( (status=gtpcen(&posvec)) < 0 ) goto exit;

      litstr.lit_type = C_VEC_VA;
      litstr.lit.vec_va.x_val = posvec.x_gm;
      litstr.lit.vec_va.y_val = posvec.y_gm;
      litstr.lit.vec_va.z_val = posvec.z_gm;
      pmclie( &litstr, pexnpt);
      }
/*
***Fast referens.
*/
    else
      {
/*
***Skapa referens.
*/
      typ = ARCTYP+CURTYP;
      if ( (status=genrfs(53,&typ,&exnpt1,&end,&right,POSMEN)) < 0 ) goto exit;
/*
***Om arc, s�tt parameterv�rdet till 0.
*/
      if ( typ == ARCTYP )
        {
        litstr.lit_type = C_FLO_VA;
        litstr.lit.float_va = 0.0;
        pmclie( &litstr, &exnpt2);
        }
/*
***Om kurva l�s i parameterv�rde.
*/
      else
        {
        if ( (status=genflt(208,dstr,istr,&exnpt2)) < 0 ) goto exit;
        strcpy(dstr,istr);
        }
/*
***Skapa argumentlistan.
*/
      pmtcon( exnpt1, (pm_ptr)NULL, &retla, &dummy);
      pmtcon( exnpt2, retla, &arglst, &dummy);
/*
***Skapa funktionsuttrycksnoden.
*/
      stlook("CENTRE",&kind,&ref);
      pmcfue( ref, arglst, pexnpt);
      }
/*
***Avslutning.
*/
exit:
    if ( posmod > 0 ) posmod = 7;
    return(status);
  }

/********************************************************/
/*!******************************************************/

        short igpmon()

/*      S�tter posmod = 0, dvs. g�r att genpos alltid
 *      k�r med hela pos-menyn.
 *
 *      FV:      0 => Ok.
 *
 *      (C)microform ab 15/11/88 J. Kjellander
 *
 ******************************************************!*/

  {
    posmod = 0;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short igpmof()

/*      S�tter posmod = 1, dvs. g�r att genpos till
 *      att b�rja med v�ljer gnpabs() och sedan det
 *      sist valda.
 *
 *      FV:      0 => Ok.
 *
 *      (C)microform ab 15/11/88 J. Kjellander
 *
 ******************************************************!*/

  {
    posmod = 1;

    return(0);
  }

/********************************************************/
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
 *      2006-12-08 Tagit bort gpgtla(), J.Kjellander
 *
 ******************************************************!*/

  {
    char    pektkn;
    short   ix,iy,status;
    DBetype typmsk,pektyp;
    DBptr   la;
    char    idstr[V3STRLEN+1];
    DBint   win_id;

/*
***L�s h�rkors.
*/
loop:
#ifdef UNIX
    WPgtsc(FALSE,&pektkn,&ix,&iy,&win_id);
#endif
#ifdef WIN32
    msgtsc(FALSE,&pektkn,&ix,&iy,&win_id);
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
    if ( (la=WPgtla(win_id,typmsk,ix,iy,&pektyp,end,right)) < 0 ) goto typerr;
    DBget_id(la,idvek);
    idvek->seq_val = abs(idvek->seq_val);
/*
***Storheten hittad, highlighting.
*/
finish:
    WPhgen(GWIN_ALL,la,TRUE);
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
          WPepmk(GWIN_ALL);
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
    DBint   win_id;

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
#ifdef UNIX
    WPgtsc(TRUE,&pektkn,&ix1,&iy1,&win_id);
#endif
#ifdef WIN32
    msgtsc(TRUE,&pektkn,&ix1,&iy1,&win_id);
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
#ifdef UNIX
    WPgtsc(TRUE,&pektkn,&ix2,&iy2,&win_id);
#endif
#ifdef WIN32
    msgtsc(TRUE,&pektkn,&ix2,&iy2,&win_id);
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
#ifdef UNIX
    WPgmla(win_id,ix1,iy1,ix2,iy2,mode,TRUE,&nref,typvek,lavek);
#endif
#ifdef WIN32
    WPgmla(win_id,ix1,iy1,ix2,iy2,mode,TRUE,&nref,typvek,lavek);
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
#ifdef UNIX
    status = WPgmc3(&pektkn,vecptr,TRUE);
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
      WPepmk(GWIN_ALL);
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
    double x,y,xg,yg,ng;

/*
***L�s in 2D modellkoordinater via h�rkors.
*/
loop:
    WPgtmc(&pektkn,&x,&y,TRUE);
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
    DBfloat   t;
    DBetype   typ;
    DBptr   la;
    bool    end,right;
    short   status;
    DBId    idvek[MXINIV];
    DBAny  gmpost;

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
    if ( EXon(idvek,t,(DBfloat)0.0,vecptr) < 0 )
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
    if ( EXon (idvek,t,(DBfloat)0.0,vecptr) < 0 )
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
    DBAny  gmpost;
    DBSeg   arcseg[4];

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
