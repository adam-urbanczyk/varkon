/**********************************************************************
*
*    wp12.c
*    ====
*
*    This file is part of the VARKON WindowPac Library.
*    URL: http://www.varkon.com
*
*    This file includes:
*
*    wpwciw();      Create WPIWIN, CRE_WIN in MBS
*    wpxpiw();      Expose routine for WPIWIN
*    wpbtiw();      Button routine for WPIWIN
*    wpcriw();      Crossing routine for WPIWIN 
*    wpkeiw();      Key routine for WPIWIN
*    wpcmiw();      ClientMessage routine for WPIWIN
*    wpdliw();      Kills WPIWIN 
*
*    wpcrfb();      Create WPBUTT, CRE_FBUTTON in MBS
*    wpmcbu();      Create WPBUTT, CRE_BUTTON in MBS
*    wpwcbu();      Create WPBUTT, wpw-version
*    wpxpbu();      Expose routine for WPBUTT
*    wpbtbu();      Button routine for WPBUTT
*    wpcrbu();      Crossing routine for WPBUT
*    wpgtbu();      Get routine for WPBUTT, GET_BUTTON in MBS
*    wpdlbu();      Kills WPBUTT
*    wpgcbu();      Returns button color
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
#include "../include/WP.h"

extern Window       xgwin;

/*!******************************************************/

        short wpwciw(
        short   x,
        short   y,
        short   dx,
        short   dy,
        char   *label,
        v2int  *id)

/*      Skapar WPIWIN-f�nster.
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
 *      Felkod: WP1052 = wpwtab full.
 *              WP1062 = Fel fr�n malloc().
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    short                i;
    XSetWindowAttributes xwina;
    unsigned long        xwinm;
    XSizeHints           xhint;
    Window               xwin_id;
    WPIWIN              *iwinptr;

/*
***Skapa ett ledigt f�nster-ID.
*/
    if ( (*id=wpwffi()) < 0 ) return(erpush("WP1052",label));
/*
***S�tt f�rg mm.
*/
    xwina.background_pixel  = wpgcol(WP_BGND);
    xwina.border_pixel      = BlackPixel(xdisp,xscr);
    xwina.override_redirect = False;
    xwina.save_under        = False;

    xwinm = ( CWBackPixel        | CWBorderPixel |
              CWOverrideRedirect | CWSaveUnder );  
/*
***Skapa ett popup-f�nster med 1 pixels ram.
*/
    xwin_id = XCreateWindow(xdisp,DefaultRootWindow(xdisp),x,y,dx,dy,1,
                            CopyFromParent,InputOutput,
                            CopyFromParent,xwinm,&xwina);

    xhint.flags  = USPosition | USSize;   
    xhint.x      = x;
    xhint.y      = y;
    xhint.width  = dx;
    xhint.height = dy; 
    XSetNormalHints(xdisp,xwin_id,&xhint);

    wpmaps(label); 
    XStoreName(xdisp,xwin_id,label);   
    XSetTransientForHint(xdisp,xwin_id,xgwin);
    XSelectInput(xdisp,xwin_id,ExposureMask | KeyPressMask);
/*
***WPIWIN f�r ej d�das av en WINDOW-Manager som tex. Motif.
*/
    wpsdpr(xwin_id);
/*
***Skapa ett WPIWIN.
*/
    if ( (iwinptr=(WPIWIN *)v3mall(sizeof(WPIWIN),"wpwciw")) == NULL )
      return(erpush("WP1062",label));

    iwinptr->id.w_id = *id;
    iwinptr->id.p_id =  (wpw_id)NULL;
    iwinptr->id.x_id =  xwin_id;

    iwinptr->geo.x =  x;
    iwinptr->geo.y =  y;
    iwinptr->geo.dx =  dx;
    iwinptr->geo.dy =  dy;

    iwinptr->mapped =  FALSE;

    for ( i=0; i<WP_IWSMAX; ++i) iwinptr->wintab[i].ptr = NULL;
