/**********************************************************************
*
*    wp9.c
*    ====
*
*    This file is part of the VARKON WindowPac Library.
*    URL: http://www.varkon.com
*
*    This file includes:
*
*    wpialt();   Input alternative
*    wpilst();   Input from list
*    wpilse();   Input from list with edit
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
#include <math.h>

/*!*******************************************************/

     bool wpialt(
     char *ps,
     char *ts,
     char *fs,
     bool  pip)

/*   L�ser in alternativ (1 tecken) typ j/n, +/- etc.
 *
 *   In: ps  = Promptstr�ng
 *       ts  = TRUE-str�ng
 *       fs  = FALSE-str�ng
 *       pip = Inledande pip, Ja/Nej
 *
 *   Ut: Inget.
 *
 *   FV: TRUE  om svar = 1:a tecknet i tsnum
 *       FALSE om svar = 1:a tecknet i fsnum
 *
 *   (C)microform ab 24/6/92 J. Kjellander
 *
 *   12/12/93 Omarbetad, J. Kjellander
 *   7/11/95  Resurs f�r titel, J. Kjellander
 *   1996-02-09 Edit och geometry-resurs, J. Kjellander
 *
 *******************************************************!*/

 {
    char     tkn;
    short    status,x,y,dx,dy,lx,ly,px,py,ay,lm;
    char     title[81],ps1[V3STRLEN+1],ps2[V3STRLEN+1];
    v2int    iwin_id,pmt_id,true_id,false_id,but_id,edit_id;
    bool     svar;
    int      ix,iy,tdx,tdy;
    unsigned int dum1,dum2;
    char    *type[20];
    XrmValue value;

/*
***Ev. inledande pip.
*/
    if ( pip ) XBell(xdisp,100);
/*
***F�r att nu kunna best�mma storleken p� sj�lva
***alternativ-f�nstret utg�r vi fr�n promt-str�ngen,
***och ber�knar promt-f�nstrets storlek. En l�ng promt
***kan beh�va delas i 2 rader. Max storlek i X-led �r
***80% av sk�rmen. Blir f�nstret st�rre delar vi.
*/
    px = (short)(1.1*wpstrl(ps));
    py = (short)(1.5*wpstrh());
/*
***Vi utg�r fr�n att promten �r det som tar mest
***plats i X-led och ber�knar alternativf�nstrets l�ngd
***d�refter.
*/
    lx = wpstrl(" ");
    dx = lx + px + lx;

    if ( dx > 0.8*DisplayWidth(xdisp,xscr) ) 
      {
      wpdivs(ps,(int)(0.8*DisplayWidth(xdisp,xscr)),&tdx,&tdy,ps1,ps2);
      px = tdx;
      py = tdy;
      dx = lx + px + lx;
      }
    else
      ps1[0] = '\0';
/*
***Ber�kna luft yttre, knapparnas h�jd, luft mellan och
***huvudf�nstrets h�jd.
*/  
    ly = wpstrh();
    ay = (short)(3.0*wpstrh()); 
    lm = (short)(0.8*ly);

    dy = ly + py + lm + ay + ly;  
/*
***Alternativf�nstrets l�ge.
*/
    ix  = (DisplayWidth(xdisp,xscr)-dx)/2;
    iy  = (DisplayHeight(xdisp,xscr)-dy)/2;

    if ( XrmGetResource(xresDB,"varkon.alternative.geometry",
                               "Varkon.Alternative.Geometry",
                                                type,&value) )
      XParseGeometry((char *)value.addr,&ix,&iy,&dum1,&dum2);

    x = (short)ix;
    y = (short)iy;
/*
***F�nstertitel.
*/
    if ( !wpgrst("varkon.alternative.title",title) )
      strcpy(title,"Alternativ");
/*
***Skapa sj�lva alternativf�nstret som ett WPIWIN.
*/
    status = wpwciw(x,y,dx,dy,title,&iwin_id);
/*
***Skapa promt, om ps1 <> "" kr�vs 2 rader. 
*/
    x  = lx;
    y  = ly;
    dx = px;
    dy = py;

    if ( ps1[0] == '\0' )
      status = wpmcbu((wpw_id)iwin_id,x,y,dx,dy,(short)0,
                              ps,"","",WP_BGND,WP_FGND,&pmt_id);
    else
      {
      status = wpmcbu((wpw_id)iwin_id,x,y,dx,dy/2,(short)0,
                              ps1,"","",WP_BGND,WP_FGND,&pmt_id);
      status = wpmcbu((wpw_id)iwin_id,x,y+dy/2,dx,dy/2,(short)0,
                              ps2,"","",WP_BGND,WP_FGND,&pmt_id);
      }
/*
***Skapa true-knapp.
*/
    x = 0.2*px;
    y  = ly + py + lm;
    if ( wpstrl(ts) > wpstrl(fs) ) dx = 1.5*wpstrl(ts);
    else                           dx = 1.5*wpstrl(fs);
    if ( dx < 6*wpstrl(" ") )      dx = 6*wpstrl(" ");
    dy = ay;

    status = wpmcbu((wpw_id)iwin_id,x,y,dx,dy,(short)3,
                            ts,ts,"",WP_BGND,WP_FGND,&true_id);
/*
***Skapa FALSE-knapp.
*/
    x  = lx + px  - (short)(0.2*px) - dx;

    status = wpmcbu((wpw_id)iwin_id,x,y,dx,dy,(short)3,
                            fs,fs,"",WP_BGND,WP_FGND,&false_id);
/*
***En o�ndligt liten raml�s edit f�r att kunna l�sa tecken.
*/
   status = wpmced((wpw_id)iwin_id,1,1,1,1,(short)0,
                   "",1,&edit_id);
/*
***Klart f�r visning.
*/
    wpwshw(iwin_id);
/*
***V�nta p� action. Service-niv� f�r key-event = SLEVEL_ALL
***s� att vi kan ta hand om varje tecken.
*/
loop:
    status = wpwwtw(iwin_id,SLEVEL_ALL,&but_id);
/*
***Har det h�nt n�t i TRUE- eller FALSE-f�nstret ?
*/
    if      ( but_id == true_id  ) svar = TRUE;
    else if ( but_id == false_id ) svar = FALSE;
/*
***Eller i det lilla osynliga edit-f�nstret ?
*/
    else if ( but_id == edit_id  )
      {
      wpgted(iwin_id,edit_id,&tkn);
      if      ( tkn == *ts ) svar = TRUE;
      else if ( tkn == *fs ) svar = FALSE;
      else
        {
        XBell(xdisp,100);
        goto loop;
        }
      }
    else goto loop;
/*
***Dags att sluta.
*/
    status = wpwdel(iwin_id);

    return(svar);
  }

