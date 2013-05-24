/**********************************************************************
*
*    ms32.c
*    ======
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     mspl01();   Autoplot
*     msplot();   Plot view
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
*    (C) 1984-1999, Johan Kjellander, Microform AB
*    (C) 200-2004, Johan Kjellander, �rebro university
*
***********************************************************************/

#include "../../../sources/DB/include/DB.h"
#include "../../../sources/IG/include/IG.h"
#include "../../../sources/GP/include/GP.h"
#include "../../../sources/WP/include/WP.h"

static bool initpl(); /* Initierar */
static int  pltply(); /* Ritar polyline */
static int  exitpl(); /* Avslutar */

extern double  x[],y[];
extern char    a[];
extern int     ncrdxy;
extern bool    pltflg;
extern VY      actvy;
extern DBTmat  actvym;
extern GMDATA  v3dbuf;

/*!******************************************************/

       short mspl01()

/*      Plottar allt p� sk�rmen med autoskala.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: Inget.
 *
 *      (C)microform ab 29/11/95 J. Kjellander
 *
 ******************************************************!*/

  {
   msplot(&actvy);

   return(0);
  }

/********************************************************/
/*!******************************************************/

        int   msplot(plotvy)
        VY   *plotvy;

/*      Plottar vy p� skrivaren.
 *
 *      In:
 *           plotvy = Vy att plotta.
 *
 *      Ut:  Inget.
 *
 *      FV:  0 => Ok
 *
 *      (C)microform ab 29/11/95 J. Kjellander
 *
 ******************************************************!*/

 {
   DBptr   la;
   DBetype type;
   short   curpen;
   char    str[V3STRLEN+1];
   double  tmpcn;
   DBId    dummy;
   GMUNON  gmpost;
   DBTmat  pmat;
   GMSEG  *sptarr[6],*segptr,arcseg[4];
   WPGWIN  gwin;

/*
***G�r det att initiera plottern.
*/
   if ( !initpl("VARKON-Plot",&gwin) ) 
     {
     MessageBox(ms_main,"Kan ej initiera skrivare/plotter !",
               "Systemfel",MB_ICONSTOP | MB_OK);
     return(0);
     }
/*
***Nu vet vi uppl�sning och fysyisk storlek p� plotarean och
***kan d�rmed initiera resten av WPGWIN-posten.
*/
   gwin.vy.scrwin.xmin = 0;
   gwin.vy.scrwin.ymin = 0;
   gwin.vy.scrwin.xmax = gwin.geo.dx;
   gwin.vy.scrwin.ymax = gwin.geo.dy;
/*
***Nu skall den VY vi f�tt som indata mappas till v�rt WPGWIN.
*/
   gwin.vy.modwin.xmin = plotvy->vywin[0];
   gwin.vy.modwin.xmax = plotvy->vywin[2];
   gwin.vy.modwin.ymin = plotvy->vywin[1];
   gwin.vy.modwin.ymax = plotvy->vywin[3];

   gwin.vy.vydist = plotvy->vydist;
   gwin.vy.vy_3D  = plotvy->vy3d;

   gwin.vy.vymat.k11 = actvym.g11;
   gwin.vy.vymat.k12 = actvym.g12;
   gwin.vy.vymat.k13 = actvym.g13;
  
   gwin.vy.vymat.k21 = actvym.g21;
   gwin.vy.vymat.k22 = actvym.g22;
   gwin.vy.vymat.k23 = actvym.g23;
  
   gwin.vy.vymat.k31 = actvym.g31;
   gwin.vy.vymat.k32 = actvym.g32;
   gwin.vy.vymat.k33 = actvym.g33;
/*
***Normalisera. D�rmed justeras modellf�nstret s� att det f�r
***plotterns proportioner. Dessutom ber�knas k1x,k2x,k1y och k2y.
*/
   msnrgw(&gwin);
/*
***Medela grapac.
*/
   msfixg(&gwin);
/*
***F�rdubbla kurvnoggrannheten och s�tt plot-flaggan.
*/
   gpgtcn(&tmpcn);
   if ( 2*tmpcn < 100 ) gpstcn(2*tmpcn);
   else                 gpstcn(100.0);
   pltflg = TRUE;
/*
***Div. initiering.
*/
   curpen = -1;
/*
***B�rja rita.
*/
   DBget_pointer('F',&dummy,&la,&type);
loop:
   while (DBget_pointer('N',&dummy,&la,&type) == 0)
     {
     ncrdxy = 0;
     switch(type)
       {
/*
***Punkt.
*/
       case POITYP:
       DBread_point(&gmpost.poi_un,la);
       gpdrpo(&gmpost.poi_un,la,CLIP);
       break;
/*
***Linje.
*/
       case LINTYP:
       DBread_line(&gmpost.lin_un,la);
       gpdrli(&gmpost.lin_un,la,CLIP);
       break;
/*
***Cirkelb�ge.
*/
       case ARCTYP:
       DBread_arc(&gmpost.arc_un,arcseg,la);
       gpdrar(&gmpost.arc_un,arcseg,la,CLIP);
       break;
/*
***Kurva.
*/
       case CURTYP:
       DBread_curve(&gmpost.cur_un,&segptr,NULL,la);
       gpdrcu(&gmpost.cur_un,segptr,la,CLIP);
       DBfree_segments(segptr);
       break;
/*
***Yta.
*/
       case SURTYP:
       DBread_surface(&gmpost.sur_un,la);
       DBread_srep_curves(&gmpost.sur_un,sptarr);
       gpdrsu(&gmpost.sur_un,sptarr,la,CLIP);
       DBfree_srep_curves(sptarr);
       break;
/*
***Koordinatsystem.
*/
       case CSYTYP:
       DBread_csys(&gmpost.csy_un,&pmat,la);
       gpdrcs(&gmpost.csy_un,&pmat,la,CLIP);
       break;
/*
***B_plan.
*/
       case BPLTYP:
       DBread_bplane(&gmpost.bpl_un,la);
       gpdrbp(&gmpost.bpl_un,la,CLIP);
       break;
/*
***Text.
*/
       case TXTTYP:
       DBread_text(&gmpost.txt_un,str,la);
       gpdrtx(&gmpost.txt_un,str,la,CLIP);
       break;
/*
***L�ngdm�tt.
*/
       case LDMTYP:
       DBread_ldim(&gmpost.ldm_un,la);
       gpdrld(&gmpost.ldm_un,la,CLIP);
       break;
/*
***Diameterm�tt.
*/
       case CDMTYP:
       DBread_cdim(&gmpost.cdm_un,la);
       gpdrcd(&gmpost.cdm_un,la,CLIP);
       break;
/*
***Radiem�tt.
*/
       case RDMTYP:
       DBread_rdim(&gmpost.rdm_un,la);    
       gpdrrd(&gmpost.rdm_un,la,CLIP);
       break;
/*
***Vinkelm�tt.
*/
       case ADMTYP:
       DBread_adim(&gmpost.adm_un,la);
       gpdrad(&gmpost.adm_un,la,CLIP);
       break;
/*
***Snitt.
*/
       case XHTTYP:
       DBread_xhatch(&gmpost.xht_un,v3dbuf.crd,la);
       gpdrxh(&gmpost.xht_un,v3dbuf.crd,la,CLIP);
       break;
/*
***Part och grupp.
*/
       case PRTTYP:
       case GRPTYP:
       goto loop;
       break;
       }
/*
***Skriv till plotter.
*/
   if ( ncrdxy > 0 )
     {
     if (gmpost.hed_un.pen != curpen)
       SelectObject(gwin.dc,msgcol(curpen=gmpost.hed_un.pen));
     pltply(&gwin,ncrdxy-1,x,y,a);
     }
   }
/*
***�terst�ll kurvnoggrannhet mm.
*/
   gpstcn(tmpcn); pltflg = FALSE;
   msfixg(NULL);
   exitpl(gwin.dc);
/*
***Slut.
*/
   return(0);
 }

