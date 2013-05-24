/**********************************************************************
*
*    ms3.c
*    =====
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     msdl01();   New module box
*     msdl02();   Scale box
*     msdl03();   String input box
*     msdl07();   Levels box
*     msdl08();   Curve accuracsy box
*     msialt();   Input alternative box
*     msview();   View box
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
#include "../../../sources/EX/include/EX.h"
#include "../../../sources/WP/include/WP.h"
#include "../../../sources/GP/include/GP.h"
#include <commctrl.h>


BOOL APIENTRY cbkd01(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY cbkd02(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY cbkd03(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY cbkd04(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY cbkd07(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY cbkd08(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY cbkd09(HWND,UINT,WPARAM,LPARAM);

static struct
{
short typ;
short att;
} dl01;                      /* Intefacestruktur f�r dialog 01 */

static struct
{
double skala;
char   vynamn[GPVNLN+1];
} dl02;                      /* Interfacestruktur f�r dialog 02 */

static struct
{
char   *dp;
char  **ip;
char  **ds;
char  **is;
short  *ml;
int     ns;
} dl03;                      /* Interfacestruktur f�r dialog 03 */


static struct
{
char   *ps;
char   *ts;
char   *fs;
} dl04;
                             /* Interfacestruktur f�r dialog 04 */
static struct
{
int    level;
int    count;
int    state;
} dl07;                      /* Interfacestruktur f�r dialog 07 */

static struct
{
double cnog;
} dl08;                      /* Interfacestruktur f�r dialog 08 */


extern char jobnam[];

/*!******************************************************/

        short  msdl01(typ,att)
        short *typ;
        short *att;

/*      Dialogbox f�r modultyp och attribut vid skapande
 *      av ny modul. Anropas av ige22:iginmo().
 *
 *      In: typ = Pekare till utdata.
 *          att = Pekare till utdata.
 *
 *      Ut: *typ = Modultyp, _2D eller _3D.
 *          *att = Attribut, LOCAL, GLOBAL eller BASIC.
 *
 *      FV: 0      = Ok.
 *          REJECT = Avbyt.
 *
 *      (C)microform ab 8/11/95 J. Kjellander
 *
 ******************************************************!*/

 {
 /*
***Lite test av dialogboxar.
*/
   if ( DialogBox(ms_inst,
                  MAKEINTRESOURCE(IDD_DIALOG1),
                  ms_main,
                 (DLGPROC)cbkd01) == IDOK )
     {
    *typ = dl01.typ;
    *att = dl01.att;
     return(0);
     }
   else return(REJECT);
 }

/********************************************************/
/*!******************************************************/

        BOOL APIENTRY cbkd01(dlg,message,wp,lp)
        HWND dlg;
        UINT message;
        WPARAM wp;
        LPARAM lp;

/*      Callback f�r msdl01(), modultyp och attribut.
 *
 *      In: dlg     = Dialogboxens f�nsterid.
 *          message = Typ av meddelande.
 *          wp,lp   = Ytterligare data.
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 8/11/95 J. Kjellander
 *
 ******************************************************!*/
 {
   char     titbuf[V3STRLEN],cancel[V3STRLEN],okey[V3STRLEN];
   int      x,y,mdx,mdy,ddx,ddy;
   RECT     mr,dr;
   COLORREF color;

   switch ( message )
     {
     case WM_INITDIALOG:
/*
***Rubrik.
*/
     if ( !msgrst("MODULE.TITLE",titbuf) ) strcpy(titbuf,"Ny modul : ");
     strcat(titbuf,jobnam);
     SetWindowText(dlg,titbuf);
/*
***Defaultv�rden p� radioknappar.
*/
     CheckRadioButton(dlg,IDC_RADIO1,IDC_RADIO3,IDC_RADIO3);
     CheckRadioButton(dlg,IDC_RADIO4,IDC_RADIO5,IDC_RADIO4);
/*
***Knapparna l�ngst ner.
*/
     if ( !msgrst("INPUT.OKEY",okey) ) strcpy(okey,"Okey");
     if ( !msgrst("INPUT.REJECT",cancel) ) strcpy(cancel,"Avbryt");

     SetDlgItemText(dlg,IDOK,okey);
     SetDlgItemText(dlg,IDCANCEL,cancel);

/*
***Placering mitt i huvudf�nstret.
*/
     GetWindowRect(ms_main,&mr);
     mdx = mr.right - mr.left;
     mdy = mr.bottom - mr.top;
     GetWindowRect(dlg,&dr);
     ddx = dr.right - dr.left;
     ddy = dr.bottom - dr.top;

     x = (mdx - ddx)/2;
     y = (mdy - ddy)/2;

     SetWindowPos(dlg,HWND_TOP,x,y,0,0,SWP_NOSIZE);
     return(TRUE);
/*
***F�rger.
*/
     case WM_CTLCOLORDLG:
     case WM_CTLCOLORBTN:
     color = GetSysColor(COLOR_MENU);
     SetBkColor((HDC)wp,color);
     return((BOOL)GetStockObject(LTGRAY_BRUSH));
/*
***Clickning i OK eller CANCEL-rutan.
*/
     case WM_COMMAND:
     switch ( LOWORD(wp) )
       {
       case IDOK:
       if      ( IsDlgButtonChecked(dlg,IDC_RADIO1) == 1 ) dl01.att = LOCAL;
       else if ( IsDlgButtonChecked(dlg,IDC_RADIO2) == 1 ) dl01.att = GLOBAL;
       else                                                dl01.att = BASIC;

       if      ( IsDlgButtonChecked(dlg,IDC_RADIO4) == 1 ) dl01.typ = _2D;
       else                                                dl01.typ = _3D;

       case IDCANCEL:
       EndDialog(dlg,wp);
       return(TRUE);
       }
     }
   return(FALSE);
 }

