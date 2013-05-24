/**********************************************************************
*
*    ms11.c
*    ======
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     mswini();   Inits the windowhandler
*     mswexi();   Exits the windowhandler
*     msregc();   Registers window classes
*     mswrep();   Repaint window
*     mswres();   Resize window
*     mswbut();   Button handler
*     msrpmw();   Repaint main window
*
*     mswshw();   SHOW_WIN in MBS
*     mswwtw();   WAIT_WIN in MBS
*     mswdel();   Delete main window and DEL_WIN in MBS
*     mswdls();   Delete sub window and DEL_WIN in MBS
*
*     mswffi();   Find lowest free window index
*     mswfpw();   Id of parent window
*     mswgwp();   Id to C-ptr
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
*    (C) 2000-2004, Johan Kjellander, �rebro university
*
***********************************************************************/

#include "../../../sources/DB/include/DB.h"
#include "../../../sources/IG/include/IG.h"
#include "../../../sources/WP/include/WP.h"

LRESULT CALLBACK mscbmw(HWND,UINT,WPARAM,LPARAM);  /* Callback f�r VARKON_MAIN_CLASS */
LRESULT CALLBACK mswcbk(HWND,UINT,WPARAM,LPARAM);  /* Callback f�r VARKON_IWIN_CLASS */

extern bool igbflg;

/*
******************************************************!*/

WPWIN wpwtab[WTABSIZ];

/* wpwtab �r en tabell med typ och pekare till f�nster.
   Typ �r en kod som anger vilken typ av f�nster det r�r
   sig om tex. TYP_IWIN f�r ett input-f�nster fr�n MBS.
   Pekaren �r en C-pekare som pekar p� en structure av
   den aktuella typen tex. WPIWIN f�r ett input-f�nster.

   Alla element i wpwtab initieras av mswini() till NULL.
   N�r ett nytt f�nster skapas f�r det som ID l�gsta lediga
   plats i wpwtab och n�r det deletas nollst�lls platsen
   igen.
*/

WPMES32 ms_lstmes;

/* Sist inkomna meddelande.
*/


extern char    jobnam[];
extern HPEN    ms_black,ms_darkg,ms_normg,ms_liteg,ms_white;
extern HCURSOR ms_grcu;

extern LRESULT CALLBACK mscblw(HWND,UINT,WPARAM,LPARAM);
extern LRESULT CALLBACK mscbgw(HWND,UINT,WPARAM,LPARAM);
extern LRESULT CALLBACK mscbrw(HWND,UINT,WPARAM,LPARAM);
extern LRESULT CALLBACK mscbew(HWND,UINT,WPARAM,LPARAM);



/*!******************************************************/

        int mswini()

/*      Initierar wpw-paketet. Skapar applikationens
 *      huvudf�nster.
 *
 *      In: Inget.   
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *      2004-10-14 don't show window in batch-mode
 *
 ******************************************************!*/

  {
    int   i,x,y,dx,dy;
    char  buf[20],sndpth[V3PTHLEN+1];

/*
***Nollst�ll f�nster-tabellen.
*/
   for ( i=0; i<WTABSIZ; ++i)
     {
     wpwtab[i].typ = TYP_UNDEF;
     wpwtab[i].ptr = NULL;
     }
/*
***Registrera f�nsterklasser.
*/
   if ( !msregc() ) return((int)GetLastError());
/*
***Skapa huvudf�nstret.
*/
   x  = CW_USEDEFAULT;
   y  = CW_USEDEFAULT;
   dx = CW_USEDEFAULT;
   dy = CW_USEDEFAULT;

   if ( msgrst("APPWIN.X",buf) )  sscanf(buf,"%d",&x);
   if ( msgrst("APPWIN.Y",buf) )  sscanf(buf,"%d",&y);
   if ( msgrst("APPWIN.DX",buf) ) sscanf(buf,"%d",&dx);
   if ( msgrst("APPWIN.DY",buf) ) sscanf(buf,"%d",&dy);

   ms_main = CreateWindowEx(0,
                            VARKON_MAIN_CLASS,
                            "VARKON",
   		       	   		    WS_OVERLAPPEDWINDOW,
		  		   		    x,y,dx,dy,
		  			        NULL,
		  	   	            NULL,
						    ms_inst,
						    NULL);

   if ( ms_main == NULL ) return((int)GetLastError());

   if (!igbflg) {
      ShowWindow(ms_main,SW_SHOWNORMAL);
      }
/*
***Default cursor i grafiska f�nster = pil.
*/
   ms_grcu = LoadCursor(NULL,IDC_ARROW);
/*
***Ett v�lkomnande ljud.
*/
   sprintf(sndpth,"%sV3_start.wav",v3genv(VARKON_SND));
   sndPlaySound(sndpth,SND_SYNC);

   return(0);
  }

/********************************************************/
/*!******************************************************/

        int mswexi()

/*      Avslutar wpw-paketet.
 *
 *      In: Inget.   
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    char  sndpth[V3PTHLEN+1];
    short i;

/*
***Ev. debug.
*/
#ifdef DEBUG
    if ( dbglev(MSPAC) == 1 )
      {
      fprintf(dbgfil(MSPAC),"***Start-mswexi***\n");
      fflush(dbgfil(MSPAC));
      }
#endif
/*
***D�da alla f�nster i f�nster-tabellen.
*/
   for ( i=0; i<WTABSIZ; ++i)
     if ( wpwtab[i].ptr != NULL ) mswdel((v2int)i);
/*
***Ett avslutande ljud.
*/
   sprintf(sndpth,"%sV3_end.wav",v3genv(VARKON_SND));
   sndPlaySound(sndpth,SND_SYNC);


