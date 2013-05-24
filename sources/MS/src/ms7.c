/**********************************************************************
*
*    ms7.c
*    =====
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     msmini();   Inits menues
*     msmexi();   Closes all menues
*     mspamu();   Push active menu
*     mswcmw();   Create menue window
*     msrcmw();   Update menu
*     msifae();   Select from menu
*
*     mscrmu();   Creates the menu trees
*     mssmmu();   Activates menu tree
*     msshmu();   Show menu
*     mshdmu();   Hide menu
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
#include "../../../sources/WP/include/WP.h"

static wpw_id   men_id;

/* men_id �r ID f�r meny-f�nstret. */

static HMENU poptab[MNUMAX];
static int   cmdant;

/* Handtag till menyerna och antal kommandon */

extern MNUDAT  *actmeny;
extern MNUDAT   mnutab[];
extern MNUALT  *ms_cmdtab[];

static HWND     altid[25];
static int      fh,fl,ly; 

/*!******************************************************/

        int msmini()

/*      Initierar menyf�nsterhanteringen. Skapar meny-
 *      f�nstret.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 * 
 *      Felkoder: WP1252 = Kan ej skapa menyf�nster p� %s
 *
 *      (C)microform ab 1/11/95 J. Kjellander
 *
 ******************************************************!*/

  {
   int   i,x,y,dx,dy,status; 
   char  title[V3STRLEN+1];
   v2int w_id;

/*
***Placering och storlek.
*/
   x  = y  = 25;
   dx = dy = 20;
/*
***F�nstertitel.
*/
   if ( !msgrst("MENU.TITLE",title) ) strcpy(title,"MENY");

   status = mswcmw(x,y,dx,dy,title,&w_id);
   if ( status < 0 ) return(erpush("WP1252","WIN32"));
/*
***Inga menyer �nnu aktiva.
*/
   actmeny = NULL;
/*
***Eller skapade.
*/
   for ( i=0; i<MNUMAX; ++i ) poptab[i] = NULL;
/*
***Slut.
*/
   return(0);
  }

/********************************************************/
/*!******************************************************/

        int msmexi()

/*      D�dar alla menyer.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 * 
 *      (C)microform ab 1997-01-20, J. Kjellander
 *
 ******************************************************!*/

  {
   int i; 

   for ( i=0; i<MNUMAX; ++i )
     {
     if ( poptab[i] != NULL )
       {
       DestroyMenu(poptab[i]);
       poptab[i] = NULL;
       }
     }
/*
***Slut.
*/
   return(0);
  }

/********************************************************/
/*!*******************************************************/

     int mspamu(meny)
     MNUDAT *meny;  

/*   G�r meny aktiv.
 *
 *   In: *meny = pekare till en viss meny.
 *
 *   Ut: Inget.
 *
 *   FV: 0. 
 *
 *   (C)microform ab 1/11/95 J. Kjellander
 *
 *******************************************************!*/

 {

/*
***Aktiv meny.
*/
    actmeny = meny;   
/*
***Skapa nya alternativf�nster.
*/
    msrcmw();
/*
***Slut.
*/
    return(0);
 }

/*********************************************************/
/*!******************************************************/

        int   mswcmw(x,y,dx,dy,label,id)
        int   x,y,dx,dy;
        char   *label;
        v2int  *id;

/*      Skapar ett WPIWIN-f�nster f�r en meny.
 *
 *      In: x     = L�ge i X-led.
 *          y     = L�ge i Y-led.   
 *          dx    = Storlek i X-led.
 *          dy    = Storlek i Y-led.
 *          label = F�nstertitel.
 *          id    = Pekare till utdata.
 *
 *      Ut: *id = Giltigt entry i wpwtab.
 *
 *      (C)microform ab 1/11/95 J. Kjellander
 *
 ******************************************************!*/

  {
    short   status;
    WPIWIN *iwinpt;

/*
***Skapa ett WPIWIN-f�nster. Spara ID i static-variabeln 
***men_id. Denna anv�nds av msrcmw().
*/
    status = mswciw(x,y,dx,dy,label,id);
    if ( status < 0 ) return(status);
    men_id = *id;
/*
***Vad blev resultatet.
*/
    iwinpt = (WPIWIN *)wpwtab[(wpw_id)*id].ptr;

    return(0);
  }

