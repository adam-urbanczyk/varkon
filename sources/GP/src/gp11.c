/**********************************************************************
*
*    gp11.c
*    ======
*
*    This file is part of the VARKON Graphics Library.
*    URL: http://www.varkon.com
*
*    klpply();    Clip polyline
*    klplin();    Clip line
*    klpdot();    Clip point
*
*    klp();       Clip vector
*    klptst();    Clip vector with additional output
*    klpplq();    Clip test polyline quickly
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
#include "../include/GP.h"

extern VY     actvy;

/*!******************************************************/

   bool klpply(
        int    kmin,
        int   *kmax,
        double x[],
        double y[],
        char a[])

/* 
*        Klipper en polylinje
*
*       IN: (Oklippt)
*          kmin+1:      Offset till polylinjestart.
*          kmax:        Offset till polylinjeslut.
*          x� kmin+1 �,y� kmin+1 �,a� kmin+1 �: Startpunkt med status
*          ....
*          x� kmax �,y� kmax �,a� kmax �:       Slutpunkt med status
* 
*       UT: (Klippt)
*          kmax:        Offset till polylinjeslut.
*          a: Status (Bit(ENDSIDE) modifierad) om mer �n 50%
*             av polylinjen klippts bort.
* 
*       FV:
*          klpply: TRUE = N�gon del av polylinjen �r synlig.
*
*
*       (C)microform ab 15/7-85 Ulf Johansson
*
*        6/4/89  Avlusad, J. Kjellander
*
********************************************************!*/

{
   int    i,j,l;
   double vec[4],sav0,sav1;

/*
***Initiering.
*/
   i = j = kmin + 1;
/*
***Leta upp 1:a synliga vektor. Om ingen synlig, returnera FALSE.
*/
   do
     { 
     if ( i >= *kmax )
       {
       *kmax = kmin;
       return(FALSE);
       } 
     vec[0] = x[i];
     vec[1] = y[i];
     vec[2] = sav0 = x[++i];
     vec[3] = sav1 = y[i];
     } while ( klp(vec,actvy.vywin) == -1 );
/*
***I vec ligger nu en vektor som efter klippning syns, helt
***eller delvis! B�rja med en sl�ckt f�rflyttning till
***startpunkten och en normal f�rflyttning till n�sta punkt.
*/
   x[j] = vec[0];
   y[j] = vec[1];
   a[j] &= a[i-1] & ~VISIBLE; /* Varf�r & framf�r = ???? */

   x[++j] = vec[2];
   y[j] = vec[3];
   a[j] = a[i];
/*
***Loopa igenom resten av polylinjen.
*/
   while ( ++i <= *kmax )
     {
     vec[0] = sav0;
     vec[1] = sav1;
     vec[2] = sav0 = x[i];
     vec[3] = sav1 = y[i];
/*
***Klipp aktuell vektor. 0 => Vektorn helt innanf�r.
***                      2 => �nde 2 utanf�r.
*/
     switch ( klp(vec,actvy.vywin) )
       {
       case 0:
       case 2:
       x[++j] = vec[2];
       y[j] = vec[3];
       a[j] = a[i];            /* Detta �r fel !!! */
       break;
/*
*** 1 => �nde 1 utanf�r.
*** 3 => B�da �ndarna utanf�r. Vektorn klippt p� tv� st�llen.
*/
       case 1: 
       case 3:
       if ( ++j == i )
         { /* En koordinat blir tv� */    
         if ( ++i >= PLYMXV )
           { /* Om det finns plats, */
           *kmax = j;
           return(TRUE);
           }
         for ( l = ++(*kmax); l >= j; l-- )
           { /* fixa utrymme */
           x[l] = x[l-1];
           y[l] = y[l-1];
           a[l] = a[l-1];
           }
         }
       x[j] = vec[0];
       y[j] = vec[1];
       a[j] = a[i-1] & ~VISIBLE;    /* Osynlig */
       x[++j] = vec[2];
       y[j] = vec[3];
       a[j] = a[i];
       break;
       }
     }
/*
***Uppdatera polylinjeslut.
*/
     *kmax = j;

     return(TRUE);
}

/********************************************************/
/*!******************************************************/

   bool klplin(
        int    k,
        double x[],
        double y[],
        char   a[])