/********************************************************/
/*!******************************************************/

        int     msdl02(skala,vynamn)
        double *skala;
        char   *vynamn;

/*      Dialogbox f�r sk�rmskala.
 *
 *      In: skala  = Pekare till nuvarande skala.
 *          vynamn = Namn p� aktuell vy.
 *
 *      Ut: *skala = Ny skalfaktor.
 *
 *      FV: 0      = Ok.
 *          REJECT = Avbyt.
 *
 *      (C)microform ab 10/11/95 J. Kjellander
 *
 ******************************************************!*/

 {
 /*
 ***Initiering.
 */
    dl02.skala = *skala;
    strcpy(dl02.vynamn,vynamn);
 /*
***Ut med boxen.
*/
   if ( DialogBox(ms_inst,
                  MAKEINTRESOURCE(IDD_DIALOG2),
                  ms_main,
                 (DLGPROC)cbkd02) == IDOK )
     {
    *skala = dl02.skala;
     return(0);
     }
   else return(REJECT);
 }

/********************************************************/
/*!******************************************************/

        BOOL APIENTRY cbkd02(dlg,message,wp,lp)
        HWND dlg;
        UINT message;
        WPARAM wp;
        LPARAM lp;

/*      Callback f�r msdl02(), sk�rmskala.
 *
 *      In: dlg     = Dialogboxens f�nsterid.
 *          message = Typ av meddelande.
 *          wp,lp   = Ytterligare data.
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 10/11/95 J. Kjellander
 *
 ******************************************************!*/
 {
   char     titbuf[V3STRLEN],skalbuf[V3STRLEN+1],half[V3STRLEN],
            doub[V3STRLEN],okey[V3STRLEN],cancel[V3STRLEN];
   int      x,y,mdx,mdy,ddx,ddy;
   RECT     mr,dr;
   COLORREF color;

   switch ( message )
     {
     case WM_INITDIALOG:
/*
***Rubrik.
*/
     if ( !msgrst("SCALE.TITLE",titbuf) ) strcpy(titbuf,"Skala f�r vy : ");
     strcat(titbuf,dl02.vynamn);
     SetWindowText(dlg,titbuf);
/*
***Knapptexter.
*/
     if ( !msgrst("SCALE.HALF",half) ) strcpy(half,"Halva");
     SetDlgItemText(dlg,IDC_BUTTON1,half);

     if ( !msgrst("SCALE.DOUBLE",doub) ) strcpy(doub,"Dubbla");
     SetDlgItemText(dlg,IDC_BUTTON3,doub);
/*
***Defaultv�rden f�r editboxen.
*/
     sprintf(skalbuf,"%g",dl02.skala);
     SetDlgItemText(dlg,IDC_EDIT1,skalbuf);
/*
***Knapparna l�ngst ner.
*/
     if ( !msgrst("INPUT.OKEY",okey) ) strcpy(okey,"Okey");
     if ( !msgrst("INPUT.REJECT",cancel) ) strcpy(cancel,"Avbryt");

     SetDlgItemText(dlg,IDOK,okey);
     SetDlgItemText(dlg,IDCANCEL,cancel);
/*
***Placering mitt i huvudf�nstret.
*/
     GetWindowRect(ms_main,&mr);
     mdx = mr.right - mr.left;
     mdy = mr.bottom - mr.top;
     GetWindowRect(dlg,&dr);
     ddx = dr.right - dr.left;
     ddy = dr.bottom - dr.top;

     x = (mdx - ddx)/2;
     y = (mdy - ddy)/2;

     SetWindowPos(dlg,HWND_TOP,x,y,0,0,SWP_NOSIZE);
     return(TRUE);
/*
***F�rger.
*/
     case WM_CTLCOLORDLG:
     case WM_CTLCOLORBTN:
     color = GetSysColor(COLOR_MENU);
     SetBkColor((HDC)wp,color);
     return((BOOL)GetStockObject(LTGRAY_BRUSH));
/*
***Clickning i OK eller CANCEL-rutan.
*/
     case WM_COMMAND:
     switch ( LOWORD(wp) )
       {
       case IDC_BUTTON1:
       dl02.skala /= 2.0;
       EndDialog(dlg,IDOK);
       return(TRUE);

       case IDC_BUTTON2:
       dl02.skala = 1.0;
       EndDialog(dlg,IDOK);
       return(TRUE);

       case IDC_BUTTON3:
       dl02.skala *= 2.0;
       EndDialog(dlg,IDOK);
       return(TRUE);

       case IDOK:
       GetDlgItemText(dlg,IDC_EDIT1,skalbuf,V3STRLEN);
       if ( sscanf(skalbuf,"%lf",&dl02.skala) < 1 )
         {
         msmbox("Scale error",skalbuf,0);
         return(FALSE);
         }

       case IDCANCEL:
       EndDialog(dlg,wp);
       return(TRUE);
       }
     }
   return(FALSE);
 }

