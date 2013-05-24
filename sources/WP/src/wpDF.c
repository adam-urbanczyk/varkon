/**********************************************************************
*
*    wpDF.c
*    ======
*
*    This file is part of the VARKON WindowPac Library.
*    URL: http://www.tech.oru.se/cad/varkon
*
*    This file includes:
*
*    WPgtla();    Selects nearest entity
*    WPgmla();    Selects all entities in rectangle
*    WPgmlw();    WPgmla() with rubberband rectangle
*
*    WPsply();    Store polyline in displayfile
*
*    WPfobj();    Search for specific entity in displayfile
*    WPdobj();    Display/erase current entity
*    WProbj();    Delete entity
*
*    WPhgen();    Highlight entity in one or all windows
*    WPerhg();    Erase all highligt markers
*
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
#include "../include/WP.h"
#include <math.h>

#define DF_ERASED  -1      /* Kod f�r suddat DF-objekt */

extern short   actpen;

#ifdef WIN32
extern int     msgtsw();
#endif

/*!******************************************************/

        DBptr    WPgtla(
        DBint    win_id,
        DBetype  typmsk,
        short    ix,
        short    iy,
        DBetype *typ,
        bool    *ends,
        bool    *right)

/*      Returnerar GM-adress f�r storhet n�rmast
 *      f�nster-positionen ix, iy och med typ som
 *      st�mmer med typmsk.
 *
 *      In: win_id => F�nster att g�ra s�kning i.
 *          typmsk => Mask f�r ev. begr�nsad s�kning
 *          ix,iy  => Koordinat i sk�rmkoordinater
 *          typ    => Pekare till DBetype-variabel.
 *
 *      Ut: *typ   => Typ f�r den funna storheten.
 *                    Odefinierad om ingen storhet
 *                    hittats.
 *          *ends  => Sk�rmpos. ligger n�rmast obj. slut
 *          *right => Sk�rmpos. ligger p� obj. h�gra sida.
 *
 *
 *      FV: >= 0  => GM-adress f�r n�rmsta storhet.
 *            -1  => Ingen storhet hittat.
 *
 *      (C)microform ab 10/1-95 J. Kjellander
 *
 *      2006-12-08 Removed gpgtla(), J.Kjellander
 *
 ******************************************************!*/

  {
    DBint   i,n,j;
    DBint   imin,nmin,ilim,dist,mdist;
    DBint   t1=0,t2=0;
    DBint   ixt,iyt,idx,idy;
    char    a='\0';
    DBfloat   tt;
    DFPOST *dfi,*dfn;
    WPWIN  *winptr;
    WPGWIN *gwinpt;

/*
***Initiering.
*/
    mdist = 2000000000;
    imin = -1;   /* Pekare till n�rmsta objekt */
    nmin = 0;    /* Pekare till n�rmsta vektor */
/*
***Vilket f�nster �r det fr�ga om ?
*/
    if ( (winptr=WPwgwp((wpw_id)win_id)) != NULL  &&
          winptr->typ == TYP_GWIN ) gwinpt = (WPGWIN *)winptr->ptr;
    else return(-1);
/*
***Om ingen DF finns �r ingen s�kning m�jlig.
*/
    if ( gwinpt->df_adr == NULL ) return(-1);
/*
***H�r b�rjar loopen f�r hela displayfilen.
*/
    for ( i=0; i<gwinpt->df_ptr;  i += 3 + dfi->hdr.nvec)
       {
       dfi = gwinpt->df_adr + i;
       if (((dfi->hdr.type & typmsk) > 0) && ((dfi+1)->la != DF_ERASED))
         {
/*
***Objekt typ punkt.
*/
         if (dfi->hdr.nvec == 0 )
           { 
           ixt = (DBint)(ix - (dfi+2)->vec.x);
           iyt = (DBint)(iy - (dfi+2)->vec.y);
          
           if ((dist = ixt*ixt + iyt*iyt) < mdist)
             {
             mdist = dist;
             imin = i;
             a = (dfi+2)->vec.a;
             }
           }
/*
***Objekt typ polyline.
*/
         else
           {
           ilim = i + 2 + dfi->hdr.nvec;
           for ( n=i + 2; n<ilim; n++ )
              {
              dfn = gwinpt->df_adr + n;
              idx = (dfn+1)->vec.x - dfn->vec.x;
              idy = (dfn+1)->vec.y - dfn->vec.y;
              ixt = ix - dfn->vec.x;
              iyt = iy - dfn->vec.y;
              j = 0;
              if ( idx != 0  ||  idy != 0 )
                {
                if ( (t1=idx*ixt + idy*iyt) > 0 ) 
                  {                                     /* t >= 0 */
                  if ( t1 >= (t2 = idx*idx + idy*idy) )
                    {
                    ixt -= idx;                         /* t >= 1 */
                    iyt -= idy;
                    j = 1;
                    }
                  else
                    {        
                    ixt -= (idx*t1)/t2;                 /* 0 < t < 1 */
                    iyt -= (idy*t1)/t2;
                    j = 2;
                    }
                  }
                }
/*
***�r denna vektor n�rmast hittills?
*/            
              if ( (dist=ixt*ixt + iyt*iyt) < mdist )
                {
                if ( ((dfn+1)->vec.a & VISIBLE) == VISIBLE )
                  {
                  mdist = dist;
                  imin = i;
                  nmin = n;
                  if (j == 0) a = dfn->vec.a;             /* t = 0 */
                  else if (j == 1) a = (dfn+1)->vec.a;    /* t = 1 */
                  else if (2*t1 < t2) a = dfn->vec.a;     /* t < 0.5 */
                  else a = (dfn+1)->vec.a;                /* t >= 0.5 */
                  }
                }
             }
          }
       }
    }
/*
***Avslutning.
*/
    if ( imin != -1 )
      {
      gwinpt->df_cur = imin;           /* Objektet blir aktuellt */
      dfi = gwinpt->df_adr + imin;
      *typ = dfi->hdr.type;
/*
***Om polylinje, ber�kna �nde och sida.
*/
      if ( dfi->hdr.nvec != 0 )
        {
        *ends = ((a & ENDSIDE) != 0);
        *right = TRUE;

        dfn = gwinpt->df_adr + nmin;
        idx = (dfn+1)->vec.y - dfn->vec.y;
        idy = dfn->vec.x - (dfn+1)->vec.x;
        ixt = ix - dfn->vec.x;
        iyt = iy - dfn->vec.y;
        tt = idx*idx + idy*idy;

        if ( tt > 0.0 )
          {
          tt = SQRT(tt);
          tt = idx*ixt/tt + idy*iyt/tt;
          if ( tt < 0.0 ) *right = FALSE;
          }
        }
      return((dfi+1)->la);
      }

    else return(-1);
  }
     