/*
***Lagra f�nstret i f�nstertabellen.
*/
    wpwtab[*id].typ = TYP_IWIN;
    wpwtab[*id].ptr = (char *)iwinptr;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        bool wpxpiw(
        WPIWIN *iwinptr)

/*      Expose-rutin f�r WPIWIN med vidh�ngande sub-f�nster.
 *      Denna event-rutin servar alltid eventet. Anropande
 *      rutin har redan avgjort att Expose skall g�ras just
 *      p� detta f�nster.
 *
 *      In: iwinptr = C-pekare till WPIWIN.
 *
 *      Ut: Alltid TRUE.
 *
 *      Felkod: .
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    short   i;
    char   *subptr;

/*
***F�rst expose p� alla sub-f�nster.
*/
    for ( i=0; i<WP_IWSMAX; ++i )
      {
      subptr = iwinptr->wintab[i].ptr;
      if ( subptr != NULL )
        {
        switch ( iwinptr->wintab[i].typ )
          {
          case TYP_EDIT:
          wpxped((WPEDIT *)subptr);
          break;

          case TYP_BUTTON:
          wpxpbu((WPBUTT *)subptr);
          break;

          case TYP_ICON:
          wpxpic((WPICON *)subptr);
          break;
          }
        }
      }
/*
***WPIWIN-f�nstret sj�lvt har inga texter etc.
***att g�ra expose p� !
*/

    return(TRUE);
  }

/********************************************************/
/*!******************************************************/

        bool  wpbtiw(
        WPIWIN       *iwinptr,
        XButtonEvent *butev,
        wpw_id       *serv_id)

/*      Button-rutin f�r WPIWIN med vidh�ngande sub-f�nster.
 *      Kollar om muspekning skett i n�got av WPIWIN-f�nstrets
 *      subf�nster och servar is�fall eventet.
 *
 *      In: iwinptr = C-pekare till WPIWIN.
 *          butev   = X-but event.
 *          serv_id = Pekare till utdata.
 *
 *      Ut: *serv_id = ID f�r subf�nster som servat eventet.
 *
 *      Fv: TRUE  = Eventet servat.
 *          FALSE = Detta f�nster ej inblandat.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    short   i;
    char   *subptr;
    WPBUTT *butptr;
    WPEDIT *edtptr,*focptr;
    WPICON *icoptr;

/*
***WPIWIN sj�lvt kan inte generera ButtonEvent:s,
***bara sub-f�nstren.
*/
    for ( i=0; i<WP_IWSMAX; ++i )
      {
      subptr = iwinptr->wintab[i].ptr;
      if ( subptr != NULL )
        {
        switch ( iwinptr->wintab[i].typ )
          {
          case TYP_BUTTON:
          butptr = (WPBUTT *)subptr;
          if ( butev->window == butptr->id.x_id )
            {
            wpbtbu(butptr);
           *serv_id = butptr->id.w_id;
            return(TRUE);
            }
          break;

          case TYP_EDIT:
          edtptr = (WPEDIT *)subptr;
          if ( butev->window == edtptr->id.x_id )
            {
            focptr = wpffoc(iwinptr,FOCUS_EDIT);
           *serv_id = focptr->id.w_id;
            return(wpbted(edtptr,butev));
            }
          break;

          case TYP_ICON:
          icoptr = (WPICON *)subptr;
          if ( butev->window == icoptr->id.x_id )
            {
            wpbtic(icoptr);
           *serv_id = icoptr->id.w_id;
            return(TRUE);
            }
          break;
          }
        }
      }

    return(FALSE);
  }

/********************************************************/
/*!******************************************************/

        bool wpcriw(
        WPIWIN         *iwinpt,
        XCrossingEvent *croev)

