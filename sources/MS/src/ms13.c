/**********************************************************************
*
*    ms13.c
*    ======
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     msmced();   Create WPEDIT in MBS
*     mswced();   Create WPEDIT
*     msgted();   Get WPEDIT
*     msdled();   Delete WPEDIT
*     msfoed();   Set focus on WPEDIT
*     msffoc();   Find edit with focus
*     msgedp();   HWND to C-ptr
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

LRESULT CALLBACK mscbed(HWND,UINT,WPARAM,LPARAM); /* Callback f�r WPEDIT */

/*!******************************************************/

        int     msmced(pid,x,y,dx,dy,bw,str,ntkn,eid)
        wpw_id  pid;
        int     x,y,dx,dy,bw;
        char   *str;
        int     ntkn;
        v2int  *eid;

/*      Skapar WPEDIT-f�nster.
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
 *      (C)microform ab 26/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
    char    errbuf[80];
    int     i;
    WPWIN  *winptr;
    WPIWIN *iwinptr;
    WPEDIT *edtptr;

/*
***Fixa C-pekare till f�r�lderns entry i wpwtab.
*/
    if ( (winptr=mswgwp(pid)) == NULL )
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
    mswced(iwinptr->id.ms_id,x,y,dx,dy,bw,str,ntkn,&edtptr);
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
    ShowWindow(edtptr->id.ms_id,SW_SHOWNORMAL);
/*
***Skicka defaulttexten till WIN32-f�nstret.
*/
    SendMessage(edtptr->id.ms_id,WM_SETTEXT,(WPARAM)0,(LPARAM)edtptr->dstr);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        int      mswced(ms_pid,x,y,dx,dy,bw,str,ntkn,outptr)
        HWND     ms_pid;
        int      x,y,dx,dy,bw;
        char    *str;
        int      ntkn;
        WPEDIT **outptr;

/*      Skapar WPEDIT-f�nster.
 *
 *      In: ms_pid = F�r�ldra f�nstrets MS-id.
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
 *      (C)microform ab 26/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
   DWORD   style;
   HWND    win32_id;
   WPEDIT *edtptr;

/*
***F�nstrets "style".
*/
   style = WS_CHILD   | WS_BORDER   | ES_AUTOHSCROLL |
           WS_VISIBLE | WS_TABSTOP  | WS_GROUP;
/*
***Skapa f�nstret.
*/
   win32_id = CreateWindow("EDIT",
                           "",
   		                   style,
		                   x,
		      	           y,
		  	               dx,
		  	               dy,
		  	               ms_pid,
		  	               NULL,
			               ms_inst,
			               NULL);
/*
***Skapa en WPEDIT.
*/
    if ( (edtptr=(WPEDIT *)v3mall(sizeof(WPEDIT),"mswced"))
                                                  == NULL ) return(-2);

    edtptr->id.w_id  = NULL;
    edtptr->id.p_id  = NULL;
    edtptr->id.ms_id = win32_id;

    edtptr->geo.x  =  x;
    edtptr->geo.y  =  y;
    edtptr->geo.dx =  dx;
    edtptr->geo.dy =  dy;

    if ( ntkn < 0 ) ntkn = 0;
    if ( ntkn > V3STRLEN ) ntkn = V3STRLEN;
    if ( strlen(str) > (unsigned int)ntkn ) str[ntkn] = '\0';
    strcpy(edtptr->str,str);
    strcpy(edtptr->dstr,str);
    edtptr->tknmax = ntkn;

    edtptr->scroll = 0;
    edtptr->curpos = 0;
    edtptr->focus  = FALSE;
/*
***Om default text finns s�tter vi default-s�kringen.
*/
    if ( strlen(edtptr->dstr) > 0 ) edtptr->fuse = TRUE;
    else                            edtptr->fuse = FALSE;
/*
***Instance subclassing.
*/
    edtptr->ms_cbk = (WNDPROC)SetWindowLong(win32_id,GWL_WNDPROC,(LONG)mscbed);

   *outptr = edtptr;

    return(0);
  }

/********************************************************/
/*!*******************************************************/

LRESULT CALLBACK mscbed(HWND win32_id,UINT message,WPARAM wp,LPARAM lp)
       