/********************************************************/
/*!******************************************************/

        short   WPgmla(
        DBint   win_id,
        short   ix1,
        short   iy1,
        short   ix2,
        short   iy2,
        short   mode,
        bool    hl,
        short  *nla,
        DBetype typvek[],
        DBptr   lavek[])

/*      Returnerar LA f�r alla DF-objekt inom/utom ett
 *      f�nster.
 *
 *      In: ix1,iy1  =  F�nstrets nedre v�nstra h�rn.
 *          ix2,iy2  =  F�nstrets �vre h�gra h�rn.
 *          mode     =  0 = Alla helt inom
 *                      1 = Alla helt eller delvis inom
 *                      2 = Alla helt utom
 *                      3 = Alla helt eller delvis utom
 *          hl       = TRUE => Highligting
 *          *nla     =  Max antal objekt som f�r returneras.
 *          typvek�0�=  Typmask f�r ev begr�nsad s�kning
 *          win_id   =  F�nster att leta i.
 *
 *      Ut: *nla     =  Antal objekt som uppfyller villkoren.
 *          *typvek  =  Typ f�r nla objekt.
 *          *lavek   =  LA f�r nla objekt.
 *
 *      FV:  0
 *
 *      (C) microform ab 13/1/95 J. Kjellander
 *
 *      1997-04-10 Bug, J.Kjellander
 *      2006-12-08 Removed gpgtla(), J.Kjellander
 *
 ******************************************************!*/

 {
    DBint   i,j,k;
    short   maxobj,nv,x,y,nhit,nin,nut;
    DBetype orgtyp;
    bool    inside,outside,hit;
    DFPOST *df;
    WPWIN  *winptr;
    WPGWIN *gwinpt,tmpwin;
    WPGRPT  pt1,pt2;

/*
***Initiering.
*/
    tmpwin.vy.modwin.xmin = ix1;
    tmpwin.vy.modwin.ymin = iy1;
    tmpwin.vy.modwin.xmax = ix2;
    tmpwin.vy.modwin.ymax = iy2;


    orgtyp = typvek[0];
    nhit = 0;
    maxobj = *nla;
/*
***Vilket f�nster �r det fr�ga om ?
*/
    if ( (winptr=WPwgwp((wpw_id)win_id)) != NULL  &&
          winptr->typ == TYP_GWIN ) gwinpt = (WPGWIN *)winptr->ptr;
    else return(-1);
/*
***H�r b�rjar s�kning genom hela displayfilen.
*/
    df = gwinpt->df_adr;
    i  = 0;

    while ( i < gwinpt->df_ptr )
      {
/*
***S�k upp n�sta icke suddade objekt av r�tt typ.
*/
      if ( (df+i+1)->la != DF_ERASED  &&  ((df+i)->hdr.type & orgtyp) > 0 )
        {
        nv = (df+i)->hdr.nvec;
/*
***Klassificera objekt av typen "punkt".
*/
        if ( nv == 0 )
          {
          x = (df+i+2)->vec.x;  y = (df+i+2)->vec.y;
          inside = ( (x > ix1) && (x < ix2) && 
                     (y > iy1) && (y < iy2) );
          outside  = !inside;
          }
/*
***Klassificera objekt av typen "polyline".
*/
        else
          {
          j = i+2;
          k = j+nv;
          nin = nut = 0;
          for ( ; j<k; ++j )
            {
            if ( ((df+j+1)->vec.a & VISIBLE) == VISIBLE )
              {
              pt1.x = (df+j)->vec.x;     pt1.y = (df+j)->vec.y;
              pt2.x = (df+j+1)->vec.x; pt2.y = (df+j+1)->vec.y;

              switch ( WPclin(&tmpwin,&pt1,&pt2) )
                {
                case -1: ++nut; break;
                case  0: ++nin; break;
                default: ++nut; ++nin; break;
                }
              }
            }
          inside = outside = FALSE;
          if ( nin > 0 ) inside = TRUE;
          if ( nut > 0 ) outside = TRUE;
          }
/*
***Uppfyller objektet de st�llda kraven ?
*/
        hit = FALSE;
        switch ( mode)
          {
          case 0: if ( inside  &&  !outside ) hit = TRUE; break;
          case 1: if ( inside ) hit = TRUE; break;
          case 2: if ( outside  &&  !inside ) hit = TRUE; break;
          case 3: if ( outside ) hit = TRUE; break;
          }
        if ( hit )
          {
          if ( nhit == maxobj ) goto end;
          lavek[nhit] = (df+i+1)->la;
          typvek[nhit] = (df+i)->hdr.type;
          if ( hl ) WPhgen(GWIN_ALL,lavek[nhit],TRUE);
          nhit++;
          }
        }
/*
***N�sta objekt i DF.
*/
      i += 3 + (df+i)->hdr.nvec;
      }
/*
***Slut.
*/
end:
     *nla = nhit;

     return(0);
 }