#ifdef DEBUG
    if ( dbglev(MSPAC) == 1 )
      {
      fprintf(dbgfil(MSPAC),"***Slut-mswexi***\n");
      fflush(dbgfil(MSPAC));
      }
#endif

    return(0);
  }

/********************************************************/
/*!******************************************************/

        bool msregc()

/*      Registrerar dom f�nsterklasser som V3
*       anv�nder sig av.
 *
 *      In: Inget.   
 *
 *      Ut: Inget.   
 *
 *      FV: TRUE = Ok, FALSE = Fel fr�n RegisterClass().
 *
 *      (C)microform ab 30/11/95 J. Kjellander
 *
 *      1998-01-13 WPRWIN, J.Kjellander
 *
 ******************************************************!*/

  {
    WNDCLASS  class;

/*
***B�rja med att registrera huvudf�nstrets (applikationens) klass.
***Det �r bara huvudf�nstret (det enda) som har denna klass med
***egen vidh�ngande callback.
*/
   class.hInstance     = ms_inst;
   class.lpszClassName = VARKON_MAIN_CLASS;
   class.lpfnWndProc   = mscbmw;
   class.style         = 0;
   class.hIcon         = LoadIcon(ms_inst,(LPCTSTR)IDI_ICON1);
   class.hCursor       = LoadCursor(NULL,IDC_ARROW);
   class.lpszMenuName  = 0;
   class.cbClsExtra    = 0;
   class.cbWndExtra    = 0;
   class.hbrBackground = GetStockObject(LTGRAY_BRUSH);

   if ( RegisterClass(&class) == 0 ) return(FALSE);
/*
***Sen en klass f�r WPIWIN, WPBUTT och WPEDIT.
*/
   class.hInstance     = ms_inst;
   class.lpszClassName = VARKON_IWIN_CLASS;
   class.lpfnWndProc   = mswcbk;
   class.style         = 0;
   class.hIcon         = LoadIcon(NULL,(LPCTSTR)IDI_APPLICATION);
   class.hCursor       = LoadCursor(NULL,IDC_ARROW);
   class.lpszMenuName  = 0;
   class.cbClsExtra    = 0;
   class.cbWndExtra    = 0;
   class.hbrBackground = GetStockObject(LTGRAY_BRUSH);

   if ( RegisterClass(&class) == 0 ) return(FALSE);
/*
***Registrera ocks� klasser f�r grafiska f�nster. Grafiska
***f�nster har annan bakgrundsf�rg och egen callback. Ingen cursor
***eftersom detta skall sk�tas av v3. Huvudf�nstret kan inte d�das.
*/
   class.hInstance     = ms_inst;
   class.lpszClassName = VARKON_MGWIN_CLASS;
   class.lpfnWndProc   = mscbgw;
   class.style         = CS_OWNDC | CS_NOCLOSE;
   class.hIcon         = LoadIcon(NULL,(LPCTSTR)IDI_APPLICATION);
   class.hCursor       = NULL;
   class.lpszMenuName  = NULL;
   class.cbClsExtra    = 0;
   class.cbWndExtra    = 0;
   class.hbrBackground = GetStockObject(WHITE_BRUSH);

   if ( RegisterClass(&class) == 0 ) return(FALSE);

   class.hInstance     = ms_inst;
   class.lpszClassName = VARKON_GWIN_CLASS;
   class.lpfnWndProc   = mscbgw;
   class.style         = CS_OWNDC;
   class.hIcon         = LoadIcon(NULL,(LPCTSTR)IDI_APPLICATION);
   class.hCursor       = NULL;
   class.lpszMenuName  = NULL;
   class.cbClsExtra    = 0;
   class.cbWndExtra    = 0;
   class.hbrBackground = GetStockObject(WHITE_BRUSH);

   if ( RegisterClass(&class) == 0 ) return(FALSE);
   class.hInstance     = ms_inst;
   class.lpszClassName = VARKON_RWIN_CLASS;
   class.lpfnWndProc   = mscbrw;
   class.style         = CS_OWNDC;
   class.hIcon         = LoadIcon(NULL,(LPCTSTR)IDI_APPLICATION);
   class.hCursor       = NULL;
   class.lpszMenuName  = NULL;
   class.cbClsExtra    = 0;
   class.cbWndExtra    = 0;
   class.hbrBackground = GetStockObject(WHITE_BRUSH);

   if ( RegisterClass(&class) == 0 ) return(FALSE);
/*
***Samt en klass f�r listf�nster som ocks� har en egen callback.
*/
   class.hInstance     = ms_inst;
   class.lpszClassName = VARKON_LWIN_CLASS;
   class.lpfnWndProc   = mscblw;
   class.style         = 0;
   class.hIcon         = LoadIcon(ms_inst,(LPCTSTR)IDI_ICON1);
   class.hCursor       = LoadCursor(NULL,IDC_ARROW);
   class.lpszMenuName  = 0;
   class.cbClsExtra    = 0;
   class.cbWndExtra    = 0;
   class.hbrBackground = GetStockObject(WHITE_BRUSH);

   if ( RegisterClass(&class) == 0 ) return(FALSE);
/*
***Samt en klass f�r editf�nster med egen callback.
*/
   class.hInstance     = ms_inst;
   class.lpszClassName = VARKON_EWIN_CLASS;
   class.lpfnWndProc   = mscbew;
   class.style         = 0;
   class.hIcon         = LoadIcon(ms_inst,(LPCTSTR)IDI_ICON1);
   class.hCursor       = LoadCursor(NULL,IDC_ARROW);
   class.lpszMenuName  = 0;
   class.cbClsExtra    = 0;
   class.cbWndExtra    = 0;
   class.hbrBackground = GetStockObject(LTGRAY_BRUSH);

   if ( RegisterClass(&class) == 0 ) return(FALSE);
/*
***Slut.
*/
   return(TRUE);
  }