/********************************************************/
/*!*******************************************************/

     short wpilst(
     int   x,
     int   y,
     char *rubrik,
     char *strlst[],
     int   actalt,
     int   nstr,
     int  *palt)

/*   Visar en lista med alternativ och returnerar vilket
 *   alternativ som valts eller status.
 *
 *   In: x,y    => F�nstrets l�ge
 *       rubrik => F�nsterrubrik
 *       strlst => Array med pekare till alternativtexter
 *       actalt => Aktivt alternativ eller -1 f�r inget.
 *       nstr   => Antal alternativ
 *       palt   => Pekare till utdata
 *
 *   Ut: *alt = Det valda alternativet.
 *
 *   FV:      0 = OK.
 *       REJECT = Avbryt.
 *
 *   (C)microform ab 3/1/95 J. Kjellander
 *
 *******************************************************!*/

 {
   char    *str,reject[81],help[81];
   int      i,rad,radant,main_x,main_y;
   short    status,main_dx,main_dy,alt_x,alt_y,alth,altlen,ly,lm;
   v2int    iwin_id,alt_id[WP_IWSMAX],but_id,help_id,reject_id;

/*
***Texter f�r avbryt och hj�lp.
*/
   if ( !wpgrst("varkon.input.reject",reject) ) strcpy(reject,"Avbryt");
   if ( !wpgrst("varkon.input.help",help) )     strcpy(help,"Hj{lp");
/*
***F�r att nu kunna best�mma storleken p� sj�lva
***alternativ-f�nstret utg�r vi fr�n  2 kolumner med
***knappar d�r knapparnas l�ngd best�ms av den l�ngsta
***alternativtexten.
*/
   altlen = 0;

   for ( i=0; i<nstr; ++i )
     if ( wpstrl(strlst[i]) > altlen ) altlen = wpstrl(strlst[i]);

   if ( wpstrl(reject) > altlen ) altlen = wpstrl(reject);
   if ( wpstrl(help)   > altlen ) altlen = wpstrl(help);

   altlen *= 1.3;
/*
***Hur m�nga rader blir det ?
*/
   radant = (int)(ceil((nstr/2.0)-0.1));
/*
***Ber�kna luft yttre, knapparnas h�jd, luft mellan och
***huvudf�nstrets h�jd.
*/
   ly   = (short)(0.8*wpstrh());
   alth = (short)(1.6*wpstrh()); 
   lm   = (short)(1.4*wpstrh());

   main_dx = ly + altlen + lm + altlen + ly;
   main_dy = ly + radant*(alth + ly) +  ly + 2*wpstrh() + ly;  
/*
***Skapa sj�lva alternativf�nstret som ett WPIWIN.
*/
   wpposw(x,y,main_dx+10,main_dy+25,&main_x,&main_y);
   wpwciw((short)main_x,(short)main_y,main_dx,main_dy,rubrik,&iwin_id);
/*
***Skapa alternativen.
*/
   for ( rad=0; rad<radant; ++rad )
     {
/*
***Knapp i v�nstra kolumnen.
*/
     alt_x  = ly;
     alt_y  = ly + rad*(alth + ly);
     str    = strlst[2*rad];

     if ( 2*rad == actalt )
       status = wpmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,(short)1,
                               str,str,"",WP_FGND,WP_BGND,&alt_id[2*rad]);
     else
       status = wpmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,(short)1,
                               str,str,"",WP_BGND,WP_FGND,&alt_id[2*rad]);
