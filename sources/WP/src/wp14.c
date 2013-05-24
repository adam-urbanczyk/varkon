/**********************************************************************
*
*    wp14.c
*    ======
*
*    This file is part of the VARKON WindowPac Library.
*    URL: http://www.varkon.com
*
*    This file includes:
*
*    wpmcic();   Create WPICON, CRE_ICON in MBS
*    wpcrfi();   CreateWPICON, CRE_FICON in MBS
*    wpwcic();   Create WPICON, wpw-version
*    wpxpic();   Expose routine for WPICON
*    wpbtic();   Button routine for WPICON
*    wpcric();   Crossing routine for WPICON
*    wpgtic();   Get routine for WPICON, GET_ICON in MBS
*    wpdlic();   Kills WPICON
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

/*!******************************************************/

        short wpmcic(
        wpw_id  pid,
        short   x,
        short   y,
        short   bw,
        char   *fnam,
        short   cb,
        short   cf,
        v2int  *iid)

/*      Skapar WPICON-f�nster och l�nkar in i ett WPIWIN.
 *
 *      In: pid   = F�r�lder.
 *          x     = L�ge i X-led.
 *          y     = L�ge i Y-led.   
 *          bw    = Ramens bredd.
 *          fnam  = Ikon-fil.
 *          cb    = Bakgrundsf�rg.
 *          cf    = F�rgrundsf�rg.
 *          iid   = Pekare till utdata.
 *
 *      Ut: *iid = Giltigt entry i f�r�lderns wintab.
 *
 *      Felkod: WP1262 = F�r�ldern %s finns ej.
 *              WP1272 = F�r�ldern %s �r ej ett WPIWIN.
 *              WP1282 = F�r m�nga subf�nster i %s.
 *
 *      (C)microform ab 13/1/94 J. Kjellander
 *
 ******************************************************!*/

  {
    char                 errbuf[80];
    short                i,status;
    WPWIN               *winptr;
    WPIWIN              *iwinptr;
    WPICON              *icoptr;

/*
***Fixa C-pekare till f�r�lderns entry i wpwtab.
*/
    if ( (winptr=wpwgwp(pid)) == NULL )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1262",errbuf));
      }
/*
***Kolla att det �r ett WPIWIN och fixa en pekare till
***f�r�lder-f�nstret sj�lvt.
*/
    if ( winptr->typ != TYP_IWIN )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1272",errbuf));
      }
    else iwinptr = (WPIWIN *)winptr->ptr;
/*
***Skapa ID f�r den nya ikonen, dvs. fixa
***en ledig plats i f�r�lderns f�nstertabell.
*/
    i = 0;
    while ( i < WP_IWSMAX  &&  iwinptr->wintab[i].ptr != NULL ) ++i;

    if ( i == WP_IWSMAX )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1282",errbuf));
      }
    else *iid = i;
/*
***Skapa ikonen.
*/
    if ( (status=wpwcic(iwinptr->id.x_id,x,y,bw,fnam,cb,cf,
                                        &icoptr)) < 0 ) return(status);
/*
***L�nka in den i WPIWIN-f�nstret.
*/
    iwinptr->wintab[*iid].typ = TYP_ICON;
    iwinptr->wintab[*iid].ptr = (char *)icoptr;

    icoptr->id.w_id = *iid;
    icoptr->id.p_id =  pid;
/*
***Om WPIWIN-f�nstret redan �r mappat skall ikonen mappas nu.
*/
    if ( iwinptr->mapped ) XMapWindow(xdisp,icoptr->id.x_id);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short wpcrfi(
        int    pid,
        short  x,
        short  y,
        char  *filnam,
        char  *akod,
        short  anum,
        v2int *iid)