/********************************************************/
/*!*******************************************************/

LRESULT CALLBACK mscbmw(HWND win32_id,UINT message,WPARAM wp,LPARAM lp)
       

/*      Callbackrutin f�r VARKON:s huvudf�nster.
 *
 *      In: win32_id = F�nster som det h�nt n�got i (ms_main).
 *          message  = Vad som h�nt.
 *          wp,lp    = Ytterligare data.
 *
 *      Ut: Inget.   
 *
 *      (C)microform ab 30/11/95 J. Kjellander
 *
 *      1997-05-16 D�da under uppstart, J.Kjellander
 *
 ******************************************************!*/

  {
   char title[V3STRLEN],lagra[V3STRLEN];

/*
***Huvudf�nstrets menyer genererar WM_COMMAND som servas
***annorst�des. D�rf�r servar vi WM_COMMAND bara genom att
***spara dom som alla andra meddelanden i ms_lstmes.
***�vriga meddelanden som h�nf�r sig
***till huvudf�nstret servar vi h�r.
*/
   ms_lstmes.wh  = win32_id;
   ms_lstmes.msg = message;
   ms_lstmes.wp  = wp;
   ms_lstmes.lp  = lp;

   switch ( ms_lstmes.msg )
	   {
      case WM_PAINT:
	  msrpmw();
	  break;

      case WM_SIZE:
      RedrawWindow(win32_id,NULL,NULL,RDW_INVALIDATE | RDW_ERASE | RDW_INTERNALPAINT);
	  break;
/*
***D�da-knappen. Om jobnam = "" befinner vi oss i uppstart-l�ge
***och avslutar utan att lagra osv. 
*/
      case WM_CLOSE:
      if ( *jobnam == '\0' )
        {
        ExitProcess(0);
        }
      else
        {
        if ( !msgrst("EXIT.TITLE",title) ) strcpy(title,"Sluta VARKON");
        if ( !msgrst("EXIT.TEXT",lagra) ) strcpy(lagra,"Vill du lagra f�rst ?");

        switch ( MessageBox(ms_main,lagra,
                            title,MB_YESNOCANCEL | MB_ICONQUESTION) )
          {
          case IDYES:
          igexsa();
          v3exit();
          break;

          case IDNO:
          igexsn();
          v3exit();
          break;

          default:
          return(0);
          break;
          }
        break;
        }

	   default:
	   return DefWindowProc(win32_id,message,wp,lp);
	   }

   return(0);
  }

/********************************************************/
/*!******************************************************/

        bool mswrep(win32_id)
        HWND win32_id;

/*      Repaint-rutinen f�r wpw-f�nstren. Letar upp 
 *      r�tt f�nster och anropar dess repaint-rutin.
 *
 *      In: ms_id = WIN32 f�nster ID.
 *
 *      Ut: TRUE  = Eventet servat.   
 *          FALSE = Eventet ej i n�got wpw-f�nster.
 *
 *      (C)microform ab 23/10/95 J. Kjellander
 *
 *      1998-01-13 WPRWIN, J.Kjellander
 *
 ******************************************************!*/

  {
    int     i,j;
    WPIWIN *iwinpt;
    WPGWIN *gwinpt;
    WPBUTT *buttpt;
    WPICON *icoptr;
    WPRWIN *rwinpt;

#ifdef DEBUG
    if ( dbglev(MSPAC) == 11 )
      {
      fprintf(dbgfil(MSPAC),"***Start-mswrep***\n");
	  fprintf(dbgfil(MSPAC),"win32_id=%d\n",win32_id);
      fflush(dbgfil(MSPAC));
      }
#endif

/*
***S�k igenom hela wpwtab efter f�nster.
*/
    for ( i=0; i<WTABSIZ; ++i)
      {
      if ( wpwtab[i].ptr != NULL )
        {
/*
***Vilken typ av f�nster �r det ?
*/
        switch ( wpwtab[i].typ )
          {
/*
***WPIWIN-f�nster. Kolla f�nstret sj�lvt och 
***s�k igenom alla sub-f�nster.
*/
          case TYP_IWIN:
          iwinpt = (WPIWIN *)wpwtab[i].ptr;
          if ( iwinpt->id.ms_id == win32_id ) return(msrpiw(iwinpt));

          for ( j=0; j<WP_IWSMAX; ++j )
            {
            if ( iwinpt->wintab[j].ptr != NULL )
              {
              switch ( iwinpt->wintab[j].typ ) 
                {
                case TYP_BUTTON:
                buttpt = (WPBUTT *)iwinpt->wintab[j].ptr;
                if ( buttpt->id.ms_id == win32_id )
                  {
                  msrpbu(buttpt);
                  return(TRUE);
                  }
                break;

                case TYP_ICON:
                icoptr = (WPICON *)iwinpt->wintab[j].ptr;
                if ( icoptr->id.ms_id == win32_id )
                  {
                  msrpic(icoptr);
                  return(TRUE);
                  }
                break;
                }
              }
            }
          break;
/*
***Grafiskt f�nster.
*/
          case TYP_GWIN:
          gwinpt = (WPGWIN *)wpwtab[i].ptr;
          if ( gwinpt->id.ms_id == win32_id )return(msrpgw(gwinpt));

          for ( j=0; j<WP_GWSMAX; ++j )
            {
            if ( gwinpt->wintab[j].ptr != NULL )
              {
              switch ( gwinpt->wintab[j].typ ) 
                {
                case TYP_BUTTON:
                buttpt = (WPBUTT *)gwinpt->wintab[j].ptr;
                if ( buttpt->id.ms_id == win32_id )
                  {
                  msrpbu(buttpt);
                  return(TRUE);
                  }
                break;

                case TYP_ICON:
                icoptr = (WPICON *)gwinpt->wintab[j].ptr;
                if ( icoptr->id.ms_id == win32_id )
                  {
                  msrpic(icoptr);
                  return(TRUE);
                  }
                break;
                }
              }
            }
          break;
/*
***Renderingsf�nster.
*/
          case TYP_RWIN:
          rwinpt = (WPRWIN *)wpwtab[i].ptr;
          if ( rwinpt->id.ms_id == win32_id )return(msrprw(rwinpt));

          for ( j=0; j<WP_RWSMAX; ++j )
            {
            if ( rwinpt->wintab[j].ptr != NULL )
              {
              switch ( rwinpt->wintab[j].typ ) 
                {
                case TYP_BUTTON:
                buttpt = (WPBUTT *)rwinpt->wintab[j].ptr;
                if ( buttpt->id.ms_id == win32_id )
                  {
                  msrpbu(buttpt);
                  return(TRUE);
                  }
                break;

                case TYP_ICON:
                icoptr = (WPICON *)rwinpt->wintab[j].ptr;
                if ( icoptr->id.ms_id == win32_id )
                  {
                  msrpic(icoptr);
                  return(TRUE);
                  }
                break;
                }
              }
            }
          break;
          }
        }
     }
/*
***Det kan vara medelanderaden det g�ller.
*/
   if ( msrpma(win32_id) ) return(TRUE);
/*
***Ingen tr�ff.
*/
#ifdef DEBUG
    if ( dbglev(MSPAC) == 11 )
      {  
      fprintf(dbgfil(MSPAC),"status = FALSE\n");
      fprintf(dbgfil(MSPAC),"***Slut-mswrep***\n");
      fflush(dbgfil(MSPAC));
      }
#endif

   return(FALSE);
  }