/********************************************************/
/*!******************************************************/

        int  msdl03(dp,ip,ds,is,ml,ns)
        char *dp;
        char *ip[];
        char *ds[];
        char *is[];
        short ml[];
        int   ns;

/*      mspac:s igmsip f�r en, tv� eller tre str�ngar.
 *
 *      In: Samma som ige21:igmsip().
 *
 *      Ut: is = Inmatade str�ngar.
 *
 *      FV: 0      = Ok.
 *          REJECT = Avbyt.
 *
 *      (C)microform ab 28/11/95 J. Kjellander
 *
 ******************************************************!*/

 {
   LPTSTR box_id;

/*
***Initiering.
*/
   dl03.dp = dp;
   dl03.ip = ip;
   dl03.ds = ds;
   dl03.is = is;
   dl03.ml = ml;
   dl03.ns = ns;
/*
***Ut med boxen, IDD_DIALOG3 = 1 str�ng, IDD_DIALOG4 = 2 och IDD_DIALOG5 = 3.
*/
   switch ( ns )
     {
     case 1: box_id = MAKEINTRESOURCE(IDD_DIALOG3); break;
     case 2: box_id = MAKEINTRESOURCE(IDD_DIALOG4); break;
     case 3: box_id = MAKEINTRESOURCE(IDD_DIALOG5); break;
     }

   if ( DialogBox(ms_inst,
                  box_id,
                  ms_main,
                 (DLGPROC)cbkd03) == IDOK )
     {
     if ( ns == 1 && strlen(is[0]) == 0 ) return(REJECT);
     else                                 return(0);
     }
   else return(REJECT);
 }

/********************************************************/
/*!******************************************************/

        BOOL APIENTRY cbkd03(dlg,message,wp,lp)
        HWND dlg;
        UINT message;
        WPARAM wp;
        LPARAM lp;

