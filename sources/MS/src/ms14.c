/**********************************************************************
*
*    ms14.c
*    ======
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     msmcic();   Create WPICON, CRE_ICON in MBS
*     mscrfi();   Create WPICON, CRE_FICON in MBS
*     mswcic();   Create WPICON
*     msrpic();   Repaint WPICON
*     msbtic();   Button handler
*     msdlic();   Delete WPICON
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

/*!******************************************************/

        int     msmcic(pid,x,y,bw,fnam,cb,cf,iid)
        wpw_id  pid;
        int   x,y,bw;
        char   *fnam;
        int   cb,cf;
        v2int  *iid;

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
 *      (C)microform ab 30/11/95 J. Kjellander
 *
 ******************************************************!*/

  {
    char    errbuf[80];
    int     i,status;
    WPWIN  *winptr;
    WPIWIN *iwinptr;
    WPICON *icoptr;

/*
***Fixa C-pekare till f�r�lderns entry i wpwtab.
*/
    if ( (winptr=mswgwp(pid)) == NULL )
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
    if ( (status=mswcic(iwinptr->id.ms_id,x,y,bw,fnam,cb,cf,
                                        &icoptr)) < 0 ) return(status);
/*
***L�nka in den i WPIWIN-f�nstret.
*/
    iwinptr->wintab[*iid].typ = TYP_ICON;
    iwinptr->wintab[*iid].ptr = (char *)icoptr;

    icoptr->id.w_id = *iid;
    icoptr->id.p_id =  pid;
/*
***Visa resultatet.
*/
    ShowWindow(icoptr->id.ms_id,SW_SHOWNORMAL);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        int     mscrfi(pid,x,y,filnam,akod,anum,iid)
        wpw_id  pid;
        int     x,y;
        char   *filnam;
        char   *akod;
        int     anum;
        v2int  *iid;

/*      Skapar snabbvalsikon i grafiskt f�nster, CRE_FICON.
 *
 *      In: pid    = F�r�lder.
 *          x,y    = Placering.
 *          filnam = V�gbeskrivning till ikonfil.
 *          akod   = Aktionskod.
 *          anum   = Aktionsnummer.
 *          iid    = Pekare till utdata.
 *
 *      Ut: *bid = Giltigt entry i f�r�lderns wintab.
 *
 *      Felkod: WP1522 = F�r�ldern %s finns ej.
 *              WP1532 = F�r�ldern %s �r av fel typ.
 *              WP1542 = F�r m�nga subf�nster i %s.
 *              WP1552 = %s �r en ok�nd aktionskod.
 *
 *      (C)microform ab 1996-05-20 J. Kjellander
 *
 ******************************************************!*/

  {
    char    errbuf[80];
    int     i,status,action;
    WPWIN  *winptr;
    WPGWIN *gwinpt;
    WPICON *icoptr;

/*
***Vilken aktionskod ?
*/
    if ( akod[1] != '\0' ) return(erpush("WP1552",akod));

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
***Fixa C-pekare till f�r�lderns entry i wpwtab.
*/
    if ( (winptr=mswgwp(pid)) == NULL )
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
***Skapa ikonen.
*/
    status = mswcic(gwinpt->id.ms_id,x,y,1,
                    filnam,WP_BGND,WP_FGND,&icoptr);
    if ( status < 0 ) return(status);
/*
***L�nka in den i WPGWIN-f�nstret.
*/
    gwinpt->wintab[*iid].typ = TYP_ICON;
    gwinpt->wintab[*iid].ptr = (char *)icoptr;

    icoptr->id.w_id = *iid;
    icoptr->id.p_id =  pid;
/*
***Aktion.
*/
    icoptr->acttyp = action;
    icoptr->actnum = anum;
/*
***Visa resultatet.
*/
    ShowWindow(icoptr->id.ms_id,SW_SHOWNORMAL);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        int      mswcic(ms_pid,x,y,bw,fnam,cb,cf,outptr)
        HWND     ms_pid;
        int      x,y,bw;
        char    *fnam;
        int      cb,cf;
        WPICON **outptr;

/*      Skapar WPICON-f�nster.
 *
 *      In: ms_pid = F�r�ldra f�nstrets WIN32-id.
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
 *      (C)microform ab 1996-04-19 J. Kjellander
 *
 *      1997-12-17 Bugfix VC version 5, J.Kjellander
 *
 ******************************************************!*/

  {
    int     dx,dy,colant;
    void   *pixptr,*colptr,*bmiptr;
    size_t  n;
    FILE   *fp;
    HWND    win32_id;
    WPICON *icoptr;
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;

/*
***�ppna BMP-filen.
*/
   if ( v3facc(fnam,'R') )
     {
     fp = fopen(fnam,"rb");
/*
***L�s Bitmapfileheader.
*/
     fread((char *)&bmfh,sizeof(bmfh),1,fp);
/*
***L�s Bitmapinfoheader.
*/
     fread((char *)&bmih,sizeof(bmih),1,fp);
/*
***Hur m�nga f�rger ing�r i ev. f�rgtabell.
*/
     if ( bmih.biClrUsed > 0 ) colant = bmih.biClrUsed;
     else
       {
       switch ( bmih.biBitCount )
         {
         case 1:  colant =   2; break;
         case 4:  colant =  16; break;
         case 8:  colant = 256; break;
         default: colant =   0; break;
         }
       }
/*
***L�s ev. f�rgtabell.
*/
     if ( colant > 0 )
       {
       if ( (colptr=v3mall(colant*sizeof(RGBQUAD),"mswcic")) == NULL )
          return(erpush("WP1292",fnam));
       n = fread(colptr,sizeof(RGBQUAD),colant,fp);
       }
     else colptr = NULL;
/*
***Allokera minne f�r pixlar och l�s in dom.
*/
     if ( (pixptr=v3mall(bmih.biSizeImage,"mswcic")) == NULL )
       return(erpush("WP1292",fnam));
     fseek(fp,bmfh.bfOffBits,SEEK_SET);
     n = fread(pixptr,bmih.biSizeImage,1,fp);
     fclose(fp);
     }
   else return(erpush("WP1212",fnam));
/*
***F�nstrets storlek = bitmappens storlek + f�nsterram. Av n�gon
***mystisk orsak returnerar GetSystemMetrics v�rdet 2 men om man
***tittar p� sk�rmen med f�rstoringsglas ser man att det �r 3 pixels
***tjock ram, allts� l�gger vi till 2 extra tills vi rett ut vad det
***beror p�!!!!OBS OBS OBS OBS!!!!!
*/
/*
***Borttaget igen 1997-12-17. Verkar ha varit en bug i VC++
***version 2 som r�ttats i version 5.
   dx = bmih.biWidth + 2*GetSystemMetrics(SM_CXDLGFRAME) + 2;
   dy = abs(bmih.biHeight) + 2*GetSystemMetrics(SM_CYDLGFRAME) + 2;
*/

   dx = bmih.biWidth + 2*GetSystemMetrics(SM_CXDLGFRAME);
   dy = abs(bmih.biHeight) + 2*GetSystemMetrics(SM_CYDLGFRAME);
/*
***Skapa WIN32-f�nster.
*/
   win32_id = CreateWindow(VARKON_ICON_CLASS,
                           "",
   		  		   		   WS_CHILD | WS_DLGFRAME,
		  				   x,
		  			       y,
		  				   dx,
		  				   dy,
		  				   ms_pid,
		  				   NULL,
						   ms_inst,
						   NULL);
/*
***Skapa en WPICON.
*/
    if ( (icoptr=(WPICON *)v3mall(sizeof(WPICON),"mswcic")) == NULL )
       return(erpush("WP1292",fnam));

    icoptr->id.w_id  = NULL;
    icoptr->id.p_id  = NULL;
    icoptr->id.ms_id = win32_id;

    icoptr->geo.x =  x;
    icoptr->geo.y =  y;
    icoptr->geo.dx = dx;
    icoptr->geo.dy = dy;
/*
***Skapa BITMAPINFO = BITMAPINFOHEADER + ev. f�rgtabell.
*/
    if ( (bmiptr=v3mall(bmih.biSize + colant*sizeof(RGBQUAD),
      "mswcic")) == NULL ) return(erpush("WP1292",fnam));
    V3MOME(&bmih,bmiptr,bmih.biSize);
    V3MOME(colptr,(char *)bmiptr+bmih.biSize,colant*sizeof(RGBQUAD));
    v3free((char *)colptr,"mswcic");
/*
***Ikonens DIB = BITMAPINFO + pixlar.
*/
    icoptr->bmiptr = bmiptr;
    icoptr->pixptr = pixptr;

   *outptr = icoptr;
/*
***Slut.
*/
    return(0);
  }

/********************************************************/
/*!******************************************************/

        bool msrpic(icoptr)
        WPICON *icoptr;

/*      Repaint-rutin f�r WPICON.
 *
 *      In: icoptr = C-pekare till WPICON.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 1996-04-19 J. Kjellander
 *
 ******************************************************!*/

  {
    int         nlines,dx,dy;
	 HDC         dc;
    PAINTSTRUCT ps;

#ifdef DEBUG
   if ( dbglev(WINPAC) == 14 )
     {
     fprintf(dbgfil(WINPAC),"***Start-msrpic***\n");
     fprintf(dbgfil(WINPAC),"win32_id=%d\n",icoptr->id.ms_id);
     fflush(dbgfil(WINPAC));
     }
#endif

/*
***Ikonens DC.
*/
   dc = BeginPaint(icoptr->id.ms_id,&ps);
/*
***Kopiera bitmappen.
*/
   dx = icoptr->bmiptr->bmiHeader.biWidth;
   dy = icoptr->bmiptr->bmiHeader.biHeight;

   nlines = SetDIBitsToDevice(dc,0,0,dx,dy,
                              0,0,0,dy,
                              icoptr->pixptr,
                              icoptr->bmiptr,
                              NULL);
/*
***Sl�pp DC't.
*/
   EndPaint(icoptr->id.ms_id,&ps);
/*
***Slut.
*/
#ifdef DEBUG
   if ( dbglev(WINPAC) == 14 )
     {
     fprintf(dbgfil(WINPAC),"nlines=%d\n",nlines);
     fprintf(dbgfil(WINPAC),"***Slut-msrpic***\n\n");
     fflush(dbgfil(WINPAC));
     }
#endif

    return(TRUE);
  }

/********************************************************/
/*!******************************************************/

        bool msbtic(icoptr)
        WPICON *icoptr;

/*      Button-rutin f�r WPICON.
 *
 *      In: icoptr = C-pekare till WPICON.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 1996-04-19 J. Kjellander
 *
 ******************************************************!*/

  {
    return(TRUE);
  }

/********************************************************/
/*!******************************************************/

        int     msdlic(icoptr)
        WPICON *icoptr;

/*      D�dar en WPICON.
 *
 *      In: icotptr = C-pekare till WPICON.
 *
 *      Ut: Inget.   
 *
 *      Felkod: .
 *
 *      (C)microform ab 1996-04-19 J. Kjellander
 *
 ******************************************************!*/

  {

/*
***L�mna tillbaks DIB:en.
*/
    v3free((char *)(icoptr->bmiptr),"msdlic");
    v3free((char *)(icoptr->pixptr),"msdlic");
/*
***L�mna tillbaks dynamiskt allokerat minne.
*/
    v3free((char *)icoptr,"msdlic");

    return(0);
  }

/********************************************************/
