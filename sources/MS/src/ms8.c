/**********************************************************************
*
*    ms8.c
*    =====
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     msmsip();   Multiple string input
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

static int  crewin();   /* Skapar huvudf�nster */
static int  crepag();   /* Skapar en sida */

typedef struct msidat
{
int     scr_dx;            /* Sk�rmens storlek i X-led */
int     scr_dy;            /* Sk�rmens storlek i Y-led */
v2int   iwin_id;           /* Huvudf�nstrets ID */
WPIWIN *iwinpt;            /* C-pekare till huvudf�nster */
int     iwin_dx,iwin_dy;   /* Huvudf�nstrets storlek */
int     tdx,tdy;           /* Huvudpromtens storlek */
int     lx,ly,lm;          /* Luft i olika riktningar */
int     ih;                /* Inmatningsf�ltens h�jd */
int     ah;                /* Alternativrutors h�jd */
v2int   ok_id;             /* Okey-knappens ID */
int     ok_x,ok_y;         /* Okey-knappens placering */
int     ok_dx,ok_dy;       /* Okey-knappens storlek */
v2int   av_id;             /* Avbryt-knappens ID */
int     av_x,av_y;         /* Avbryt-knappens placering */
int     av_dx,av_dy;       /* Avbryt-knappens storlek */
v2int   hlp_id;            /* Hj�lp-knappens ID */
int     hlp_x,hlp_y;       /* Hj�lp-knappens placering */
int     hlp_dx,hlp_dy;     /* Hj�lp-knappens storlek */
v2int   nxt_id;            /* Bl�ddra-knappens ID */
v2int   ed_id[V2MPMX];     /* Inmatningsf�ltens ID */
v2int   pmt_id[V2MPMX];    /* Promtars ID */
v2int   but_id[V2MPMX];    /* Pos/ref-knappars ID */
int     max_ed;            /* Max antal inmatningsf�lt som f�r plats */
int     start_ed;          /* 1:a inmatnigsf�lt p� aktiv sida */
int     ant_ed;            /* Aktivt antal inmatningsf�lt */
int     actpag,pagant;     /* Aktiv sida och antal sidor */
} MSIDAT;

/*
***MSIDAT �r en struct som h�ller alla n�dv�ndiga data om
***en viss inmatning. wpmsip() har en s�dan som lokal variabel
***och kommunicerar med sina underordnade rutiner via en pekare
***till denna. D�rmed �r wpmsip() reentrant.
*/

static short iwin_x = 30;
static short iwin_y = 30;
 
/*
***iwin_x och iwin_y �r huvudf�nstrets l�ge p� sk�rmen.
***Om anv�ndaren under en inmatning flyttar f�nstret uppdateras
***dessa s� att f�nstret n�sta g�ng kommer p� det nya st�llet.
*/

static char title [81];        /* varkon.input.title */
static char page  [81];        /* varkon.input.page  */
static char okey  [81];        /* varkon.input.okey  */
static char reject[81];        /* varkon.input.reject*/
static char next  [81];        /* varkon.input.next  */
static char help  [81];        /* varkon.input.help  */

/*
***Ovanst�ende variabler h�ller texter fr�n resursfilen.
*/
extern short    modtyp;

/*!*******************************************************/

     int   msmsip(dps,ips,ds,is,maxtkn,typarr,nstr)
     char *dps;
     char *ips[];
     char *ds[];
     char *is[];
     short maxtkn[];
     int   typarr[];
     short nstr;