/*      Callbackrutin f�r WPEDIT-f�nster.
 *
 *      In: win32_id = F�nster som det h�nt n�got i.
 *          message  = Vad som h�nt.
 *          wp,lp    = Ytterligare data.
 *
 *      Ut: Inget.   
 *
 *      (C)microform ab 26/10/95 J. Kjellander
 *
 *      1996-11-26 TAB & fuse, J.Kjellander
 *      1996-12-09 CR, J.Kjellander
 *      1997-02-06 Bug currid, J.Kjellander
 *
 ******************************************************!*/

  {	
  wpw_id  currid,nextid;
  WPEDIT *edtptr;
  WPBUTT *butptr;
  WPWIN  *winptr;
  WPIWIN *iwinpt;
  WPARAM  wp_butt;
  LPARAM  lp_butt;

/*
***Fixa C-pekare till WPEDIT och WPIWIN.
*/
  edtptr = msgedp(win32_id);
  winptr = mswgwp(edtptr->id.p_id);
  iwinpt = (WPIWIN *)winptr->ptr;
/*
***Tecknen CR och TAB processas h�r.
*/
  switch ( message )
    {
    case WM_CHAR:
/*
***Om det �r CR och det finns en defaultknapp skickar vi
***ett meddelande till IWIN-f�nstret om att man klickat
***i knappen.
*/
    if      ( wp == '\r' )
      {
      if ( (butptr=msdefb(iwinpt->id.w_id)) != NULL )
        {
        wp_butt = MAKEWPARAM(0,BN_CLICKED);
        lp_butt = (LPARAM)butptr->id.ms_id;
        SendMessage(iwinpt->id.ms_id,WM_COMMAND,wp_butt,lp_butt);
        return(0);
        }
      }
/*
***Om det �r TAB flyttar vi fokus till n�sta edit eller knapp.
*/
    else if ( wp == '\t' )
	  {
      currid = msffoc(iwinpt,FOCUS_EDBU);
      nextid = msffoc(iwinpt,NEXT_EDBU);
	  if ( currid != -1 ) msfoeb(iwinpt,currid,FALSE);
	  if ( nextid != -1 ) msfoeb(iwinpt,nextid,TRUE);
	  return(0);
	  }
/*
***Alla andra tecken utl�ser defaulttexts�kringen.
*/
	else
	  edtptr->fuse = FALSE;
    break;
/*
***Fokus p�. if ( currid != -1 ) tillagt 1997-02-06 JK
*/
    case WM_SETFOCUS:
    currid = msffoc(iwinpt,FOCUS_EDBU);
    if ( currid != -1 ) msfoeb(iwinpt,currid,FALSE);
    if ( edtptr->fuse ) PostMessage(edtptr->id.ms_id,EM_SETSEL,0,-1);
    edtptr->focus = TRUE;
    if ( (butptr=msdefb(iwinpt->id.w_id)) != NULL )
      SendMessage(butptr->id.ms_id,BM_SETSTYLE,BS_DEFPUSHBUTTON,TRUE);
    iwinpt->focus_id = edtptr->id.w_id;
    break;
/*
***Fokus av.
*/
    case WM_KILLFOCUS:
    edtptr->focus = FALSE;
    break;
/*
***F�r att f� CR och TAB.
*/
    case WM_GETDLGCODE:
    return(DLGC_WANTALLKEYS);
    }
/*
***EDIT-klassens ursprungliga callback-rutin.
*/
  return(CallWindowProc(edtptr->ms_cbk,win32_id,message,wp,lp));
  }

/********************************************************/
/*!******************************************************/

        int    msgted(iwin_id,edit_id,str)
        v2int  iwin_id;
        v2int  edit_id;
        char  *str;

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
 *      (C)microform ab 26/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
    char    errbuf[80];
    int     ntkn;
    WPWIN  *winptr;
    WPIWIN *iwinptr;
    WPEDIT *editptr;

/*
***Fixa C-pekare till f�r�lderns entry i wpwtab.
*/
    if ( (winptr=mswgwp((wpw_id)iwin_id)) == NULL )
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
***H�mta texten fr�n editf�nstret.
*/
    ntkn = SendMessage(editptr->id.ms_id,
                       WM_GETTEXT,
                      (WPARAM)V3STRLEN,
                      (LPARAM)str);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        int     msdled(edtptr)
        WPEDIT *edtptr;