/********************************************************/
/*!******************************************************/

        bool mswres(win32_id)
        HWND win32_id;

/*      Resize-rutinen f�r wpw-f�nstren. Letar upp 
 *      r�tt f�nster och anropar dess resize-rutin.
 *
 *      In: ms_id = WIN32 f�nster ID.
 *
 *      Ut: TRUE  = Resize utf�rd.
 *          FALSE = Inget av wp:s f�nster.
 *
 *      Felkod: .
 *
 *      (C)microform ab 15/11/95 J. Kjellander
 *
 ******************************************************!*/

  {
    int     i;
    WPLWIN *lwinpt;
    WPGWIN *gwinpt;

/*
***S�k igenom hela wpwtab efter f�nster.
*/
    for ( i=0; i<WTABSIZ; ++i)
      {
      if ( wpwtab[i].ptr != NULL )
        {
/*
***Vilken typ av f�nster �r det ?
*/
        switch ( wpwtab[i].typ )
          {
/*
***WPLWIN-f�nster.
*/
          case TYP_LWIN:
          lwinpt = (WPLWIN *)wpwtab[i].ptr;
/*          if ( lwinpt->id.ms_id == win32_id ) return(msrslw(lwinpt)); */
          break;
/*
***Grafiskt f�nster.
*/
          case TYP_GWIN:
          gwinpt = (WPGWIN *)wpwtab[i].ptr;
/*          if ( gwinpt->id.ms_id == win32_id )return(msrpgw(gwinpt)); */
          break;
          }
        }
     }
/*
***Ingen tr�ff.
*/
   return(FALSE);
  }

/********************************************************/
/*!******************************************************/

        bool mswbut(butmes,serv_id)
        WPMES32 *butmes;
        wpw_id  *serv_id;

/*      Button-rutinen f�r wpw-f�nstren. Kollar
 *      vilken typ av f�nster det �r och anropar
 *      r�tt rutin f�r jobbet.
 *
 *      In: butmes   = Message.
 *          serv_id  = Pekare till utdata.
 *
 *      Ut: *serv_id = ID f�r subf�nster som servat meddelandet.
 *
 *      FV. TRUE  = Meddelandet har servats.
 *          FALSE = Meddelandetg�llde inga av dessa f�nster.
 *
 *      (C)microform ab 24/10/95 J. Kjellander
 *
 *       1998-01-14 WPRWIN, J.Kjellander
 *
 ******************************************************!*/

  {
    short   i;
    WPWIN  *winptr;
    WPIWIN *iwinpt;
    WPGWIN *gwinpt;
    WPRWIN *rwinpt;

/*
***S�k igenom wpwtab och anropa alla f�nstrens
***respektive butt-hanterare. Den som vill k�nnas vid
***eventet tar hand om det.
*/
    for ( i=0; i<WTABSIZ; ++i )
      {
      if ( (winptr=mswgwp((wpw_id)i)) != NULL )
        {
        switch ( winptr->typ )
          {
          case TYP_IWIN:
          iwinpt = (WPIWIN *)winptr->ptr;
          if ( msbtiw(iwinpt,butmes,serv_id) ) return(TRUE);
          break;

          case TYP_GWIN:
          gwinpt = (WPGWIN *)winptr->ptr;
          if ( msbtgw(gwinpt,butmes,serv_id) ) return(TRUE);
          break;

          case TYP_RWIN:
          rwinpt = (WPRWIN *)winptr->ptr;
          if ( msbtrw(rwinpt,butmes,serv_id) ) return(TRUE);
          break;
          }
        }
      }

    return(FALSE);
  }

/********************************************************/
/*!******************************************************/

        void msrpmw()

