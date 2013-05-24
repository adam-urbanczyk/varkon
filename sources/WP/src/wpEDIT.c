/**********************************************************************
*
*    wpEDIT.c
*    ========
*
*    This file is part of the VARKON WindowPac Library.
*    URL: http://www.tech.oru.se/cad/varkon
*
*    This file includes:
*
*    WPmced();   Create WPEDIT, MBS-version
*    WPwced();   Create WPEDIT, wpw-version
*    WPxped();   Expose routine for WPEDIT
*    WPbted();   Button routine for WPEDIT
*    WPcred();   Crossing routine for WPEDIT
*    WPkeed();   Key routine for WPEDIT
*    WPgted();   Get routine for WPEDIT, GET_EDIT in MBS
*    WPuped();   Replace text in WPEDIT
*    WPdled();   Kill WPEDIT
*
*   *WPffoc();   Which WPEDIT has focus ?
*    WPfoed();   Focus On/Off for WPEDIT
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

#define XK_MISCELLANY
#define XK_LATIN1
#ifdef UNIX
#include <X11/keysymdef.h>
#endif

extern MNUALT       smbind[];

static short edmapk(XKeyEvent *keyev, short *sym, char *t);

/*!******************************************************/

        short WPmced(
        wpw_id  pid,
        short   x,
        short   y,
        short   dx,
        short   dy,
        short   bw,
        char   *str,
        short   ntkn,
        DBint  *eid)

/*      Create WPEDIT window.
 *
 *      In: pid   = F�r�lder.
 *          x     = L�ge i X-led.
 *          y     = L�ge i Y-led.   
 *          dx    = Storlek i X-led.
 *          dy    = Storlek i Y-led.
 *          bw    = Border-width.
 *          str   = Text.
 *          ntkn  = Max antal tecken.
 *          eid   = Pekare till utdata.
 *
 *      Ut: *eid = Giltigt entry i f�r�lderns wintab.
 *
 *      Felkod: WP1302 = F�r�ldern %s finns ej.
 *              WP1312 = F�r�ldern %s �r ej ett WPIWIN.
 *              WP1322 = F�r m�nga subf�nster i %s.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    char                 errbuf[80];
    short                i;
    WPWIN               *winptr;
    WPIWIN              *iwinptr;
    WPEDIT              *edtptr;

/*
***Fixa C-pekare till f�r�lderns entry i wpwtab.
*/
    if ( (winptr=WPwgwp(pid)) == NULL )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1302",errbuf));
      }
/*
***Kolla att det �r ett WPIWIN och fixa en pekare till
***f�r�lder-f�nstret sj�lvt.
*/
    if ( winptr->typ != TYP_IWIN )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1312",errbuf));
      }
    else iwinptr = (WPIWIN *)winptr->ptr;
/*
***Skapa ID f�r  ny edit, dvs fixa
***en ledig plats i f�r�lderns f�nstertabell.
*/
    i = 0;
    while ( i < WP_IWSMAX  &&  iwinptr->wintab[i].ptr != NULL ) ++i;

    if ( i == WP_IWSMAX )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1322",errbuf));
      }

   *eid = i;
/*
***Skapa edit.
*/
    WPwced(iwinptr->id.x_id,x,y,dx,dy,bw,str,ntkn,&edtptr);
/*
***L�nka in den i WPIWIN-f�nstret.
*/
    iwinptr->wintab[*eid].typ = TYP_EDIT;
    iwinptr->wintab[*eid].ptr = (char *)edtptr;

    edtptr->id.w_id = *eid;
    edtptr->id.p_id =  pid;
/*
***Om WPIWIN-f�nstret redan �r mappat skall editen mappas nu.
*/
    if ( iwinptr->mapped ) XMapWindow(xdisp,edtptr->id.x_id);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short WPwced(
        Window   px_id,
        short    x,
        short    y,
        short    dx,
        short    dy,
        short    bw,
        char    *str,
        short    ntkn,
        WPEDIT **outptr)