/*      Crossing-rutin f�r WPIWIN med vidh�ngande sub-f�nster.
 *      Kollar om Leave/Enter skett i n�got av WPIWIN-f�nstrets
 *      subf�nster och servar is�fall eventet.
 *
 *      In: iwinpt = C-pekare till WPIWIN.
 *          croev  = X-cro event.
 *
 *      Ut: TRUE  = Eventet servat.
 *          FALSE = Detta f�nster ej inblandat.
 *
 *      Felkod: .
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    short   i;
    char   *subptr;
    WPBUTT *butptr;
    WPEDIT *edtptr;
    WPICON *icoptr;

/*
***G� igenom alla sub-f�nster.
*/
    for ( i=0; i<WP_IWSMAX; ++i )
      {
      subptr = iwinpt->wintab[i].ptr;
      if ( subptr != NULL )
        {
        switch ( iwinpt->wintab[i].typ )
          {
          case TYP_BUTTON:
          butptr = (WPBUTT *) subptr;
          if ( croev->window == butptr->id.x_id )
            {
            if ( croev->type == EnterNotify ) wpcrbu(butptr,TRUE);
            else wpcrbu(butptr,FALSE);
            return(TRUE);
            }
          else if ( croev->type == EnterNotify  &&  butptr->hlight == TRUE )
            wpcrbu(butptr,FALSE);
          break;

          case TYP_EDIT:
          edtptr = (WPEDIT *) subptr;
          if ( croev->window == edtptr->id.x_id )
            {
            wpcred(edtptr,croev);
            return(TRUE);
            }
          break;

          case TYP_ICON:
          icoptr = (WPICON *) subptr;
          if ( croev->window == icoptr->id.x_id )
            {
            wpcric(icoptr,croev);
            return(TRUE);
            }
          break;
          }
        }
      }

    return(FALSE);
  }

/********************************************************/
/*!******************************************************/

        bool wpkeiw(
        WPIWIN     *iwinpt,
        XKeyEvent  *keyev,
        int         slevel,
        wpw_id     *serv_id)

/*      Key-rutin f�r WPIWIN med vidh�ngande sub-f�nster.
 *      Om WPIWIN-f�nstret har ett subf�nster (WPEDIT) med
 *      input-focus = TRUE servas eventet.
 *
 *      In: iwinpt  = C-pekare till WPIWIN.
 *          keyev   = X-Key event.
 *          slevel  = Service-niv�.
 *          serv_id = Pekare till utdata.
 *
 *      Ut: *serv_id = ID f�r WPEDIT som servat eventet.
 *
 *      FV: TRUE  = Eventet servat.
 *          FALSE = Detta f�nster ej inblandat.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    WPEDIT *edtptr;

/*
***Finns sub-f�nster med focus ?.
*/
    edtptr = wpffoc(iwinpt,FOCUS_EDIT);
/*
***Om ja, prova att serva eventet. Vissa tangenttryckningar
***servas "lokal" utan att wpkeed returnerar TRUE. Exempel
***p� detta �r n�r man flyttar cursorn med piltangenterna till
***h�ger eller v�nster. 
*/
    if ( edtptr != NULL )
      {
      if ( wpkeed(edtptr,keyev,slevel) )
        {
       *serv_id = edtptr->id.w_id;
        return(TRUE);
        }
      }
/*
***Kommer vi hit beror det antingen p� att inget WPEDIT-f�nster
***har fokus just nu eller att det som har fokus inte anser att
***key-eventet �r av den typ som skall signaleras upp�t och f�
***wpwwtw() att g�ra return. Tex. piltangenter i sidled.
*/
   return(FALSE);
  }

/********************************************************/
/*!******************************************************/

        bool wpcmiw(
        WPIWIN               *iwinpt,
        XClientMessageEvent  *clmev)

