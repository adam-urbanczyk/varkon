/**********************************************************************
*
*    wp27.c
*    ======
*
*    This file is part of the VARKON WindowPac Library.
*    URL: http://www.varkon.com
*
*    This file includes:
*
*    wpdrdm();    Draw dimension
*    wpdldm();    Delete dimension
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
#include "../../GP/include/GP.h"
#include "../include/WP.h"

extern short    actpen;

static short drawdm(WPGWIN *gwinpt, GMUNON *dimpek, DBptr la, bool draw);

/*!******************************************************/

        short wpdrdm(
        GMUNON *dimpek,
        DBptr   la,
        v2int   win_id)

/*      Ritar ett l�ngdm�tt, diameterm�tt, radiem�tt
 *      eller vinkelm�tt.
 *
 *      In: dimpek => Pekare till m�tt-post.
 *          la     => GM-adress.
 *          win_id => F�nster att rita i.
 *
 *      Ut: Inget.
 *
 *      FV:      0 => Ok.
 *
 *      (C) microform ab 27/1/95 J. Kjellander
 *
 ******************************************************!*/

 {
   int     i;
   WPWIN  *winptr;
   WPGWIN *gwinpt;

/*
***Om den �r blankad �r det enkelt.
*/
   if ( dimpek->hed_un.blank) return(0);
/*
***Loopa igenom alla WPGWIN-f�nster utom 1:a.
*/
   for ( i=1; i<WTABSIZ; ++i )
     {
     if ( (winptr=wpwgwp((wpw_id)i)) != NULL  &&
           winptr->typ == TYP_GWIN ) 
       {
       gwinpt = (WPGWIN *)winptr->ptr;
/*
***Skall vi rita i detta f�nster ?
*/
       if ( win_id == GWIN_ALL  ||  win_id == gwinpt->id.w_id )
         {
/*
***Ja, ligger det p� en niv� som �r t�nd i detta f�nster ?
*/
         if ( wpnivt(gwinpt,dimpek->hed_un.level) )
           {
/*
***Ja. Kolla att r�tt f�rg �r inst�lld.
*/
           if ( dimpek->hed_un.pen != actpen ) wpspen(dimpek->hed_un.pen);
/*
***Sen �r det bara att rita.
*/
           drawdm(gwinpt,dimpek,la,TRUE);
           }
         }
       }
     }

   return(0);
 }

/********************************************************/
/*!******************************************************/

        short wpdldm(
        GMUNON *dimpek,
        DBptr   la,
        v2int   win_id)

/*      Suddar ett m�tt.
 *
 *      In: dimpek => Pekare till m�tt-post.
 *          la     => GM-adress.
 *          win_id => F�nster att sudda i.
 *
 *      Ut: Inget.
 *
 *      FV:      0 => Ok.
 *
 *      (C) microform ab 27/1/95 J. Kjellander
 *
 ******************************************************!*/

 {
   int     i;
   DBetype typ;
   WPWIN  *winptr;
   WPGWIN *gwinpt;

/*
***Loopa igenom alla WPGWIN-f�nster utom 1:a.
*/
   for ( i=1; i<WTABSIZ; ++i )
     {
     if ( (winptr=wpwgwp((wpw_id)i)) != NULL  &&
           winptr->typ == TYP_GWIN ) 
       {
       gwinpt = (WPGWIN *)winptr->ptr;
/*
***Skall vi sudda i detta f�nster ?
*/
       if ( win_id == GWIN_ALL  ||  win_id == gwinpt->id.w_id )
         {
/*
***Ja. Om den finns i DF kan vi sudda snabbt.
*/
         if ( wpfobj(gwinpt,la,dimpek->hed_un.type,&typ) )
           {
           wpdobj(gwinpt,FALSE);
           wprobj(gwinpt);
           }
/*
***Om den nu ligger p� en sl�ckt niv� eller �r blankad g�r vi
***inget mer. Annars f�r vi �terskapa polylinjen och sudda l�ngsamt.
*/
         else
           {
           if ( !wpnivt(gwinpt,dimpek->hed_un.level)  ||
                               dimpek->hed_un.blank) return(0);
           drawdm(gwinpt,dimpek,la,FALSE);
           }
         }
       }
     }

   return(0);
 }

/********************************************************/
/*!******************************************************/

        static short drawdm(
        WPGWIN *gwinpt,
        GMUNON *dimpek,
        DBptr   la,
        bool    draw)

/*      Ritar/suddar ett m�tt i ett visst f�nster.
 *      Vid ritning lagras objektet samtidigt i DF.
 *
 *      In: gwinpt => Pekare till f�nster.
 *          dimpek => Pekare till m�tt-post.
 *          la     => GM-adress.
 *          draw   => TRUE = Rita, FALSE = Sudda
 *
 *      Ut:  Inget.
 *
 *      FV: Inget.
 *
 *      (C)microform ab 27/1/95 J. Kjellander
 *
 ******************************************************!*/

 {
   double x[PLYMXV],y[PLYMXV];
   char   a[PLYMXV];
   int    k;
   GMTXT  txt;

/*
***St�ll om grapac:s vy s� att detta f�nster g�ller.
*/
   wpfixg(gwinpt);
/*
***Skapa grafisk representation, dvs. polylinje.
*/
   k = -1;

   switch ( dimpek->hed_un.type )
     {
     case LDMTYP:
     x[0] = dimpek->ldm_un.p3_ld.x_gm;
     y[0] = dimpek->ldm_un.p3_ld.y_gm;
     a[0] = 0;
     gpplld(&dimpek->ldm_un,&k,x,y,a);
     break;

     case CDMTYP:
     x[0] = dimpek->cdm_un.p3_cd.x_gm;
     y[0] = dimpek->cdm_un.p3_cd.y_gm;
     a[0] = 0;
     gpplcd(&dimpek->cdm_un,&k,x,y,a);
     break;

     case RDMTYP:
     gpplrd(&dimpek->rdm_un,&txt,&k,x,y,a);
     break;

     case ADMTYP:
     gpplad(&dimpek->adm_un,&txt,&k,x,y,a);
     break;
    }
/*
***�terst�ll grapac:s ursprungliga vy igen.
*/
   wpfixg(NULL);
/*
***Klipp polylinjen. Om den �r synlig (helt eller delvis ),
***rita den.
*/
   if ( wpcply(gwinpt,-1,&k,x,y,a) )
     {
     if ( draw  &&  dimpek->hed_un.hit )
       {
       if ( wpsply(gwinpt,k,x,y,a,la,dimpek->hed_un.type) )
         wpdobj(gwinpt,TRUE);
       else
         return(erpush("GP0012",""));
       }
     else wpdply(gwinpt,k,x,y,a,draw);
     }

   return(0);
 }

/***********************************************************/