/********************************************************/
/*!******************************************************/

        short WPgmlw(
        DBptr    lavek[],
        short   *idant,
        DBetype  typvek[],
        short    hitmod)

/*      Returnerar idant LA mha. WPgtsw() (f�nster)
 *      och WPgmla(). Anv�nds av IDENT_2().
 *
 *      In: idmat     = Pekare till resultat.
 *         *idant     = Max antal ID.
 *          typvek[0] = �nskad typ.
 *          hitmod    = 0 = Alla helt inom
 *                      1 = Alla helt eller delvis inom
 *                      2 = Alla helt utom
 *                      3 = Alla helt eller delvis utom
 *
 *      Ut: *idmat  = Identiteter.
 *          *idant  = Verkligt antal ID.
 *          *typvek = Typer.
 *
 *      FV: 0, REJECT, GOMAIN fr�n WPgtsw().
 *
 *      (C)microform ab 1998-03-19 J. Kjellander
 *
 *      1998-04-29 Bug nref -> short, J.Kjellander
 *
 ******************************************************!*/

  {
    short   status,nref;
    int     ix1,ix2,iy1,iy2,tmp;
    bool    ends;
    bool    right;
    WPGWIN *gwinpt;

/*
***Initiering.
*/
   nref   = *idant;
  *idant  = 0;
/*
***L�s in f�nsterkoordinater.
*/
#ifdef UNIX
   status = WPgtsw(&gwinpt,&ix1,&iy1,&ix2,&iy2,WP_RUB_RECT,FALSE);
   if ( status == REJECT  ||  status == GOMAIN ) return(status);
#endif
#ifdef WIN32
   status = (short)msgtsw(&gwinpt,&ix1,&iy1,&ix2,&iy2,WP_RUB_RECT,FALSE);
   if ( status == REJECT  ||  status == GOMAIN ) return(status);
#endif
/*
***Sortera koordinaterna.
*/
   if ( ix2 < ix1 ) { tmp = ix1; ix1 = ix2; ix2 = tmp; }
   if ( iy2 < iy1 ) { tmp = iy1; iy1 = iy2; iy2 = tmp; }
/*
***Om p1=p2 (status=-1) har man bara clickat p� musen utan att r�ra den.
***D� anv�nder vi WPgtla(), annars WPgmla().
*/
   if ( status == -1 )
     {
     lavek[0] = WPgtla(gwinpt->id.w_id,typvek[0],(short)ix1,(short)iy1,
                                    &typvek[0],&ends,&right);
     if ( lavek[0] > 0 ) *idant = 1;
     else                *idant = 0;
     }
   else
     {
     WPgmla(gwinpt->id.w_id,(short)ix1,(short)iy1,(short)ix2,(short)iy2,
                                    hitmod,FALSE,&nref,typvek,lavek);
    *idant = nref;
     }
/*
***Slut.
*/

   return(0);
  }