/*      ClientMessage-rutinen f�r WPIWIN.
 *
 *      In: iwinpt  = C-pekare till WPIWIN.
 *          clmev   = X-event.
 *
 *      FV: TRUE  = Eventet servat.
 *          FALSE = Detta f�nster ej inblandat.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
/*
***IWIN-f�nster kan normalt inte ta hand om ClientMessage.
***Om det �r WM_DELETE_WINDOW servar vi genom att pipa lite.
*/
   if ( clmev->message_type ==
        XInternAtom(xdisp,"WM_PROTOCOLS",False) &&
        clmev->data.l[0]    == 
        XInternAtom(xdisp,"WM_DELETE_WINDOW",False) )
     {
     XBell(xdisp,100);
     return(TRUE);
     }
   else return(FALSE);
  }

/********************************************************/
/*!******************************************************/

        short wpdliw(
        WPIWIN *iwinptr)

/*      D�dar en WPIWIN med vidh�ngande sub-f�nster.
 *
 *      In: iwinptr = C-pekare till WPIWIN.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    short   i;
    char   *subptr;

/*
***D�da alla sub-f�nster.
*/
    for ( i=0; i<WP_IWSMAX; ++i )
      {
      subptr = iwinptr->wintab[i].ptr;
      if ( subptr != NULL )
        {
        switch ( iwinptr->wintab[i].typ )
          {
          case TYP_EDIT:
          wpdled((WPEDIT *)subptr);
          break;

          case TYP_BUTTON:
          wpdlbu((WPBUTT *)subptr);
          break;

          case TYP_ICON:
          wpdlic((WPICON *)subptr);
          break;
          }
        }
      }
/*
***L�mna tillbaks dynamiskt allokerat minne.
*/
    v3free((char *)iwinptr,"wpdliw");
 
    return(0);
  }

/********************************************************/
/*!******************************************************/

        short wpcrfb(
        int    pid,
        short  x,
        short  y,
        short  dx,
        short  dy,
        char  *butstr,
        char  *akod,
        short  anum,
        v2int *bid)

/*      Skapar snabbvalsknapp i grafiskt f�nster.
 *
 *      In: pid    = ID f�r grafiskt f�nster.
 *          x,y    = Placering.
 *          dx,dy  = Storlek.
 *          butstr = Knapptext.
 *          akod   = Aktionskod.
 *          anum   = Aktionsnummer.
 *          bid    = Pekare till resultat.
 *
 *      Ut: *bid = Knappens ID.
 *
 *      Felkod: 
 *              WP1512 = %s �r en otill�ten aktionskod.
 *              WP1482 = F�nstret %s finns ej
 *              WP1492 = F�nstret %s �r av fel typ
 *              WP1502 = F�nster %s �r fullt
 *
 *      (C)microform ab 1996-05-20 J. Kjellander
 *
 ******************************************************!*/

  {
    short    status,action;
    int      i;
    char     errbuf[80];
    WPWIN   *winptr;
    WPGWIN  *gwinpt;
    WPBUTT  *butptr;

/*
***Vilken aktionskod ?
*/
    if ( akod[1] != '\0' ) return(erpush("WP1512",akod));

    switch ( akod[0] )
      {
      case 'f': action = FUNC;  break;
      case 'm': action = MENU;  break;
      case 'p': action = PART;  break;
      case 'r': action = RUN;   break;
      case 'M': action = MFUNC; break;
  
      default: return(erpush("WP1512",akod));
      break;
      }
/*
***Fixa C-pekare till det grafiska f�nstrets entry i wpwtab.
*/
    if ( (winptr=wpwgwp(pid)) == NULL )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1482",errbuf));
      }
/*
***Kolla att det �r ett WPGWIN och fixa en pekare till
***f�r�lder-f�nstret sj�lvt.
*/
    if ( winptr->typ != TYP_GWIN )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1492",errbuf));
      }
    else gwinpt = (WPGWIN *)winptr->ptr;
/*
***Skapa ID f�r den nya knappen, dvs fixa
***en ledig plats i f�r�lderns f�nstertabell.
*/
    i = 0;
    while ( i < WP_GWSMAX  &&  gwinpt->wintab[i].ptr != NULL ) ++i;

    if ( i == WP_GWSMAX )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1502",errbuf));
      }
    else *bid = i;