/* 
*       Klipper en linje.
*
*       IN: (Oklippt)
*          k+1:     Offset till linjestart
*          k+2:     Offset till linjeslut
*          x� k+1 �,y� k+1 �,a� k+1 �: Startpunkt med status
*          x� k+2 �,y� k+2 �,a� k+2 �: Slutpunkt med status
*
*       UT: (Klippt)
*          a: Status (Bit(ENDSIDE) modifierad) om mer �n 50%
*             av linjen klippts bort.
*
*       FV:
*          klplin: TRUE = N�gon del av linjen �r synlig.
*
*
*       (c) Microform AB 15/7-85 Ulf Johansson
*
*******************************************************!*/
{
     int    i,j,l;
     double vec[ 4 ];
     double t1,t2;

     i = j = k + 1;

     vec[ 0 ] = x[ i ];                 /* P1 */
     vec[ 1 ] = y[ i ];
     vec[ 2 ] = x[ ++j ];               /* P2 */
     vec[ 3 ] = y[ j ];

     l = klptst(vec,actvy.vywin,&t1,&t2);

     if (l != 0)
          if (l > 0)
               if (l == 1) {
                                                 /* P1 klippt */
                    x[ i ] = vec[ 0 ];
                    y[ i ] = vec[ 1 ];
                    if (t1 > 0.5)
                         if ((a[ j ] & ENDSIDE) == ENDSIDE)
                              a[ i ] |= ENDSIDE;  /* Enable */
                         else
                              a[ i ] &= ~ENDSIDE; /* Disable */

               } else if (l == 2) {
                                                 /* P2 klippt */
                    x[ j ] = vec[ 2 ];
                    y[ j ] = vec[ 3 ];
                    if (t2 > 0.5)
                         if ((a[ i ] & ENDSIDE) == ENDSIDE)
                              a[ j ] |= ENDSIDE;  /* Enable */
                         else
                              a[ j ] &= ~ENDSIDE; /* Disable */

               } else {
                                                 /* Klippt i b�da �ndar */
                    x[ i ] = vec[ 0 ];
                    y[ i ] = vec[ 1 ];
                    x[ j ] = vec[ 2 ];
                    y[ j ] = vec[ 3 ];
                    if (t1 > 0.5)
                         if ((a[ j ] & ENDSIDE) == ENDSIDE)
                              a[ i ] |= ENDSIDE;  /* Enable */
                         else
                              a[ i ] &= ~ENDSIDE; /* Disable */
                    else if (t2 > 0.5)
                         if ((a[ i ] & ENDSIDE) == ENDSIDE)
                              a[ j ] |= ENDSIDE;  /* Enable */
                         else
                              a[ j ] &= ~ENDSIDE; /* Disable */

               }               
          else
               return(FALSE);

     return(TRUE);
}
/********************************************************/
/*!******************************************************/

   bool klpdot(
        int    k,
        double x[],
        double y[])


/* 
*       Klipper en punkt
*
*       IN: (Oklippt)
*          k+1:     Offset till punkt
*          x� k+1 �,y� k+1 �,a� k+1 �: Punkt med status
*
*       UT: (Klippt)
*
*       FV:
*          klpdot: TRUE = Punkten ligger i f�nstret.
*
*
*       (c) Microform AB 15/7-85 Ulf Johansson
*
********************************************************!*/
{
     int i;

     i = k + 1;

     return ((x[ i ] > actvy.vywin[ 0 ]) &&
             (x[ i ] < actvy.vywin[ 2 ]) &&
             (y[ i ] > actvy.vywin[ 1 ]) &&
             (y[ i ] < actvy.vywin[ 3 ]));
}
/********************************************************/
/*!******************************************************/

   short klp( 
        double *v,
        double *w)