/*      Callback f�r msdl03().
 *
 *      In: dlg     = Dialogboxens f�nsterid.
 *          message = Typ av meddelande.
 *          wp,lp   = Ytterligare data.
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 28/11/95 J. Kjellander
 *
 *      1997-04-16 ml[i]+1, J.Kjellander
 *
 ******************************************************!*/
 {
   char     okey[V3STRLEN],cancel[V3STRLEN],help[V3STRLEN];
   int      i,x,y,mdx,mdy,ddx,ddy;
   RECT     mr,dr;
   COLORREF color;

   switch ( message )
     {
     case WM_INITDIALOG:
/*
***Dialogboxens rubrik.
*/
     SetWindowText(dlg,dl03.dp);
/*
***Inmatningsf�ltens promtar.
*/
     for ( i=0; i<dl03.ns; ++i )
       {
       SetDlgItemText(dlg,IDC_STATIC1+i,dl03.ip[i]);
/*
***Defaultv�rden f�r editboxarna.
*/
       SetDlgItemText(dlg,IDC_EDIT1+i,dl03.ds[i]);
       }
/*
***Knapparna l�ngst ner.
*/
     if ( !msgrst("INPUT.OKEY",okey) ) strcpy(okey,"Okey");
     if ( !msgrst("INPUT.REJECT",cancel) ) strcpy(cancel,"Avbryt");
     if ( !msgrst("INPUT.HELP",help) ) strcpy(help,"Hj�lp");

     SetDlgItemText(dlg,IDOK,okey);
     SetDlgItemText(dlg,IDCANCEL,cancel);
     SetDlgItemText(dlg,IDC_BUTTON1,help);
/*
***Placering mitt i huvudf�nstret.
*/
     GetWindowRect(ms_main,&mr);
     mdx = mr.right - mr.left;
     mdy = mr.bottom - mr.top;
     GetWindowRect(dlg,&dr);
     ddx = dr.right - dr.left;
     ddy = dr.bottom - dr.top;

     x = (mdx - ddx)/2;
     y = (mdy - ddy)/2;

     SetWindowPos(dlg,HWND_TOP,x,y,0,0,SWP_NOSIZE);
     return(TRUE);
/*
***F�rger.
*/
     case WM_CTLCOLORDLG:
     case WM_CTLCOLORBTN:
     case WM_CTLCOLORSTATIC:
     color = GetSysColor(COLOR_MENU);
     SetBkColor((HDC)wp,color);
     return((BOOL)GetStockObject(LTGRAY_BRUSH));
/*
***Clickning i OK eller CANCEL-rutan.
*/
     case WM_COMMAND:
     switch ( LOWORD(wp) )
       {
       case IDOK:
       for ( i=0; i<dl03.ns; ++i )
         GetDlgItemText(dlg,IDC_EDIT1+i,dl03.is[i],(int)dl03.ml[i]+1);

       case IDCANCEL:
       EndDialog(dlg,wp);
       return(TRUE);

       case IDC_BUTTON1:
       ighelp();
       return(TRUE);
       }
     }
   return(FALSE);
 }

/********************************************************/
/*!******************************************************/

        bool  msialt(ps,ts,fs,pip)
        char *ps;
        char *ts;
        char *fs;
        bool  pip;

/*      Dialogbox f�r msialt().
 *
 *      In: ps  = Promt.
 *          ts  = TRUE-str�ng.
 *          fs  = FALSE-str�ng.
 *          pip = Inledande ljud ja/nej.
 *
 *      FV: TRUE eller FALSE.
 *
 *      (C)microform ab 10/11/95 J. Kjellander
 *
 ******************************************************!*/

 {
 /*
 ***Initiering.
 */
    dl04.ps = ps;
    dl04.ts = ts;
    dl04.fs = fs;
/*
***Skall vi l�ta lite ?
*/
   if ( pip ) MessageBeep(MB_ICONQUESTION);
/*
***Ut med boxen.
*/
   if ( DialogBox(ms_inst,MAKEINTRESOURCE(IDD_DIALOG6),
                  ms_main,(DLGPROC)cbkd04) == IDOK ) return(TRUE);

   else return(FALSE);
 }

/********************************************************/
/*!******************************************************/

        BOOL APIENTRY cbkd04(dlg,message,wp,lp)
        HWND dlg;
        UINT message;
        WPARAM wp;
        LPARAM lp;

/*      Callback f�r msialt().
 *
 *      In: dlg     = Dialogboxens f�nsterid.
 *          message = Typ av meddelande.
 *          wp,lp   = Ytterligare data.
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 10/11/95 J. Kjellander
 *
 ******************************************************!*/

 {
   char     titbuf[V3STRLEN],help[V3STRLEN];
   int      x,y,mdx,mdy,ddx,ddy;
   RECT     mr,dr;
   COLORREF color;

   switch ( message )
     {
     case WM_INITDIALOG:
/*
***Titel.
*/
     if ( !msgrst("ALTERNATIVE.TITLE",titbuf) ) strcpy(titbuf,"Alternativ");
     SetWindowText(dlg,titbuf);
/*
***Promt.
*/
     SetDlgItemText(dlg,IDC_STATIC1,dl04.ps);
/*
***True och False-knappen.
*/
     SetDlgItemText(dlg,IDC_BUTTON1,dl04.ts);
     SetDlgItemText(dlg,IDC_BUTTON2,dl04.fs);
/*
***Hj�lp-knappen.
*/
     if ( !msgrst("INPUT.HELP",help) ) strcpy(help,"Hj�lp");
     SetDlgItemText(dlg,IDC_BUTTON3,help);
/*
***Placering mitt i huvudf�nstret.
*/
     GetWindowRect(ms_main,&mr);
     mdx = mr.right - mr.left;
     mdy = mr.bottom - mr.top;
     GetWindowRect(dlg,&dr);
     ddx = dr.right - dr.left;
     ddy = dr.bottom - dr.top;

     x = (mdx - ddx)/2;
     y = (mdy - ddy)/2;

     SetWindowPos(dlg,HWND_TOP,x,y,0,0,SWP_NOSIZE);
     return(TRUE);
/*
***F�rger.
*/
     case WM_CTLCOLORDLG:
     case WM_CTLCOLORBTN:
     case WM_CTLCOLORSTATIC:
     color = GetSysColor(COLOR_MENU);
     SetBkColor((HDC)wp,color);
     return((BOOL)GetStockObject(LTGRAY_BRUSH));
/*
***Clickning i OK eller CANCEL-rutan.
*/
     case WM_COMMAND:
     switch ( LOWORD(wp) )
       {
       case IDC_BUTTON1:
       EndDialog(dlg,IDOK);
       return(TRUE);

       case IDC_BUTTON2:
       EndDialog(dlg,IDCANCEL);
       return(TRUE);

       case IDC_BUTTON3:
       ighelp();
       return(TRUE);
       }
     }
   return(FALSE);
 }