/*
***Prova att skapa en tryckknapp.
*/
    status = wpwcbu(gwinpt->id.x_id,x,y,dx,dy,(short)1,
                        butstr,butstr,"",WP_BGND,WP_FGND,&butptr);

    if ( status < 0 ) return(status);
/*
***L�nka in den i WPGWIN-f�nstret.
*/
    gwinpt->wintab[*bid].typ = TYP_BUTTON;
    gwinpt->wintab[*bid].ptr = (char *)butptr;

    butptr->id.w_id = *bid;
    butptr->id.p_id =  pid;

    XMapWindow(xdisp,butptr->id.x_id);
/*
***Aktion.
*/
    butptr->acttyp = action;
    butptr->actnum = anum;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short wpmcbu(
        wpw_id  pid,
        short   x,
        short   y,
        short   dx,
        short   dy,
        short   bw,
        char   *str1,
        char   *str2,
        char   *fstr,
        short   cb,
        short   cf,
        v2int  *bid)

/*      Skapar WPBUTT-f�nster och l�nkar in i ett WPIWIN.
 *      CRE_BUTTON i MBS.
 *
 *      In: pid   = F�r�lder.
 *          x     = L�ge i X-led.
 *          y     = L�ge i Y-led.   
 *          dx    = Storlek i X-led.
 *          dy    = Storlek i Y-led.
 *          bw    = Border-width.
 *          str1  = Text i l�ge off/FALSE.
 *          str2  = Text i l�ge on/TRUE.
 *          fstr  = Fontnamn eller "" (default).
 *          cb    = Bakgrundsf�rg.
 *          cf    = F�rgrundsf�rg.
 *          bid   = Pekare till utdata.
 *
 *      Ut: *bid = Giltigt entry i f�r�lderns wintab.
 *
 *      Felkod: WP1072 = F�r�ldern %s finns ej.
 *              WP1082 = F�r�ldern %s �r ej ett WPIWIN.
 *              WP1092 = F�r m�nga subf�nster i %s.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    char                 errbuf[80];
    short                i,status;
    WPWIN               *winptr;
    WPIWIN              *iwinptr;
    WPBUTT              *butptr;

/*
***Fixa C-pekare till f�r�lderns entry i wpwtab.
*/
    if ( (winptr=wpwgwp(pid)) == NULL )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1072",errbuf));
      }
/*
***Kolla att det �r ett WPIWIN och fixa en pekare till
***f�r�lder-f�nstret sj�lvt.
*/
    if ( winptr->typ != TYP_IWIN )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1082",errbuf));
      }
    else iwinptr = (WPIWIN *)winptr->ptr;
/*
***Skapa ID f�r den nya knappen, dvs fixa
***en ledig plats i f�r�lderns f�nstertabell.
*/
    i = 0;
    while ( i < WP_IWSMAX  &&  iwinptr->wintab[i].ptr != NULL ) ++i;

    if ( i == WP_IWSMAX )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1092",errbuf));
      }
    else *bid = i;
/*
***Skapa knappen.
*/
    if ( (status=wpwcbu(iwinptr->id.x_id,x,y,dx,dy,bw,
                        str1,str2,fstr,cb,cf,&butptr)) < 0 ) return(status);
/*
***L�nka in den i WPIWIN-f�nstret.
*/
    iwinptr->wintab[*bid].typ = TYP_BUTTON;
    iwinptr->wintab[*bid].ptr = (char *)butptr;

    butptr->id.w_id = *bid;
    butptr->id.p_id =  pid;
/*
***Om WPIWIN-f�nstret redan �r mappat skall knappen mappas nu.
*/
    if ( iwinptr->mapped ) XMapWindow(xdisp,butptr->id.x_id);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short wpwcbu(
        Window   px_id,
        short    x,
        short    y,
        short    dx,
        short    dy,
        short    bw,
        char    *str1,
        char    *str2,
        char    *fstr,
        short    cb,
        short    cf,
        WPBUTT **outptr)