/********************************************************/
/*!******************************************************/

 static bool    initpl(rubrik,gwinpt)
        char   *rubrik;
        WPGWIN *gwinpt;

/*      Initierar en plotter.
 *
 *      In:
 *          rubrik = Plottjobbets namn.
 *
 *      Ut: 
 *         *gwinpt = WPGWIN-post med DC och geometri.
 *
 *      FV: TRUE  = OK, DC skapat.
 *          FALSE = F�r inget DC.
 *
 *      (C)microform ab 29/11/95 J. Kjellander
 *
 ******************************************************!*/

  {
   DOCINFO    di;

   static PRINTDLG pd;

/*
***Initiera Printdata.
*/
   pd.lStructSize         = sizeof(PRINTDLG);
   pd.hwndOwner           = ms_main;
   pd.hDevMode            = NULL;
   pd.hDevNames           = NULL;
   pd.nFromPage           = 1;
   pd.nToPage             = 1;
   pd.nMinPage            = 0;
   pd.nMaxPage            = 0;
   pd.nCopies             = 0;
   pd.lpfnSetupHook       = NULL;
   pd.lpSetupTemplateName = NULL;
   pd.lpfnPrintHook       = NULL;
   pd.lpPrintTemplateName = NULL;
   pd.lCustData           = NULL;
   pd.Flags               = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION |
                            PD_PRINTSETUP;
/*
***Anropa PrintDlg och erh�ll p� s� vis ett DC.
*/
   if ( PrintDlg(&pd) )
     {
/*
***Printerns DC.
*/
     gwinpt->dc = pd.hDC;
/*
***Hur stor �r plotarean i pixels och hur stora �r pixlarna ?
*/
     gwinpt->geo.x  = 0;
     gwinpt->geo.y  = 0,
     gwinpt->geo.dx = GetDeviceCaps(pd.hDC,HORZRES);
     gwinpt->geo.dy = GetDeviceCaps(pd.hDC,VERTRES);
     gwinpt->geo.psiz_x = (double)GetDeviceCaps(pd.hDC,HORZSIZE) / (double)gwinpt->geo.dx;
     gwinpt->geo.psiz_y = (double)GetDeviceCaps(pd.hDC,VERTSIZE) / (double)gwinpt->geo.dy;
/*
***Initiera printern.
*/
     di.cbSize      = sizeof(DOCINFO);
     di.lpszDocName = rubrik;
     di.lpszOutput  = NULL;

     StartDoc(pd.hDC,&di);
     StartPage(pd.hDC);

     return(TRUE);
     }
   else return(FALSE);
  }