/*      Repaint-rutin f�r V3:s huvudf�nster.
 *
 *      In: Inget.
 *
 *      Ut: Inget.   
 *
 *      (C)microform ab 1996-03-13 J. Kjellander
 *
 ******************************************************!*/

  {
   int         pdy;
   POINT       p[4];
   RECT        rect;
   HDC         dc;
   PAINTSTRUCT ps;

/*
***Fixa ett DC.
*/
   dc = BeginPaint(ms_main,&ps); 
/*
***Hur stort �r f�nstret ?
*/
   GetClientRect(ms_main,&rect);
/*
***Hur h�g promtrad nertill ?
*/
   pdy = 15;
/*
***Skugga runt insidan av arbetsytan.
*/
   SelectObject(dc,ms_darkg);

   p[0].x = 0;
   p[0].y = rect.bottom - pdy - 6;
   p[1].x = 0;
   p[1].y = 0;
   p[2].x = rect.right - 1;
   p[2].y = 0;
   Polyline(dc,p,3);

   SelectObject(dc,ms_black);

   p[0].x = 1;
   p[0].y = rect.bottom - pdy - 7;
   p[1].x = 1;
   p[1].y = 1;
   p[2].x = rect.right - 2;
   p[2].y = 1;
   Polyline(dc,p,3);


   SelectObject(dc,ms_white);

   p[0].x = rect.right - 1;
   p[0].y = 0;
   p[1].x = rect.right - 1;
   p[1].y = rect.bottom - pdy - 5;
   p[2].x = -1;
   p[2].y = rect.bottom - pdy - 5;
   Polyline(dc,p,3);

   SelectObject(dc,ms_liteg);

   p[0].x = rect.right - 2;
   p[0].y = 1;
   p[1].x = rect.right - 2;
   p[1].y = rect.bottom - pdy - 6;
   p[2].x = 0;
   p[2].y = rect.bottom - pdy - 6;
   Polyline(dc,p,3);
/*
***Skugga runt insidan av promtraden.
*/
   SelectObject(dc,ms_darkg);

   p[0].x = 0;
   p[0].y = rect.bottom - 1;
   p[1].x = 0;
   p[1].y = rect.bottom - pdy - 2;
   p[2].x = rect.right - 1;
   p[2].y = rect.bottom - pdy - 2;
   Polyline(dc,p,3);

   SelectObject(dc,ms_white);

   p[0].x = rect.right - 1;
   p[0].y = rect.bottom - pdy - 2;
   p[1].x = rect.right - 1;
   p[1].y = rect.bottom - pdy + 1;
   p[2].x = rect.right - 14;
   p[2].y = rect.bottom - 1;
   p[3].x = 0;
   p[3].y = rect.bottom - 1;
   Polyline(dc,p,4);

   p[0].x = rect.right - 2;
   p[0].y = rect.bottom - pdy + 6;
   p[1].x = rect.right - 10;
   p[1].y = rect.bottom - 1;
   Polyline(dc,p,2);

   p[0].x = rect.right - 2;
   p[0].y = rect.bottom - pdy + 10;
   p[1].x = rect.right - 6;
   p[1].y = rect.bottom - 1;
   Polyline(dc,p,2);

   SelectObject(dc,ms_darkg);

   p[0].x = rect.right - 2;
   p[0].y = rect.bottom - pdy + 3;
   p[1].x = rect.right - 13;
   p[1].y = rect.bottom - 1;
   Polyline(dc,p,2);
   p[0].x = rect.right - 2;
   p[0].y = rect.bottom - pdy + 4;
   p[1].x = rect.right - 12;
   p[1].y = rect.bottom - 1;
   Polyline(dc,p,2);

   p[0].x = rect.right - 2;
   p[0].y = rect.bottom - pdy + 7;
   p[1].x = rect.right - 9;
   p[1].y = rect.bottom - 1;
   Polyline(dc,p,2);
   p[0].x = rect.right - 2;
   p[0].y = rect.bottom - pdy + 8;
   p[1].x = rect.right - 8;
   p[1].y = rect.bottom - 1;
   Polyline(dc,p,2);

   p[0].x = rect.right - 2;
   p[0].y = rect.bottom - pdy + 11;
   p[1].x = rect.right - 5;
   p[1].y = rect.bottom - 1;
   Polyline(dc,p,2);
   p[0].x = rect.right - 2;
   p[0].y = rect.bottom - pdy + 12;
   p[1].x = rect.right - 4;
   p[1].y = rect.bottom - 1;
   Polyline(dc,p,2);
/*
***L�mna tillbaks DC:t.
*/
	EndPaint(ms_main,&ps);
/*
***Slut.
*/
   return;
  }

/********************************************************/
/*!******************************************************/

        int mswshw(w_id)
        v2int w_id;

/*      Visar ett f�nster.
 *
 *      In: w_id  = Entry i wpwtab.
 *
 *      Ut: Inget.   
 *
 *
 *      (C)microform ab 23/10/95 J. Kjellander
 *
 *      1996-12-09 Fokus p� knappar, J.Kjellander
 *      1997-03-07 SetActive.., J.Kjellander
 *      1998-01-13 WPRWIN, J.Kjellander
 *
 ******************************************************!*/

  {
   wpw_id   editid;
   HWND     win32_id;
   WPWIN   *winptr;
   WPIWIN  *iwinpt;
   WPLWIN  *lwinpt;
   WPGWIN  *gwinpt;
   WPBUTT  *buttpt;
   WPRWIN  *rwinpt;

/*
***Fixa en C-pekare till f�nstrets entry i wpwtab.
*/
   if ( (winptr=mswgwp(w_id)) == NULL ) return(-2);
/*
***Vilken typ av f�nster �r det ?
*/
   switch ( winptr->typ )
     {
/*
***WPIWIN-f�nster. Om det ineh�ller n�n WPEDIT s�tt focus p� den f�rsta.
***Om inga editar finns men en defaultknapp s�tter vi fokus p� den.
*/
     case TYP_IWIN:
     iwinpt = (WPIWIN *)winptr->ptr;
     win32_id = iwinpt->id.ms_id;
     editid = msffoc(iwinpt,FIRST_EDIT);
     if ( editid > -1 ) msfoeb(iwinpt,editid,TRUE);
     else
       {
       buttpt = msdefb(w_id);
       if ( buttpt != NULL ) msfoeb(iwinpt,buttpt->id.w_id,TRUE);
       }
     break;

     case TYP_GWIN:
     gwinpt = (WPGWIN *)winptr->ptr;
     win32_id = gwinpt->id.ms_id;
     break;

     case TYP_RWIN:
     rwinpt = (WPRWIN *)winptr->ptr;
     win32_id = rwinpt->id.ms_id;
     break;

     case TYP_LWIN:
     lwinpt = (WPLWIN *)winptr->ptr;
     win32_id = lwinpt->id.ms_id;
     break;

     default:
     return(-2);
     }
/*
***Visa resultatet och aktivera f�nstret.
*/  
   ShowWindow(win32_id,SW_SHOWNORMAL);
   UpdateWindow(win32_id);
//JKJKJK   SetActiveWindow(iwinpt->id.ms_id);

   return(0);
  }