/*   WIN32-versionen av igmsip(). Multiple string input.
 *   Formul�rorienterad inl�sning av en eller flera str�ngar.
 *   Om antalet st�ngar �r st�rre �n vad som f�r plats
 *   p� sk�rmen skapas flera sidor.
 *
 *   In: dps    = Dialogboxens Promptstr�ng
 *       ips    = Inmatningsf�ltens Promtstr�ngar
 *       ds     = Default-str�ngar
 *       is     = Pekare till resultat.
 *       maxtkn = Max l�ngder p� inmatat resultat.
 *       typarr = F�lttyper.
 *       nstr   = Antal str�ngar.
 *
 *   Ut: *is = nstr stycken textstr�ngar.
 *
 *   FV:      0 = Ok.
 *       REJECT = Inmatningen avbruten.
 *
 *   (C)microform ab 29/10/95 J. Kjellander
 *
 *    1996-12-13 typarr, J.Kjellander
 *
 *******************************************************!*/

 {
    MSIDAT  md;    /* Lokala data som delas med andra rutiner */

    v2int    wait_id;
    pm_ptr   expr;
    DBetype  typ;
    bool     right,end;
    int      status,i,j;
    char     edtbuf[V3STRLEN+1],tmpbuf[V3STRLEN+1];
    WPEDIT  *edtptr;


/*
***Vilka texter skall vi ha i f�nsterram och p� knappar ?
*/
   if ( !msgrst("INPUT.TITLE",title) ) strcpy(title,"Inmatning");
   if ( !msgrst("INPUT.PAGE",page) ) strcpy(page,"Sida");
   if ( !msgrst("INPUT.OKEY",okey) ) strcpy(okey,"Okej");
   if ( !msgrst("INPUT.REJECT",reject) ) strcpy(reject,"Avbryt");
   if ( !msgrst("INPUT.NEXT",next) ) strcpy(next,"Bl�ddra");
   if ( !msgrst("INPUT.HELP",help) ) strcpy(help,"Hj�lp");
/*
***Nu skapar vi sj�lva huvudf�nstret.
*/
    crewin(&md,dps,ips,nstr);
/*
***F�r att alla ut-str�ngar s�kert skall f� v�rden
***kopierar vi till att b�rja med default-str�ngarna
***till dem.
*/
    for ( i=0; i<nstr; ++i ) strcpy(is[i],ds[i]);
/*
***Ut med inmatningsf�lten. Vi b�rjar med sida 1.
*/
    md.actpag = 1;

loop1:
     crepag(&md,ips,is,maxtkn,typarr,nstr);
/*
***V�nta p� klickning i knapp eller h�ndelse i edit-f�nster.
*/
loop2:
    mswwtw(md.iwin_id,SLEVEL_V3_INP,&wait_id);
/*
***Klickning i OK-knapp. Med bara ett inmatningsf�lt och
***inget svar (tomt) betyder detta REJECT. Annars OK.
*/
    if ( wait_id == md.ok_id )
      {
      if ( nstr == 1 )
        {
        msgted(md.iwin_id,md.ed_id[0],edtbuf);
        if ( edtbuf[0] == '\0' ) status = REJECT;
        else                     status = 0;
        }
      else status = 0;
      }
/*
***Klickning i Avbryt-knapp.
*/
    else if ( wait_id == md.av_id ) status = REJECT;
/*
***Klickning i Hj�lp-knapp.
*/
    else if ( wait_id == md.hlp_id )
      {
      ighelp();
      goto loop2;
      }
/*
***Klickning i Bl�ddra-knapp.
*/
    else if ( wait_id == md.nxt_id )
      {
      ++md.actpag;
      if ( md.actpag > md.pagant ) md.actpag = 1;
      for ( i=md.start_ed,j=0; i<md.start_ed+md.ant_ed; ++i,++j )
        msgted(md.iwin_id,md.ed_id[j],is[i]);
      goto loop1;
      }
/*
***Ingen av ovanst�ende !
***D� m�ste det vara en Pos/ref-knapp.
*/
    else
      {
      for ( i=0; i<nstr; ++i )
        {
        if ( wait_id == md.but_id[i] )
          {
          ShowWindow(md.iwinpt->id.ms_id,SW_HIDE);
          igplma(ips[i],IG_MESS);
          pmmark();
          if ( typarr[i] == C_VEC_VA )
            status = (int)genpos((short)0,&expr);
          else
            {
            typ = ALLTYP;
            status = (int)genref((short)0,&typ,&expr,&right,&end);
            }
          pmrele();
          igrsma();
          ShowWindow(md.iwinpt->id.ms_id,SW_SHOW);
          if ( status < 0 ) goto loop2;
          pprexs(expr,modtyp,tmpbuf,V3STRLEN);
          edtptr = (WPEDIT *)md.iwinpt->wintab[(wpw_id)(wait_id-1)].ptr;
          SendMessage(edtptr->id.ms_id,WM_SETTEXT,(WPARAM)0,(LPARAM)tmpbuf);
          msfoeb(md.iwinpt,(wpw_id)(wait_id-1),TRUE);
          goto loop2;
          }
        }
      goto loop2;
      }
/*
***Om status = 0 l�ser vi av textf�nstren.
*/
    if ( status == 0 )
      {
      for ( i=md.start_ed,j=0; i<md.start_ed+md.ant_ed; ++i,++j )
        msgted(md.iwin_id,md.ed_id[j],is[i]);
      }
/*
***Dags att d�da hela skiten.
*/
    mswdel(md.iwin_id);
/*
***Slut.
*/

    return(status);
 }