/*      Skapar WPEDIT-f�nster.
 *
 *      In: px_id  = F�r�ldra f�nstrets X-id.
 *          x      = L�ge i X-led.
 *          y      = L�ge i Y-led.   
 *          dx     = Storlek i X-led.
 *          dy     = Storlek i Y-led.
 *          bw     = Border-width.
 *          str    = Text.
 *          ntkn   = Max antal tecken.
 *          outptr = Pekare till utdata.
 *
 *      Ut: *outptr = Pekare till WPEDIT.
 *
 *      Felkod: .
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 *      12/4/95  ntkn > V3STRLEN, J. Kjellander
 *
 ******************************************************!*/

  {
    XSetWindowAttributes xwina;
    unsigned long        xwinm;
    Window               xwin_id;
    WPEDIT              *edtptr;

/*
***Skapa f�nstret i X. Ramen samma f�rg som bakgrunden.
*/
    xwina.border_pixel      = WPgcol(WP_BGND);
    xwina.background_pixel  = WPgcol(WP_BGND);
    xwina.override_redirect = True;
    xwina.save_under        = False;

    xwinm = ( CWBackPixel        | CWBorderPixel |
              CWOverrideRedirect | CWSaveUnder );  

    xwin_id = XCreateWindow(xdisp,px_id,x,y,dx,dy,bw,
                            DefaultDepth(xdisp,xscr),
                            InputOutput,CopyFromParent,xwinm,&xwina);
/*
***Edit-f�nster skall det kunna clickas i. 
*/
    XSelectInput(xdisp,xwin_id,ButtonPressMask);
/*
***Skapa en WPEDIT.
*/
    if ( (edtptr=(WPEDIT *)v3mall(sizeof(WPEDIT),"WPwced"))
                                                  == NULL ) return(-2);

    edtptr->id.w_id = (wpw_id)NULL;
    edtptr->id.p_id = (wpw_id)NULL;
    edtptr->id.x_id = xwin_id;

    edtptr->geo.x =  x;
    edtptr->geo.y =  y;
    edtptr->geo.dx =  dx;
    edtptr->geo.dy =  dy;
    edtptr->geo.bw =  bw;

    if ( ntkn < 0 ) ntkn = 0;
    if ( ntkn > V3STRLEN ) ntkn = V3STRLEN;
    if ( strlen(str) > (unsigned int)ntkn ) str[ntkn] = '\0';
    strcpy(edtptr->str,str);
    strcpy(edtptr->dstr,str);
    edtptr->tknmax = ntkn;

    edtptr->scroll = 0;
    edtptr->curpos = 0;
    edtptr->fuse   = TRUE;
    edtptr->focus  = FALSE;

   *outptr = edtptr;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        bool WPxped(
        WPEDIT *edtptr)

/*      Expose-rutin f�r WPEDIT.
 *
 *      In: edtptr = C-pekare till WPEDIT.
 *
 *      Ut: Inget.   
 *
 *      Fv: Alltid TRUE.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    int  x,y,tknf;
    char str[V3STRLEN+1];

/*
***Ber�kna textens l�ge.
*/
    x =  WPstrl(" ");
    y =  WPftpy(edtptr->geo.dy);
/*
***Skriv ut den del av texten som syns i f�nstret, dvs.
***inte det som �r utscrollat. Antal tillg�ngliga tecken-
***positioner �r i princip s� m�nga som f�r plats men vi
***b�rjar alltid med en tom f�r sysn skull och reserverar
***dessutom en tom p� slutet f�r cursorn.
*/
    strcpy(str,&edtptr->str[edtptr->scroll]);

    tknf = (edtptr->geo.dx/WPstrl(" ")) - 2;
    if ( tknf > 0 ) str[tknf] = '\0';

    WPwstr(edtptr->id.x_id,x,y,str);
/*
***Om f�nstret har focus, �ven en cursor. Tecken 152 i font 1.
***X-koordinat ber�knas med font 0 aktiv men Y-koordinat med
***font 1.
*/
    if ( edtptr->focus  &&  edtptr->curpos >= 0 )
      {
      x = x + (edtptr->curpos - edtptr->scroll)*WPstrl(" ") -1;
      WPsfnt(1);
      y = WPftpy(edtptr->geo.dy);
      str[0] = 152; str[1] = '\0';
      XDrawString(xdisp,edtptr->id.x_id,xgc,x,y,str,1);
      WPsfnt(0);
      }
/*
***Test av 3D-look.
*/
    if ( edtptr->geo.bw > 0 )
      WPd3db((char *)edtptr,TYP_EDIT);
/*
***Slut.
*/
    return(0);
  }

/********************************************************/
/*!******************************************************/

        bool WPbted(
        WPEDIT       *edtptr,
        XButtonEvent *butev)