/*
***Knapp i h�gra kolumnen.
*/
     if ( nstr > (2*rad)+1 )
       {
       alt_x  = ly + altlen + lm;
       str    = strlst[(2*rad)+1];

       if ( (2*rad)+1 == actalt )
         status = wpmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,(short)1,
                          str,str,"",WP_FGND,WP_BGND,&alt_id[(2*rad)+1]);
       else
         status = wpmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,(short)1,
                          str,str,"",WP_BGND,WP_FGND,&alt_id[(2*rad)+1]);
       }
     }
/*
***Avbryt och hj�lp.
*/
   alt_x  = ly;
   alt_y  = ly + radant*(alth + ly) + ly;
   alth   = 2*wpstrh();
   status = wpmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,(short)2,
                           reject,reject,"",WP_BGND,WP_FGND,&reject_id);

   alt_x  = ly + altlen + lm;
   status = wpmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,(short)2,
                           help,help,"",WP_BGND,WP_FGND,&help_id);
/*
***Klart f�r visning.
*/
   wpwshw(iwin_id);
/*
***V�nta p� action. Service-niv� f�r key-event saknar
***intresse i sammanhanget.
*/
   status = 0;
loop:
   wpwwtw(iwin_id,SLEVEL_NONE,&but_id);
/*
***Avbryt.
*/
   if ( but_id == reject_id ) status = REJECT;
/*
***Hj�lp.
*/
   else if ( but_id == help_id )
     {
     ighelp();
     goto loop;
     }
/*
***Ett alternativ.
*/
   else
     {
     for ( i=0; i < nstr; ++i )
       {
       if ( but_id == alt_id[i] )
         {
        *palt = i;
         goto exit;
         }
       }
     goto loop;
     }
