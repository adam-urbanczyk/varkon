/**********************************************************************
*
*    gp1.c
*    =====
*
*    This file is part of the VARKON Graphics Library.
*    URL: http://www.varkon.com
*
*    gpinit();   Init GP
*    gpexit();   Close GP
*    gpstvi();   Sets view transformation (actvym)
*    gpsvpt();   Sets viewport (actvy)
*    gpsgsz();   Sets size of graphical area in millimeters
*    gpgvpt();   Returns viewport
*    gpswin();   Activates viewport
*    gpgwin();   Returns active viewport
*    gpfwin();   Normalizes viewport proportions
*    gpgovy();   Returns previous viewport
*    gpstcn();   Sets active curve accuracy
*    gpgtcn();   Returns active curve accuracy
*    gpgtpn();   Returns active pen number
*    gpsrsk();   Sets active drawing scale
*    gpgrsk();   Returns active drawing scale
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
#include "../../WP/include/WP.h"
#include "../../GE/include/GE.h"
#include "../include/GP.h"


bool   gpgenz;
VY     actvy;
DBTmat actvym;
VY     oldvy;

/* gpgenz best�mmer om gpdrxx()-rutiner skall n�ja sig
   med att generera X- och Y-koordinater eller om ocks�
   Z-koordinater skall genereras, tex. f�r hide.
   actvy �r den aktiv vyn, actvy.vydist = perspektivavst�nd.
   actvym �r T-matris f�r plan projicering till aktiv vy.
   oldvy �r f�reg�ende vy. */


double  viewpt[4];            /* Sk�rmf�nster */
double  k1x,k1y,k2x,k2y;      /* Transformationskonstanter */
double  curnog;               /* Kurvnoggrannhet */


gmint  dfcur;
gmint  dfpek;

/*     dfcur pekar p� f�rsta posten i aktuellt grafiskt objekt i df
       dfpek pekar p� sista posten i df. F�r ej vara st�rre 
       �n sysize.df - 1. */


DFPOST *df;

/*     df �r en array av DFPOST:er. I df lagras en "kopia"
       av de vektorer som f�r tillf�llet syns p� den grafiska
       bildsk�rmen. Alla variabler som anv�nds som pekare in
       i df �r short. df allokeras dynamiskt mha. malloc() */

double  x[300000],y[300000],z[300000];
char    a[300000];

/*

double  x[PLYMXV];
double  y[PLYMXV];
double  z[PLYMXV];
char    a[PLYMXV];
*/

int     ncrdxy;

/* X-, Y- och Z-koordinater, samt antal och status f�r sist genererade 
   polylinje */


short pmkx[PMKMAX];
short pmky[PMKMAX];
short npmk;

/* pmkx,pmky �r en array med sk�rmkoordinater f�r pek-
   m�rken. npmk �r antalet t�nda pekm�rken. Initieras
   till 0 i gpinit(). */


short gptrty;

/* gptrty h�ller reda p� vilken grafisk terminal som anv�nds.
   Se local.h f�r en beskrivning av terminaler. */


short tmode;

/* tmode h�ller reda p� i vilken mode terminalen befinner
   sig. 0 = Alfanumerisk mode. 1 = Grafisk mode. */


short actpen = -1;

/* actpen �r den aktiva pennans (f�rgens) nummer. actpen
   initieras till 1. */

