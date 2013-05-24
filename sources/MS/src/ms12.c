/**********************************************************************
*
*    ms12.c
*    ======
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     mswciw();   Create WPIWIN
*     msrpiw();   Repaint WPIWIN
*     msbtiw();   Button handler
*     msdliw();   Delete WPIWIN
*
*     mscrfb();   Create FBUTTON
*     mscrdb();   Create DBUTTON
*     msmcbu();   Create BUTTON
*     mswcbu();   Create button
*     msrpbu();   Repaint button
*     msbtbu();   Button handler
*     msgtbu();   Get button
*     msdlbu();   Delete button
*     msdefb();   Ptr to default button
*     msgbup();   Button id to C-ptr
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

LRESULT CALLBACK mscbbu(HWND,UINT,WPARAM,LPARAM); /* Callback f�r WPBUTT */

/*!******************************************************/

        int   mswciw(x,y,dx,dy,label,id)
        int     x,y,dx,dy;
        char   *label;
        v2int  *id;

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
 *      (C)microform ab 23/10/95 J. Kjellander
 *
 *       1997-09-24 wpl.length, J.Kjellander
 *
 ******************************************************!*/

 {
   int             i;
   WPIWIN         *iwinptr;
   HWND            win32_id;
   WINDOWPLACEMENT wpl;
 
/*
***F�nstrets placering p� sk�rmen skall vara relativt VARKON:s main.
*/
   wpl.length = sizeof(WINDOWPLACEMENT);
   GetWindowPlacement(ms_main,&wpl);

   x += wpl.rcNormalPosition.left;
   y += wpl.rcNormalPosition.top;
/*
***Skapa ett ledigt f�nster-ID.
*/
   if ( (*id=mswffi()) < 0 ) return(erpush("WP1052",label));
/*
***Skapa WIN32-f�nster.
*/
   win32_id = CreateWindowEx(WS_EX_DLGMODALFRAME,
                             VARKON_IWIN_CLASS,
                             label,
                             WS_CAPTION | WS_CLIPSIBLINGS | WS_POPUP,
	        	      	     x,
		            	     y,
		                     dx,
		  	                 dy + GetSystemMetrics(SM_CYCAPTION),
		  	                 ms_main,
		  	                 NULL,
			                 ms_inst,
			                 NULL);
/*
***F�r att ett WPIWIN inte skall skymmas av grafiska f�nster
***tycks det beh�vas ett "Bring to top". WPIWIN och WPGWIN har
***ju samma f�r�lder n�mligen ms_main.
*/
    BringWindowToTop(win32_id);
/*
***Skapa ett WPIWIN.
*/
    if ( (iwinptr=(WPIWIN *)v3mall(sizeof(WPIWIN),"mswciw")) == NULL )
      return(erpush("WP1062",label));

    iwinptr->id.w_id  = *id;
    iwinptr->id.p_id  =  NULL;
    iwinptr->id.ms_id =  win32_id;

    iwinptr->geo.x  =  x;
    iwinptr->geo.y  =  y;
    iwinptr->geo.dx =  dx;
    iwinptr->geo.dy =  dy;
/*
***�nnu inget subf�nster med fokus.
*/
    iwinptr->focus_id = -1;
/*
***Nollst�ll subf�nstertabellen.
*/
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

        bool msrpiw(iwinpt)
        WPIWIN *iwinpt;

/*      Repaintrutin f�r WPIWIN.
 *
 *      In: iwinptr = C-pekare till WPIWIN.
 *
 *      Ut: Alltid TRUE.
 *
 *
 *      (C)microform ab 23/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
    
/*
***Ev. debug.
*/
#ifdef DEBUG
    if ( dbglev(MSPAC) == 12 )
      {
      fprintf(dbgfil(MSPAC),"***Start-msrpiw***\n");
      fprintf(dbgfil(MSPAC),"ms_id=%d\n",iwinpt->id.ms_id);
      fflush(dbgfil(MSPAC));
      }
#endif
/*
***WPIWIN-f�nstret sj�lvt har inga texter etc.
***att g�ra expose p� !
*/ 


#ifdef DEBUG
    if ( dbglev(MSPAC) == 12 )
      {
      fprintf(dbgfil(MSPAC),"***Slut-msrpiw***\n\n");
      fflush(dbgfil(MSPAC));
      }
#endif

    return(FALSE);
  }