/*      Button-rutin f�r WPEDIT.
 *
 *      In: edtptr = C-pekare till WPEDIT.
 *          butev  = X-event.
 *
 *      Ut: Inget.   
 *
 *      FV: TRUE  = Eventet servat.
 *          FALSE = Eventet g�ller ej detta f�nster.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    int      x,tknpos,nvis;
    WPWIN   *winptr;
    WPIWIN  *iwinpt;
    WPEDIT  *actptr;

/*
***Om detta f�nster inte har input-focus skall det 
***ha det nu ! Det f�nster som ev. hade det tidigare
***skal d� inte ha focus l�ngre. F�r att s�kert bli av
***med cursorn suddar vi d� hela f�nstret och ritar om det.
***Eftersom fokus-byte kan returnera TRUE s�tts gamla
***f�nstrets symbol till SMBNONE s� att ingen skall tro
***att det �r en tangenttryckning som genererat fokus-bytet.
*/
    if ( edtptr->focus == FALSE )
      {
      winptr = WPwgwp(edtptr->id.p_id);
      iwinpt = (WPIWIN *)winptr->ptr;
      actptr = WPffoc(iwinpt,FOCUS_EDIT);
      if ( actptr != NULL )
        {
        XClearWindow(xdisp,actptr->id.x_id);
        WPfoed(actptr,FALSE);
        actptr->symbol = SMBNONE;
        }
      WPfoed(edtptr,TRUE);
      return(TRUE);
      }
/*
***Om f�nstret redan har input-focus handlar det om att
***placera cursorn.
*/
    else
      {
      x = butev->x;
      tknpos = (int)((double)x/(double)WPstrl(" "));
      nvis = strlen(edtptr->str) - edtptr->scroll;
      if ( tknpos > nvis ) tknpos = nvis+1;
      if ( tknpos < 1 ) tknpos = 1;
      edtptr->curpos = tknpos - 1 + edtptr->scroll;
      XClearWindow(xdisp,edtptr->id.x_id);
      WPxped(edtptr);

      return(FALSE);
      }
  }

/********************************************************/
/*!******************************************************/

        bool WPcred(
        WPEDIT         *edtptr,
        XCrossingEvent *croev)

/*      Crossing-rutin f�r WPEDIT. Anropande rutin har
 *      konstaterat att eventet g�ller detta f�nster men 
 *      skickar med croev f�r att vi skall kunna avg�ra
 *      om det �r enter eller leave.
 *
 *      In: edtptr = C-pekare till WPEDIT.
 *          croev  = X-crossing event.
 *
 *      Ut: Inget.   
 *
 *      FV: TRUE  = Eventet servat.
 *          FALSE = Eventet g�ller ej detta f�nster.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
/*
***F�r tillf�llet utnyttjas inte CrossingEvents i WPEDIT:s.
*/
    return(FALSE);
  }

/********************************************************/
/*!******************************************************/

        bool WPkeed(
        WPEDIT    *edtptr,
        XKeyEvent *keyev,
        int        slevel)

/*      Key-rutin f�r WPEDIT. Servar eventet och returnerar
 *      status TRUE/FALSE beroende p� vilken tangent som 
 *      tryckts ned och vilken service-niv� som angetts.
 *
 *      In: edtptr = C-pekare till WPEDIT.
 *          keyev  = X-key event.
 *          slevel = Service-niv�.
 *
 *          SLEVEL_ALL    => Inga servas lokalt.
 *          SLEVEL_V3_INP => SMBCHAR + SMBBACKSP +
 *                           SMBLEFT + SMBRIGHT  servas lokalt.
 *          SLEVEL_MBS    => Dessutom SMBMAIN, SMBPOSM och SMBHELP
 *          SLEVEL_NONE   => Alla event servas lokalt.
 *
 *      Ut: Inget.   
 *
 *      FV: TRUE  = Eventet servat.
 *          FALSE = Eventet g�ller ej detta f�nster.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    short    symbol;
    int      tknant,tknwin,i;
    char     tkn;
    bool     servat;
    WPWIN   *winptr;
    WPIWIN  *iwinpt;
    WPEDIT  *nxtptr;

/*
***Initiering.
*/
    tknant = strlen(edtptr->str);
    tknwin = edtptr->geo.dx/WPstrl(" ") - 2;
    servat = FALSE;