/********************************************************/
/********************************************************/

   bool WPsply(
        WPGWIN  *gwinpt,
        int      k,
        double   x[],
        double   y[],
        char     a[],
        DBptr    la,
        DBetype  typ)

/*      Lagrar grapac-polyline i DF.
 *
 *      In: k   => Antal vektorer
 *          x   => X-koordinater
 *          y   => Y-koordinater
 *          a   => Vektorstatus
 *          la  => Storhetens adress i GM.
 *          typ => Typ av storhet.
 *
 *      FV: TRUE = Vektorn fick plats i displayfilen.
 *
 *      Felkod: GP0112 - Kan ej malloc()/realloc()
 *
 *      (C) microform ab 8/1-95 J. Kjellander
 *
 *       2004-07-22 Mesh, J.Kjellander, �rebro university
 *
 *********************************************************/
 {
   DBint   i;
   DFPOST *df;

/*
***Se till att plats finns.
*/
loop:
   if ( gwinpt->df_ptr + k + 3 >= gwinpt->df_all )
     {
     if ( gwinpt->df_adr == NULL )
       {
       if ( (gwinpt->df_adr=(DFPOST *)v3mall((unsigned)
                               (PLYMXV*sizeof(DFPOST)),"WPsply")) == NULL )
         {
         erpush("GP0112","");
         errmes();
         return(FALSE);
         }
       else
         {
         gwinpt->df_all = PLYMXV;
         goto loop;
         }
       }
     else
       {
       if ( (gwinpt->df_adr=(DFPOST *)v3rall((char *)gwinpt->df_adr,
               (unsigned)((gwinpt->df_all+PLYMXV)*sizeof(DFPOST)),
                                                    "WPsply")) == NULL )
         {
         erpush("GP0112","");
         errmes();
         return(FALSE);
         }
       else
         {
         gwinpt->df_all += PLYMXV;
         goto loop;
         }
       }
     }
/*
***Lagra objektet. F�rst headern.
*/
   gwinpt->df_cur = gwinpt->df_ptr + 1;
   df = gwinpt->df_adr + gwinpt->df_cur;

   df->hdr.type = typ;             /* Objektets typ */
   df->hdr.nvec = k;               /* Antal vektorer */
   df->hdr.hili = 0;               /* Ingen highlight */
   df++;
   df->la       = la;              /* Adress i GM */
/*
***Vektorerna.
*/
   for ( i=0; i<=k; i++ )
     {
     df++;
     df->vec.x = (short)(gwinpt->vy.k1x + gwinpt->vy.k2x*x[i]);
     df->vec.y = (short)(gwinpt->vy.k1y + gwinpt->vy.k2y*y[i]);
     df->vec.a = a[i];
     }
/*
***Uppdatera DF-pekaren.
*/
   gwinpt->df_ptr += k + 3;

   return(TRUE);
 }