/*      Skapar WPBUTT-f�nster.
 *
 *      In: px_id  = F�r�ldra f�nstrets X-id.
 *          x      = L�ge i X-led.
 *          y      = L�ge i Y-led.   
 *          dx     = Storlek i X-led.
 *          dy     = Storlek i Y-led.
 *          bw     = Border-width.
 *          str1   = Text i l�ge off/FALSE.
 *          str2   = Text i l�ge on/TRUE.
 *          fstr   = Fontnamn eller "".
 *          cb     = Bakgrundsf�rg.
 *          cf     = F�rgrundsf�rg.
 *          outptr = Pekare till utdata.
 *
 *      Ut: *outptr = Pekare till WPBUTT.
 *
 *      Felkod: WP1102 = Fonten %s finns ej.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    XSetWindowAttributes xwina;
    unsigned long        xwinm;
    Window               xwin_id;
    WPBUTT              *butptr;

/*
***Skapa f�nstret i X.
*/
    xwina.background_pixel  = wpgcbu(wpwfpx(px_id),cb);
    xwina.border_pixel      = wpgcbu(wpwfpx(px_id),WP_BGND);
    xwina.override_redirect = True;
    xwina.save_under        = False;

    xwinm = ( CWBackPixel        | CWBorderPixel |
              CWOverrideRedirect | CWSaveUnder );  

    xwin_id = XCreateWindow(xdisp,px_id,x,y,dx,dy,bw,CopyFromParent,
                            InputOutput,CopyFromParent,xwinm,&xwina);
/*
***Om knappen har ram skall den ocks� kunna clickas i och
***highligtas.
***Utan ram �r den bara en "label".
*/
    if ( bw > 0 ) XSelectInput(xdisp,xwin_id,ButtonPressMask |
                                             EnterWindowMask |
                                             LeaveWindowMask);
/*
***Skapa en WPBUTT.
*/
    if ( (butptr=(WPBUTT *)v3mall(sizeof(WPBUTT),"wpwcbu")) == NULL )
       return(erpush("WP1112",str1));

    butptr->id.w_id = (wpw_id)NULL;
    butptr->id.p_id = (wpw_id)NULL;
    butptr->id.x_id = xwin_id;

    butptr->geo.x =  x;
    butptr->geo.y =  y;
    butptr->geo.dx =  dx;
    butptr->geo.dy =  dy;
    butptr->geo.bw =  bw;

    butptr->color.bckgnd = cb;
    butptr->color.forgnd = cf;

    if ( strlen(str1) > 80 ) str1[80] = '\0';
    strcpy(butptr->stroff,str1);
    if ( strlen(str2) > 80 ) str2[80] = '\0';
    strcpy(butptr->stron,str2);

    butptr->status = FALSE;
    butptr->hlight = FALSE;

    if ( fstr[0] == '\0' ) butptr->font = 0;
    else if ( (butptr->font=wpgfnr(fstr)) < 0 )
                         return(erpush("WP1102",fstr));

   *outptr = butptr;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        bool wpxpbu(
        WPBUTT *butptr)