/*
***Mappa tangenttryckningen till V3-symbol.
***Logga symbolen i editen.
*/
    edmapk(keyev,&symbol,&tkn);
    edtptr->symbol = symbol;
/*
***Vidtag �tg�rd..
*/
    switch ( symbol )
      {
      case SMBCHAR:
/*
***Insert, finns det plats f�r fler tecken ?
***Det finns det alltid om fuse = TRUE !
*/
      if ( !edtptr->fuse && (tknant >= edtptr->tknmax) ) XBell(xdisp,100);
/*
***Ja, kolla om det �r f�rsta g�ngen ett tecken kommer till detta
***f�nster, dvs om fuse = TRUE. Is�fall blir det inte vanlig insert
***utan f�rst suddning av gammal text och sedan insert. Om cursorn
***inte st�r i pos 0 g�ller dock detta inte.
*/
      else
        {          
        if ( edtptr->fuse  &&  edtptr->curpos == 0 )
          {
          *edtptr->str = '\0';
          tknant = 0;
          }
        for ( i=tknant++; i>=edtptr->curpos; i--)
          edtptr->str[i+1] = edtptr->str[i];
        edtptr->str[edtptr->curpos++] = tkn;
        if ( tknant > tknwin ) ++edtptr->scroll;
        if ( edtptr->fuse ) edtptr->fuse = FALSE;
        }
      break;
/*
***Backspace = sudda.
*/
      case SMBBACKSP:
      if ( edtptr->fuse ) edtptr->fuse = FALSE;

      if ( edtptr->curpos > 0 )
        {
        for ( i= --edtptr->curpos; i<tknant; i++)
          edtptr->str[i] = edtptr->str[i+1];
        tknant--;
        }
      else if ( tknant > 0 )
        {
        for ( i=edtptr->curpos; i<tknant; i++)
          edtptr->str[i] = edtptr->str[i+1];
        tknant--;
        }
      else
        {
        strcpy(edtptr->str,edtptr->dstr);
        tknant = strlen(edtptr->str);
        if ( tknant == 0 ) XBell(xdisp,100);
        else edtptr->fuse = TRUE;
        }
      if ( edtptr->curpos < edtptr->scroll )
        edtptr->scroll = edtptr->curpos;
      break;
/*
***Pil till v�nster.
*/
      case SMBLEFT:
      if ( edtptr->curpos > 0 )
        {
        --edtptr->curpos;
        if ( edtptr->curpos < edtptr->scroll ) --edtptr->scroll;
        }
      else XBell(xdisp,100);
      break;
/*
***Pil till h�ger.
*/
      case SMBRIGHT:
      if ( edtptr->curpos < tknant )
        {
        ++edtptr->curpos;
        if ( edtptr->curpos > edtptr->scroll + tknwin ) ++edtptr->scroll;
        }
      else XBell(xdisp,100);
      break;
/*
***Pil upp.
*/
      case SMBUP:
      winptr = WPwgwp(edtptr->id.p_id);
      iwinpt = (WPIWIN *)winptr->ptr;
      nxtptr = WPffoc(iwinpt,PREV_EDIT);
      WPfoed(edtptr,FALSE);
      WPfoed(nxtptr,TRUE);
      if ( slevel == SLEVEL_MBS  ||
           slevel == SLEVEL_V3_INP ) servat = TRUE;
      break;
/*
***Pil ner.
*/
      case SMBDOWN:
      case SMBRETURN:
      winptr = WPwgwp(edtptr->id.p_id);
      iwinpt = (WPIWIN *)winptr->ptr;
      nxtptr = WPffoc(iwinpt,NEXT_EDIT);
      WPfoed(edtptr,FALSE);
      WPfoed(nxtptr,TRUE);
      if ( slevel == SLEVEL_MBS  ||
           slevel == SLEVEL_V3_INP ) servat = TRUE;
      break;
/*
***Huvudmenyn <TAB>, Help <CTRL>a och Pos-menyn <CTRL>b.
*/
      case SMBMAIN:
      case SMBPOSM:
      case SMBHELP:
      if ( slevel == SLEVEL_V3_INP ) servat = TRUE;
      break;
      }
/*
***Edit-f�nstrets text-str�ng �r nu editerad.
***Sudda f�nstret och g�r expose.
*/
    XClearWindow(xdisp,edtptr->id.x_id);
    WPxped(edtptr);