/*      Klipper en vektor mot ett f�nster.
*
*       IN:
*            v: Vektor.
*            w: F�nster.
*
*       UT:
*            v: Vektor enl FV.
*         
*       FV:
*            -1: Vektorn �r oklippt och ligger utanf�r f�nstret
*             0: Vektorn �r oklippt och ligger innanf�r f�nstret.
*             1: Vektorn �r klippt i punkt 1 (start).
*             2: Vektorn �r klippt i punkt 2 (slut).
*             3: Vektorn �r klippt i b�de punkt 1 och punkt 2.
*
*
*       (c) Microform AB 1984 M. Nelson
*
*
*       REVIDERAD:
*
*       15/7-85 Ulf Johansson
*
********************************************************!*/

  {
    register double *p1,*p2,*win;
    short sts1,sts2;

        sts1 = sts2 = 0;
        p1 = v;
        p2 = p1 + 2;
        win = w;

        /* Om punkt 1 ligger utanf�r f�nstret, klipp till f�nsterkanten */

        if (*p1 < *win) {

            if (*p2 < *win)
                return(-1);          /* Hela vektorn v�nster om f�nstret */

            *(p1+1) += (*(p2+1) - *(p1+1))*(*win - *p1)/(*p2 - *p1);
            *p1 = *win;
            sts1 = 1;
        } else if (*p1 > *(win+2)) {

            if (*p2 > *(win+2))
                return(-1);               /* Hela vektorn h�ger om f�nstret */

            *(p1+1) += (*(p2+1) - *(p1+1))*(*(win+2) - *p1)/(*p2 - *p1);
            *p1 = *(win+2);
            sts1 = 1; 
        }

        if (*(p1+1) < *(win+1)) {

            if (*(p2+1) < *(win+1))
                return(-1);               /* Hela vektorn nedanf�r f�nstret */

            *p1 += (*p2 - *p1)*(*(win+1) - *(p1+1))/(*(p2+1) - *(p1+1));
            *(p1+1) = *(win+1);
            sts1 = 1;
        } else if (*(p1+1) > *(win+3)) {

            if (*(p2+1) > *(win+3))
                return(-1);               /* Hela vektor ovanf�r f�nstret */

            *p1 += (*p2 - *p1)*(*(win+3) - *(p1+1))/(*(p2+1) - *(p1+1));
            *(p1+1) = *(win+3);
            sts1 = 1;
        }

        if (sts1 == 1)                    /* Punkt 1 klippt */
            if ((*p1 < *win) ||
                (*p1 > *(win+2)) ||
                (*(p1+1) < *(win+1)) ||
                (*(p1+1) > *(win+3)))
                 return(-1);              /* Hela vektorn utanf�r f�nstret */


       /* Punkt 1 ligger innanf�r f�nstret, klipp punkt 2 om utanf�r. */ 

        if (*p2 < *win) {

            *(p2+1) += (*(p2+1) - *(p1+1))*(*win - *p2)/(*p2 - *p1);
            *p2 = *win;
            sts2 = 2;
        } else if (*p2 > *(win+2)) {

            *(p2+1) += (*(p2+1) - *(p1+1))*(*(win+2) - *p2)/(*p2 - *p1);
            *p2 = *(win+2);
            sts2 = 2;
        }

        if (*(p2+1) < *(win+1)) {

            *p2 += (*p2 - *p1)*(*(win+1) - *(p2+1))/(*(p2+1) - *(p1+1));
            *(p2+1) = *(win+1);
            sts2 = 2;
        } else if (*(p2+1) > *(win+3)) {

            *p2 += (*p2 - *p1)*(*(win+3) - *(p2+1))/(*(p2+1) - *(p1+1));
            *(p2+1) = *(win+3);
            sts2 = 2;
        }

        return(sts1 + sts2);
  }

/********************************************************/
/*!******************************************************/

   short klptst( 
        double *v,
        double *w,
        double *t1,
        double *t2)