/********************************************************/
/*!******************************************************/

        int  msdl07(level,count,state)
        int *level;
        int *count;
        int *state;

/*      Dialogbox f�r niv�er.
 *
 *      In: level = Pekare till urdata.
 *          count = Pekare till utdata.
 *          state = Pekare till utdata.
 *
 *      Ut: *level = F�rsta niv�.
 *          *count = Antal ytterligare.
 *          *state = WP_UBLANKL, WP_BLANKL eller WP_LISTL.
 *
 *      FV: 0      = Ok.
 *          REJECT = Avbyt.
 *
 *      (C)microform ab 1996-12-12 J. Kjellander
 *
 ******************************************************!*/

 {
 /*
 ***Initiering.
 */
    dl07.level = 0;
    dl07.count = 1;
    dl07.state = WP_UBLANKL;
 /*
***Ut med boxen.
*/
   if ( DialogBox(ms_inst,
                  MAKEINTRESOURCE(IDD_DIALOG7),
                  ms_main,
                 (DLGPROC)cbkd07) == IDOK )
     {
    *level = dl07.level;
    *count = dl07.count;
    *state = dl07.state;
     return(0);
     }
   else return(REJECT);
 }

/********************************************************/
/*!******************************************************/

        BOOL APIENTRY cbkd07(dlg,message,wp,lp)
        HWND dlg;
        UINT message;
        WPARAM wp;
        LPARAM lp;