/********************************************************/
/*!******************************************************/

        bool WPfobj(
        WPGWIN  *gwinpt,
        DBptr    la,
        DBetype  typmsk,
        DBetype *typ)

/*      Kolla om en storhet med viss la finns i ett
 *      f�nsters DF.
 *
 *      In: gwinpt => Pekare till WPGWIN-f�nster.
 *          la     => Storhetens adress i GM.
 *          typmsk => Typmask f�r ev. begr�nsad s�kning.
 *
 *      Ut: *typ            Den funna storhetens typ.
 *          *gwinpt->df_cur Om storheten finns.
 *
 *      FV:  TRUE  = Storheten finns i DF:en.
 *           FALSE = Storheten finns ej i DF:en.
 *
 *      (C) microform ab 10/1-95  J. Kjellander
 *
 ******************************************************!*/

 {
   DBint   i;
   DFPOST *df;

/*
***Om ingen DF finns �r det inget tvivel om saken.
*/
   if ( gwinpt->df_adr == NULL ) return(FALSE);
/*
***Kanske �r det aktuellt objekt som s�kes ?
*/
   if ( gwinpt->df_cur >= 0 )
     {
     df = gwinpt->df_adr + gwinpt->df_cur;
     if ( ((df+1)->la == la ) && ((df->hdr.type & typmsk) > 0) )
       {
      *typ = df->hdr.type;
       return(TRUE);
       }
     }
/*
***Inte, d� �r det b�st att s�ka igenom hela displayfilen.
***F�r vi tr�ff g�r vi objektet till nytt aktuellt objekt.
*/
   i = 0;

   while ( i < gwinpt->df_ptr )
     {
     df = gwinpt->df_adr + i;
     if ( ((df+1)->la == la) && ((df->hdr.type & typmsk) > 0) )
       {
       gwinpt->df_cur = i;
      *typ = df->hdr.type;
       return(TRUE);
       }
     i += 3 + df->hdr.nvec;
     }

   return(FALSE);
 }

/********************************************************/
/********************************************************/

   bool WPdobj(
        WPGWIN *gwinpt,
        bool    s)