/*********************************************************/
/*!*******************************************************/

     static int crewin(md,dps,ips,nstr)
     MSIDAT *md;
     char *dps;
     char *ips[];
     short nstr;

/*   Skapar huvudf�nster med dom knappar som alltid finns
 *   i ett huvudf�nster.
 *
 *   In: md  = Pekare till data.
 *
 *   Ut: *md = Lagrar ber�knade data.
 *
 *   FV:      0 = Ok.
 *
 *   (C)microform ab 29/10/95 J. Kjellander
 *
 *******************************************************!*/

 {
    char   str1[V3STRLEN+1];  /* Huvudpromt rad 1 */
    char   str2[V3STRLEN+1];  /* Huvudpromt rad 2 */
    int    max,i,x,y,dx,dy;
    v2int  pmt_id;
    int    tdx,tdy;
    WPWIN *winptr;

/*
***Hur stor �r sk�rmen ?
*/
    md->scr_dx = msmwsx();
    md->scr_dy = msmwsy();
/*
***Utg�ngsl�get f�r dialogboxens bredd �r halva sk�rmen.
*/
    md->iwin_dx = (int)(0.5*md->scr_dx);
/*
***Luft i X-led, Y-led, mellan och alternativrutors h�jd.
*/
    md->lx = msstrl("w");
    md->ly = (int)(0.6*msstrh());
    md->lm = (int)(0.5*msstrh());
    md->ih = (int)(1.8*msstrh());
    md->ah = (int)(2.2*msstrh()); 
/*
***1:a steget �r att avg�ra hur bred dialogboxen beh�ver vara.
***Kolla om promttexten skall delas upp i 2 rader.
***L�ngsta raden = md.tdx. Oavsett antal rader �r plats-
***behovet i Y-led = md.tdy.
*/
    msdivs(dps,md->iwin_dx,&tdx,&tdy,str1,str2);
    md->tdx = tdx+5;  /* +5 1996-06-12 JK */
    md->tdy = tdy;
    max = md->tdx;
/*
***Ber�kna st�rsta verkliga platsbehovet f�r inmatnings-promt och
***max antal tecken som f�r matas in.
*/
    for ( i=0; i<nstr; ++i )
      if ( msstrl(ips[i]) > max ) max = msstrl(ips[i]);
      
    if ( 40*msstrl("w") > max ) max = 40*msstrl("w");
/*
***Ber�kna huvudf�nstrets verkliga bredd = dx. Om platsbehovet f�r
***texterna och inmatningsf�nstret �r mindre �n halva sk�rmen g�r
***vi dialogboxen mindre.
*/  
    max   = md->lx + max + md->lx;
    md->iwin_dx = md->lx + md->iwin_dx + md->lx;
    if ( md->iwin_dx > max ) md->iwin_dx = max;
/*
***Ber�kna huvudf�nstrets h�jd.
*/  
    md->max_ed = nstr;
loop:
    md->iwin_dy = md->ly + md->tdy + md->lm + 
                  md->max_ed*(msstrh() + md->lm + md->ih + md->lm) +
                  md->ly + md->ah + md->ly;
/*
***F�r det plats p� sk�rmen ? Om inte, minska antal inmatnings-
***f�lt och prova igen.
*/
    if ( md->iwin_dy > md->scr_dy - GetSystemMetrics(SM_CYCAPTION) -
                                  2*GetSystemMetrics(SM_CYFRAME) )
      {
      md->max_ed -= 1;
      goto loop;
      }
/*
***Ber�kna antal sidor som beh�vs.
*/
    if ( nstr > md->max_ed )
      md->pagant = 1 + (int)((double)nstr/(double)md->max_ed);
    else
      md->pagant = 1;
/*
***Inga inmatningsf�lt p� sk�rmen �nnu.
*/
    md->ant_ed = 0;
/*
***Placering tar vi samma som sist. Om f�nstret �r st�rre den h�r
***g�ngen s� att det sticker ut utanf�r sk�rmen backar vi lite.
*/
    if ( iwin_x + md->iwin_dx + 30 > md->scr_dx )
      iwin_x = md->scr_dx - md->iwin_dx - 30;
    if ( iwin_y + md->iwin_dy + 30 > md->scr_dy )
      iwin_y = md->scr_dy - md->iwin_dy - 30;
/*
***Skapa huvudf�nster i form av ett WPIWIN. 
*/
    mswciw(iwin_x,iwin_y,md->iwin_dx,md->iwin_dy,
                                              "",&md->iwin_id);
    winptr = mswgwp(md->iwin_id);
    md->iwinpt = (WPIWIN *)winptr->ptr;
/*
***Huvudf�nstrets rubrik, noll en eller tv� rader.
***Om dps = "" skall vi inte skapa n�gon button.
*/
    x = md->lx;
    y = md->ly;

    if ( *str1 != '\0' )
      {
      if ( *str2 == '\0' )
        msmcbu((wpw_id)md->iwin_id,x,y,md->tdx,md->tdy,0,
                                str1,"","",WP_BGND,WP_FGND,&pmt_id);
      else
        {
        msmcbu((wpw_id)md->iwin_id,x,y,md->tdx,md->tdy/2,0,
                                str1,"","",WP_BGND,WP_FGND,&pmt_id);
        msmcbu((wpw_id)md->iwin_id,x,y+md->tdy/2,md->tdx,md->tdy/2,0,
                                str2,"","",WP_BGND,WP_FGND,&pmt_id);
        }
      }
/*
***Ber�kna Ok-f�nstrets storlek och placering och skapa.
*/
    if ( msstrl(okey) > msstrl(reject) )
      md->ok_dx = md->lx + msstrl(okey) + md->lx;
    else
      md->ok_dx = md->lx + msstrl(reject) + md->lx;
    md->ok_dy = md->ah;
    md->ok_x  = md->lx;
    md->ok_y  = md->ly + md->tdy + md->lm +
                md->max_ed*(msstrh() + md->lm + md->ih + md->lm) + md->ly;

    mscrdb((wpw_id)md->iwin_id,md->ok_x,md->ok_y,md->ok_dx,md->ok_dy,
                        2,okey,"",WP_BGND,WP_FGND,&md->ok_id);
/*
***Ber�kna Reject-f�nstrets storlek och placering och skapa.
*/
    md->av_dx = md->lx + msstrl(reject) + md->lx;
    md->av_dy = md->ah;
    md->av_x  = md->lx + md->ok_dx + md->lx;
    md->av_y  = md->ok_y;

    msmcbu((wpw_id)md->iwin_id,md->av_x,md->av_y,md->av_dx,md->av_dy,
                  2,reject,reject,"",WP_BGND,WP_FGND,&md->av_id);
/*
***Ev. bl�ddra-knapp.
*/
    if ( md->pagant > 1 )
      {
      dx = md->lx + msstrl(next) + md->lx;
      dy = md->ah;
      x  = md->lx + md->ok_dx + md->lx + md->av_dx + md->lx;
      y  = md->ok_y;

      msmcbu((wpw_id)md->iwin_id,x,y,dx,dy,
               2,next,next,"",WP_BGND,WP_FGND,&md->nxt_id);
      }
/*
***Ber�kna Hj�lp-f�nstrets storlek och placering och skapa.
*/
    md->hlp_dx = md->lx + msstrl(help) + md->lx;
    md->hlp_dy = md->ah;
    md->hlp_x  = md->iwin_dx - md->lx - md->hlp_dx;
    md->hlp_y  = md->ok_y;

    msmcbu((wpw_id)md->iwin_id,md->hlp_x,md->hlp_y,md->hlp_dx,md->hlp_dy,
                2,help,help,"",WP_BGND,WP_FGND,&md->hlp_id);
/*
***Mappa manuellt.  wpwshw() s�tter iwinpt->mapped = TRUE,
***det vill vi inte eftersom vi vill mappa alla edit-f�nster
***samtidigt med ett anrop och inte automatiskt n�r de skapas.
*/
    mswshw(md->iwinpt->id.w_id);
/*
***Slut.
*/
    return(0);
 }