/*      Klipper en vektor mot ett f�nster samt ger ett m�tt p� hur
*       mycket som klippts bort.
*
*       IN:
*            v: Vektor.
*            w: F�nster.
*
*
*       UT:
*            v: Vektor enl FV.
*            t1: Anger hur mycket som klippts bort vid punkt 1
*                0.0 < t1 < 1.0.
*            t2: Anger hur mycket som klippts bort vid punkt 2
*                0.0 < t2 < 1.0.
*         
*       FV:
*            -1: Vektorn �r oklippt och ligger utanf�r f�nstret
*             0: Vektorn �r oklippt och ligger innanf�r f�nstret.
*             1: Vektorn �r klippt i punkt 1.
*             2: Vektorn �r klippt i punkt 2.
*             3: Vektorn �r klippt i b�de punkt 1 och punkt 2.
*
*
*       (c) Microform AB 1984 M. Nelson
*
*
*       REVIDERAD:
*
*       15/7/85  Bug, Ulf Johansson
*       29/10/86 *p1 == *p2, J. Kjellander
*
********************************************************!*/

  {
    register double *p1,*p2,*win;
    short sts1,sts2;
    double d1,d2;

        sts1 = sts2 = 0;
        p1 = v;
        p2 = p1 + 2;
        win = w;

        /* Om punkt 1 ligger utanf�r f�nstret, klipp till f�nsterkanten */

        if (*p1 < *win) {

            if (*p2 < *win)
                return(-1);          /* Hela vektorn v�nster om f�nstret */
            d1 = (*win - *p1)/(*p2 - *p1);
            *(p1+1) += (*(p2+1) - *(p1+1))*d1;
            *p1 = *win;
            sts1 = 1;
        } else if (*p1 > *(win+2)) {

            if (*p2 > *(win+2))
                return(-1);               /* Hela vektorn h�ger om f�nstret */
            d1 = (*(win+2) - *p1)/(*p2 - *p1);
            *(p1+1) += (*(p2+1) - *(p1+1))*d1;
            *p1 = *(win+2);
            sts1 = 1; 
        }

        if (*(p1+1) < *(win+1)) {

            if (*(p2+1) < *(win+1))
                return(-1);               /* Hela vektorn nedanf�r f�nstret */
            d2 = (*(win+1) - *(p1+1))/(*(p2+1) - *(p1+1));
            *p1 += (*p2 - *p1)*d2;
            *(p1+1) = *(win+1);
            sts1 += 2;
        } else if (*(p1+1) > *(win+3)) {

            if (*(p2+1) > *(win+3))
                return(-1);               /* Hela vektor ovanf�r f�nstret */
            d2 = (*(win+3) - *(p1+1))/(*(p2+1) - *(p1+1));
            *p1 += (*p2 - *p1)*d2;
            *(p1+1) = *(win+3);
            sts1 += 2;
        }

        if (sts1 != 0) {                   /* Punkt 1 klippt */
            if ((*p1 < *win) ||
                (*p1 > *(win+2)) ||
                (*(p1+1) < *(win+1)) ||
                (*(p1+1) > *(win+3)))
                 return(-1);              /* Hela vektorn utanf�r f�nstret */
            if ( *p1 == *p2 && *(p1+1) == *(p2+1) ) return(-1); /*861029JK */
        
            if (sts1 == 1)
                 *t1 = d1;
            else if (sts1 == 2)
                 *t1 = d2;
            else
                 *t1 = d1 + d2 - d1*d2;  /* Kompensation f�r tv�stegsklipp */

            sts1 = 1;
        }


       /* Punkt 1 ligger innanf�r f�nstret, klipp punkt 2 om utanf�r. */ 

        if (*p2 < *win) {
            d1 = (*win - *p2)/(*p1 - *p2);
            *(p2+1) -= (*(p2+1) - *(p1+1))*d1;
            *p2 = *win;
            sts2 = 1;
        } else if (*p2 > *(win+2)) {
            d1 = (*(win+2) - *p2)/(*p1 - *p2);
            *(p2+1) -= (*(p2+1) - *(p1+1))*d1;
            *p2 = *(win+2);
            sts2 = 1;
        }

        if (*(p2+1) < *(win+1)) {
            d2 = (*(win+1) - *(p2+1))/(*(p1+1) - *(p2+1));
            *p2 -= (*p2 - *p1)*d2;
            *(p2+1) = *(win+1);
            sts2 += 2;
        } else if (*(p2+1) > *(win+3)) {
            d2 = (*(win+3) - *(p2+1))/(*(p1+1) - *(p2+1));
            *p2 -= (*p2 - *p1)*d2;
            *(p2+1) = *(win+3);
            sts2 += 2;
        }

        if (sts2 != 0) {                   /* Punkt 2 klippt */
        
            if ( *p1 == *p2 && *(p1+1) == *(p2+1) ) return(-1); /*861029JK */
            if (sts2 == 1)
                 *t2 = d1;
            else if (sts2 == 2)
                 *t2 = d2;
            else
                 *t2 = d1 + d2 - d1*d2;  /* Kompensation f�r tv�stegsklipp */

            sts2 = 2;

            if (sts1 != 0)
                 *t2 *= (1.0 - *t1);     /* kompensation f�r punkt 1-klipp */
        }

        return(sts1 + sts2);
  }

/********************************************************/
/*!******************************************************/

  short klpplq(
        int    kmin,
        int   *kmax,
        double x[],
        double y[])

/* 
*        Klipptestar en polylinje
*
*      In: kmin+1:      Offset till polylinjestart.
*          kmax:        Offset till polylinjeslut.
*          x� kmin+1 �,y� kmin+1 � Startpunkt.
*          ....
*          x� kmax �,y� kmax �     Slutpunkt.
* 
*       Ut: Inget.
* 
*       FV: -1 = Helt osynlig.
*            0 = Helt synlig.
*            1 = Delvis synlig.
*
*       (C)microform ab 2/4/91 J. Kjellander
*
********************************************************!*/

{
   int    i;
   double vec[4];

/*
***Initiering.
*/
   i = kmin + 1;
/*
***Leta upp f�rsta synliga vektor. Om alla helt osynliga,
***returnera -1 dvs. hela polylinjen osynlig.
*/
   do
     { 
     if ( i >= *kmax ) return(-1);
     vec[0] = x[i];
     vec[1] = y[i];
     vec[2] = x[++i];
     vec[3] = y[i];
     } while ( klp(vec,actvy.vywin) == -1 );
/*
***Om inte alla men minst en �r helt osynlig �r hela polylinjen
***delvis synlig, returnera +1.
*/
   if ( i > kmin+2 ) return(1);
/*
***Om inte ens f�rsta vektorn �r helt osynlig, kolla
***ist�llet hur m�nga som �r helt synliga.
*/
   i = kmin + 1;

   while ( i < *kmax )
     { 
     vec[0] = x[i];
     vec[1] = y[i];
     vec[2] = x[++i];
     vec[3] = y[i];
     if ( klp(vec,actvy.vywin) != 0 ) return(1);
     }
/*
***Alla vektorer helt synliga.
*/
   return(0);

  }

/********************************************************/