/********************************************************/
/*!*******************************************************/

     int msrcmw()

/*   Skapar nya alternativ-f�nster i meny-f�nstret.
 *
 *   In: Inget.
 *
 *   Ut: Inget.
 *
 *   (C)microform ab 1/11/95 J. Kjellander
 *
 *******************************************************!*/

 {
    int     mendx,mendy;
    char   *ftitel = "MENY";
    int     bw,lm,tl,mrl,i,tlmax,ftl; 
    v2int   rubr_id,alt_id;
    MNUALT *altptr;
    WPIWIN *mwinpt;
    WPBUTT *butptr;

/*
***Lite initiering.
*/
    altptr = actmeny->alt;
    tlmax = 0;
    mrl = msstrl(actmeny->rubr);
    ftl = msstrl(ftitel);
/*
***Ber�kning av menyh�jd.
*/
    ly = (int)(0.5*msstrh());
    lm = (int)(0.5*msstrh());
    fh = (int)(1.5*msstrh());
    mendy = lm + fh + (actmeny->nalt - 1)*lm + actmeny->nalt*fh + ly;
/*
***Ber�kning av maximal textl�ngd.
*/
    for (i=0; i<actmeny->nalt; i++)
       {
       tl = msstrl(altptr->str);
       ++altptr;
       if ( tl > tlmax) tlmax = tl;
       } 
/*
***Kolla �ven meny-rubrikens och f�nstertitelns l�ngd.
*/
       if ( mrl > tlmax) tlmax = mrl;
       if ( ftl > tlmax) tlmax = ftl;
/*
***Ber�kning av menyns bredd.
*/    
    fl = (int)(1.2*tlmax);
    mendx = ly + fl + ly;
/*
***D�da nuvarande rubrik och alternativ-f�nster.
*/
    mwinpt = (WPIWIN *)wpwtab[men_id].ptr;

    for ( i=0; i<WP_IWSMAX; ++i )
      {
      butptr = (WPBUTT *)mwinpt->wintab[i].ptr;
      if ( butptr != NULL )
        {
        DestroyWindow(butptr->id.ms_id);
        msdlbu(butptr);
        mwinpt->wintab[i].ptr = NULL;
        mwinpt->wintab[i].typ = TYP_UNDEF;
        }
      else break;
      }
/*
***L�t meny-f�nstret �ndra storlek.
*/
    mwinpt->geo.dx = (short)mendx;
    mwinpt->geo.dy = (short)mendy;

    SetWindowPos(mwinpt->id.ms_id,NULL,25,25,
                 mendx,mendy + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME),
                 SWP_NOMOVE | SWP_NOZORDER);
/*
***Skapa menyrubrik-f�nster.
*/
    msmcbu(men_id,ly,lm,fl,fh,0,actmeny->rubr,actmeny->rubr,
                                     "",WP_BGND,WP_FGND,&rubr_id);
/*
***Skapa alternativ-f�nster.
*/
    altptr = actmeny->alt;
    bw = GetSystemMetrics(SM_CXFRAME);

    for ( i=0; i<actmeny->nalt; i++ )
      {
      msmcbu(men_id,ly,lm + fh + i*(fh + lm),fl-2*GetSystemMetrics(SM_CXFRAME),fh,bw,
                   altptr->str,altptr->str,"",WP_BGND,WP_FGND,&alt_id);
      butptr = (WPBUTT *)mwinpt->wintab[alt_id].ptr;
      altid[i] = butptr->id.ms_id;
      ++altptr;
      }
/*
***S�tt showstate = show.
*/
    ShowWindow(mwinpt->id.ms_id,SW_SHOW);
/*
***Slut.
*/
    return(0);
 }

/*********************************************************/
/*!******************************************************/

        bool msifae(win32_id,altptr)
        HWND     win32_id;
        MNUALT **altptr;