/********************************************************/
/********************************************************/

 static int     pltply(gwinpt,k,x,y,a)
        WPGWIN *gwinpt;
        short   k;
        double  x[];
        double  y[];
        char    a[];

/*      Plottar en polylinje.
 *
 *      In: gwinpt => Pekare till f�nster att rita i.
 *          k      => Antal vektorer
 *          x      => X-koordinater
 *          y      => Y-koordinater
 *          a      => T�nd/Sl�ck.
 *
 *      Ut: Inget.
 *
 *      (C) microform ab 30/11/95 J. Kjellander
 *
 *********************************************************/

  {
   int     i,np;
   POINT   ip[PLYMXV];

/*
***Initiering.
*/
   np = 0;
/*
***Kopiera t�nda f�rflyttningar till ip och rita med
***Polyline(..).
*/
   for ( i=0; i<=k; i++ )
     {
     if ( (a[i] & VISIBLE) == VISIBLE ) 
       {
       ip[np].x = (short)(gwinpt->vy.k1x + gwinpt->vy.k2x*x[i]);
       ip[np].y = gwinpt->geo.dy -
                  (short)(gwinpt->vy.k1y + gwinpt->vy.k2y*y[i]);
       np++;
       }
     else
       {
       if ( np > 0 )
         {
         Polyline(gwinpt->dc,ip,np);
         np = 0;
         }
       ip[np].x = (short)(gwinpt->vy.k1x + gwinpt->vy.k2x*x[i]);
       ip[np].y = gwinpt->geo.dy -
                  (short)(gwinpt->vy.k1y + gwinpt->vy.k2y*y[i]);
       np++;
       }
     }
/*
***Gl�m inte sista skv�tten.
*/
   if ( np > 0 ) Polyline(gwinpt->dc,ip,np);

   return(0);
  }

/********************************************************/
/*!******************************************************/

 static int exitpl(dc)
        HDC dc;

/*      Avslutar plottning.
 *
 *      In:
            dc = Plotterns DC.
 *
 *      Ut: 
 *          Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 29/11/95 J. Kjellander
 *
 ******************************************************!*/

  {

   EndPage(dc);
   EndDoc(dc);
   DeleteDC(dc);

   return(0);
  }

/********************************************************/