/*      Expose-rutin f�r WPBUTT.
 *
 *      In: buttptr = C-pekare till WPBUTT.
 *
 *      Ut: Inget.   
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 *      1996-12-12 V�nsterjust. lablar, J.Kjellander
 *      1998-03-27 OpenGL f�r AIX, J.Kjellander
 *
 ******************************************************!*/

  {
    int      x,y;
    char     text[81];
    GC       but_gc;
    WPRWIN  *rwinpt;
    XFontStruct *xfs;         /*********ny********/

/*
***Vilken text skall f�nstret inneh�lla ?
*/
    if ( butptr->status ) strcpy(text,butptr->stron);
    else                  strcpy(text,butptr->stroff);
/*
***Mappa till 8-bitar ASCII.
*/
    wpmaps(text);
/*
***Om det �r en knapp med ram, ber�kna textens l�ge s�
***att den hamnar mitt i f�nstret.
*/
    x = (butptr->geo.dx - wpstrl(text))/2;
    y =  wpftpy(butptr->geo.dy);
/*
***Vilket GC skall anv�ndas ? Om knappen sitter i
***ett WPRWIN m�ste vi anv�nda  dess GC eftersom
***knappen is�fall delar dess visual. Knappar i
***�vriga typer av f�nster kan anv�nda xgc.
*/
    switch ( wpwtab[butptr->id.p_id].typ )
      {
      case TYP_RWIN:
      rwinpt = (WPRWIN *)wpwtab[butptr->id.p_id].ptr;
      but_gc = rwinpt->win_gc;
      xfs = wpgfnt(0);       /*****ny****/
      XSetFont(xdisp,but_gc,xfs->fid);    /*******ny*******/
      break;

      default:
      but_gc = xgc;
      wpsfnt(butptr->font);       /*****ny****/
      break;
      }
/*
***S�tt f�rger.
*/
    if ( butptr->color.bckgnd != WP_BGND )
      XSetBackground(xdisp,but_gc,wpgcbu(butptr->id.p_id,butptr->color.bckgnd));
    if ( butptr->color.forgnd != WP_FGND )
      XSetForeground(xdisp,but_gc,wpgcbu(butptr->id.p_id,butptr->color.forgnd));
/*
***Skriv ut.
*/
    XDrawImageString(xdisp,butptr->id.x_id,but_gc,x,y,text,strlen(text));
/*
***Tills vidare �terst�ller vi aktiv font och
***f�rger till default igen.
*/
    wpsfnt(0);

    if ( butptr->color.bckgnd != WP_BGND )
      XSetBackground(xdisp,but_gc,wpgcbu(butptr->id.p_id,WP_BGND));
    if ( butptr->color.forgnd != WP_FGND )
      XSetForeground(xdisp,but_gc,wpgcbu(butptr->id.p_id,WP_FGND));
/*
***Test av 3D-ram.
*/
    if ( butptr->geo.bw > 0 )
      wpd3db((char *)butptr,TYP_BUTTON);
/*
***Slut.
*/
    return(TRUE);
  }

/********************************************************/
/*!******************************************************/

        bool wpbtbu(
        WPBUTT *butptr)

/*      Button-rutin f�r WPBUTT.
 *
 *      In: buttptr = C-pekare till WPBUTT.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {

/*
***Muspekning i button-f�nster. Status togglas
***till motsatta v�rdet.
*/
    if ( butptr->status == 0 ) butptr->status = TRUE;
    else                       butptr->status = FALSE;
/*
***Sudda f�nstret och g�r expose.
*/
    XClearWindow(xdisp,butptr->id.x_id);
    wpxpbu(butptr);
 
    return(TRUE);
  }

/********************************************************/
/*!******************************************************/

        bool wpcrbu(
        WPBUTT *butptr,
        bool    enter)

/*      Crossing-rutin f�r WPBUTT.
 *
 *      In: iwinpt = C-pekare till WPBUTT.
 *          enter  = TRUE  => Enter
 *                   FALSE => Leave
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {

/*
***Enter => Highligt, dvs. bl� ram.
*/
    if ( enter == TRUE )
      {
      XSetWindowBorder(xdisp,butptr->id.x_id,wpgcbu(butptr->id.p_id,WP_NOTI));
      butptr->hlight = TRUE;
      }
/*
***Leave => Ram med samma f�rg som bakgrunden igen.
*/
    else                            
      {
      XSetWindowBorder(xdisp,butptr->id.x_id,wpgcbu(butptr->id.p_id,WP_BGND));
      butptr->hlight = FALSE;
      }

    return(TRUE);
  }