/*      D�dar en WPEDIT.
 *
 *      In: edttptr = C-pekare till WPEDIT.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 26/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
/*
***L�mna tillbaks dynamiskt allokerat minne.
*/
    v3free((char *)edtptr,"msdled");
    return(0);
  }

/********************************************************/
/*!******************************************************/

        wpw_id  msffoc(iwinptr,code)
        WPIWIN *iwinptr;
        int     code;

/*      Letar efter subf�nster i ett WPIWIN och returnerar
 *      en pekare till det som efterfr�gats eller NULL om
 *      det inte finns.
 *
 *      In: iwinptr = C-pekare till WPIWIN.
 *          code    = FIRST_EDBU  => F�rsta i wintab oavsett fokus.
 *                  = NEXT_EDBU   => 1:a efter den som har fokus
 *                  = PREV_EDBU   => 1:a f�re den som har fokus
 *                  = FOCUS_EDBU  => Den som har fokus.
 *
 *      Ut: Inget.   
 *
 *      FV: ID f�r knapp/edit eller -1.
 *
 *      (C)microform ab 27/10/95 J. Kjellander
 *
 *      1996-12-09 Knappar, J.Kjellander
 *
 ******************************************************!*/

  {
    short   i;
    char   *subptr;
    wpw_id  foc_id;
    WPEDIT *edtptr;
    WPBUTT *butptr;

/*
***Leta igenom alla sub-f�nster efter WPEDIT och WPBUTT.
*/
    switch ( code )
      {
/*
***F�rsta EDIT. Ej Button.
*/
      case FIRST_EDIT:
      for ( i=0; i<WP_IWSMAX; ++i )
        {
        subptr = iwinptr->wintab[i].ptr;
        if ( subptr != NULL  &&  iwinptr->wintab[i].typ == TYP_EDIT ) return(i);
        }
      return(-1);
/*
***F�rsta EDIT eller BUTTON.
*/
      case FIRST_EDBU:
      for ( i=0; i<WP_IWSMAX; ++i )
        {
        subptr = iwinptr->wintab[i].ptr;
        if ( subptr != NULL )
          {
          if ( iwinptr->wintab[i].typ == TYP_EDIT ) return(i);
          if ( iwinptr->wintab[i].typ == TYP_BUTTON )
            {
            butptr = (WPBUTT *)subptr;
            if ( !butptr->label ) return(i);
            }
          }
        }
      return(-1);
/*
***Sista.
*/
      case LAST_EDBU:
      for ( i=WP_IWSMAX-1; i>=0; --i )
        {
        subptr = iwinptr->wintab[i].ptr;
        if ( subptr != NULL )
          {
          if ( iwinptr->wintab[i].typ == TYP_EDIT ) return(i);
          if ( iwinptr->wintab[i].typ == TYP_BUTTON )
            {
            butptr = (WPBUTT *)subptr;
            if ( !butptr->label ) return(i);
            }
          }
        }
      return(-1);
/*
***N�sta.
*/
      case NEXT_EDBU:
      foc_id = msffoc(iwinptr,FOCUS_EDBU);
      if ( foc_id == -1 ) return(-1);

      for ( i=0; i<WP_IWSMAX; ++i )
        {
        subptr = iwinptr->wintab[i].ptr;
        if ( subptr != NULL )
          {
          if ( iwinptr->wintab[i].typ == TYP_EDIT    &&  i > foc_id ) return(i);
          if ( iwinptr->wintab[i].typ == TYP_BUTTON  &&  i > foc_id )
            {
            butptr = (WPBUTT *)subptr;
            if ( !butptr->label ) return(i);
            }
          }
        }
      return(msffoc(iwinptr,FIRST_EDBU));
/*
***F�reg�ende.
*/
      case PREV_EDBU:
      foc_id = msffoc(iwinptr,FOCUS_EDBU);
      if ( foc_id == -1 ) return(-1);

      for ( i=WP_IWSMAX-1; i>=0; --i )
        {
        subptr = iwinptr->wintab[i].ptr;
        if ( subptr != NULL )
          {
          if ( iwinptr->wintab[i].typ == TYP_EDIT    &&  i < foc_id ) return(i);
          if ( iwinptr->wintab[i].typ == TYP_BUTTON  &&  i < foc_id )
            {
            butptr = (WPBUTT *)subptr;
            if ( !butptr->label ) return(i);
            }
          }
        }
      return(msffoc(iwinptr,LAST_EDBU));
/*
***Aktivt.
*/
      case FOCUS_EDBU:
      for ( i=0; i<WP_IWSMAX; ++i )
        {
        subptr = iwinptr->wintab[i].ptr;
        if ( subptr != NULL )
          {
          switch ( iwinptr->wintab[i].typ )
            {
            case TYP_EDIT:
            edtptr = (WPEDIT *)subptr;
            if ( edtptr->focus ) return(i);
            break;

            case TYP_BUTTON:
            butptr = (WPBUTT *)subptr;
            if ( butptr->focus ) return(i);
            break;
            }
          }
        }
      return(-1);
      }
    return(-1);
  }