/*      Skapar snabbvalsikon (CRE_FICON) i grafiskt f�nster.
 *
 *      In: pid    = ID f�r grafiskt f�nster.
 *          x,y    = Placering.
 *          filnam = V�gbeskrivning till ikonfil
 *          akod   = Aktionskod.
 *          anum   = Aktionsnummer.
 *          iid    = Pekare till resultat.
 *
 *      Ut: *iid = Ikonens ID.
 *
 *      Felkod: 
 *              WP1552 = %s �r en otill�ten aktionskod.
 *              WP1522 = F�nstret %s finns ej
 *              WP1532 = F�nstret %s �r av fel typ
 *              WP1542 = F�nster %s �r fullt
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
    WPICON  *icoptr;

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
  
      default: return(erpush("WP1552",akod));
      break;
      }
/*
***Fixa C-pekare till det grafiska f�nstrets entry i wpwtab.
*/
    if ( (winptr=wpwgwp(pid)) == NULL )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1522",errbuf));
      }
/*
***Kolla att det �r ett WPGWIN och fixa en pekare till
***f�r�lder-f�nstret sj�lvt.
*/
    if ( winptr->typ != TYP_GWIN )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1532",errbuf));
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
      return(erpush("WP1542",errbuf));
      }
    else *iid = i;
/*
***Prova att skapa en ikon.
*/
    status = wpwcic(gwinpt->id.x_id,x,y,(short)1,
                        filnam,WP_BGND,WP_FGND,&icoptr);

    if ( status < 0 ) return(status);
/*
***L�nka in den i WPGWIN-f�nstret.
*/
    gwinpt->wintab[*iid].typ = TYP_ICON;
    gwinpt->wintab[*iid].ptr = (char *)icoptr;

    icoptr->id.w_id = *iid;
    icoptr->id.p_id =  pid;

    XMapWindow(xdisp,icoptr->id.x_id);
/*
***Aktion.
*/
    icoptr->acttyp = action;
    icoptr->actnum = anum;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short wpwcic(
        Window   px_id,
        short    x,
        short    y,
        short    bw,
        char    *fnam,
        short    cb,
        short    cf,
        WPICON **outptr)

