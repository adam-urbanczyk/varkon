/**********************************************************************
*
*    ms9.c
*    =====
*
*    This file is part of the VARKON MS-library including
*    Microsoft WIN32 specific parts of the Varkon
*    WindowPac library.
*
*    This file includes:
*
*     msilst();   Input from list
*     msilse();   Input from list with edit
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

/*!*******************************************************/

     int   msilst(x,y,rubrik,strlst,actalt,nstr,palt)
     int   x,y;
     char *rubrik;
     char *strlst[];
     int   actalt;
     int   nstr;
     int  *palt;

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
 *   (C)microform ab 27/10/95 J. Kjellander
 *
 *******************************************************!*/

 {
   char    *str,reject[81],help[81];
   int      i,rad,radant,main_x,main_y,
            status,main_dx,main_dy,alt_x,alt_y,alth,altlen,ly,lm;
   v2int    iwin_id,alt_id[WP_IWSMAX],but_id,help_id,reject_id;


/*
***Texter f�r avbryt och hj�lp.
*/
   if ( !msgrst("INPUT.REJECT",reject) ) strcpy(reject,"Avbryt");
   if ( !msgrst("INPUT.HELP",help) ) strcpy(help,"Hj�lp");
/*
***F�r att nu kunna best�mma storleken p� sj�lva
***alternativ-f�nstret utg�r vi fr�n  2 kolumner med
***knappar d�r knapparnas l�ngd best�ms av den l�ngsta
***alternativtexten.
*/
   altlen = 0;

   for ( i=0; i<nstr; ++i )
     if ( msstrl(strlst[i]) > altlen ) altlen = msstrl(strlst[i]);

   if ( msstrl(reject) > altlen ) altlen = msstrl(reject);
   if ( msstrl(help)   > altlen ) altlen = msstrl(help);

   altlen *= 1.3;
/*
***Hur m�nga rader blir det ?
*/
   radant = (int)(ceil((nstr/2.0)-0.1));
/*
***Ber�kna luft yttre, knapparnas h�jd, luft mellan och
***huvudf�nstrets h�jd.
*/
   ly   = 0.8*msstrh();
   alth = 1.6*msstrh(); 
   lm   = 1.4*msstrh();

   main_dx = ly + altlen + lm + altlen + ly;
   main_dy = ly + radant*(alth + ly) +  ly + 2*msstrh() + ly;  
/*
***Skapa sj�lva alternativf�nstret som ett WPIWIN.
*/
/*   wpposw(x,y,main_dx+10,main_dy+25,&main_x,&main_y); */

   main_x = x;
   main_y = y;
   mswciw(main_x,main_y,main_dx,main_dy,rubrik,&iwin_id);
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
       status = msmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,1,
                               str,str,"",WP_FGND,WP_BGND,&alt_id[2*rad]);
     else
       status = msmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,1,
                               str,str,"",WP_BGND,WP_FGND,&alt_id[2*rad]);
/*
***Knapp i h�gra kolumnen.
*/
     if ( nstr > (2*rad)+1 )
       {
       alt_x  = ly + altlen + lm;
       str    = strlst[(2*rad)+1];

       if ( (2*rad)+1 == actalt )
         status = msmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,1,
                          str,str,"",WP_FGND,WP_BGND,&alt_id[(2*rad)+1]);
       else
         status = msmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,1,
                          str,str,"",WP_BGND,WP_FGND,&alt_id[(2*rad)+1]);
       }
     }
/*
***Avbryt och hj�lp.
*/
   alt_x  = ly;
   alt_y  = ly + radant*(alth + ly) + ly;
   alth   = 2*msstrh();
   status = mscrdb((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,1,
                           reject,"",WP_BGND,WP_FGND,&reject_id);

   alt_x  = ly + altlen + lm;
   status = msmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,1,
                           help,help,"",WP_BGND,WP_FGND,&help_id);
/*
***Klart f�r visning.
*/
   mswshw(iwin_id);
/*
***V�nta p� action. Service-niv� f�r key-event saknar
***intresse i sammanhanget.
*/
   status = 0;
loop:
   mswwtw(iwin_id,SLEVEL_NONE,&but_id);
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
   mswdel(iwin_id);

   return(status);
 }

/********************************************************/
/*!*******************************************************/

     int   msilse(x,y,rubrik,defstr,strlst,actalt,nstr,altstr)
     int   x,y;
     char *rubrik;
     char *defstr;
     char *strlst[];
     int   actalt;
     int   nstr;
     char *altstr;