/*
***Om beg�rd service-niv� �r att alla key-events skall returneras
***servade (inga servas lokalt) s�tter vi servat = TRUE oavsett
***vilken tangent det handlade om.
*/
    if ( slevel == SLEVEL_ALL ) servat = TRUE;
/*
***Slut.
*/
    return(servat);
  }

/********************************************************/
/*!******************************************************/

        short WPgted(
        DBint  iwin_id,
        DBint  edit_id,
        char  *str)

/*      Get-rutin f�r WPEDIT.
 *
 *      In: iwin_id = Huvudf�nstrets id.
 *          edit_id = Edit-f�nstrets id.
 *
 *      Ut: str = Aktuell text.
 *
 *      Felkod: WP1162 = F�r�ldern %s finns ej.
 *              WP1172 = F�r�ldern %s ej WPIWIN.
 *              WP1182 = Knappen %s finns ej.
 *              WP1192 = %s �r ej en knapp.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    char    errbuf[80];
    WPWIN  *winptr;
    WPIWIN *iwinptr;
    WPEDIT *editptr;

/*
***Fixa C-pekare till f�r�lderns entry i wpwtab.
*/
    if ( (winptr=WPwgwp((wpw_id)iwin_id)) == NULL )
      {
      sprintf(errbuf,"%d",(int)iwin_id);
      return(erpush("WP1162",errbuf));
      }
/*
***Kolla att det �r ett WPIWIN.
*/
    if ( winptr->typ != TYP_IWIN )
      {
      sprintf(errbuf,"%d",(int)iwin_id);
      return(erpush("WP1172",errbuf));
      }
/*
***Fixa en C-pekare till WPIWIN.
*/
    iwinptr = (WPIWIN *)winptr->ptr;
/*
***Kolla om subf�nstret med angivet id finns och �r
***av r�tt typ.
*/
    if ( iwinptr->wintab[(wpw_id)edit_id].ptr == NULL )
      {
      sprintf(errbuf,"%d",(int)edit_id);
      return(erpush("WP1182",errbuf));
      }

    if ( iwinptr->wintab[(wpw_id)edit_id].typ != TYP_EDIT )
      {
      sprintf(errbuf,"%d",(int)edit_id);
      return(erpush("WP1192",errbuf));
      }
/*
***Fixa en C-pekare till WPEDIT.
*/
    editptr = (WPEDIT *)iwinptr->wintab[(wpw_id)edit_id].ptr;
/*
***Returnera str�ng.
*/
    strcpy(str,editptr->str);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short WPuped(
        WPEDIT *edtptr,
        char   *newstr)