/*      Callback f�r msdl07(), niv�er.
 *
 *      In: dlg     = Dialogboxens f�nsterid.
 *          message = Typ av meddelande.
 *          wp,lp   = Ytterligare data.
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 1996-12-12 J. Kjellander
 *
 ******************************************************!*/
 {
   char     buf[V3STRLEN],
            okey[V3STRLEN],cancel[V3STRLEN];
   int      x,y,mdx,mdy,ddx,ddy;
   RECT     mr,dr;
   COLORREF color;

static char levbuf[V3STRLEN] = "";
static char cntbuf[V3STRLEN] = "";
static int  state = WP_UBLANKL;


   switch ( message )
     {
     case WM_INITDIALOG:
/*
***Rubrik.
*/
     if ( !msgrst("LEVEL.TITLE",buf) ) strcpy(buf,"Niv�er");
     SetWindowText(dlg,buf);
/*
***Lablar f�r editarna.
*/
     if ( !msgrst("LEVEL.FIRST",buf) ) strcpy(buf,"Niv�");
     SetDlgItemText(dlg,IDC_STATIC1,buf);
     if ( !msgrst("LEVEL.COUNT",buf) ) strcpy(buf,"Antal");
     SetDlgItemText(dlg,IDC_STATIC2,buf);
/*
***Defaultv�rden f�r edititarna.
*/
     if ( *levbuf == '\0' ) strcpy(levbuf,"0");
     SetDlgItemText(dlg,IDC_EDIT1,levbuf);
     if ( *cntbuf == '\0' ) strcpy(cntbuf,"1");
     SetDlgItemText(dlg,IDC_EDIT2,cntbuf);
/*
***Texter till radioknapparna.
*/
     if ( !msgrst("LEVEL.BLANK",buf) ) strcpy(buf,"Sl�ck");
     SetDlgItemText(dlg,IDC_RADIO1,buf);
     if ( !msgrst("LEVEL.UNBLANK",buf) ) strcpy(buf,"T�nd");
     SetDlgItemText(dlg,IDC_RADIO2,buf);
     if ( !msgrst("LEVEL.LIST",buf) ) strcpy(buf,"Lista");
     SetDlgItemText(dlg,IDC_RADIO3,buf);
/*
***Defaultv�rden p� radioknappar.
*/
     switch ( state )
       {
       case WP_BLANKL:
       CheckRadioButton(dlg,IDC_RADIO1,IDC_RADIO3,IDC_RADIO1);
       break;

       case WP_UBLANKL:
       CheckRadioButton(dlg,IDC_RADIO1,IDC_RADIO3,IDC_RADIO2);
       break;

       case WP_LISTL:
       CheckRadioButton(dlg,IDC_RADIO1,IDC_RADIO3,IDC_RADIO3);
       break;
       }
/*
***OK och Cancel..
*/
     if ( !msgrst("INPUT.OKEY",okey) ) strcpy(okey,"Okey");
     if ( !msgrst("INPUT.REJECT",cancel) ) strcpy(cancel,"Avbryt");

     SetDlgItemText(dlg,IDOK,okey);
     SetDlgItemText(dlg,IDCANCEL,cancel);
/*
***Placering mitt i huvudf�nstret.
*/
     GetWindowRect(ms_main,&mr);
     mdx = mr.right - mr.left;
     mdy = mr.bottom - mr.top;
     GetWindowRect(dlg,&dr);
     ddx = dr.right - dr.left;
     ddy = dr.bottom - dr.top;

     x = (mdx - ddx)/2;
     y = (mdy - ddy)/2;

     SetWindowPos(dlg,HWND_TOP,x,y,0,0,SWP_NOSIZE);
     return(TRUE);
/*
***F�rger.
*/
     case WM_CTLCOLORDLG:
     case WM_CTLCOLORBTN:
     case WM_CTLCOLORSTATIC:
     color = GetSysColor(COLOR_MENU);
     SetBkColor((HDC)wp,color);
     return((BOOL)GetStockObject(LTGRAY_BRUSH));
/*
***Clickning i OK eller CANCEL-rutan.
*/
     case WM_COMMAND:
     switch ( LOWORD(wp) )
       {
       case IDOK:
       GetDlgItemText(dlg,IDC_EDIT1,levbuf,V3STRLEN);
       GetDlgItemText(dlg,IDC_EDIT2,cntbuf,V3STRLEN);
       if      ( IsDlgButtonChecked(dlg,IDC_RADIO1) == 1 ) state = WP_BLANKL;
       else if ( IsDlgButtonChecked(dlg,IDC_RADIO2) == 1 ) state = WP_UBLANKL;
       else                                                state = WP_LISTL;

       if ( sscanf(levbuf,"%d",&dl07.level) < 1 )
         {
         Beep(1000,1000);
         return(FALSE);
         }
       if ( dl07.level < 0  ||  dl07.level > 1999 )
         {
         Beep(1000,1000);
         return(FALSE);
         }

       if ( sscanf(cntbuf,"%d",&dl07.count) < 1  ||  dl07.count == 0 )
         {
         Beep(1000,1000);
         return(FALSE);
         }
       if ( dl07.level+dl07.count < -1  ||  dl07.level+dl07.count > 2000 )
         {
         Beep(1000,1000);
         return(FALSE);
         }

       dl07.state = state;

       case IDCANCEL:
       EndDialog(dlg,wp);
       return(TRUE);
       }
     }
   return(FALSE);
 }

/********************************************************/
/*!******************************************************/

        int  msdl08(cnog)
        double *cnog;

/*      Dialogbox f�r kurvnoggrannhet.
 *
 *      In: cnog  = Pekare till aktuell kurvnoggrannhetsfaktor.
 *
 *      Ut: *cnog = Uppdaterad kurvnoggrannhet.
 *
 *      FV: 0      = Ok.
 *          REJECT = Avbyt.
 *
 *      (C)microform ab 1998-11-05 J. Kjellander
 *
 ******************************************************!*/

 {
 /*
 ***Initiering.
 */
    dl08.cnog = *cnog;
 /*
***Ut med boxen.
*/
   if ( DialogBox(ms_inst,
                  MAKEINTRESOURCE(IDD_DIALOG8),
                  ms_main,
                 (DLGPROC)cbkd08) == IDOK )
     {
    *cnog = dl08.cnog;
     return(0);
     }
   else return(REJECT);
 }

/********************************************************/
/*!******************************************************/

        BOOL APIENTRY cbkd08(dlg,message,wp,lp)
        HWND dlg;
        UINT message;
        WPARAM wp;
        LPARAM lp;