/********************************************************/
/*!******************************************************/

        bool  msbtiw(iwinptr,butmes,serv_id)
        WPIWIN  *iwinptr;
        WPMES32 *butmes;
        wpw_id  *serv_id;

/*      Button-rutin f�r WPIWIN med vidh�ngande sub-f�nster.
 *      Kollar om muspekning skett i n�got av WPIWIN-f�nstrets
 *      subf�nster och servar is�fall meddelandet.
 *
 *      In: iwinptr = C-pekare till WPIWIN.
 *          butmes  = Message.
 *          serv_id = Pekare till utdata.
 *
 *      Ut: *serv_id = ID f�r subf�nster som servat meddelandet.
 *
 *      Fv: TRUE  = Meddelandet servat.
 *          FALSE = Detta f�nster ej inblandat.
 *
 *      (C)microform ab 24/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
    short   i;
    char   *subptr;
    WPBUTT *butptr;
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
/*
***Om det �r en button kan den bara orsaka en h�ndelse
***om den �r klickbar, dvs. har ram. Om det �r en label
***returnerar msbtbu() FALSE.
*/
          case TYP_BUTTON:
          butptr = (WPBUTT *)subptr;
          if ( butmes->wh == butptr->id.ms_id )
            {
            if ( msbtbu(butptr) )
              {
             *serv_id = butptr->id.w_id;
              return(TRUE);
              }
            }
          break;

          case TYP_ICON:
          icoptr = (WPICON *)subptr;
          if ( butmes->wh == icoptr->id.ms_id )
            {
            msbtic(icoptr);
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

        int   msdliw(iwinptr)
        WPIWIN *iwinptr;

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
          case TYP_BUTTON:
          msdlbu((WPBUTT *)subptr);
          break;

          case TYP_EDIT:
          msdled((WPEDIT *)subptr);
          break;

          case TYP_ICON:
          msdlic((WPICON *)subptr);
          break;
          }
        }
      }
/*
***L�mna tillbaks dynamiskt allokerat minne.
*/
    v3free((char *)iwinptr,"msdliw");
/*
***Kanske finns n�t annat WPIWIN med ett subf�nster som
***nu vill ha fokus.
*/



    return(0);
  }

/********************************************************/
/*!******************************************************/

        int     mscrfb(pid,x,y,dx,dy,butstr,akod,anum,bid)
        wpw_id  pid;
        int     x,y,dx,dy;
        char   *butstr;
        char   *akod;
        int     anum;
        v2int  *bid;