/* 
 *      Ritar (suddar) aktuellt (dfcur) grafiskt objekt.
 *
 *      In: gwinpt => Pekare till f�nster att rita i.
 *          s      => Rita/Sudda.
 *
 *      Ut: Inget.
 *
 *      FV: TRUE.
 *
 *      (C) microform ab 8/1-95 J. Kjellander
 *
 *      1996-12-17 WIN32, J.Kjellander
 *
 *********************************************************/

 {
   int     i,np,nvec;
   DFPOST *df;

#ifdef UNIX
   XPoint  ip[PLYMXV];
#endif
#ifdef WIN32
   POINT   ip[PLYMXV];
#endif
/*
***Initiering.
*/
   np   = 0;
   df   = gwinpt->df_adr + gwinpt->df_cur;
   nvec = df->hdr.nvec;
   df  += 2;
/*
***Rita.
*/
   if ( s )
     {
     for ( i=0; i<=nvec; i++ )
       {
/*
***T�nd f�rflyttning.
*/
       if ( (df->vec.a & VISIBLE) == VISIBLE ) 
         {
         ip[np].x = df->vec.x;
         ip[np].y = gwinpt->geo.dy - df->vec.y;
         df++, np++;
         }
/*
***Sl�ckt.
*/
        else
         {
         if ( np > 0 )
           {
#ifdef UNIX
           XDrawLines(xdisp,gwinpt->id.x_id,
                            gwinpt->win_gc,ip,np,CoordModeOrigin);
           XDrawLines(xdisp,gwinpt->savmap,
                            gwinpt->win_gc,ip,np,CoordModeOrigin);
#endif
#ifdef WIN32
           Polyline(gwinpt->dc,ip,np);
           Polyline(gwinpt->bmdc,ip,np);
#endif
           np = 0;
           }
         ip[np].x = df->vec.x;
         ip[np].y = gwinpt->geo.dy - df->vec.y;
         df++, np++;
         }
       }
/*
***Dom sista t�nda.
*/
     if ( np > 0 )
       {
#ifdef UNIX
       XDrawLines(xdisp,gwinpt->id.x_id,gwinpt->win_gc,ip,np,CoordModeOrigin);
       XDrawLines(xdisp,gwinpt->savmap,gwinpt->win_gc,ip,np,CoordModeOrigin);
#endif
#ifdef WIN32
       Polyline(gwinpt->dc,ip,np);
       Polyline(gwinpt->bmdc,ip,np);
#endif
       np = 0;
       }
     }
/*
***Sudda. H�r g�r vi return direkt utan att t�mma buffrar
***eftersom annars buffrar kommer att t�mmas tv� g�nger varav
***ena g�ngen tom.
*/
   else
     {
     if ( actpen != 0 ) WPspen(0);
     return(WPdobj(gwinpt,TRUE));
     }
/*
***T�m outputbufferten.
*/
#ifdef UNIX
   XFlush(xdisp);
#endif

   return(TRUE);
 }
/********************************************************/
/********************************************************/

        bool WProbj(
        WPGWIN *gwinpt)

/*      Suddar aktuellt grafiskt objekt ur en DF.
 *
 *      In: gwinpt => Pekare till WPGWIN-f�nster.
 *
 *      Ut: Inget.
 *
 *      FV: TRUE.
 *
 *      (C) microform ab 11/1-95 J. Kjellander
 *
 *********************************************************/

 {
   DFPOST *df;

/*
***Om f�nstrets DF har ett aktuellt objekt markerar
***vi det som suddat. Om det �r highlightat suddar vi
***f�rst highlighten fr�n sk�rmen.
*/
   if ( gwinpt->df_cur >= 0 )
     {
     df = gwinpt->df_adr + gwinpt->df_cur;
     if ( df->hdr.hili == TRUE ) WPhgen(gwinpt->id.w_id,(df+1)->la,FALSE);
     (df+1)->la = DF_ERASED;
     gwinpt->df_cur = -1;
     }

   return(TRUE);
 }

/********************************************************/
/*!******************************************************/

        short WPhgen(
        DBint win_id,
        DBptr la,
        bool  draw)