/*
***Dags att sluta.
*/
exit:
   wpwdel(iwin_id);

   return(status);
 }

/********************************************************/
/*!*******************************************************/

     short wpilse(
     int   x,
     int   y,
     char *rubrik,
     char *defstr,
     char *strlst[],
     int   actalt,
     int   nstr,
     char *altstr)

/*   Visar en lista med alternativ samt ett inmatningsf�lt
 *   l�ngst ned.
 *
 *   In: x,y    => F�nstrets l�ge
 *       rubrik => F�nsterrubrik
 *       defstr => Default text i editen.
 *       strlst => Array med pekare till alternativtexter
 *       actalt => Aktivt alternativ eller -1 f�r inget.
 *       nstr   => Antal alternativ
 *       altstr => Pekare till utdata
 *
 *   Ut: *altstr = Det valda alternativet.
 *
 *   FV:      0 = OK.
 *       REJECT = Avbryt.
 *
 *   (C)microform ab 25/9/95 J. Kjellander
 *
 *   1988-04-01 defstr mm., J.Kjellander
 *   2004-09-03 JNLGTH->31, J.Kjellander, �rebro university
 *
 *******************************************************!*/

  {
   char    *str,okey[81],reject[81],help[81];
   int      i,rad,radant,main_x,main_y,width,height,kol,kolant,
            alt;
   short    status,main_dx,main_dy,alt_x,alt_y,alth,altlen,
            edtlen,ly,lm;
   v2int    iwin_id,alt_id[WP_IWSMAX],but_id,okey_id,help_id,
            reject_id,edit_id;

/*
***Texter f�r ok, avbryt och hj�lp.
*/
   if ( !wpgrst("varkon.input.okey",okey) )     strcpy(okey,"Okej");
   if ( !wpgrst("varkon.input.reject",reject) ) strcpy(reject,"Avbryt");
   if ( !wpgrst("varkon.input.help",help) )     strcpy(help,"Hj�lp");
/*
***Hur l�ng �r den l�ngsta alternativtexten.
***Alternativrutor samt ok, avbryt och hj�lp g�rs altlen*1.3.
*/
   altlen = 0;

   for ( i=0; i<nstr; ++i )
     if ( wpstrl(strlst[i]) > altlen ) altlen = wpstrl(strlst[i]);

   if ( wpstrl(okey)   > altlen ) altlen = wpstrl(okey);
   if ( wpstrl(reject) > altlen ) altlen = wpstrl(reject);
   if ( wpstrl(help)   > altlen ) altlen = wpstrl(help);

   altlen *= 1.3;
/*
***Hur l�ngt skall inmatningsf�nstret vara ?
*/
   edtlen = (short)(JNLGTH*wpstrl("A")*1.3);
/*
***Ber�kna luft yttre, knapparnas h�jd och luft mellan.
*/
   ly   = (short)(0.5*wpstrh());
   alth = (short)(1.6*wpstrh()); 
   lm   = (short)(1.0*wpstrh());
/*
***Hur stor �r sk�rmen ?
*/
   width  = DisplayWidth(xdisp,xscr);
   height = DisplayHeight(xdisp,xscr);
/*
***Vi utg�r fr�n minst 3 kolumner.
***Hur m�nga rader blir det med 3 kolumner ?
*/
   kolant = 3;

kloop:
   radant = (int)(ceil((nstr/(double)kolant)-0.1));
/*
***Ber�kna huvudf�nstrets storlek.
*/
   main_dx = ly + (kolant-1)*(altlen + lm) + altlen + ly;
   main_dy = ly + (radant+1)*(alth + ly) +  2*ly + 2*wpstrh() + ly;  
/*
***F�r det plats i X-led. Om inte kan vi inget g�ra. D�
***f�r vi n�ja oss med att inte visa alltihop.
*/
   if ( main_dx > width )
     {
   --kolant;
     nstr = radant*kolant + 1;
     strcpy(strlst[nstr-1],"?????");
     goto kloop;
     }
/*
***F�r det plats i Y-led ? Om inte provar vi med fler kolumner.
*/
   else if ( main_dy > height-30 )
     {
   ++kolant;
     goto kloop;
     }
/*
***F�r rubriken plats ?
*/
   if ( wpstrl(rubrik) >  main_dx-30 ) main_dx = wpstrl(rubrik)+30;
   if ( main_dx > width-30 ) main_dx = width - 30;
/*
***Skapa sj�lva alternativf�nstret som ett WPIWIN.
*/
   wpposw(x,y,main_dx+10,main_dy+25,&main_x,&main_y);
   wpwciw((short)main_x,(short)main_y,main_dx,main_dy,rubrik,&iwin_id);
/*
***Skapa alternativen.
*/
   for ( rad=0; rad<radant; ++rad )
     {
     alt_y  = ly + rad*(alth+ly);

     for ( kol=0; kol<kolant; ++kol )
       {
       alt_x  = ly + kol*(altlen+lm);
       alt    = rad*kolant + kol;
       str    = strlst[alt];

       if ( alt < nstr )
         {
         if ( alt == actalt )
           status = wpmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,(short)1,
                                   str,str,"",WP_FGND,WP_BGND,&alt_id[alt]);
         else
           status = wpmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,(short)1,
                                   str,str,"",WP_BGND,WP_FGND,&alt_id[alt]);
         }
       }
     }