/*      Skapar snabbvalsknapp i grafiskt f�nster, CRE_FBUTTON.
 *
 *      In: pid    = F�r�lder.
 *          x,y    = Placering.
 *          dx,dy  = Storlek.
 *          butstr = Knapptetx.
 *          akod   = Aktionskod.
 *          anum   = Aktionsnummer.
 *          bid    = Pekare till utdata.
 *
 *      Ut: *bid = Giltigt entry i f�r�lderns wintab.
 *
 *      Felkod: WP1482 = F�r�ldern %s finns ej.
 *              WP1492 = F�r�ldern %s �r av fel typ.
 *              WP1502 = F�r m�nga subf�nster i %s.
 *              WP1512 = %s �r en ok�nd aktionskod.
 *
 *      (C)microform ab 1996-05-20 J. Kjellander
 *
 ******************************************************!*/

  {
    char    errbuf[80];
    int     i,status,action;
    WPWIN  *winptr;
    WPGWIN *gwinpt;
    WPBUTT *butptr;

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
***Fixa C-pekare till f�r�lderns entry i wpwtab.
*/
    if ( (winptr=mswgwp(pid)) == NULL )
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
***Skapa knappen.
*/
    status = mswcbu(gwinpt->id.ms_id,x,y,dx,dy,1,
                    butstr,butstr,"",WP_BGND,WP_FGND,FUNC_BUTTON,*bid,&butptr);
    if ( status < 0 ) return(status);
/*
***L�nka in den i WPGWIN-f�nstret.
*/
    gwinpt->wintab[*bid].typ = TYP_BUTTON;
    gwinpt->wintab[*bid].ptr = (char *)butptr;

    butptr->id.w_id = *bid;
    butptr->id.p_id =  pid;
/*
***Aktion.
*/
    butptr->acttyp = action;
    butptr->actnum = anum;
/*
***Visa resultatet.
*/
    ShowWindow(butptr->id.ms_id,SW_SHOWNORMAL);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        int     msmcbu(pid,x,y,dx,dy,bw,str1,str2,fstr,cb,cf,bid)
        wpw_id  pid;
        int     x,y,dx,dy,bw;
        char   *str1,*str2,*fstr;
        int    cb,cf;
        v2int  *bid;

/*      CRE_BUTTON i MBS.
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
 *      (C)microform ab 24/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
    char    errbuf[80];
    short   i,status;
    WPWIN  *winptr;
    WPIWIN *iwinptr;
    WPBUTT *butptr;

/*
***Fixa C-pekare till f�r�lderns entry i wpwtab.
*/
    if ( (winptr=mswgwp(pid)) == NULL )
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
    if ( (status=mswcbu(iwinptr->id.ms_id,x,y,dx,dy,bw,
                        str1,str2,fstr,cb,cf,NORM_BUTTON,*bid,&butptr)) < 0 ) return(status);
/*
***L�nka in den i WPIWIN-f�nstret.
*/
    iwinptr->wintab[*bid].typ = TYP_BUTTON;
    iwinptr->wintab[*bid].ptr = (char *)butptr;

    butptr->id.w_id = *bid;
    butptr->id.p_id =  pid;
/*
***Visa resultatet.
*/
    ShowWindow(butptr->id.ms_id,SW_SHOWNORMAL);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        int     mscrdb(pid,x,y,dx,dy,bw,str,fstr,cb,cf,bid)
        wpw_id  pid;
        int     x,y,dx,dy,bw;
        char   *str,*fstr;
        int    cb,cf;
        v2int  *bid;

/*      CRE_DBUTTON i MBS.
 *
 *      In: pid   = F�r�lder.
 *          x     = L�ge i X-led.
 *          y     = L�ge i Y-led.   
 *          dx    = Storlek i X-led.
 *          dy    = Storlek i Y-led.
 *          bw    = Border-width.
 *          str   = Text
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
 *              WP1582 = Defaultknapp finns redan
 *
 *      (C)microform ab 1996-12-09 J. Kjellander
 *
 ******************************************************!*/

  {
    char    errbuf[80];
    short   i,status;
    WPWIN  *winptr;
    WPIWIN *iwinptr;
    WPBUTT *butptr;

/*
***Fixa C-pekare till f�r�lderns entry i wpwtab.
*/
    if ( (winptr=mswgwp(pid)) == NULL )
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
***Kolla att det inte redan finns en default-knapp i detta
***huvudf�nster.
*/
    if ( msdefb(pid) != NULL )
      {
      sprintf(errbuf,"%d",(int)pid);
      return(erpush("WP1582",errbuf));
      }
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
    if ( (status=mswcbu(iwinptr->id.ms_id,x,y,dx,dy,bw,
                        str,str,fstr,cb,cf,DEF_BUTTON,*bid,&butptr)) < 0 ) return(status);
/*
***L�nka in den i WPIWIN-f�nstret.
*/
    iwinptr->wintab[*bid].typ = TYP_BUTTON;
    iwinptr->wintab[*bid].ptr = (char *)butptr;

    butptr->id.w_id = *bid;
    butptr->id.p_id =  pid;
/*
***Visa resultatet.
*/
    ShowWindow(butptr->id.ms_id,SW_SHOWNORMAL);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        int   mswcbu(ms_pid,x,y,dx,dy,bw,str1,str2,fstr,cb,cf,mode,bid,outptr)
        HWND     ms_pid;
        int      x,y,dx,dy,bw;
        char    *str1,*str2,*fstr;
        int      cb,cf;
        int      mode;
        int      bid;
        WPBUTT **outptr;

/*      Skapar WPBUTT-f�nster.
 *
 *      In: ms_pid = F�r�ldraf�nstrets WIN32-id.
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
 *          mode   = FUNC_BUTTON, NORM_BUTTON eller DEF_BUTTON
 *          bid    = ID.
 *          outptr = Pekare till utdata.
 *
 *      Ut: *outptr = Pekare till WPBUTT.
 *
 *      Felkod: WP1102 = Fonten %s finns ej.
 *
 *      (C)microform ab 24/10/95 J. Kjellander
 *
 *      1996-05-22 Fonter, J.Kjellander
 *      1996-12-06 mode, J.Kjellander
 *
 ******************************************************!*/

  {
    int     style;
    WPBUTT *butptr;
    HWND    win32_id;

/*
***Vilken sorts knapp �r det fr�gan om ?
*/
   switch ( mode )
     {
/*
***Snabbvalsknapp i grafiskt f�nster.
*/
     case FUNC_BUTTON:
     style = WS_VISIBLE | WS_CHILD;
     if ( bw > 0 ) style = style | WS_DLGFRAME;

     win32_id = CreateWindow(VARKON_BUTT_CLASS,
                             "",
      		                 style,
           		             x,
	   	                     y,
	        	             dx,
		                     dy,
		     	             ms_pid,
		                    (HMENU)bid,
		      	             ms_inst,
		      	             NULL);
     break;
/*
***Normal MBS-knapp.
*/
     case DEF_BUTTON:
     case NORM_BUTTON:
     if ( bw == 0 )
       return(mswcbu(ms_pid,x,y,dx,dy,bw,str1,str2,fstr,cb,cf,FUNC_BUTTON,bid,outptr));
     win32_id = CreateWindow("BUTTON",
                             str1,
     		                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON ,
        		             x,
		                     y,
		                     dx,
		                     dy,
		  	                 ms_pid,
		  	                (HMENU)bid,
			                 ms_inst,
			                 NULL);
     break;
     }
/*
***Skapa en WPBUTT.
*/
   if ( (butptr=(WPBUTT *)v3mall(sizeof(WPBUTT),"mswcbu")) == NULL )
      return(erpush("WP1112",str1));

   butptr->id.w_id  = NULL;
   butptr->id.p_id  = NULL;
   butptr->id.ms_id = win32_id;

   butptr->geo.x  =  x;
   butptr->geo.y  =  y;
   butptr->geo.dx =  dx;
   butptr->geo.dy =  dy;

   butptr->color.bckgnd = cb;
   butptr->color.forgnd = cf;

   if ( strlen(str1) > 80 ) str1[80] = '\0';
   strcpy(butptr->stroff,str1);
   if ( strlen(str2) > 80 ) str2[80] = '\0';
   strcpy(butptr->stron,str2);

   butptr->status = FALSE;
/*  
***Om knappen har ram skall den ocks� kunna clickas i.
***Utan ram �r den bara en "label".
*/
   if ( bw > 0 ) butptr->label = FALSE;
   else          butptr->label = TRUE;
/*
***Font.
*/
   if ( fstr[0] == '\0' ) butptr->font = 0;
   else if ( (butptr->font=msgfnr(fstr)) < 0 )
                         return(erpush("WP1102",fstr));
/*
***�nnu kan knappen inte ha focus.
*/
   butptr->focus = FALSE;
/*
***�r det en defaultknapp noterar vi det.
*/
   if ( mode == DEF_BUTTON ) butptr->def = TRUE;
   else                      butptr->def = FALSE;
/*
***Instance subclassing och font.
*/
   if ( mode != FUNC_BUTTON )
     {
     SendMessage(win32_id,WM_SETFONT,(WPARAM)msgfnt(butptr->font),MAKELPARAM(FALSE,0));
     butptr->ms_cbk = (WNDPROC)SetWindowLong(win32_id,GWL_WNDPROC,(LONG)mscbbu);
     }
/*
***Slut.
*/
   *outptr = butptr;

    return(0);
  }

/********************************************************/
/*!*******************************************************/

LRESULT CALLBACK mscbbu(HWND win32_id,UINT message,WPARAM wp,LPARAM lp)
       

/*      Callbackrutin f�r WPBUTT-f�nster.
 *
 *      In: win32_id = F�nster som det h�nt n�got i.
 *          message  = Vad som h�nt.
 *          wp,lp    = Ytterligare data.
 *
 *      Ut: Inget.   
 *
 *      (C)microform ab 1996-12-09 J. Kjellander
 *
 *      1997-02-10 Bug currid=-1, J.Kjellander
 *
 ******************************************************!*/

  {	
  wpw_id  currid,nextid;
  WPARAM  wp_butt;
  LPARAM  lp_butt;
  WPBUTT *butptr,*defbut;
  WPWIN  *winptr;
  WPIWIN *iwinpt;

/*
***Fixa C-pekare till WPBUTT och WPIWIN. Om inte det g�r
***finns inte f�nstret i wp (tex. under skapandet) och
***d� �verl�ter vi jobbet till default-proceduren.
*/
  butptr = msgbup(win32_id);
  if ( butptr != NULL )
    {
    winptr = mswgwp(butptr->id.p_id);
    iwinpt = (WPIWIN *)winptr->ptr;
    }
  else return(DefWindowProc(win32_id,message,wp,lp));
/*
***TAB och SPACE/CR processas h�r.
*/
  switch ( message )
    {
    case WM_CHAR:
    if ( wp == '\r' )
      {
      wp_butt = (WPARAM)(BN_CLICKED << 16);
      lp_butt = (LPARAM)butptr->id.ms_id;
      SendMessage(iwinpt->id.ms_id,WM_COMMAND,wp_butt,lp_butt);
      return(0);
      }
/*
***Om det �r TAB flyttar vi fokus till n�sta edit eller knapp.
*/
    else if ( wp == '\t' )
	  {
      currid = msffoc(iwinpt,FOCUS_EDBU);
      nextid = msffoc(iwinpt,NEXT_EDBU);
	  msfoeb(iwinpt,currid,FALSE);
	  msfoeb(iwinpt,nextid,TRUE);
	  return(0);
	  }
	else break;
/*
***Fokus p�.
*/
    case WM_SETFOCUS:
    currid = msffoc(iwinpt,FOCUS_EDBU);
    if ( currid != -1 ) msfoeb(iwinpt,currid,FALSE);
    butptr->focus = TRUE;
    if ( (defbut=msdefb(iwinpt->id.w_id)) != NULL  &&
          butptr != defbut )
      SendMessage(defbut->id.ms_id,BM_SETSTYLE,BS_PUSHBUTTON,TRUE);
    SendMessage(butptr->id.ms_id,BM_SETSTYLE,BS_DEFPUSHBUTTON,TRUE);
    iwinpt->focus_id = butptr->id.w_id;
    return(0);
/*
***Fokus av.
*/
    case WM_KILLFOCUS:
    butptr->focus = FALSE;
    SendMessage(butptr->id.ms_id,BM_SETSTYLE,BS_PUSHBUTTON,TRUE);
    return(0);
/*
***F�r att f�nga TAB.
*/
    case WM_GETDLGCODE:
    return(DLGC_WANTALLKEYS);
    }
/*
***BUTTON-klassens ursprungliga callback-rutin.
*/
  return(CallWindowProc(butptr->ms_cbk,win32_id,message,wp,lp));
  }

/********************************************************/
/*!******************************************************/

        bool msrpbu(butptr)
        WPBUTT *butptr;

/*      Repaint-rutin f�r FUNC-WPBUTT, dvs. knappar
 *      skapade med VARKON:s egen f�nsterklass.
 *
 *      In: buttptr = C-pekare till WPBUTT.
 *
 *      (C)microform ab 24/10/95 J. Kjellander
 *
 *      1996-05-22 Fonter, J.Kjellander
 *
 ******************************************************!*/

  {
   int         x,y;
   char        text[81];
   TEXTMETRIC  tm;
   SIZE        size;
   HDC         dc;
   PAINTSTRUCT ps;


/*
***Knappens DC.
*/
   dc = BeginPaint(butptr->id.ms_id,&ps);
/*
***Vilken text skall f�nstret inneh�lla ?
*/
   if ( butptr->status ) strcpy(text,butptr->stron);
   else                  strcpy(text,butptr->stroff);
/*
***Knappens font.
*/
   SelectObject(dc,msgfnt(butptr->font));
/*
***Ber�kna textens l�ge s� att den hamnar mitt i f�nstret.
*/
   GetTextExtentPoint32(dc,text,strlen(text),&size);
   x = (butptr->geo.dx - size.cx)/2 - 2;

   GetTextMetrics(dc,&tm);
   y = butptr->geo.dy - tm.tmDescent + tm.tmAscent - tm.tmInternalLeading;
   y = (int)floor(y/2.0 - 1);
/*
***F�rger 7 �r ok.
*/
   SetBkColor(dc,PALETTEINDEX(7));             /* 7 = Gr� */
   SetTextColor(dc,PALETTEINDEX(0));           /* 1 = Svart */
/*
***Skriv ut.
*/
   mswstr(dc,x,y,text);
/*
***Sl�pp DC't.
*/
   EndPaint(butptr->id.ms_id,&ps);
/*
***Slut.
*/

   return(TRUE);
  }

/********************************************************/
/*!******************************************************/

        bool msbtbu(butptr)
        WPBUTT *butptr;

/*      Button-rutin f�r WPBUTT.
 *
 *      In: buttptr = C-pekare till WPBUTT.
 *
 *      Ut: Inget.   
 *
 *      (C)microform ab 24/10/95 J. Kjellander
 *
 ******************************************************!*/

  {

/*
***Muspekning i button-f�nster. Om det bara �r en
***label g�r vi ingenting utan returnerar FALSE.
*/
   if ( butptr->label ) return(FALSE);
/*
***�r det en defaultknapp r�cker det att returnera TRUE.
*/
   if ( butptr->def ) return(TRUE);
/*
***Toggla status till motsatta v�rdet.
*/
   if ( butptr->status == 0 )
     {
     butptr->status = TRUE;
     SendMessage(butptr->id.ms_id,WM_SETTEXT,(WPARAM)0,(LPARAM)butptr->stron);
     }
   else
     {
     butptr->status = FALSE;
     SendMessage(butptr->id.ms_id,WM_SETTEXT,(WPARAM)0,(LPARAM)butptr->stroff);
     }
 
   return(TRUE);
  }

/********************************************************/
/*!******************************************************/

        int    msgtbu(iwin_id,butt_id,status)
        v2int  iwin_id;
        v2int  butt_id;
        v2int *status;

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
 *      (C)microform ab 30/12/95 J. Kjellander
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
    if ( (winptr=mswgwp((wpw_id)iwin_id)) == NULL )
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

        int     msdlbu(butptr)
        WPBUTT *butptr;

/*      D�dar en WPBUTT.
 *
 *      In: buttptr = C-pekare till WPBUTT.
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
    v3free((char *)butptr,"msdlbu");
    return(0);
  }

/********************************************************/
/*!******************************************************/

        WPBUTT  *msdefb(iwin_id)
        wpw_id   iwin_id;

/*      Returnerar pekare till ett WPIWIN-f�nsters
 *      defaultknapp om s�dan finns annars NULL.
 *
 *      In: iwin_id = Huvudf�nster att leta i.
 *
 *      Ut: Inget.
 *
 *      Fv: <> NULL => Pekare till defaultknapp
 *           = NULL => Det fins ingen defaultknapp
 *
 *      (C)microform ab 1996-12-09 J. Kjellander
 *
 ******************************************************!*/

  {
    int     i;
    char   *subptr;
    WPIWIN *iwinpt;
    WPBUTT *butptr;

/*
***F�rst en pekare till WPIWIN-f�nstret.
*/
    if ( (iwinpt=(WPIWIN *)wpwtab[iwin_id].ptr) == NULL  ||
                           wpwtab[iwin_id].typ != TYP_IWIN ) return(NULL);
/*
***S�k igenom alla subf�nster.
*/
    for ( i=0; i<WP_IWSMAX; ++i )
      {
      subptr = iwinpt->wintab[i].ptr;
      if ( subptr != NULL )
        {
        if ( iwinpt->wintab[i].typ == TYP_BUTTON )
          {
          butptr = (WPBUTT *)subptr;
          if ( butptr->def ) return(butptr);
          }
        }
      }

    return(NULL);
  }

/********************************************************/
/*!******************************************************/

        WPBUTT *msgbup(win32_id)
        HWND win32_id;

/*      Returnerar C-pekare till knapp med visst ms_id.
 *
 *      In: ms_id = WIN32 f�nster ID.
 *
 *      FV: Pekare till WPBUTT.
 *
 *      (C)microform ab 1996-12-09 J. Kjellander
 *
 ******************************************************!*/

  {
    int     i,j;
    WPIWIN *iwinpt;
    WPBUTT *buttpt;

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
             if ( iwinpt->wintab[j].typ == TYP_BUTTON ) 
               {
               buttpt = (WPBUTT *)iwinpt->wintab[j].ptr;
               if ( buttpt->id.ms_id == win32_id ) return(buttpt);
               }
             }
           }
         }
       }
     }

   return(NULL);
  }

/********************************************************/