/*      Byter ut texten i ett WPEDIT-f�nster.
 *
 *      In: edtptr = Pekare till WPEDIT
 *          newstr = Ny text
 *
 *      Ut: Inget.
 *
 *      Felkod: .
 *
 *      (C)microform ab 1996-12-12 J. Kjellander
 *
 ******************************************************!*/

  {
   int ntkn;

/*
***Kolla str�ngl�ngden.
*/
    ntkn = strlen(newstr);
    if ( ntkn < 0 ) ntkn = 0;
    if ( ntkn > V3STRLEN ) ntkn = V3STRLEN;
    if ( ntkn > edtptr->tknmax ) ntkn = edtptr->tknmax;
    newstr[ntkn] = '\0';
/*
***Lagra den nya texten i editen.
*/
    strcpy(edtptr->str,newstr);

    edtptr->scroll = 0;
    edtptr->curpos = 0;
    edtptr->fuse   = TRUE;
/*
***Uppdatera sk�rmen.
*/
    WPxped(edtptr);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short WPdled(
        WPEDIT *edtptr)

/*      D�dar en WPEDIT.
 *
 *      In: edttptr = C-pekare till WPEDIT.
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
    v3free((char *)edtptr,"WPdled");
    return(0);
  }

/********************************************************/
/*!******************************************************/

        WPEDIT *WPffoc(
        WPIWIN *iwinptr,
        int     code)

/*      Letar efter WPEDIT:s i ett WPIWIN och returnerar
 *      en pekare till det som efterfr�gats eller NULL om
 *      det inte finns.
 *
 *      In: iwinptr = C-pekare till WPIWIN.
 *          code    = FIRST_EDIT  => F�rsta i wintab oavsett fokus.
 *                  = NEXT_EDIT   => 1:a efter den som har fokus
 *                  = PREV_EDIT   => 1:a f�re den som har fokus
 *                  = FOCUS_EDIT  => Den som har fokus.
 *
 *      Ut: Inget.   
 *
 *      FV: Pekare till WPEDIT eller NULL.
 *
 *      (C)microform ab 14/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    short   i;
    char   *subptr;
    wpw_id  foc_id;
    WPEDIT *edtptr;

/*
***Leta igenom alla sub-f�nster efter WPEDIT.
*/
    switch ( code )
      {
/*
***F�rsta.
*/
      case FIRST_EDIT:
      for ( i=0; i<WP_IWSMAX; ++i )
        {
        subptr = iwinptr->wintab[i].ptr;
        if ( subptr != NULL  &&  iwinptr->wintab[i].typ == TYP_EDIT )
          return((WPEDIT *)subptr);
        }
      return(NULL);
/*
***Sista.
*/
      case LAST_EDIT:
      for ( i=WP_IWSMAX-1; i>=0; --i )
        {
        subptr = iwinptr->wintab[i].ptr;
        if ( subptr != NULL  &&  iwinptr->wintab[i].typ == TYP_EDIT )
          return((WPEDIT *)subptr);
        }
      return(NULL);
/*
***N�sta.
*/
      case NEXT_EDIT:
      edtptr = WPffoc(iwinptr,FOCUS_EDIT);
      if ( edtptr == NULL ) return(NULL);
      else foc_id = edtptr->id.w_id;

      for ( i=0; i<WP_IWSMAX; ++i )
        {
        subptr = iwinptr->wintab[i].ptr;
        if ( subptr != NULL  &&  iwinptr->wintab[i].typ == TYP_EDIT )
          {
          edtptr = (WPEDIT *)subptr;
          if ( edtptr->id.w_id > foc_id ) return(edtptr);
          }
        }
      edtptr = WPffoc(iwinptr,FIRST_EDIT);
      return(edtptr);
/*
***F�reg�ende.
*/
      case PREV_EDIT:
      edtptr = WPffoc(iwinptr,FOCUS_EDIT);
      if ( edtptr == NULL ) return(NULL);
      else foc_id = edtptr->id.w_id;

      for ( i=WP_IWSMAX-1; i>=0; --i )
        {
        subptr = iwinptr->wintab[i].ptr;
        if ( subptr != NULL  &&  iwinptr->wintab[i].typ == TYP_EDIT )
          {
          edtptr = (WPEDIT *)subptr;
          if ( edtptr->id.w_id < foc_id ) return(edtptr);
          }
        }
      edtptr = WPffoc(iwinptr,LAST_EDIT);
      return(edtptr);
/*
***Aktivt.
*/
      case FOCUS_EDIT:
      for ( i=0; i<WP_IWSMAX; ++i )
        {
        subptr = iwinptr->wintab[i].ptr;
        if ( subptr != NULL  &&  iwinptr->wintab[i].typ == TYP_EDIT )
          {
          edtptr = (WPEDIT *)subptr;
          if ( edtptr->focus == TRUE ) return(edtptr);
          }
        }
      return(NULL);
      }
   return(NULL);
  }

/********************************************************/
/*!******************************************************/

        short WPfoed(
        WPEDIT *edtptr,
        bool    mode)

/*      Fokus-rutin f�r WPEDIT.
 *
 *      In: edtptr = C-pekare till WPEDIT.
 *          mode   = TRUE  => Fokus p�.
 *                   FALSE => Fokus av.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 14/12/93 J. Kjellander
 *
 ******************************************************!*/

  {

/*
***Fokus P� !
*/
    if ( mode == TRUE )
      {
      XSetWindowBorder(xdisp,edtptr->id.x_id,WPgcol(WP_NOTI));
      edtptr->focus = TRUE;
      }
/*
***Fokus AV !
*/
    else                            
      {
      XSetWindowBorder(xdisp,edtptr->id.x_id,WPgcol(WP_BGND));
      edtptr->focus = FALSE;
      }
/*
***F�r att resultatet skall synas (cursorn) kr�vs nu ett expose.
*/
    WPxped(edtptr);

    return(0);
  }

/********************************************************/
/*!******************************************************/

 static short edmapk(
        XKeyEvent *keyev, 
        short     *sym,
        char      *t)