/*
***Edit-f�ltet.
*/
   if ( edtlen > main_dx - 2*ly) edtlen = main_dx - 2* ly;
   
   alt_x  = (main_dx-edtlen)/2;
   alt_y  = ly + radant*(alth + ly) + ly;

   strcpy(altstr,defstr);
   status = wpmced((wpw_id)iwin_id,alt_x,alt_y,edtlen,alth,(short)1,
                   altstr,JNLGTH,&edit_id);
/*
***Ok, avbryt och hj�lp.
*/
   alt_x  = ly;
   alt_y  = 2*ly + (radant+1)*(alth + ly) + ly;
   alth   = 2*wpstrh();
   status = wpmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,(short)2,
                           okey,okey,"",WP_BGND,WP_FGND,&okey_id);

   alt_x  = ly + altlen + ly;
   status = wpmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,(short)2,
                           reject,reject,"",WP_BGND,WP_FGND,&reject_id);

   alt_x  = main_dx - ly - altlen;
   status = wpmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,(short)2,
                           help,help,"",WP_BGND,WP_FGND,&help_id);
/*
***Klart f�r visning.
*/
   wpwshw(iwin_id);
/*
***V�nta p� action.
*/
   status = 0;
loop:
   wpwwtw(iwin_id,SLEVEL_V3_INP,&but_id);
/*
***Okey.
*/
   if ( but_id == okey_id )
     {
     wpgted(iwin_id,edit_id,altstr);
     if ( altstr[0] == '\0' )
       {
       XBell(xdisp,100);
       goto loop;
       }
     else goto exit;
     }
/*
***Avbryt.
*/
   if ( but_id == reject_id ) status = REJECT;
/*
***Hj�lp.
*/
   else if ( but_id == help_id )
     {
     ighelp();
     goto loop;
     }
/*
***Inmatning i edit-f�ltet. Tomt f�lt gillar vi inte.
*/
   else if ( but_id == edit_id )
     {
     wpgted(iwin_id,edit_id,altstr);
     if ( altstr[0] == '\0' )
       {
       XBell(xdisp,100);
       goto loop;
       }
     else goto exit;
     }
/*
***Ett alternativ.
*/
   else
     {
     for ( i=0; i<nstr; ++i )
       {
       if ( but_id == alt_id[i] )
         {
         strcpy(altstr,strlst[i]);
         goto exit;
         }
       }
     goto loop;
     }
/*
***Dags att sluta.
*/
exit:
   wpwdel(iwin_id);
   XFlush(xdisp);

   return(status);
 }

/********************************************************/