GPTATT gpttab[] =
 { 
  {"\033:4014;1T","\033[",    9,  5, 200, 229,  739, 817,"",       ""},
  {"\036",        "\030",     1,  1, 207, 164, 1022, 778,"\033/1d","\033/0d"},
  {"",            "",        10, 10, 220, 175,  910, 700,"",       ""},
  {"\035",        "\030",     1,  1, 260, 192, 1022, 778,"\033x",  "\033`"},
  {"\035",        "\030",     1,  1, 370, 280, 1022, 778,"\033x",  "\033`"},
  {"\033[?38h",   "\033[?38l",1,  1, 168, 255,  779,1023,"\037\033/1d"},
  {"",            "",         1,  1, 168, 255,  779,1023,"", ""},
  {"",            "",         1,  1, 168, 255,  779,1023,"",       ""},
  {"\035",        "\030",     1,  1, 291, 224, 1022, 778,"\033x",  "\033`"},
  {"\033%!0", "\037\033%!1",  1,  1, 238, 176, 1022, 778,"\033ML0","\033ML"},
  {"\035",        "\030",     1,  1, 258, 206, 1022, 778,"\033/1d","\033/0d"},
  {"",            "",         1,  1, 168, 255,  779,1023,"",       ""},
  {"",            "\037",   215, 62, 180, 166,  808, 733,"\033/1d","\033/0d"},
  {"",            "",         0,  0,   0,   0,    0,   0,"",        ""},
  {"\033%!0", "\037\033%!1",  1,  1, 340, 270, 1022, 778,"\033ML0","\033ML"},
  {"",        "\037",   195, 62, 210, 175,  828, 737,   "\033[37m","\033[30m"},
  {"",            "\037",   195, 62, 210, 175,  828, 737,"\033[30m","\033[37m"},
  {"",            "",         0,  0,   0,   0,    0,   0,"",        ""},
  {"",            "",         0,  0,   0,   0,    0,   0,"",        ""}
 };/*.              .       !.   . ! .    .  ! .     .  !  .         .
      .              .       !.   . ! .    .  ! .     .  !  .         .
      t              t       !g   g ! g    g  ! g     g  !  p         p
      o              o       !p   p ! p    p  ! p     p  !  e         e 
      4              a       !g   g ! g    g  ! g     g  !  n         n 
      0              n       !o   o ! s    s  ! n     n  !  0         1 
      1              s       !r   r ! z    z  ! p     p  !               
      0              i       !x   y ! x    y  ! x     y  !              
                             !      !         !          !               
        Terminal-mode        !Origo !Sk�rmstor!Uppl�sning!T�nd/sl�ckt f�rfl. */

/* gpttab �r en tabell med egenskaper f�r olika terminaler. 
   Variabeln gptrty h�ller reda p� vilken grafisk terminal 
   i gpttab som anv�nds. 1=ABC1600 2=V550 3=IP32 4=MG400 5=MG420 
   6=FT4600 7=ALFA och BATCH 8=MG700 9=T4207 10=MO2000 11=VT100
   12=N220G 13=CGI 14=MX7250 15=MSCOLOUR 16=MSMONO 17=X-WINDOW
   18=MSWIN
*/

char to4010[10],toansi[10];

/* to4010 �r den str�ng som f�r terminalen att v�xla till
   4010-mode. toansi v�xlar till VT100-mode. */


short  gpsnpx,gpsnpy;
short  gpgorx,gpgory;
short  gpgnpx,gpgnpy;
double gpgszx,gpgszy;
short  gptnpx,gptnpy;

/* gpsnpx och gpsnpy �r hela sk�rmens storlek i pixels.
   gpgorx och gpgory �r den pixel som utg�r grafiska areans origo.
   gpgnpx och gpgnpy �r grafiska areans storlek i pixels.
   gpgszx och gpgszy �r grafiska areans storlek i millimeter.
   gptnxp och gptnpy �r grafisk texts storlek i pixels. */


char pen0[10],pen1[10];

/* pen0 �r den str�ng som f�r terminalen att v�xla till
   penna noll, dvs. sudda-mode. pen1 v�xlar till penna 1,
   dvs rit-mode. */


double ritskl;

/* Aktiv ritningsskala. Initieras till 1.0 i gpinit */


bool pltflg;

/* Plottflagga. TRUE om plottning p�g�r. Anv�nds av gppltx()
   f�r att avg�ra om f�renklad text f�r anv�ndas. Initieras av
   gpinit() och s�tts/s�nks av gpmeta(). */