/********************************************************/
/*!*******************************************************/

     int mswwtw(iwin_id,slevel,subw_id)
     v2int  iwin_id;
     v2int  slevel;
     v2int *subw_id;

/*   Message-loop f�r MBS-rutinen WAIT_WIN. L�gger sig
 *   och v�ntar p� meddelanden i det WPIWIN-f�nster som angetts.
 *
 *   In: iwin_id = ID f�r huvudf�nstret.
 *       slevel  = Service-niv� f�r key-event.
 *       subw_id = Pekare till utdata.
 *
 *   Ut: *subw_id = ID f�r det subf�nster d�r ett event intr�ffat.
 *
 *   Felkoder : WP1202 = iwin_id %s �r ej ett f�nster
 *
 *   (C)microform ab 23/10/95 J. Kjellander
 *
 *******************************************************!*/

 {
    wpw_id  par_id,serv_id;
    char    errbuf[80];
    WPWIN  *winptr;
    WPIWIN *iwinpt;
    MSG     message;

/*
***Kolla att f�nstret finns.
*/
   if ( (winptr=mswgwp((wpw_id)iwin_id)) == NULL )
     {
     sprintf(errbuf,"%d",(int)iwin_id);
     return(erpush("WP1202",errbuf));
     }
/*
***Om det �r ett WPIWIN, aktivera det s� det ser lite
***trevligare ut. Highligtad titelbalk etc. OBS. Aktivering
***flyttad till mswshw() 970307 JK. Anledningen �r att g�ra
***det m�jligt f�r ett WPLWIN all l�gga sig ovanp� ett WPIWIN
***i en wait_win()-loop.
*/
   switch ( winptr->typ )
     {
     case TYP_IWIN:
     iwinpt = (WPIWIN *)winptr->ptr;
/*     SetActiveWindow(iwinpt->id.ms_id); */

     if ( iwinpt->focus_id > -1 ) msfoeb(iwinpt,iwinpt->focus_id,TRUE);

     break;
     }
/*
***Meddelandeloop.
*/
loop:
   if ( GetMessage(&message,NULL,0,0) == TRUE )
     {
/*
***Om ett meddelande fanns att h�mta skickar vi det vidare
***till f�nstrets callback-rutin. Vissa meddelanden servas d�r
***men n�gra l�mnas kvar till l�ngre ner.
*/
     TranslateMessage(&message);    /* Translate virtual keycodes */
                                    /* to ASCII */
     DispatchMessage(&message);	    /* Tillbaks till windows igen */
    	                            /* och sen till callback-rutinen */
/*
***Om det var ett meddelande om en tangent eller mustryckning
***tar vi hand om det h�r.
*/
     switch ( ms_lstmes.msg )
       {
/*
***Musknapp. Tryckning i annat f�nster �n det vi v�ntar p� bryr
***vi oss inte om tillsvidare.
*/
       case WM_LBUTTONDOWN:
       par_id = mswfpw(ms_lstmes.wh);
       if ( par_id != iwin_id ) goto loop;

       if ( mswbut(&ms_lstmes,&serv_id) )
         {
         *subw_id = (v2int)serv_id;
          return(TRUE);
         }
       else goto loop;
/*
***Musclickning i f�nster skapat med f�nsterklass = BUTTON.
*/
       case WM_COMMAND:
       if ( HIWORD(ms_lstmes.wp) == BN_CLICKED )
         {
         par_id = mswfpw((HWND)ms_lstmes.lp);
         if ( par_id != iwin_id ) goto loop;

         ms_lstmes.wh = (HWND)ms_lstmes.lp;       
         if ( mswbut(&ms_lstmes,&serv_id) )
           {
           *subw_id = (v2int)serv_id;
            return(TRUE);
           }
         else goto loop;
         }
       else goto loop;

       default:
       goto loop;
       }
     }
/*
***Slut.
*/
#ifdef DEBUG
    if ( dbglev(MSPAC) == 1 )
      {
      fprintf(dbgfil(MSPAC),"***Slut-mswwtw***\n\n");
      fflush(dbgfil(MSPAC));
      }
#endif

   return(0);
 }

/*********************************************************/
/*!*******************************************************/

LRESULT CALLBACK mswcbk(HWND win32_id,UINT message,WPARAM wp,LPARAM lp)
       