/*      �vers�tter keycode och state i ett key-event
 *      till en V3-symbol samt eventuellt ASCII-tecken.
 *
 *      In: keyev  = Pekare till key-event.
 *          sym    = Pekare till utdata.
 *          t      = Pekare till utdata.
 *
 *      Ut: *sym    = Motsvarande V3-symbol, tex. SMBUP.
 *          *t      = ASCII-kod om symbol = SMBCHAR.
 * 
 *      Fv:  0.
 *
 *      (C)microform ab 10/12/93 J. Kjellander
 *
 *      1998-04-21 8-Bitars ASCII, J.Kjellander
 *      1998-04-21 8-Bitars ASCII, R.Svedin
 *
 ******************************************************!*/

  {
    char            tknbuf[11],tkn='\0';
    short           symbol;
    bool            numlock = FALSE;
    bool            shift   = FALSE;
    KeySym          keysym;
    XComposeStatus  costat;
    int             ntkn = 0;

/*
***Numlock = Mod2Mask har det visat
***sig under ODT, detta �r inte n�dv�ndigtvis standard.
*/
    if ( ((keyev->state & Mod2Mask)  > 0) ) numlock = TRUE;
/*
***Var shift-tangenten nere ?
*/
    if ( ((keyev->state & ShiftMask)  > 0) ) shift = TRUE;
/*
***Vi b�rjar med att anv�nda LookupString f�r att ta reda p�
***vilken keysym det var. Vi kunde anv�nda LookupKeysym() men
***LookupString() tar h�nsyn till shift, numlock etc. �t oss
***p� ett b�ttre s�tt (h�rdvaruoberoende).
*/
    ntkn = XLookupString(keyev,tknbuf,10,&keysym,&costat);
/*
***Vissa symboler skall mappas p� ett f�r V3 speciellt s�tt.
*/
    symbol = SMBNONE;

    switch ( keysym )
      {
/*
***�, � och �.
*/
      case XK_aring:      symbol = SMBCHAR; tkn = '�' ; break;
      case XK_Aring:      symbol = SMBCHAR; tkn = '�' ; break;
      case XK_adiaeresis: symbol = SMBCHAR; tkn = '�' ; break;
      case XK_Adiaeresis: symbol = SMBCHAR; tkn = '�' ; break;
      case XK_odiaeresis: symbol = SMBCHAR; tkn = '�' ; break;
      case XK_Odiaeresis: symbol = SMBCHAR; tkn = '�' ; break;
/*
***Backspace och return (samt enter).
*/
      case XK_BackSpace:  symbol = SMBBACKSP; break;
      case XK_Return:     symbol = SMBRETURN; break;
/*
***Delete-tangenten skall i shiftad version mappas
***till '.' Med normal shift mappas den till KP_Separator
***och med numlock inte alls.
*/
      case XK_Delete:
      if ( numlock == TRUE )
        {
        symbol = SMBCHAR;
        tkn = '.';
        }
      else symbol = SMBBACKSP; 
      break;
/*
***Keypad-separator skall mappas till '.'
*/
      case XK_KP_Separator: symbol = SMBCHAR; tkn = '.'; break;
/*
***Piltangenter.
*/
      case XK_Up:     symbol = SMBUP;    break;
      case XK_Down:   symbol = SMBDOWN;  break;
      case XK_Left:   symbol = SMBLEFT;  break;
      case XK_Right:  symbol = SMBRIGHT; break;
/*
***�vriga tangenter anv�nder vi LookupString():s mappning.
***Vissa ASCII-koder under 32 �r till�tna.
*/
      default:
      if ( ntkn == 1 )
        {
        switch ( tkn = tknbuf[0] )
          {
          case 13:
          case 10:
          symbol = SMBRETURN;
          break;

          case 8:
          symbol = SMBBACKSP;
          break;

          default:
          if ( tkn > 31 )                   symbol = SMBCHAR;
          else if ( tkn == *smbind[7].str ) symbol = SMBMAIN;
          else if ( tkn == *smbind[8].str ) symbol = SMBHELP;
          else if ( tkn == *smbind[9].str ) symbol = SMBPOSM;
          break;
          }
        }
      }
/*
***Slut.
*/
    *sym = symbol;
    *t   = tkn;

    return(0);
  }

/********************************************************/