/*      Skapar WPICON-f�nster.
 *
 *      In: px_id  = F�r�ldra f�nstrets X-id.
 *          x      = L�ge i X-led.
 *          y      = L�ge i Y-led.   
 *          fnam   = Bitmap-fil.
 *          cb     = Bakgrundsf�rg.
 *          cf     = F�rgrundsf�rg.
 *          outptr = Pekare till utdata.
 *
 *      Ut: *outptr = Pekare till WPICON.
 *
 *      Felkod: WP1212 = Kan ej l�sa bitmapfilen %s
 *              WP1292 = Fel fr�n malloc()
 *
 *      (C)microform ab 13/1/94 J. Kjellander
 *
 ******************************************************!*/

  {
    XSetWindowAttributes xwina;
    unsigned long        xwinm;
    unsigned int         dx,dy;
    int                  status,x_hot,y_hot;
    Window               xwin_id;
    Pixmap               bitmap;
    WPICON              *icoptr;

/*
***Prova att l�sa bitmap fr�n fil.
*/
    status = XReadBitmapFile(xdisp,px_id,fnam,&dx,&dy,
                                       &bitmap,&x_hot,&y_hot);

    if ( status != BitmapSuccess ) return(erpush("WP1212",fnam));
/*
***Skapa f�nstret i X.
*/
    xwina.background_pixel  = wpgcol(cb);
    xwina.border_pixel      = wpgcol(WP_BGND);
    xwina.override_redirect = True;
    xwina.save_under        = False;

    xwinm = ( CWBackPixel        | CWBorderPixel |
              CWOverrideRedirect | CWSaveUnder );  

    if ( bw > 0 )
      {
      dx += 2*bw + 2;
      dy += 2*bw + 2;
      }

    xwin_id = XCreateWindow(xdisp,px_id,x,y,dx,dy,bw,
                            DefaultDepth(xdisp,xscr),
                            InputOutput,CopyFromParent,xwinm,&xwina);
/*
***Input events.
*/
    if ( bw > 0 ) XSelectInput(xdisp,xwin_id,ButtonPressMask |
                                             EnterWindowMask |
                                             LeaveWindowMask);
/*
***Skapa en WPICON.
*/
    if ( (icoptr=(WPICON *)v3mall(sizeof(WPICON),"wpwcic")) == NULL )
       return(erpush("WP1292",fnam));

    icoptr->id.w_id = (wpw_id)NULL;
    icoptr->id.p_id = (wpw_id)NULL;
    icoptr->id.x_id = xwin_id;

    icoptr->geo.x =  x;
    icoptr->geo.y =  y;
    icoptr->geo.dx =  (short)dx;
    icoptr->geo.dy =  (short)dy;
    icoptr->geo.bw =  bw;

    icoptr->color.bckgnd = cb;
    icoptr->color.forgnd = cf;

    icoptr->bitmap = bitmap;

   *outptr = icoptr;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        bool wpxpic(
        WPICON *icoptr)

/*      Expose-rutin f�r WPICON.
 *
 *      In: icoptr = C-pekare till WPICON.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 13/1/94 J. Kjellander
 *
 ******************************************************!*/

  {
    int dst;

/*
***F�rger.
*/
    if ( icoptr->color.bckgnd != WP_BGND )
      XSetBackground(xdisp,xgc,wpgcol(icoptr->color.bckgnd));
    if ( icoptr->color.forgnd != WP_FGND )
      XSetForeground(xdisp,xgc,wpgcol(icoptr->color.forgnd));
/*
***Kopiera bitmappen till f�nstret. Om f�nstret har ram
***kompenserar vi positionen f�r detta.
*/
    if ( icoptr->geo.bw > 0 )
      dst = icoptr->geo.bw + 1;
    else
      dst = 0;

    XCopyPlane(xdisp,icoptr->bitmap,icoptr->id.x_id,xgc,0,0,
                          icoptr->geo.dx,icoptr->geo.dy,
                          dst,dst,1);
/*
***Tills vidare �terst�ller vi f�rger till default igen.
*/
    if ( icoptr->color.bckgnd != WP_BGND )
      XSetBackground(xdisp,xgc,wpgcol(WP_BGND));
    if ( icoptr->color.forgnd != WP_FGND )
      XSetForeground(xdisp,xgc,wpgcol(WP_FGND));
/*
***Ev. 3D-ram.
*/
    if ( icoptr->geo.bw > 0 )
      wpd3db((char *)icoptr,TYP_ICON);
/*
***Slut.
*/
    return(TRUE);
  }

/********************************************************/
/*!******************************************************/

        bool wpbtic(
        WPICON *icoptr)

/*      Button-rutin f�r WPICON.
 *
 *      In: icoptr = C-pekare till WPICON.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 14/1/94 J. Kjellander
 *
 ******************************************************!*/

  {
    return(TRUE);
  }

/********************************************************/
/*!******************************************************/

        bool wpcric(
        WPICON         *icoptr,
        XCrossingEvent *croev)

/*      Crossing-rutin f�r WPICON.
 *
 *      In: icoptr = C-pekare till WPICON.
 *          croev  = X-crossing event.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 17/1/94 J. Kjellander
 *
 ******************************************************!*/

  {

/*
***Enter => Highligt, dvs. tjockare ram i annan f�rg.
*/
    if ( croev->type == EnterNotify )
      {
      XSetWindowBorder(xdisp,croev->window,wpgcol(WP_NOTI));
      }
/*
***Leave => Normal ram igen.
*/
    else                            
      {
      XSetWindowBorder(xdisp,croev->window,wpgcol(WP_BGND));
      }

    return(TRUE);
  }

/********************************************************/
/*!******************************************************/

        short wpgtic(
        v2int  iwin_id,
        v2int  butt_id,
        v2int *status)

/*      Get-rutin f�r WPICON. Anv�nds ej f�r n�rvarande.
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
    return(0);
  }

/********************************************************/
/*!******************************************************/

        short wpdlic(
        WPICON *icoptr)

/*      D�dar en WPICON.
 *
 *      In: icotptr = C-pekare till WPICON.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 13/1/94 J. Kjellander
 *
 ******************************************************!*/

  {

/*
***L�mna tillbaks pixmappen.
*/
    XFreePixmap(xdisp,icoptr->bitmap);
/*
***L�mna tillbaks dynamiskt allokerat minne.
*/
    v3free((char *)icoptr,"wpdlic");

    return(0);
  }

/********************************************************/