/*      Callbackrutin f�r WPIWIN-f�nster.
 *
 *      In: win32_id = F�nster som det h�nt n�got i.
 *          message  = Vad som h�nt.
 *          wp,lp    = Ytterligare data.
 *
 *      Ut: Inget.   
 *
 *      (C)microform ab 23/10/95 J. Kjellander
 *
 ******************************************************!*/

  {	
#ifdef DEBUG
    if ( dbglev(MSPAC) == 11 )
      {
      fprintf(dbgfil(MSPAC),"***Start-mswcbk\n***");
      fprintf(dbgfil(MSPAC),"win32_id = %d, message = %s\n",
                             win32_id,msmest(message));
      fprintf(dbgfil(MSPAC),"***Slut-mswcbk***\n");
      fflush(dbgfil(MSPAC));
      }
#endif

   ms_lstmes.wh  = win32_id;
   ms_lstmes.msg = message;
   ms_lstmes.wp  = wp;
   ms_lstmes.lp  = lp;

   switch ( ms_lstmes.msg )
	   {
	   case WM_PAINT:
	   if ( !mswrep(win32_id) )
	    return DefWindowProc(win32_id,message,wp,lp);
	   break;

	   default:
	   return DefWindowProc(win32_id,message,wp,lp);
	   }


   return(0);
  }

/********************************************************/
/*!*******************************************************/

        int mswdel(w_id)
        v2int w_id;

/*      D�dar ett huvudf�nster med alla subf�nster.
 *
 *      In: w_id   = Huvudf�nstrets entry i wpwtab.
 *
 *      Ut: Inget.   
 *
 *      Felkod: WP1222 = Huvudf�nstret finns ej.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 *      1998-01-13 WPRWIN, J.Kjellander
 *
 ******************************************************!*/

  {
    char     errbuf[80];
 
    WPWIN   *winptr;
    WPIWIN  *iwinpt;
    WPLWIN  *lwinpt;
    WPGWIN  *gwinpt;
    WPEWIN  *ewinpt;
    WPRWIN  *rwinpt;

/*
***Fixa en C-pekare till huvud-f�nstrets entry i wpwtab.
*/
    if ( (winptr=mswgwp(w_id)) == NULL )
      {
      sprintf(errbuf,"%d",(int)w_id);
      return(erpush("WP1222",errbuf));
      }
/*
***Vilken typ av f�nster �r det ?
*/
    switch ( winptr->typ )
      {
      case TYP_IWIN:
      iwinpt = (WPIWIN *)winptr->ptr;
      DestroyWindow(iwinpt->id.ms_id);
      msdliw(iwinpt);
      break;

      case TYP_LWIN:
      lwinpt = (WPLWIN *)winptr->ptr;
      DestroyWindow(lwinpt->id.ms_id);
      msdllw(lwinpt);
      break;

      case TYP_GWIN:
      gwinpt = (WPGWIN *)winptr->ptr;
      DestroyWindow(gwinpt->id.ms_id);
      msdlgw(gwinpt);
      break;

      case TYP_RWIN:
      rwinpt = (WPRWIN *)winptr->ptr;
      DestroyWindow(rwinpt->id.ms_id);
      msdlrw(rwinpt);
      break;

      case TYP_EWIN:
      ewinpt = (WPEWIN *)winptr->ptr;
      DestroyWindow(ewinpt->id.ms_id);
      msdlew(ewinpt);
      break;
      }
/*
***Stryk f�nstret ur f�nstertabellen.
*/
    winptr->typ = TYP_UNDEF;
    winptr->ptr = NULL;
   
    return(0);
  }

/********************************************************/
/*!******************************************************/

        int   mswdls(w_id,sub_id)
        v2int w_id;
        v2int sub_id;

/*      D�dar ett subf�nster.
 *
 *      In: w_id   = Huvudf�nstrets entry i wpwtab.
 *          sub_id = Subf�nstrets id.
 *
 *      Ut: Inget.   
 *
 *      Felkod: WP1222 = Huvudf�nstret finns ej.
 *              WP1232 = Subf�nstret finns ej.
 *
 *      (C)microform ab 1996-03-06 J. Kjellander
 *
 *      1996-05-20 WPGWIN, J.Kjellander
 *      2000-03-14 Clipregion, J.Kjellander
 *
 ******************************************************!*/

  {
    char     errbuf[80];
    char    *subptr;
    WPWIN   *winptr;
    WPIWIN  *iwinpt;
    WPGWIN  *gwinpt;
    WPBUTT  *butptr;
    WPEDIT  *edtptr;
    WPICON  *icoptr;

/*
***Fixa en C-pekare till huvud-f�nstrets entry i wpwtab.
*/
    if ( (winptr=mswgwp(w_id)) == NULL )
      {
      sprintf(errbuf,"%d",(int)w_id);
      return(erpush("WP1222",errbuf));
      }
/*
***Vilken typ av f�nster �r det ?
*/
    switch ( winptr->typ )
      {
/*
***WPIWIN, kolla att subf�nstret finns.
*/
      case TYP_IWIN:
      if ( sub_id < 0  ||  sub_id > WP_IWSMAX-1 )
        {
        sprintf(errbuf,"%d%%%d",(int)w_id,(int)sub_id);
        return(erpush("WP1232",errbuf));
        }
      iwinpt = (WPIWIN *)winptr->ptr;
      subptr = iwinpt->wintab[(wpw_id)sub_id].ptr;
      if ( subptr == NULL )
        {
        sprintf(errbuf,"%d%%%d",(int)w_id,(int)sub_id);
        return(erpush("WP1232",errbuf));
        }
/*
***D�da f�nstret ur wpw och WIN32.
*/
      switch ( iwinpt->wintab[(wpw_id)sub_id].typ )
        {
        case TYP_BUTTON:
        butptr = (WPBUTT *)subptr;
        DestroyWindow(butptr->id.ms_id);
        msdlbu(butptr);
        break;

        case TYP_EDIT:
        edtptr = (WPEDIT *)subptr;
        DestroyWindow(edtptr->id.ms_id);
        msdled(edtptr);
        break;

        case TYP_ICON:
        icoptr = (WPICON *)subptr;
        DestroyWindow(icoptr->id.ms_id);
        msdlic(icoptr);
        break;
        }
/*
***L�nka bort subf�nstret fr�n WPIWIN-f�nstret.
*/
      iwinpt->wintab[(wpw_id)sub_id].ptr = NULL;
      iwinpt->wintab[(wpw_id)sub_id].typ = TYP_UNDEF;
      break;
/*
***WPGWIN.
*/
      case TYP_GWIN:
      if ( sub_id < 0  ||  sub_id > WP_GWSMAX-1 )
        {
        sprintf(errbuf,"%d%%%d",(int)w_id,(int)sub_id);
        return(erpush("WP1232",errbuf));
        }
      gwinpt = (WPGWIN *)winptr->ptr;
      subptr = gwinpt->wintab[(wpw_id)sub_id].ptr;
      if ( subptr == NULL )
        {
        sprintf(errbuf,"%d%%%d",(int)w_id,(int)sub_id);
        return(erpush("WP1232",errbuf));
        }
/*
***D�da f�nstret ur wpw och WIN32.
*/
      SelectClipRgn(gwinpt->dc,NULL);

      switch ( gwinpt->wintab[(wpw_id)sub_id].typ )
        {
        case TYP_BUTTON:
        butptr = (WPBUTT *)subptr;
        DestroyWindow(butptr->id.ms_id);
        msdlbu(butptr);
        break;

        case TYP_EDIT:
        edtptr = (WPEDIT *)subptr;
        DestroyWindow(edtptr->id.ms_id);
        msdled(edtptr);
        break;

        case TYP_ICON:
        icoptr = (WPICON *)subptr;
        DestroyWindow(icoptr->id.ms_id);
        msdlic(icoptr);
        break;
        }

      gwinpt->wintab[(wpw_id)sub_id].ptr = NULL;
      gwinpt->wintab[(wpw_id)sub_id].typ = TYP_UNDEF;

      UpdateWindow(gwinpt->id.ms_id);
      SelectClipRgn(gwinpt->dc,gwinpt->clprgn);
      break;
      }

    return(0);
  }