/*********************************************************/
/*!*******************************************************/

     static int crepag(md,ips,is,maxtkn,typarr,nstr)
     MSIDAT *md;
     char *ips[];
     char *is[];
     short maxtkn[];
     int   typarr[];
     short nstr;

/*   L�ser in en sida texter med redigering i dialogbox.
 *
 *   In: md     = Pekare till gemensamma data
 *       ips    = Inmatningsf�ltens Promtstr�ngar
 *       is     = Pekare till resultat.
 *       maxtkn = Max l�ngder p� inmatat resultat.
 *       typarr = F�lttyper.
 *       nstr   = Antal str�ngar.
 *
 *   Ut: *is = nstr stycken textstr�ngar.
 *
 *   FV:      0 = Ok.
 *       REJECT = Inmatningen avbruten.
 *
 *   (C)microform ab 29/10/95 J. Kjellander
 *
 *   1996-12-09 msfoeb(), J.Kjellander
 *   1996-12-13 typarr, J.Kjellander
 *   1997-01-15 Bug bl�ddring, J.Kjellander
 *
 *******************************************************!*/

 {
    char    sidbuf[80];
    int     i,j,x,y,dx,dy,pdx;
    WPEDIT *edtptr;
    WPBUTT *butptr;
    wpw_id  edbuid;
    HWND    win32_id;

/*
***D�da nuvarande edit-f�nster, promtar samt ev. knappar
***f�r VECTOR/REF-parametrar.
*/
    for ( i=0; i<md->ant_ed; ++i )
      {
      edtptr = (WPEDIT *)md->iwinpt->wintab[md->ed_id[i]].ptr;
      win32_id = edtptr->id.ms_id;
      DestroyWindow(win32_id);
      msdled(edtptr);
      md->iwinpt->wintab[md->ed_id[i]].ptr = NULL;
      md->iwinpt->wintab[md->ed_id[i]].typ = TYP_UNDEF;
      
      butptr = (WPBUTT *)md->iwinpt->wintab[md->pmt_id[i]].ptr;
      win32_id = butptr->id.ms_id;
      DestroyWindow(win32_id);
      msdlbu(butptr);
      md->iwinpt->wintab[md->pmt_id[i]].ptr = NULL;
      md->iwinpt->wintab[md->pmt_id[i]].typ = TYP_UNDEF; 

      if ( md->but_id[i] > 0 )
        {
        butptr = (WPBUTT *)md->iwinpt->wintab[md->but_id[i]].ptr;
        win32_id = butptr->id.ms_id;
        DestroyWindow(win32_id);
        msdlbu(butptr);
        md->iwinpt->wintab[md->but_id[i]].ptr = NULL;
        md->iwinpt->wintab[md->but_id[i]].typ = TYP_UNDEF;
        }
      }
/*
***Vilka f�lt skall visas p� denna sida ?
*/
    md->start_ed = (md->actpag - 1)*md->max_ed;
    md->ant_ed   = nstr - md->start_ed;

    if ( md->ant_ed > md->max_ed ) md->ant_ed = md->max_ed;
/*
***Ber�kna inmatnings-f�nstrens placering och
***skapa dem. Om promttexter finns, skapa en label med promt,
***annars skapa ett litet tomt f�nster, pdx=1.
***Se till att inmatningsf�nstrets l�ngd s�kert blir > 0.
*/
    x = md->lx;
    y = md->ly + md->tdy +md->lm + msstrh() + md->lm;
    dx = md->iwin_dx - md->lx - md->lx - md->ih - 3;
    dy = md->ih;

    for ( i=md->start_ed,j=0; i<md->start_ed+md->ant_ed; ++i,++j )
      {
/*
***Promten.
*/
      if ( *ips[i] != '\0' ) pdx = msstrl(ips[i])+5;   /* +5 1996-06-12 JK */
      else pdx = 1;
      msmcbu(md->iwin_id,x,y-msstrh()-md->lm,pdx,msstrh(),
                 0,ips[i],"","",WP_BGND,WP_FGND,&md->pmt_id[j]);
/*
***Edit-f�ltet.
*/     
      if ( maxtkn[i] < 1 ) maxtkn[i] = 1;
      msmced(md->iwin_id,x,y,dx,dy,2,is[i],maxtkn[i],&md->ed_id[j]);
/*
***Eventuell Pos/ref-knapp.
*/
      if ( typarr[i] == C_VEC_VA  ||  typarr[i] == C_REF_VA )
        msmcbu(md->iwin_id,x+dx+2,y,md->ih+1,md->ih+1,
                 1,"V","V","",WP_BGND,WP_FGND,&md->but_id[j]);
      else md->but_id[j] = -1;
/*
***N�sta f�lt.
*/
      y += msstrh() + md->lm + dy + md->lm;
      }
/*
***Mappa de nya f�nstren och s�tt input focus p� det f�rsta.
*/
    edbuid = msffoc(md->iwinpt,FIRST_EDIT);
    if ( edbuid > -1 ) msfoeb(md->iwinpt,edbuid,TRUE);
/*
***Utskrift av aktiv sida i WM-ramen.
*/
    if ( md->pagant > 1 )
      {
      sprintf(sidbuf,"%s   %s %d(%d)",title,page,md->actpag,md->pagant);
      SetWindowText(md->iwinpt->id.ms_id,sidbuf);
      }
    else
      SetWindowText(md->iwinpt->id.ms_id,title);
/*
***Slut.
*/
    return(0);
 }

/*********************************************************/
