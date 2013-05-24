/**********************************************************************
*
*    evgrid.c
*    =======
*
*    This file is part of the VARKON Program Module Library.
*    URL: http://www.varkon.com
*
*    evgrid();     Evaluerar GRID_VIEW
*    evgrix();     Evaluerar GRIDX_VIEW
*    evgriy();     Evaluerar GRIDY_VIEW
*    evgrdx();     Evaluerar GRIDDX_VIEW
*    evgrdy();     Evaluerar GRIDDY_VIEW
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
#ifdef V3_X11
#include "../../WP/include/WP.h"
#endif
#include "../../EX/include/EX.h"

extern PMPARVA *proc_pv;  /* inproc.c *pv      Access structure for MBS routines */
extern short    proc_pc;  /* inproc.c parcount Number of actual parameters */

extern PMLITVA *func_vp;   /* Pekare till resultat. */

/*!******************************************************/

        short evgrid()

/*      Evaluerar proceduren GRID_VIEW.
 *
 *      In: extern proc_pv => Pekare till array med parameterv�rden
 *
 *      (C)microform ab 1996-02-12
 *
 *      2001-02-14 In-Param changed to Global variables, R Svedin
 *
 ******************************************************!*/

  {
    extern bool  rstron;
    extern gmflt rstrox,rstroy,rstrdx,rstrdy;
    extern short gpdrrs(),gpdlrs();

/*
***Rastret skall t�ndas. Om det redan var t�nt g�r vi ingenting.
***F�r att �ndra ett raster skall man allts� f�rst sl�cka det,
***sedan �ndra det och sist t�nda det igen.
*/
    if ( proc_pv[1].par_va.lit.int_va == 1 )
      {
      if ( !rstron )
        {
        rstron = TRUE;
        gpdrrs(rstrox,rstroy,rstrdx,rstrdy);
        }
      }
/*
***Rastret skall sl�ckas. Om det redan var sl�ckt g�r vi heller
***ingenting.
*/
    else if ( proc_pv[1].par_va.lit.int_va == 0 )
      {
      if ( rstron )
        {
        rstron = FALSE;
        gpdlrs(rstrox,rstroy,rstrdx,rstrdy);
        }
      }
/*
***Om parametern varken var 1 (T�nd) eller 0 (Sl�ck) g�r vi ingenting.
*/

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short evgrix()

/*      Evaluerar proceduren GRIDX_VIEW.
 *
 *      In: extern proc_pv => Pekare till array med parameterv�rden
 *
 *      (C)microform ab 1996-02-12
 *
 *      2001-02-14 In-Param changed to Global variables, R Svedin
 *
 ******************************************************!*/

  {
    extern gmflt rstrox;

    rstrox = proc_pv[1].par_va.lit.float_va;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short evgriy()

/*      Evaluerar proceduren GRIDY_VIEW.
 *
 *      In: extern proc_pv => Pekare till array med parameterv�rden
 *
 *      (C)microform ab 1996-02-12
 *
 *      2001-02-14 In-Param changed to Global variables, R Svedin
 *
 ******************************************************!*/

  {
    extern gmflt rstroy;

    rstroy = proc_pv[1].par_va.lit.float_va;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short evgrdx()

/*      Evaluerar proceduren GRIDDX_VIEW.
 *
 *      In: extern proc_pv => Pekare till array med parameterv�rden
 *
 *      (C)microform ab 1996-02-12
 *
 *      2001-02-14 In-Param changed to Global variables, R Svedin
 *
 ******************************************************!*/

  {
    extern gmflt rstrdx;

    rstrdx = proc_pv[1].par_va.lit.float_va;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short evgrdy()

/*      Evaluerar proceduren GRIDDY_VIEW.
 *
 *      In: extern proc_pv => Pekare till array med parameterv�rden
 *
 *      (C)microform ab 1996-02-12
 *
 *      2001-02-14 In-Param changed to Global variables, R Svedin
 *
 ******************************************************!*/

  {
    extern gmflt rstrdy;

    rstrdy = proc_pv[1].par_va.lit.float_va;

    return(0);
  }

/********************************************************/