/*      Testar om ett visst f�nster �r ett av dom
 *      alternativ-f�nster som ing�r i aktiv meny.
 *
 *      In: win32_id = F�nstrets ID.
 *          altptr = Pekare till utdata.
 *
 *      Ut: *altptr = Pekare till utpekat meny-alternativ.
 *
 *      FV: TRUE  => Ja.
 *          FALSE => Nej.
 *
 *      (C)microform ab 1/11/95 J. Kjellander
 *
 ******************************************************!*/

  {
    int  i = 0;
    bool hit = FALSE;

#ifdef DEBUG
    if ( dbglev(MSPAC) == 2 )
      {
      fprintf(dbgfil(MSPAC),"***Start-msifae***\n");
      fprintf(dbgfil(MSPAC),"F�nstrets MS-ID=%d\n",win32_id);
      }
#endif

/*
***Om ingen meny finns, returnera FALSE.
*/
    if ( actmeny == NULL ) hit = FALSE;
/*
***Om altptr == NULL �r det inte till�tet att g�ra
***menyval.
*/
    else if ( altptr == NULL ) hit = FALSE;
/*
***Leta igenom menyns f�nsterlista efter tr�ff.
*/
   else
     {
     *altptr = actmeny->alt;
  
      while ( hit == FALSE  &&  i < actmeny->nalt )
        {
        if ( win32_id == altid[i++] ) hit = TRUE;
        else *altptr = *altptr + 1;
        }
      }
/*
***Ev. debug.
*/
#ifdef DEBUG
    if ( dbglev(MSPAC) == 2 )
      {
      if ( hit == TRUE ) fprintf(dbgfil(MSPAC),"hit = TRUE\n");
      else fprintf(dbgfil(MSPAC),"hit = FALSE\n");
      fprintf(dbgfil(MSPAC),"***Slut-msifae***\n\n");
      fflush(dbgfil(MSPAC));
      }
#endif

    return(hit);
  }

/********************************************************/
/*!******************************************************/

        int mscrmu()

/*      Skapar huvudf�nstrets menyer fr�n aktuell menyfil.
 *      Anropas av ige1:iginit() varje g�ng nya menyer laddas.
 *
 *      In: Inget.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 5/11/95 J. Kjellander
 *
 *      1997-01-20 �ndrad &-hantering, J.Kjellander
 *
 ******************************************************!*/

  {
   char    altstr[V3STRLEN];
   bool    hit,status;
   int     i,j,n;
   MNUALT *altpek;

/*
***Initiering.
*/
   cmdant = 0;

   for ( i=0; i<MNUMAX; ++i )
     {
     if ( poptab[i] != NULL )
       {
       DestroyMenu(poptab[i]);
       poptab[i] = NULL;
       }
     }
/*
***I pass ett g�r vi igenom V3:s menytabell och skapar en
***tom MS-PopupMenu f�r varje V3-meny. �nnu vet vi inte vilken
***som kommer att bli huvudmeny.
*/
   for ( i=1; i<MNUMAX; ++i )
     if ( mnutab[i].rubr != NULL ) poptab[i] = CreatePopupMenu();
/*
***I pass tv� fyller vi p� med alternativen.
*/
   for ( i=1; i<MNUMAX; ++i )
     {
     if ( mnutab[i].rubr != NULL )
       {
       altpek = mnutab[i].alt;
      

       for ( j=0; j<mnutab[i].nalt; ++j )
         {
/*
***Om alternativstr�ngen ineh�ller ett &-tecken tar vi det
***annars tar vi f�rsta tecknet.
*/
         n = strlen(altpek->str);
         hit = FALSE;
         while ( n > 0 )
           {
           n--;
           if ( *(altpek->str + n ) == '&' ) hit = TRUE;
           }

         if ( !hit )
           {
           strcpy(altstr,"&");
           strcat(altstr,altpek->str);
           }
         else strcpy(altstr,altpek->str);

         if ( altpek->acttyp == MENU )
           {
           status = AppendMenu(poptab[i],
                    MF_STRING | MF_POPUP,
              (UINT)poptab[altpek->actnum],
                    altstr);
           }
         else
           {
           status = AppendMenu(poptab[i],MF_STRING,cmdant+1000,altstr);
           ms_cmdtab[cmdant++] = altpek;
           }
       ++altpek;
         }
       }
     }

    return(0);
  }