/*   Visar en lista med alternativ samt ett inmatningsf�lt
 *   l�ngst ned.
 *
 *   In: x,y    => F�nstrets l�ge
 *       rubrik => F�nsterrubrik
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
 *   (C)microform ab 28/10/95 J. Kjellander
 *
 *   1998-04-01 defstr mm., J.Kjellander
 *
 *******************************************************!*/

 {
   char    *str,okey[81],reject[81],help[81];
   int      i,rad,radant,main_x,main_y,width,height,kol,kolant,
            alt;
   int      status,main_dx,main_dy,alt_x,alt_y,alth,altlen,
            edtlen,ly,lm;
   v2int    iwin_id,alt_id[WP_IWSMAX],but_id,okey_id,help_id,
            reject_id,edit_id;
   bool     xfuse,yfuse;

/*
***Texter f�r okey, avbryt och hj�lp.
*/
   if ( !msgrst("INPUT.OKEY",okey) ) strcpy(okey,"Okej");
   if ( !msgrst("INPUT.REJECT",reject) ) strcpy(reject,"Avbryt");
   if ( !msgrst("INPUT.HELP",help) ) strcpy(help,"Hj�lp");
/*
***Hur l�ng �r den l�ngsta alternativtexten.
***Alternativrutor samt avbryt och hj�lp g�rs altlen*1.3.
*/
   altlen = 0;

   for ( i=0; i<nstr; ++i )
     if ( msstrl(strlst[i]) > altlen ) altlen = msstrl(strlst[i]);

   if ( msstrl(okey)   > altlen ) altlen = msstrl(okey);
   if ( msstrl(reject) > altlen ) altlen = msstrl(reject);
   if ( msstrl(help)   > altlen ) altlen = msstrl(help);

   altlen *= 1.3;
/*
***Hur l�ngt skall inmatningsf�nstret vara ?
*/
   edtlen = JNLGTH*msstrl("A")*1.5;
/*
***Ber�kna luft yttre, knapparnas h�jd och luft mellan.
*/
   ly   = 0.5*msstrh();
   alth = 1.6*msstrh(); 
   lm   = 1.0*msstrh();
/*
***Hur stor �r sk�rmen ?
*
   width  = msmwsx();
   height = msmwsy();
*/
   width = GetDeviceCaps(GetDC(ms_main),HORZRES);
   height= GetDeviceCaps(GetDC(ms_main),VERTRES) -
           4*GetSystemMetrics(SM_CYMENU);
/*
***Vi utg�r fr�n minst 3 kolumner.
***Hur m�nga rader blir det med 3 kolumner ?
*/
   kolant = 3;
   xfuse = yfuse = FALSE;

kloop:
   radant = (int)(ceil((nstr/(double)kolant)-0.1));
/*
***Ber�kna huvudf�nstrets storlek.
*/
   main_dx = ly + (kolant-1)*(altlen + lm) + altlen + ly;
   if ( main_dx < 3*altlen + 4*ly ) main_dx = 3*altlen + 4*ly;
   if ( main_dx < 2*ly + edtlen ) main_dx = 2*ly + edtlen;

   main_dy = ly + (radant+1)*(alth + ly) +  ly + 2*msstrh() + ly;  
/*
***F�r det plats i X-led. Om inte kan vi inget g�ra. D�
***f�r vi n�ja oss med att inte visa alltihop.
*/
   if ( xfuse && yfuse ) ;   /* Vi har provat b�de bredare och l�ngre */
   else if ( main_dx > width  &&  kolant > 1 )
     {
   --kolant;
     xfuse = TRUE;
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
     yfuse = TRUE;
     goto kloop;
     }
/*
***Skapa sj�lva alternativf�nstret som ett WPIWIN.
*/
/*   wpposw(x,y,main_dx+10,main_dy+25,&main_x,&main_y); */

   main_x = x;
   main_y = y;
   mswciw(main_x,main_y,main_dx,main_dy,rubrik,&iwin_id);
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
           status = msmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,1,
                                   str,str,"",WP_FGND,WP_BGND,&alt_id[alt]);
         else
           status = msmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,1,
                                   str,str,"",WP_BGND,WP_FGND,&alt_id[alt]);
         }
       }
     }
/*
***Edit-f�ltet.
*/
   alt_x  = (main_dx-edtlen)/2;
   alt_y  = ly + radant*(alth + ly) + ly;

   strcpy(altstr,defstr);
   status = msmced((wpw_id)iwin_id,alt_x,alt_y,edtlen,alth,1,
                   altstr,JNLGTH,&edit_id);
/*
***Okey, avbryt och hj�lp.
*/
   alt_x  = ly;
   alt_y  = ly + (radant+1)*(alth + ly) + ly;
   alth   = 2*msstrh();
   status = mscrdb((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,2,
                           okey,"",WP_BGND,WP_FGND,&okey_id);

   alt_x  = alt_x + altlen + ly;
   status = msmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,2,
                           reject,reject,"",WP_BGND,WP_FGND,&reject_id);

   alt_x  = main_dx - ly - altlen;
   status = msmcbu((wpw_id)iwin_id,alt_x,alt_y,altlen,alth,2,
                           help,help,"",WP_BGND,WP_FGND,&help_id);
/*
***Klart f�r visning.
*/
   mswshw(iwin_id);
/*
***V�nta p� action.
*/
   status = 0;
loop:
   mswwtw(iwin_id,SLEVEL_V3_INP,&but_id);
/*
***Okey.
*/
   if ( but_id == okey_id )
     {
     msgted(iwin_id,edit_id,altstr);
     if ( altstr[0] == '\0' )
       {
       status = REJECT;
       goto exit;
       }
       /*{
       MessageBeep(MB_ICONHAND);
       goto loop;
       }*/
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
     msgted(iwin_id,edit_id,altstr);
     if ( altstr[0] == '\0' )
       {
       Beep(10000,1000);
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
   mswdel(iwin_id);

   return(status);
 }

/********************************************************/