extern short  igtrty;
extern bool   igbflg;
extern V3MSIZ sysize;

#ifdef V3_X11
int xncrd;
#endif

/*!*****************************************************/

        short gpinit(
        char *term)

/*      �ppnar, dvs initierar GPAC. Denna rutin m�ste
 *      anropas innan n�got annat anrop till GPAC sker.
 *
 *      In: term = Terminaltyp.
 *
 *      Ut: Inget.
 *
 *      FV:       0 = Ok, k�r s� det ryker.
 *           GP0053 = Ok�nd terminaltyp.
 *           GP0064 = Kan ej �ppna CGI.
 *           GP0122 = Can't malloc
 *
 *      (C)microform ab 6/11/84 J. Kjellander
 *
 *      30/12/85 Pekm�rken, J. Kjellander
 *      12/1/86  IP_32, J. Kjellander
 *      26/1/86  Penna, J. Kjellander
 *      13/2/86  terminaltyp, J. Kjellander
 *      24/2/86  Ny terminal MG420 R. Svedin
 *      15/5/86  Ny sk�rmlayout IP32 R. Svedin
 *      5/1/87   Nya terminaler FT4600, ALFA, R. Svedin
 *      17/1/87  IP_32 V1.3, J. Kjellander
 *      18/4/87  BATCH, J. Kjellander
 *      7/12/87  Ny terminal MG700 B. Doverud
 *      21/1/88  Nya Terminaler T4207, MO2000, B. Doverud
 *      13/10/88 CGI, J. Kjellander
 *      20/10/88 Ny trminal N220G, B. Doverud
 *      1/2/89   Ny VY, J. Kjellander
 *      28/11/89 sysize.df, J. Kjellander
 *      28/12/89 MX7250, J. Kjellander
 *      20/2/91  MSKERMIT, J. Kjellander
 *      10/3/91  F�reg. bild, J. Kjellander
 *      16/6/92  X11, J. Kjellander
 *      19/4/93  MSKERMIT-Vax, J. Kjellander
 *      1/11/95  MSWIN, J. Kjellander
 *
 ******************************************************!*/

  {
   double  defvpt[4];
   VY      defvy;

/*
***Initiera terminal-data.
*/
   if ( strcmp("LVT100",term) == 0 )        gptrty = LVT100;
   else if ( strcmp("V550",term) == 0 )     gptrty = V550;
   else if ( strcmp("IP3215",term) == 0 )   gptrty = IP3215;
   else if ( strcmp("MG400",term) == 0 )    gptrty = MG400;
   else if ( strcmp("MG420",term) == 0 )    gptrty = MG420;
   else if ( strcmp("FT4600",term) == 0 )   gptrty = FT4600;
   else if ( strcmp("ALFA",term) == 0 )     gptrty = ALFA;
   else if ( strcmp("BATCH",term) == 0 )    gptrty = ALFA;
   else if ( strcmp("MG700",term) == 0 )    gptrty = MG700;
   else if ( strcmp("T4207",term) == 0 )    gptrty = T4207;
   else if ( strcmp("MO2000",term) == 0 )   gptrty = MO2000;
   else if ( strcmp("VT100",term) == 0 )    gptrty = VT100;
   else if ( strcmp("N220G",term) == 0 )    gptrty = N220G;
   else if ( strcmp("MX7250",term) == 0 )   gptrty = MX7250;
   else if ( strcmp("MSKERMIT",term) == 0 ) gptrty = MSCOLOUR;
/*
***F�ljande rader borttagna 19/4/93. Ger upphov till
***problem p� SAAB:s Vax:ar och beh�vs egentligen inte.
***Vi f�ruts�tter i framtiden att alla PC med MSKERMIT
***har f�rgsk�rm.
*
*    �
*    fputs("�033�2;2$u",stdout); igflsh();
*    scanf("%�^���%*c",repbuf);
*    if ( strlen(repbuf) > 10 ) gptrty = MSCOLOUR;
*    else gptrty = MSMONO;
*    �
*/

#ifdef V3_X11
   else if ( strcmp("X11",term) == 0 )      gptrty = X11;
#endif
#ifdef WIN32
   else if ( strcmp("MSWIN",term) == 0 )     gptrty = MSWIN;
#endif
   else return(erpush("GP0053",term));
/*
***In batch mode, force gptrty to ALFA.
*/
   if ( igbflg ) gptrty = ALFA;
/*
***Om VT100/T4010-terminal, disponera sk�rmen.
*/
   strcpy(to4010,gpttab[gptrty-1].to4010);
   strcpy(toansi,gpttab[gptrty-1].toansi);
   gpgorx = gpttab[gptrty-1].scorgx;
   gpgory = gpttab[gptrty-1].scorgy;
   gpgszx = gpttab[gptrty-1].scsizx;
   gpgszy = gpttab[gptrty-1].scsizy;
   gpgnpx  = gpttab[gptrty-1].npixx;
   gpgnpy  = gpttab[gptrty-1].npixy;
   strcpy(pen0,gpttab[gptrty-1].pen0);
   strcpy(pen1,gpttab[gptrty-1].pen1);

   switch ( gptrty )
     {
/*
***Nokia 220G.
*/
     case N220G:
     fputs("\033[?38h",stdout);
     igtrty = N220G;
     gpsnpx = 1024;
     gpsnpy = 795;
     gptnpx = 14;
     gptnpy = 22;
     break;
/*
***MSKERMIT, Om f�rg finns, fixa vit bakgrund.
*/
     case MSCOLOUR:
     fputs("\033[?38h\033[30m\033[47m\033\014",stdout);
     igtrty = MSCOLOUR;
     gpsnpx = 1024;
     gpsnpy = 780;
     gptnpx = 14;
     gptnpy = 22;
     break;

     case MSMONO:
     fputs("\033[?38h",stdout);
     igtrty = MSMONO;
     gpsnpx = 1024;
     gpsnpy = 780;
     gptnpx = 14;
     gptnpy = 22;
     break;
/*
***X-Windows.
*/
#ifdef V3_X11
     case X11:
     igtrty = X11;
     xncrd = 0;
     break;
#endif
/*
***WIN32.
*/
#ifdef WIN32
     case MSWIN:
     igtrty = MSWIN;
/*     xncrd = 0;*/
     break;
#endif
/*
***�vriga sk�rmar.
*/
     default:
     gpersc();
     break;
     }
/*
***Initiera displayfil och sk�rm.
*/
   dfpek = -1;
   dfcur = dfpek;
   sysize.df = 0;
   tmode = 0;
/*
***Default sk�rmf�nster och default vy. N�r vi k�r X11
***finns �nnu inga grafiska f�nster s� detta f�r ist�llet
***g�ras av winpac lite senare.
*/
   if ( gptrty != X11  &&  gptrty != MSWIN )
     {
     defvpt[0] = gpgorx;
     defvpt[1] = gpgory;
     defvpt[2] = gpgorx + gpgnpx;
     defvpt[3] = gpgory + gpgnpy;
     gpsvpt(defvpt);

     defvy.vynamn[0] ='\0';
     defvy.vyrikt.x_vy = 0.0;
     defvy.vyrikt.y_vy = 0.0;
     defvy.vyrikt.z_vy = 1.0;
     defvy.vydist = 0.0;
     defvy.vytypp = TRUE;
     defvy.vywin[0] = 0.0; defvy.vywin[1] = 0.0;
     defvy.vywin[2] = gpgszx; defvy.vywin[3] = gpgszy;
     gpstvi(&defvy);
     gpswin(&defvy);
     }
/*
***Default kurv-noggrannhet.
*/
   curnog = 1.0;
/*
***Initiering av pekm�rken.
*/
   npmk = 0;
/*
***Initiering av V3:s grafiska text-fonter.
*/
   gpinfn();
/*
***Initiera pennummer;
*/
   gpspen(1);
/*
***Initiera ritningsskala.
*/
   ritskl = 1.0;
/*
***Initiera Plottflaggan.
*/
   pltflg = FALSE;


   return(0);
  }