/*      "Highlightar" ett grafiskt objekt i ett
 *       eller flera f�nster.
 *
 *      In: win_id => ID f�r WPGWIN-f�nster eller GWIN_ALL.
 *          la     => Storhetens adress i GM.
 *          draw   => TRUE = rita, FALSE = sudda.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C) microform ab 10/1-95 J. Kjellander
 *
 *      1996-12-17 WIN32, J.Kjellander
 *      2006-12-08 Removed gphgen(), J.Kjellander
 *
 ******************************************************!*/

 {
   DBint   nv;
   short   ix=0,iy=0;
   DBetype typ;
   wpw_id  i;
   DFPOST *df;
   WPWIN  *winptr;
   WPGWIN *gwinpt;

#ifdef UNIX
   XPoint  ip[5];
#endif
#ifdef WIN32
   POINT   ip[5];
#endif

/*
***Loopa igenom alla WPGWIN-f�nster.
*/
   for ( i=0; i<WTABSIZ; ++i )
     {
     if ( ( win_id == GWIN_ALL  ||  win_id == i )  &&
          ( (winptr=WPwgwp(i)) != NULL )  &&
          ( winptr->typ == TYP_GWIN ) )
       {
       gwinpt = (WPGWIN *)winptr->ptr;
/*
***Leta upp objektet i f�nstrets DF.
*/
       if ( WPfobj(gwinpt,la,ALLTYP,&typ) )
         {
/*
***Om suddning beg�rts och objektet inte �r highlightat
***g�r det snabbt. Likas� om highligt beg�rts och objektet
***redan �r det.
*/
         df = gwinpt->df_adr + gwinpt->df_cur;
         if ( df->hdr.hili == draw ) return(0);
/*
***Objektet �r highligtat och skall suddas eller �r det inte men
***skall bli det.
***Leta upp r�tt vektor och d�rmed m�rkets placering.
*/
         nv = df->hdr.nvec;

         if ( nv == 0 )
           {
           ix = (df+2)->vec.x;
           iy = (df+2)->vec.y;
           }
         else
           {
           nv /= 2;
           df += 2 + nv;

           while ( nv >= 0 )
             {
             if ( ((df+1)->vec.a & VISIBLE) == VISIBLE )
               {
               ix = ((df+1)->vec.x + df->vec.x)/2;
               iy = ((df+1)->vec.y + df->vec.y)/2;
               break;
               }
             nv--;
             df++;
             }
           }
/*
***Markera vad som h�nt i DF-posten.
*/
         if ( nv >= 0 )
           {
           df = gwinpt->df_adr + gwinpt->df_cur;
           df->hdr.hili = draw;
/*
***Rita/Sudda.
*/
           if (  draw  &&  actpen != 2 ) WPspen(1);
           if ( !draw  &&  actpen != 0 ) WPspen(0);

           iy = gwinpt->geo.dy - iy;

           ip[0].x = ix + 4; ip[0].y = iy;
           ip[1].x = ix;     ip[1].y = iy + 4;
           ip[2].x = ix - 4; ip[2].y = iy;
           ip[3].x = ix;     ip[3].y = iy - 4;
           ip[4].x = ix + 4; ip[4].y = iy;

#ifdef UNIX
           XDrawLines(xdisp,gwinpt->id.x_id,
                            gwinpt->win_gc,ip,5,CoordModeOrigin);
           XDrawLines(xdisp,gwinpt->savmap,
                            gwinpt->win_gc,ip,5,CoordModeOrigin);
           XFlush(xdisp);
#endif
#ifdef WIN32
           Polyline(gwinpt->dc,ip,5);
           Polyline(gwinpt->bmdc,ip,5);
#endif
           }
         }
       }
     }
   return(0);
 }

/********************************************************/
/*!******************************************************/

        short WPerhg()

/*      Suddar alla highligtm�rken i alla f�nster.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: Inget.
 *
 *      (C) microform ab 11/1/95 J. Kjellander
 *
 ******************************************************!*/
 {
   DBint   i,n;
   DFPOST *df;
   WPWIN  *winptr;
   WPGWIN *gwinpt;

/*
***Loopa igenom alla WPGWIN-f�nster.
*/
   for ( i=0; i<WTABSIZ; ++i )
     {
     if ( (winptr=WPwgwp((wpw_id)i)) != NULL  &&
           winptr->typ == TYP_GWIN ) 
       {
       gwinpt = (WPGWIN *)winptr->ptr;
/*
***S�k igenom hela f�nstrets DF sekvensiellt.
*/
       n = 0;

       while ( n < gwinpt->df_ptr )
         {
/*
***�r objektet icke suddat och dessutom highligtat ?
*/
         df = gwinpt->df_adr + n;

         if ( (df+1)->la != DF_ERASED  &&  df->hdr.hili == TRUE )
           WPhgen(gwinpt->id.w_id,(df+1)->la,FALSE);
/*
***N�sta objekt i DF.
*/
         n += 3 + df->hdr.nvec;
         }
       }
     }
/*
***Sudda �ven alla eventuella pekm�rken.
*/
   WPepmk(GWIN_ALL);
/*
***Slut.
*/
    return(0);
 }

/********************************************************/