/*      Callback f�r kurvnoggrannhet().
 *
 *      In: dlg     = Dialogboxens f�nsterid.
 *          message = Typ av meddelande.
 *          wp,lp   = Ytterligare data.
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 1998-11-05 J. Kjellander
 *
 ******************************************************!*/

 {
   char     titbuf[V3STRLEN],buf[V3STRLEN];
   char     okey[V3STRLEN],cancel[V3STRLEN],help[V3STRLEN];
   int      x,y,mdx,mdy,ddx,ddy,spos;
   double   cnog;
   RECT     mr,dr;
   COLORREF color;

#define EXPONENT 3                 /* Mappning av slider->kurvnoggrannhet */
#define KONSTANT 4.64158883361278  /* 100**(1.0/EXPONENT) */


   switch ( message )
     {
     case WM_INITDIALOG:
/*
***Titel.
*/
     if ( !msgrst("CURACC.TITLE",titbuf) ) strcpy(titbuf,"Kurvnoggrannhet = ");
     gpgtcn(&cnog);
     sprintf(buf,"%g",cnog);
     strcat(titbuf,buf);
     SetWindowText(dlg,titbuf);
/*
***Edit-f�nstret.
*/
     SetDlgItemText(dlg,IDC_EDIT1,buf);
/*
***Okey, Reject och Help-knappen.
*/
     if ( !msgrst("INPUT.OKEY",okey) ) strcpy(okey,"Okey");
     if ( !msgrst("INPUT.REJECT",cancel) ) strcpy(cancel,"Avbryt");
     if ( !msgrst("INPUT.HELP",help) ) strcpy(help,"Hj�lp");

     SetDlgItemText(dlg,IDOK,okey);
     SetDlgItemText(dlg,IDCANCEL,cancel);
     SetDlgItemText(dlg,IDC_BUTTON1,help);
/*
***Skalstreck p� slidern.
*/
     SendDlgItemMessage(dlg,IDC_SLIDER1,TBM_SETRANGE,(WPARAM)TRUE,
                        (LPARAM)MAKELONG(0,100));
     gpgtcn(&cnog);
     cnog -= 0.1;
     cnog = pow(cnog,1.0/EXPONENT);
     spos = (int)(cnog/KONSTANT*100.0);
     SendDlgItemMessage(dlg,IDC_SLIDER1,TBM_SETPOS,(WPARAM)TRUE,
                        (LPARAM)spos);


     SendDlgItemMessage(dlg,IDC_SLIDER1,TBM_SETTICFREQ,(WPARAM)10,
                        (LPARAM)0);
/*
***Aktuellt v�rde i editen.
*/
     gpgtcn(&cnog);
     if ( cnog < 10.0 ) sprintf(buf,"%3.1lf",cnog);
     else sprintf(buf,"%d",(int)cnog);
     SetDlgItemText(dlg,IDC_EDIT1,buf);
/*
***Placering mitt i huvudf�nstret.
*/
     GetWindowRect(ms_main,&mr);
     mdx = mr.right - mr.left;
     mdy = mr.bottom - mr.top;
     GetWindowRect(dlg,&dr);
     ddx = dr.right - dr.left;
     ddy = dr.bottom - dr.top;

     x = (mdx - ddx)/2;
     y = (mdy - ddy)/2;

     SetWindowPos(dlg,HWND_TOP,x,y,0,0,SWP_NOSIZE);
     return(TRUE);
/*
***F�rger.
*/
     case WM_CTLCOLORDLG:
     case WM_CTLCOLORBTN:
     case WM_CTLCOLORSTATIC:
     color = GetSysColor(COLOR_MENU);
     SetBkColor((HDC)wp,color);
     return((BOOL)GetStockObject(LTGRAY_BRUSH));
/*
***Slidebaren flyttar sig.
*/
     case WM_HSCROLL:
     spos = SendDlgItemMessage(dlg,IDC_SLIDER1,TBM_GETPOS,
                                (WPARAM)0,(LPARAM)0);
     cnog = KONSTANT*spos/100.0;
     cnog = 0.1 + pow(cnog,EXPONENT);
     if ( cnog > 100.0 ) cnog = 100.0;
     if ( cnog < 10.0 ) sprintf(buf,"%3.1lf",cnog);
     else sprintf(buf,"%d",(int)cnog);
     SetDlgItemText(dlg,IDC_EDIT1,buf);
     break;
/*
***Clickning i OK eller CANCEL-rutan.
*/
     case WM_COMMAND:
     switch ( LOWORD(wp) )
       {
       case IDOK:
       GetDlgItemText(dlg,IDC_EDIT1,buf,V3STRLEN);
       if ( sscanf(buf,"%lf",&cnog) < 1 )
         {
         msmbox("Input error",buf,0);
         return(FALSE);
         }
       dl08.cnog = cnog;
       EndDialog(dlg,wp);
       return(TRUE);

       case IDCANCEL:
       EndDialog(dlg,wp);
       return(TRUE);

       case IDC_BUTTON1:
       ighelp();
       return(TRUE);
       }
     }
   return(FALSE);
 }