/********************************************************/
/*!******************************************************/

        wpw_id mswffi()

/*      Letar upp l�gsta lediga entry i wpwtab.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: Giltigt ID eller erpush().
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {
    int i;
/*
***Leta upp ledig plats i f�nstertabellen. L�mna ID = 0
***ledigt eftersom detta ID �r reserverat f�r V3:s grafiska
***huvudf�nster.
*/
    i = 1;

    while ( i < WTABSIZ  &&  wpwtab[i].ptr != NULL ) ++i;
/*
***Finns det n�gon ?
*/
    if ( i == WTABSIZ ) return(-2);
/*
***Ja, returnera ID.
*/
    else return((wpw_id)i);
  }

/********************************************************/
/*!******************************************************/

        wpw_id mswfpw(win32_id)
        HWND  win32_id;

/*      Letar upp id f�r f�r�ldern till ett sub-
 *      f�nster med visst ms-id. Om f�nstret med
 *      det angivna ms-id:t �r en f�r�lder returneras
 *      -1.
 *
 *      Denna rutin anv�nds av mswwtw() f�r att avg�ra
 *      om ett meddelande kommer fr�n ett subf�nster till det
 *      huvudf�nster som vi v�ntar p�.
 *
 *      In: win32_id  = Subf�nstrets ms-id.
 *
 *      Ut: Inget.   
 *
 *      FV: id f�r f�nster eller -1.
 *
 *      (C)microform ab 28/10/95 J. Kjellander
 *
 ******************************************************!*/

  {
    short    i,j;
    WPIWIN  *iwinpt;
    WPBUTT  *buttpt;
    WPICON  *iconpt;

/*
***S�k igenom hela wpwtab efter f�nster.
*/
    for ( i=0; i<WTABSIZ; ++i)
      {
      if ( wpwtab[i].ptr != NULL )
        {
/*
***Vilken typ av f�nster �r det ?
*/
        switch ( wpwtab[i].typ )
          {
/*
***WPIWIN-f�nster. Kolla f�nstret sj�lvt och 
***s�k igenom alla sub-f�nster. Tillsvidare �r
***det bara aktuellt med knappar.
*/
          case TYP_IWIN:
          iwinpt = (WPIWIN *)wpwtab[i].ptr;
          

          for ( j=0; j<WP_IWSMAX; ++j )
            {
            if ( iwinpt->wintab[j].ptr != NULL )
              {
              switch ( iwinpt->wintab[j].typ ) 
                {
                case TYP_BUTTON:
                buttpt = (WPBUTT *)iwinpt->wintab[j].ptr;
                if ( buttpt->id.ms_id == win32_id ) return(iwinpt->id.w_id);
                break;

                case TYP_ICON:
                iconpt = (WPICON *)iwinpt->wintab[j].ptr;
                if ( iconpt->id.ms_id == win32_id ) return(iwinpt->id.w_id);
                break;
                }
              }
            }
          break;
          }
        }
     }
/*
***Ingen tr�ff.
*/
    return((wpw_id)-1);
  }

/********************************************************/
/*!******************************************************/

        WPWIN *mswgwp(id)
        wpw_id id;

/*      �vers�tter id till C-pekare f�r motsvarande entry
 *      i wpwtab.
 *
 *      In: id = F�nstrets entry i wpwtab.
 *
 *      Ut: Inget.
 *
 *      FV: Giltig C-pekare eller NULL.
 *
 *      (C)microform ab 6/12/93 J. Kjellander
 *
 ******************************************************!*/

  {

/*
***�r det ett giltigt ID ?
*/
    if ( id < 0  ||  id >= WTABSIZ ) return(NULL);
/*
***Ja, returnera pekare om det finns n�gon.
*/
    else
      {
      if ( wpwtab[id].ptr != NULL ) return(&wpwtab[id]);
      else return(NULL);
      }
  }

/********************************************************/