/********************************************************/
/*!******************************************************/

        short gpexit()

/*      Avslutar grapac.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: Inget.
 *
 *      (C)microform ab 6/11/84 J. Kjellander
 *
 *      12/1/86  IP_32, J. Kjellander
 *      16/10/88 CGI, J. Kjellander
 *      9/11/88  N220G, J. Kjellander
 *      23/6/92  X11, J. Kjellander
 *
 ******************************************************!*/

  {

    switch ( gptrty )
      {
      case N220G:
      gpansi();
      fputs("\033[?38l",stdout);
      igtrty = VT100;
      break;

      case MSCOLOUR:
      gpersc();
      fputs("\033[?38l\033[37m\033[40m",stdout);
      gpansi();
      igtrty = VT100;
      break;

      case MSMONO:
      gpersc();
      fputs("\033[?38l",stdout);
      igtrty = VT100;
      break;

/*
***X11.
*/
#ifdef V3_X11
      case X11:
      wpclrg();
      break;
#endif
/*
***Microsoft Windows.
*/
#ifdef WIN32
      case MSWIN:
      msclrg();
      break;
#endif
/*
***�vriga.
*/
      default:
      gpersc();
      gpansi();
      break;
      }
/*
***Terminalen har nu inte l�ngre n�gra
***grafiska egenskaper.
*/
    gptrty = ALFA;
/*
***Nollst�ll fonter.
*/
    gpexfn();
/*
***Deallokera displayfil.
*/
    if ( sysize.df > 0 )
      {
      v3free(df,"gpexit");
      sysize.df = 0;
      }

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short gpstvi(
        VY *pnewvy)

/*      Tittar p� vyrikt och vydist i newvy.
 *      Mofifierar vy3d och actvym i enlighet med vyrikt.
 *      Anv�nder vydist f�r att s�tta gpgenz.
 *
 *      In: pnewvy => Pekare till vy.
 *
 *      Ut: Inget.
 *
 *      FV: Inget.
 *
 *      (C)microform ab 8/8/85 J. Kjellander
 *
 *      19/10/86 projvy, J. Kjellander
 *      1/2/89   Ny VY, J. Kjellander
 *
 ******************************************************!*/

  {
    DBVector  por,pu1,pu2;

/*
***�r det en 3D-vy ?
*/
    if ( pnewvy->vytypp == TRUE )
      {
      if ( pnewvy->vyrikt.x_vy == 0.0 && pnewvy->vyrikt.y_vy == 0.0 &&
           pnewvy->vyrikt.z_vy > 0.0 ) pnewvy->vy3d = FALSE;
      else                             pnewvy->vy3d = TRUE;
      }
    else
      {
      pnewvy->vy3d = TRUE;
      }

/*
***�r det en perspektiv-vy.
*/
    if ( pnewvy->vydist == 0.0 ) gpgenz = FALSE;
    else                         gpgenz = TRUE;
/*
***Om det �r en vy med betraktelseposition m�ste vi skapa
***en vymatris nu.
*/
    if ( pnewvy->vytypp == TRUE )
      {
/*
***Ber�kna med hj�lp av vyvektorn tv� nya vektorer som
***sp�nner upp X/Y-planet i den �nskade vyn.
*/
      pu1.y_gm = 0.0;
      if (((pnewvy)->vyrikt.x_vy*(pnewvy)->vyrikt.x_vy +
           (pnewvy)->vyrikt.z_vy*(pnewvy)->vyrikt.z_vy) < 0.001)
        {
        pu1.x_gm = 0.0;
        pu1.z_gm = 1.0;
        pu2.x_gm = 1.0;
        pu2.y_gm = 0.0;
        pu2.z_gm = 0.0;
        }
      else
        {
        pu1.x_gm = (pnewvy)->vyrikt.z_vy;
        pu1.z_gm = 0.0;
        if ((pnewvy)->vyrikt.x_vy != 0.0) pu1.z_gm = -(pnewvy)->vyrikt.x_vy;
        if (((pnewvy)->vyrikt.y_vy * (pnewvy)->vyrikt.y_vy) < 0.001)
          {
          pu2.x_gm = 0.0;
          pu2.y_gm = 1.0;
          pu2.z_gm = 0.0;
          }
        else
          {
          pu2.x_gm = (pnewvy)->vyrikt.z_vy*pu1.y_gm +
                     (pnewvy)->vyrikt.y_vy*pu1.z_gm;
          pu2.y_gm = (pnewvy)->vyrikt.z_vy*pu1.x_gm -
                     (pnewvy)->vyrikt.x_vy*pu1.z_gm;
          pu2.z_gm = (pnewvy)->vyrikt.x_vy*pu1.y_gm -
                     (pnewvy)->vyrikt.y_vy*pu1.x_gm;
          }
        }
/*
***Bilda transformationsmatris.
*/
      por.x_gm = 0.0;
      por.y_gm = 0.0;
      por.z_gm = 0.0;
  
      return(GEmktf_3p(&por,&pu1,&pu2,&actvym));
      }
/*
***Det �r en matris-vy, d� blir det enklare.
*/
    else
      {
      actvym.g11 = pnewvy->vymatr.v11;
      actvym.g12 = pnewvy->vymatr.v12;
      actvym.g13 = pnewvy->vymatr.v13;
      actvym.g14 = 0.0;

      actvym.g21 = pnewvy->vymatr.v21;
      actvym.g22 = pnewvy->vymatr.v22;
      actvym.g23 = pnewvy->vymatr.v23;
      actvym.g24 = 0.0;

      actvym.g31 = pnewvy->vymatr.v31;
      actvym.g32 = pnewvy->vymatr.v32;
      actvym.g33 = pnewvy->vymatr.v33;
      actvym.g34 = 0.0;

      actvym.g41 = 0.0;
      actvym.g42 = 0.0;
      actvym.g43 = 0.0;
      actvym.g44 = 1.0;
  
      return(0);
      }
  }

/********************************************************/
/*!******************************************************/

        short gpsvpt(
        double vpt[])

/*      S�tter aktiv viewport, dvs. den del av sk�rmen
 *      som anv�nds som grafisk area.
 *
 *      In: vpt = Array med viewport.
 *
 *      Ut: Inget.
 *
 *      FV:  0 
 *
 *      (C)microform ab 18/10/86 J. Kjellander
 *
 ******************************************************!*/

  {
    viewpt[0] = vpt[0];
    viewpt[1] = vpt[1];
    viewpt[2] = vpt[2];
    viewpt[3] = vpt[3];

    gpgnpx = (short)(viewpt[2] - viewpt[0]);
    gpgnpy = (short)(viewpt[3] - viewpt[1]);

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short gpsgsz(
        double dxmm,
        double dymm)

/*      S�tter grafiska areans (viewport:ens) storlek i mm.
 *
 *      In: dxmm = Storlek i X-led.
 *          dymm = Storlek i Y-led.
 *
 *      Ut: Inget.
 *
 *      FV:  0 
 *
 *      (C)microform ab 28/6/92 J. Kjellander
 *
 ******************************************************!*/

  {
    gpgszx = dxmm;
    gpgszy = dymm;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short gpgvpt(
        double vpt[])

/*      Returnerar aktiv viewport.
 *
 *      In: vpt = Pekare till resultat.
 *
 *      Ut: *vpt = Aktiv viewport.
 *
 *      FV:  0 
 *
 *      (C)microform ab 18/10/86 J. Kjellander
 *
 ******************************************************!*/

  {
    vpt[0] = viewpt[0];
    vpt[1] = viewpt[1];
    vpt[2] = viewpt[2];
    vpt[3] = viewpt[3];

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short gpswin(
        VY *pwin)

/*      S�tter upp ett nytt modell-f�nster och g�r det
 *      aktivt. Det nya f�nstrets max och min v�rden
 *      lagras i actvy och nya konstanter k1x,k2x,k1y
 *      och k2y ber�knas.
 *
 *      In: pwin = pekare till vy.
 *
 *      Ut: Inget.
 *
 *      FV:  0  => Ok, nytt f�nster aktivt.
 *          -1  => Otill�tet f�nster.
 *
 *      (C)microform ab 15/10/86 J. Kjellander
 *
 *      10/3/91  olview, J. Kjellander
 *      29/9/92  Test av proportioner, J. Kjellander
 *
 ******************************************************!*/

  {
    double gprop,wprop,wdx,wdy;

/*
***Ber�kna det nya modell-f�nstrets storlek.
*/
    wdx = pwin->vywin[2] - pwin->vywin[0];
    wdy = pwin->vywin[3] - pwin->vywin[1];
/*
***Kontrollera indata.
*/
    if ( wdx <= 0.0 ) return(-1);
    if ( wdy <= 0.0 ) return(-1);
/*
***Om det �nskade modell-f�nstrets proportioner inte st�mmer
***�verens med den grafiska areans i mm. s� justerar vi genom att
***g�ra det st�rre i X- eller Y-riktningen. Detta
***kan tex. intr�ffa n�r man byter sk�rm.
*/
    wprop = wdx/wdy;
    gprop = gpgszx/gpgszy;

/* Tillf�lligt borttaget pga. problem vid plottning.
***921023 JK.
*
    if ( wprop > gprop )
       �
       inc = wprop/gprop*wdy - wdy;
       pwin->vywin�1� -= inc/2.0;
       pwin->vywin�3� += inc/2.0;
       �
     else
       �
       inc = gprop/wprop*wdx - wdx;
       pwin->vywin�0� -= inc/2.0;
       pwin->vywin�2� += inc/2.0;
       �
*/

/*
***Spara det gamla f�nstret i oldvy. Om det nya
***f�nstret inte har n�got namn, finns inget gammalt 
***f�nster, tex. vid uppstart. Spara d� det nya ist�llet.
*/
    if ( pwin->vynamn[0] == '\0'  ||  actvy.vynamn[0] == '\0' )
      V3MOME((char *)pwin,(char *)&oldvy,sizeof(VY));
    else
      V3MOME((char *)&actvy,(char *)&oldvy,sizeof(VY));
/*
***Lagra det nya f�nstret i actvy.
*/
    V3MOME((char *)pwin,(char *)&actvy,sizeof(VY));
/*
***Ber�kna nya transformations-konstanter f�r trans-
***formationen 2D-modell till pixel.
*/
    k2x = (viewpt[2] - viewpt[0]) / (pwin->vywin[2] - pwin->vywin[0]);
    k2y = (viewpt[3] - viewpt[1]) / (pwin->vywin[3] - pwin->vywin[1]);

    k1x = viewpt[0] - (pwin->vywin[0])*k2x;
    k1y = viewpt[1] - (pwin->vywin[1])*k2y;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short gpgwin(
        VY *pwin)

/*      Returnerar det aktiva modell-f�nstret.
 *
 *      In: pwin = Pekare till resultat.
 *
 *      Ut: *pwin = Aktiv vy.
 *
 *      FV:  0 
 *
 *      (C)microform ab 18/10/86 J. Kjellander
 *
 ******************************************************!*/

  {
    V3MOME((char *)&actvy, (char *)pwin, sizeof(VY));
    return(0);
  }

/********************************************************/
/*!******************************************************/

        short gpfwin(
        VY *pwin)

/*      Justerar ett modellf�nster s� att det f�r
 *      samma proportioner som sk�rmf�nstret.
 *
 *      In: pwin = pekare till vy.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 9/2/94 J. Kjellander
 *
 ******************************************************!*/

  {
    double gprop,wprop,wdx,wdy,inc;

/*
***Ber�kna modell-f�nstrets storlek.
*/
    wdx = pwin->vywin[2] - pwin->vywin[0];
    wdy = pwin->vywin[3] - pwin->vywin[1];
/*
***Om det �nskade modell-f�nstrets proportioner inte st�mmer
***�verens med den grafiska areans i mm. s� justerar vi genom att
***g�ra det st�rre i X- eller Y-riktningen. Detta
***kan tex. intr�ffa n�r man byter sk�rm.
*/
    wprop = wdx/wdy;
    gprop = gpgszx/gpgszy;

    if ( wprop > gprop )
       {
       inc = wprop/gprop*wdy - wdy;
       pwin->vywin[1] -= inc/2.0;
       pwin->vywin[3] += inc/2.0;
       }
     else
       {
       inc = gprop/wprop*wdx - wdx;
       pwin->vywin[0] -= inc/2.0;
       pwin->vywin[2] += inc/2.0;
       }
/*
***Ber�kna nya transformations-konstanter f�r trans-
***formationen 2D-modell till pixel.
*/
    k2x = (viewpt[2] - viewpt[0]) / (pwin->vywin[2] - pwin->vywin[0]);
    k2y = (viewpt[3] - viewpt[1]) / (pwin->vywin[3] - pwin->vywin[1]);

    k1x = viewpt[0] - (pwin->vywin[0])*k2x;
    k1y = viewpt[1] - (pwin->vywin[1])*k2y;
/*
***Slut.
*/
    return(0);
  }

/********************************************************/
/*!******************************************************/

        short gpgovy(
        VY *pvy)

/*      Returnerar f�reg�ende vy.
 *
 *      FV:  0 
 *
 *      (C)microform ab 10/3/91 J. Kjellander
 *
 ******************************************************!*/

  {
    V3MOME((char *)&oldvy,(char *)pvy,sizeof(VY));
    return(0);
  }

/********************************************************/
/*!******************************************************/

        short gpstcn(double newcn)

/*      �ndrar kurvnogrannhet.
 *
 *      In: newcn => Ny kurvnoggrannhet.
 *
 *      Ut: Inget.
 *
 *      FV: 0
 *
 *      (C)microform ab 9/8/85 J. Kjellander
 *
 ******************************************************!*/

  {
    curnog = newcn;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short gpgtcn(
        double *cnpek)

/*      Returnerar kurvnogrannhet.
 *
 *      In: cnpek  => Pekare till kurvnoggrannhet.
 *
 *      Ut: *cnpek => Kurvnoggrannhet.
 *
 *      FV: 0
 *
 *      (C)microform ab 9/8/85 J. Kjellander
 *
 ******************************************************!*/

  {
    *cnpek = curnog;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short gpgtpn()

/*      Returnerar aktivt penn-nummer.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: Penn-nummer.
 *
 *      (C)microform ab 11/2/86 J. Kjellander
 *
 ******************************************************!*/

  {
    return(actpen);
  }

/********************************************************/
/*!******************************************************/

        short gpsrsk(
        double skala)

/*      S�tter aktiv ritningsskala.
 *
 *      In: skala = Ny ritningsskala.
 *
 *      Ut: Inget.
 *
 *      FV: 0
 *
 *      (C)microform ab 20/10/86 J. Kjellander
 *
 ******************************************************!*/

  {
    ritskl = skala;

    return(0);
  }

/********************************************************/
/*!******************************************************/

        short gpgrsk(
        double *pskala)

/*      Returnerar aktiv ritningsskala.
 *
 *      In: pskala = Pekare till resultat.
 *
 *      Ut: *pskala = Aktiv ritningsskala.
 *
 *      FV: 0
 *
 *      (C)microform ab 20/10/86 J. Kjellander
 *
 ******************************************************!*/

  {
    *pskala = ritskl;

    return(0);

  }

/********************************************************/