/********************************************************/
/*!******************************************************/

        short  msview()

/*      Dialogbox f�r att byta vy.
 *
 *      FV: 0      = Ok.
 *          REJECT = Avbyt.
 *
 *      (C)microform ab 1998-11-18 J. Kjellander
 *
 ******************************************************!*/

 {
 /*
***Lite test av dialogboxar.
*/
   if ( DialogBox(ms_inst,
                  MAKEINTRESOURCE(IDD_DIALOG9),
                  ms_main,
                 (DLGPROC)cbkd09) == IDOK )
     {
     return(0);
     }
   else return(REJECT);
 }

/********************************************************/
/*!******************************************************/

        BOOL APIENTRY cbkd09(dlg,message,wp,lp)
        HWND dlg;
        UINT message;
        WPARAM wp;
        LPARAM lp;

/*      Callback f�r msview()
 *
 *      In: dlg     = Dialogboxens f�nsterid.
 *          message = Typ av meddelande.
 *          wp,lp   = Ytterligare data.
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 8/11/95 J. Kjellander
 *
 ******************************************************!*/
 {
   char     titbuf[V3STRLEN],cancel[V3STRLEN],okey[V3STRLEN],
            help[V3STRLEN];
   char    *nparr[100];
   int      i,x,y,mdx,mdy,ddx,ddy,nnam;
   RECT     mr,dr;
   COLORREF color;

static char dflt[100] = "Default";

   switch ( message )
     {
     case WM_INITDIALOG:
/*
***Rubrik.
*/
     if ( !msgrst("MODULE.TITLE",titbuf) ) strcpy(titbuf,"Ny modul : ");
     strcat(titbuf,jobnam);
     SetWindowText(dlg,titbuf);
/*
***Knapparna l�ngst ner.
*/
     if ( !msgrst("INPUT.OKEY",okey) ) strcpy(okey,"Okey");
     if ( !msgrst("INPUT.REJECT",cancel) ) strcpy(cancel,"Avbryt");
     if ( !msgrst("INPUT.HELP",help) ) strcpy(help,"Hj�lp");

     SetDlgItemText(dlg,IDOK,okey);
     SetDlgItemText(dlg,IDCANCEL,cancel);
     SetDlgItemText(dlg,IDC_BUTTON1,help);
/*
***Fyll listan med vynamn.
*/
     EXgvnl(nparr,&nnam);
     for ( i=0; i<nnam; ++i )
     SendDlgItemMessage(dlg,IDC_COMBO1,CB_ADDSTRING,
                                (WPARAM)0,(LPARAM)nparr[i]);

     SendDlgItemMessage(dlg,IDC_COMBO1,WM_SETTEXT,0,(LPARAM)dflt); 
/*
***Placering mitt i huvudf�nstret.
*/
     GetWindowRect(ms_main,&mr);
     mdx = mr.right - mr.left;
     mdy = mr.bottom - mr.top;
     GetWindowRect(dlg,&dr);
     ddx = dr.right - dr.left;
     ddy = dr.bottom - dr.top;

     x = (mdx - ddx)/2;
     y = (mdy - ddy)/2;

     SetWindowPos(dlg,HWND_TOP,x,y,0,0,SWP_NOSIZE);
     return(TRUE);
/*
***F�rger.
*/
     case WM_CTLCOLORDLG:
     case WM_CTLCOLORBTN:
     color = GetSysColor(COLOR_MENU);
     SetBkColor((HDC)wp,color);
     return((BOOL)GetStockObject(LTGRAY_BRUSH));
/*
***Clickning i OK eller CANCEL-rutan.
*/
     case WM_COMMAND:
     switch ( LOWORD(wp) )
       {
       case IDOK:
       SendDlgItemMessage(dlg,IDC_COMBO1,WM_GETTEXT, sizeof(dflt), 
                        (LPARAM) dflt); 
       case IDCANCEL:
       EndDialog(dlg,wp);
       return(TRUE);
       }
     }
   return(FALSE);
 }

/********************************************************/