/********************************************************/
/*!******************************************************/

        int     msfoeb(iwinpt,edbuid,mode)
        WPIWIN *iwinpt;
        wpw_id  edbuid;
        bool    mode;

/*      Fokus-rutin f�r WPEDIT/WPBUTT.
 *
 *      In: iwinpt = C-pekare till huvudf�nster.
 *          edbuid = ID f�r subf�nster.
 *          mode   = TRUE  => Fokus p�.
 *                   FALSE => Fokus av.
 *
 *      Ut: Inget.   
 *
 *      (C)microform ab 27/10/95 J. Kjellander
 *
 *      1996-11-26 EM_SETSEL & fuse, J. Kjellander
 *      1996-12-09 Knappar, J.Kjellander
 *
 ******************************************************!*/

  {
   char   *subptr;
   WPEDIT *edtptr;
   WPBUTT *butptr;

/*
***Vilken sorts subf�nster �r det ?
*/
   subptr = iwinpt->wintab[edbuid].ptr;

   if ( subptr != NULL )
     {
     switch ( iwinpt->wintab[edbuid].typ )
       {
/*
***EDIT. S�tt focus p� editen och selecta texten om den
***�nnu inte har �ndrats.
*/
       case TYP_EDIT:
       edtptr = (WPEDIT *)subptr;
       if ( mode == TRUE )
         {
         SetFocus(edtptr->id.ms_id);
         if ( edtptr->fuse ) SendMessage(edtptr->id.ms_id,EM_SETSEL,0,-1);
         edtptr->focus = TRUE;
         }
       else                            
         {
         edtptr->focus = FALSE;
         }
       break;
/*
***Button. S�tt fokus p� knappen.
*/
       case TYP_BUTTON:
       butptr = (WPBUTT *)subptr;
       if ( mode == TRUE )
         {
         SetFocus(butptr->id.ms_id);
         butptr->focus = TRUE;
         }
       else
         {
         butptr->focus = FALSE;
         }
       break;
       }
     }

    return(0);
  }

/********************************************************/
/*!******************************************************/

        WPEDIT *msgedp(win32_id)
        HWND win32_id;

/*      Returnerar C-pekare till edit med visst ms_id.
 *
 *      In: ms_id = WIN32 f�nster ID.
 *
 *      FV: Pekare till WPEDIT.
 *
 *      (C)microform ab 26/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
    int     i,j;
    WPIWIN *iwinpt;
    WPEDIT *edtptr;

/*
***S�k igenom hela wpwtab efter f�nster.
*/
   for ( i=0; i<WTABSIZ; ++i)
     {
     if ( wpwtab[i].ptr != NULL )
       {
       if ( wpwtab[i].typ == TYP_IWIN )
         {
         iwinpt = (WPIWIN *)wpwtab[i].ptr;
         for ( j=0; j<WP_IWSMAX; ++j )
           {
           if ( iwinpt->wintab[j].ptr != NULL )
             {
             if ( iwinpt->wintab[j].typ == TYP_EDIT ) 
               {
               edtptr = (WPEDIT *)iwinpt->wintab[j].ptr;
               if ( edtptr->id.ms_id == win32_id ) return(edtptr);
               }
             }
           }
         }
       }
     }

   return(NULL);
  }

/********************************************************/