/********************************************************/
/*!******************************************************/

        short wpgtbu(
        v2int  iwin_id,
        v2int  butt_id,
        v2int *status)

/*      Get-rutin f�r WPBUTT.
 *
 *      In: iwin_id = Huvudf�nstrets id.
 *          butt_id = Button-f�nstrets id.
 *
 *      Ut: Inget.   
 *
 *      Felkod: WP1122 = F�r�ldern %s finns ej.
 *              WP1132 = F�r�ldern %s ej WPIWIN.
 *              WP1142 = Knappen %s finns ej.
 *              WP1152 = %s �r ej en knapp.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    char    errbuf[80];
    WPWIN  *winptr;
    WPIWIN *iwinptr;
    WPBUTT *buttptr;

/*
***Fixa C-pekare till f�r�lderns entry i wpwtab.
*/
    if ( (winptr=wpwgwp((wpw_id)iwin_id)) == NULL )
      {
      sprintf(errbuf,"%d",(int)iwin_id);
      return(erpush("WP1122",errbuf));
      }
/*
***Kolla att det �r ett WPIWIN.
*/
    if ( winptr->typ != TYP_IWIN )
      {
      sprintf(errbuf,"%d",(int)iwin_id);
      return(erpush("WP1132",errbuf));
      }
/*
***Fixa en C-pekare till WPIWIN.
*/
    iwinptr = (WPIWIN *)winptr->ptr;
/*
***Kolla om subf�nstret med angivet id finns och �r
***av r�tt typ.
*/
    if ( iwinptr->wintab[(wpw_id)butt_id].ptr == NULL )
      {
      sprintf(errbuf,"%d",(int)butt_id);
      return(erpush("WP1142",errbuf));
      }

    if ( iwinptr->wintab[(wpw_id)butt_id].typ != TYP_BUTTON )
      {
      sprintf(errbuf,"%d",(int)butt_id);
      return(erpush("WP1152",errbuf));
      }
/*
***Fixa en C-pekare till WPBUTT.
*/
    buttptr = (WPBUTT *)iwinptr->wintab[(wpw_id)butt_id].ptr;
/*
***Returnera status.
*/
    *status = buttptr->status;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short wpdlbu(
        WPBUTT *butptr)

/*      D�dar en WPBUTT.
 *
 *      In: buttptr = C-pekare till WPBUTT.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
/*
***L�mna tillbaks dynamiskt allokerat minne.
*/
    v3free((char *)butptr,"wpdlbu");
    return(0);
  }

/********************************************************/
/*!******************************************************/

 unsigned long wpgcbu(
        wpw_id p_id,
        int    colnum)

/*      Returnerar f�rg f�r WPBUTT. Om f�r�ldern �r
 *      en WPRWIN g�rs s�rskild hantering.
 *
 *      In: p_id   = ID f�r f�r�lder.
 *          colnum = VARKON f�rgnummer.
 *
 *      FV: Pixelv�rde.
 *
 *      (C)microform ab 1998-03-27 J. Kjellander
 *
 ******************************************************!*/

  {
   WPRWIN *rwinpt;

/*
***Om knappen sitter i ett WPRWIN m�ste vi returnera
***ett pixelv�rde som �r kompatibelt med det f�nstrets
***visual. �vriga f�nster anv�nder default visual.
*/
   switch ( wpwtab[p_id].typ )
     {
     case TYP_RWIN:
     rwinpt   = (WPRWIN *)wpwtab[p_id].ptr;
     switch ( colnum )
       {
       case WP_BGND: return(rwinpt->bgnd);
       case WP_FGND: return(rwinpt->fgnd);
       case WP_TOPS: return(rwinpt->tops);
       case WP_BOTS: return(rwinpt->bots);
       case WP_NOTI: return(rwinpt->noti);
       default:      return(0);
       }
     break;

     default:
     return(wpgcol(colnum));
     }
  }

/********************************************************/