/********************************************************/
/*!******************************************************/

        int mssmmu(mmain)
        int mmain;

/*      Aktiverar r�tt huvudmeny. Anropas av ige22:igload()
 *      varje g�ng ett nytt jobb laddas. Problemet �r att
 *      man vet inte s�kert n�r man laddat menyerna i igldmf()
 *      vilken som kommer att bli huvudmeny utan f�rst n�r
 *      jobbet laddats och d�rf�r m�ste man v�nta tills dess.
 *
 *      In: mmain = Huvudmeny.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 5/11/95 J. Kjellander
 *
 ******************************************************!*/

  {
   char    altstr[V3STRLEN];
   bool    hit;
   int     j,n;
   MNUALT *altpek;

/*
***Koppla loss alla alternativen fr�n popup-versionen av menyn och
***d�da den s� att vi inte f�rbrukar resurser med oanv�nda menyer.
***
********Funkar inte !!!! vartannat alternativ i huvudmenyn f�rsvinner *****
*

     for ( j=0; j<mnutab[mmain].nalt; ++j )
       {
       RemoveMenu(poptab[mmain],j,MF_BYPOSITION); 
       }

     DestroyMenu(poptab[mmain]);
/*
***poptab[mmain] �r redan skapad i form av en popup-meny
***men nu skall den g�ras om till en menybalk ist�llet.
*/
   if ( mnutab[mmain].rubr != NULL )
     {
     poptab[mmain] = CreateMenu();
/*      
***Anslut r�tt saker till menybalken.
*/
     altpek = mnutab[mmain].alt;

     for ( j=0; j<mnutab[mmain].nalt; ++j )
       {
       n = strlen(altpek->str);
       hit = FALSE;
       while ( n > 0 )
         {
         n--;
         if ( *(altpek->str + n ) == '&' ) hit = TRUE;
         }

       if ( !hit )
         {
         strcpy(altstr,"&");
         strcat(altstr,altpek->str);
         }
       else strcpy(altstr,altpek->str);

       if ( altpek->acttyp == MENU )
         {
         AppendMenu(poptab[mmain],MF_STRING | MF_POPUP,
                    (UINT)poptab[altpek->actnum],altstr);
         }
       else
         {
         AppendMenu(poptab[mmain],MF_STRING,cmdant+1000,altstr);
         ms_cmdtab[cmdant++] = altpek;
         }
     ++altpek;
       }
/*
***Aktivera.
*/
     SetMenu(ms_main,poptab[mmain]);
     }


    return(0);
  }

/********************************************************/
/*!*******************************************************/

     int msshmu(meny)
     int meny;  

/*   Visar en meny.
 *
 *   In: meny = Viken meny.
 *
 *   Ut: Inget.
 *
 *   FV: 0. 
 *
 *   (C)microform ab 9/11/95 J. Kjellander
 *
 *******************************************************!*/

 {

/*
***Ut med menyn.
*/
    mspamu(&mnutab[meny]);   
/*
***Slut.
*/
    return(0);
 }

/*********************************************************/
/*!*******************************************************/

     int mshdmu()

/*   Sl�cker visad meny.
 *
 *   In: Inget.
 *
 *   Ut: Inget.
 *
 *   FV: 0. 
 *
 *   (C)microform ab 9/11/95 J. Kjellander
 *
 *******************************************************!*/

 {
    WPIWIN *mwinpt;
  
/*
***En C-pekare till meny-f�nstret.
*/
    mwinpt = (WPIWIN *)wpwtab[men_id].ptr;
/*
***S�tt showstate = hide.
*/
    ShowWindow(mwinpt->id.ms_id,SW_HIDE);
/*
***Slut.
*/
    return(0);
 }

/*********************************************************/
