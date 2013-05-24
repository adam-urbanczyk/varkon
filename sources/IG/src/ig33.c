/*!******************************************************************/
/*  File: ig33.c                                                    */
/*  ============                                                    */
/*                                                                  */
/*  This file includes:                                             */
/*                                                                  */
/*  erinit();    Init error stack                                   */
/*  erpush();    Push error message                                 */
/*  erlerr();    Return last errcode                                */
/*  errmes();    Display error stack                                */
/*  dbgini();    Init debug                                         */
/*  dbgexi();    Exit debug                                         */
/*  dbgon();     Turn debug on                                      */
/*  dbglev();    Returns debug level                                */
/* *dbgfil();    Returns ptr to debugfile                           */
/*  v3time();    Handles time measuring                             */
/*                                                                  */
/*  This file is part of the VARKON IG Library.                     */
/*  URL:  http://www.varkon.com                                     */
/*                                                                  */
/*  This library is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU Library General Public     */
/*  License as published by the Free Software Foundation; either    */
/*  version 2 of the License, or (at your option) any later         */
/*  version.                                                        */
/*                                                                  */
/*  This library is distributed in the hope that it will be         */
/*  useful, but WITHOUT ANY WARRANTY; without even the implied      */
/*  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR         */
/*  PURPOSE.  See the GNU Library General Public License for more   */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Library General      */
/*  Public License along with this library; if not, write to the    */
/*  Free Software Foundation, Inc., 675 Mass Ave, Cambridge,        */
/*  MA 02139, USA.                                                  */
/*                                                                  */
/*  (C)Microform AB 1984-1999, Johan Kjellander, johan@microform.se */
/*                                                                  */
/********************************************************************/

#include "../../DB/include/DB.h"
#include "../include/IG.h"
#include "../include/debug.h"
#include "../include/screen.h"
#include "../../WP/include/WP.h"
#include <string.h>

#ifdef UNIX
#include <sys/times.h>
#include <unistd.h>
#endif

#define ESTKSZ 25          /* Felstackens storlek */
#define INSLEN 256         /* Max l�ngd p� insert-str�ngen */

typedef struct errrpt
  {
  char   module[3];         /* Modul, 2 tecken + �n */
  short  errnum;            /* Felnummer */
  short  sev;               /* Severity-code */
  char   insert[INSLEN+1];  /* Insert-str�ng, tecken + \n */
  } ERRRPT;

ERRRPT  errstk[ESTKSZ];

/* errstk �r felmeddelandestacken */

static struct
  {
  int level;
  FILE *fptr;
  } dbgtab[11];

/* dbgtab �r debug-niv�tabellen */

#ifdef UNIX
static struct
  {
  int     count;
  clock_t ticks;
  clock_t total;
  } timtab[10];

/* timtab �r timer-tabellen */
#endif

extern short rmarg,ialy,igtrty;
extern bool  igbflg;
extern char  jobdir[],jobnam[];

/*!******************************************************/

        short erinit()

/*      T�mmer fel-stacken.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: 0
 *
 *      (C)microform ab 6/9/85 J. Kjellander
 *
 ******************************************************!*/

  {
    short i;

    for ( i=0; i<ESTKSZ; ++i )
        {
        errstk[i].module[0] = '\0';
        errstk[i].insert[0] = '\0';
        }
    return(0);
  }

/*!******************************************************/
/*!******************************************************/

        short erpush(
        char  *code,
        char  *str)

/*      Felhanteringsrutin.
 *
 *      In: code => Pekare till str�ng inneh�llande
 *                    felkod. 6 tecken avslutat med �n.
 *          str  => Pekare till "insert"-str�ng.
 *
 *      Ut: Inget.
 *
 *      FV: Severity-kod med minustecken.
 *
 *      (C)microform ab 7/1/85 J. Kjellander
 *
 *      21/10/85 Rapport om sev=4, J. Kjellander
 *      18/2/86  igexsa() om sev=4, J. Kjellander
 *      7/3/95   max 80 tecken i str, J. Kjellander
 *      1998-03-11 INSLEN, J.Kjellander
 *      2004-02-21 igialt(), J.Kjellander
 *
 ******************************************************!*/

    {
    int   errval,sevval;
    short i;

/*
***Ev. debug.
*/
#ifdef DEBUG
    if ( dbglev(IGEPAC) == 33 )
      {
      fprintf(dbgfil(IGEPAC),"***Start-erpush***\n");
      fprintf(dbgfil(IGEPAC),"Felkod=%s\n",code);
      fprintf(dbgfil(IGEPAC),"Insert=%s\n",str);
      }
#endif
/*
***Insertstr�ngar f�r inte vara l�ngre �n INSLEN tecken.
*/
    if ( strlen(str) > INSLEN ) str[INSLEN] = '\0';
/*
***Skifta felstacken ett sn�pp.
*/
    for ( i=ESTKSZ-1; i>0; --i )
        {
        strcpy(errstk[i].module,errstk[i-1].module);
        errstk[i].errnum = errstk[i-1].errnum;
        errstk[i].sev = errstk[i-1].sev;
        strcpy(errstk[i].insert,errstk[i-1].insert);
        }
/*
***Packa upp felkoden, skapa felrapport och lagra sist i stacken.
*/
    errstk[0].module[0] = code[0];
    errstk[0].module[1] = code[1];
    errstk[0].module[2] = '\0';

    sscanf(code+2,"%d",&errval);
    errval = errval/10;
    errstk[0].errnum = errval;

    sscanf(code+5,"%d",&sevval);
    errstk[0].sev = sevval;

    strcpy(errstk[0].insert,str);
/*
***Ev. debug.
*/
#ifdef DEBUG
    if ( dbglev(IGEPAC) == 33 )
      {
      fprintf(dbgfil(IGEPAC),"Severity=%d\n",sevval);
      fprintf(dbgfil(IGEPAC),"***Slut-erpush***\n");
      }
#endif
/*
***Om severity 4 rapportera felet och bryt ner systemet.
*/
    if ( sevval == 4 )
      {
      errmes();
      if ( igtrty == X11  ||  igtrty == MSWIN ) igialt(457,458,458,TRUE);
      igexsa();
      }
/*
***Returnera severity.
*/
    return(-(errstk[0].sev));

    }

/*!******************************************************/
/*!******************************************************/

        short errmes()

/*      Rapporterar fel.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: 0
 *
 *      (C)microform ab 17/4/85 Mats Nelson
 *
 *      5/9/85     J. Kjellander, felstack.
 *      3/2/86     Bytt getchar mot iggtch R. Svedin
 *      4/3/86     VMS, J. Kjellander
 *      8/10/86    Meddelanderaden, J. Kjellander
 *      13/10/86   ersatt scanf, J. Kjellander
 *      25/8/92    X11, J. Kjellander
 *      26/4/93    Bug l�nga texter, J. Kjellander
 *      15/2/95    VARKON_ERM, J. Kjellander
 *      7/3/95     �nnu l�ngre texter, J. Kjellander
 *      15/1/96    X-resurs f�r rubrik, J. Kjellander
 *      1996-04-25 Bug filnam�20�, J.Kjellander
 *      1998-03-11 INSLEN, J.Kjellander
 *      1998-10-22 Kan ej �ppna ermfil, J.Kjellander
 *
 ******************************************************!*/

  {
    char filnam[V3PTHLEN+1],codbuf[20],linbuf[512],ferrst[512];
    char ins1[512],ins2[512],ins3[512];
    short ferrnm,fsevnm,estkpt,i1,i2,slen;
    FILE *errfil;

#ifdef X11
    char title[V3STRLEN];
#endif

/*
***Ev. debug.
*/
#ifdef DEBUG
    if ( dbglev(IGEPAC) == 33 )
      {
      fprintf(dbgfil(IGEPAC),"***Start-errmes***\n");
      fflush(dbgfil(IGEPAC));
      }
#endif
/*
***Initiering.
*/
    estkpt = 0;
/*
***Med X11/WIN32 anv�nder vi list-arean f�r felrapportering.
***Om listarean redan �r �ppen kommer den nu att �ppnas en
***g�ng till och man f�rlorar den 1:a listan. Ett s�tt �r
***att h�r alltid st�nga f�rst �ven om det inte beh�vs.
***igexla() t�l detta.
*/
    igexla();

#ifdef V3_X11
    if ( igtrty == X11 )
      {
      if ( !wpgrst("varkon.error.title",title) )
        strcpy(title,"      FELRAPPORT-FELRAPPORT-FELRAPPORT");
      wpinla(title);
      }
#endif

#ifdef WIN32
    if ( !msgrst("ERROR.TITLE",title) )
      strcpy(title,"      FELRAPPORT-FELRAPPORT-FELRAPPORT");
    msinla(title);
#endif
/*
***Skriv ut rapporter ur felstacken.
*/
loop:
#ifdef DEBUG
    if ( dbglev(IGEPAC) == 33 )
      {
      fprintf(dbgfil(IGEPAC),"\nestkpt=%hd\n",estkpt);
      fflush(dbgfil(IGEPAC));
      }
#endif
    if ( estkpt == ESTKSZ ) goto end;
    if ( errstk[estkpt].module[0] == '\0' ) goto end;
/*
***Skapa fel-filnamn och prova att �ppna filen.
*/
    strcpy(filnam,v3genv(VARKON_ERM));
    strcat(filnam,errstk[estkpt].module);
    strcat(filnam,ERMEXT);

    errfil = fopen(filnam,"r");
/*
***Filen g�r ej att �ppna.
*/
    if ( errfil == NULL )
        {
#ifdef V3_X11
        if ( igtrty == X11 )
          {
          wpalla("Can't open error message file !",(short)1);
          wpalla(filnam,(short)1);
          }
        else
#endif
#ifdef WIN32
        msalla("Can't open error message file !",(short)1);
        msalla(filnam,(short)1);
#else
          {
          igplma("errmes - can't open error file",IG_MESS);
          igssip(iggtts(3),linbuf,"",1);
          igrsma();
          igplma(filnam,IG_MESS);
          igssip(iggtts(3),linbuf,"",1);
          igrsma();
          }
#endif
        goto end;
        }
/*
***L�s en rad fr�n filen.
*/
    while(fgets(linbuf,512,errfil) != NULL)
      {
/*
***Kolla om raden b�rjar med ett heltal, dvs ett felnummer.
*/
      if ( sscanf(linbuf,"%hd",&ferrnm) == 1 )
        {
/*
***Det gjorde den, kolla om det �r r�tt nummer.
*/
        if ( ferrnm == errstk[estkpt].errnum )
          {
/*
***R�tt meddelande, l�s felnumret, sevcode och feltext.
*/
          if ( sscanf(linbuf,"%hd%hd%[\136\n]",
                    &ferrnm,&fsevnm,ferrst) == 3 )
            {
/*
***Packa upp den insertstr�ng som finns p� felstacken i
***sina upp till max tre olika delar.
*/
           ins1[0] = '\0';
           ins2[0] = '\0';
           ins3[0] = '\0';

            slen = strlen(errstk[estkpt].insert);
            for ( i1=0; i1<slen; ++i1 )
               {
               if ( errstk[estkpt].insert[i1] == '%' ) 
                 {
                 for ( i1++,i2=0 ; i1<slen; ++i1,++i2 )
                    {
                    if ( errstk[estkpt].insert[i1] == '%' ) 
                      {
                      strcpy(ins3,&errstk[estkpt].insert[i1+1]);
                      goto cont;
                      }
                    ins2[i2] = errstk[estkpt].insert[i1];
                    ins2[i2+1] = '\0';
                    }
                 }
               ins1[i1] = errstk[estkpt].insert[i1];
               ins1[i1+1] = '\0';
               }
#ifdef DEBUG
    if ( dbglev(IGEPAC) == 33 )
      {
      fprintf(dbgfil(IGEPAC),"i1=%s,i2=%s,i3=%s\n",ins1,ins2,ins3);
      fprintf(dbgfil(IGEPAC),ferrst,ins1,ins2,ins3);
      fflush(dbgfil(IGEPAC));
      }
#endif
/*
***Skriv felmeddelandet till linbuf. Klipp vid 115 tecken s� att
***sj�lva felkoden (13 tecken) s�kert kommer med.
*/
cont:
            sprintf(linbuf,ferrst,ins1,ins2,ins3);

            if ( strlen(linbuf) > 115 ) linbuf[115] = '\0';
/*
***L�gg till felkoden inom paranteser.
*/
            sprintf(codbuf," - (%s %d %d)",
                   errstk[estkpt].module,
                   errstk[estkpt].errnum,
                   errstk[estkpt].sev);
            strcat(linbuf,codbuf);
/*
***Pip och skriv ut.
*/
            fclose(errfil);
            igbell();
#ifdef V3_X11
            if ( igtrty == X11 )
              {
              wpalla(linbuf,(short)1);
              ++estkpt;
              goto loop;
              }
            else
#endif
#ifdef WIN32
            msalla(linbuf,(short)1);
            ++estkpt;
            goto loop;
#else
              {
              igplma(linbuf,IG_MESS);
              igmvac(1,ialy);
              igerar(rmarg,1);
              igmvac(1,ialy);
              igpstr(iggtts(1),REVERS);
              igflsh();
              if ( igbflg )
                {
                ++estkpt;
                goto loop;
                }
              else if ( iggtch() == ' ' )
                {
                ++estkpt;
                igrsma();
                goto loop;
                }
              else
                {
                igrsma();
                goto end;
                }
              }
#endif
            }
         }
      }
   }
/*
***Inget meddelande med r�tt nummer har hittats.
*/
    fclose(errfil);
    sprintf(linbuf,"<EOF> and no hit, errcode=%s%d%d, %s",
            errstk[estkpt].module,
            errstk[estkpt].errnum,
            errstk[estkpt].sev,
            errstk[estkpt].insert);
    igbell();

#ifdef V3_X11
    if ( igtrty == X11 ) wpalla(linbuf,(short)1);
    else
#endif
#ifdef WIN32
    msalla(linbuf,(short)1);
#else
      {
      igplma(linbuf,IG_MESS);
      igssip(iggtts(3),linbuf,"",1);
      igrsma();
      }
#endif
/*
***H�r slutar vi.
*/
end:

#ifdef V3_X11
    if ( igtrty == X11 ) wpexla(TRUE);
    else
#endif
#ifdef WIN32
    msexla(TRUE);
#else
      {
      igmvac(1,ialy);
      igerar(rmarg,1);
      }
#endif
/*
***Alla meddelanden �r nu utskrivna, reinitiera felsystemet igen.
*/
    erinit();
/*
***Ev. debug.
*/
#ifdef DEBUG
    if ( dbglev(IGEPAC) == 33 )
      {
      fprintf(dbgfil(IGEPAC),"***Slut-errmes***\n\n");
      fflush(dbgfil(IGEPAC));
      }
#endif

    return(0);
  }
/********************************************************/
/*!******************************************************/

        short erlerr()

/*      Returnerar senaste felnumer.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: Felnummer.
 *
 *      (C)microform ab 13/1/85 J. Kjellander
 *
 ******************************************************!*/

    {
    return(errstk[0].errnum);
    }

/*!******************************************************/
/*!******************************************************/

        short dbgini()

/*      Nollst�ller alla debug-niv�er och tidtagare.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 17/6/92 J. Kjellander
 *
 *      1998-04-27 Tidtagning, J.Kjellander
 *
 ******************************************************!*/

 {
   int i;

/*
***Initiera debug.
*/
   dbgtab[IGEPAC].level = 0;
   dbgtab[GRAPAC].level = 0;
   dbgtab[GEOPAC].level = 0;
   dbgtab[GMPAC].level  = 0;
   dbgtab[EXEPAC].level = 0;
   dbgtab[ANAPAC].level = 0;
   dbgtab[PMPAC].level  = 0;
   dbgtab[SURPAC].level = 0;
   dbgtab[WINPAC].level = 0;
   dbgtab[MSPAC].level  = 0;
/*
***Initiera tidtagare.
*/
   for ( i=0; i<10; ++i ) v3time(V3_TIMER_RESET,i);

   return(0);
 }

/*!******************************************************/
/*!******************************************************/

        short dbgexi()

/*      St�nger alla �ppna debug-filer.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: 0.
 *
 *      (C)microform ab 17/6/92 J. Kjellander
 *
 ******************************************************!*/

 {
    if ( dbgtab[IGEPAC].level > 0 ) fclose(dbgtab[IGEPAC].fptr);
    if ( dbgtab[GRAPAC].level > 0 ) fclose(dbgtab[GRAPAC].fptr);
    if ( dbgtab[GEOPAC].level > 0 ) fclose(dbgtab[GEOPAC].fptr);
    if ( dbgtab[GMPAC].level  > 0 ) fclose(dbgtab[GMPAC].fptr);
    if ( dbgtab[EXEPAC].level > 0 ) fclose(dbgtab[EXEPAC].fptr);
    if ( dbgtab[ANAPAC].level > 0 ) fclose(dbgtab[ANAPAC].fptr);
    if ( dbgtab[PMPAC].level  > 0 ) fclose(dbgtab[PMPAC].fptr);
    if ( dbgtab[SURPAC].level > 0 ) fclose(dbgtab[SURPAC].fptr);
    if ( dbgtab[WINPAC].level > 0 ) fclose(dbgtab[WINPAC].fptr);
    if ( dbgtab[MSPAC].level  > 0 ) fclose(dbgtab[MSPAC].fptr);

    return(0);
 }

/*!******************************************************/
/*!******************************************************/

        short dbgon(char *str)

/*      S�tter p� debug f�r viss filkatalog. Anropas
 *      av igppar() vid uppstart.
 *
 *      In: str = Kommandorads-parameter typ -DGEOPAC2
 *                strippad p� -D, dvs. endast GEOPAC2.
 *
 *      Ut: Inget.
 *
 *      FV:  0 = Ok.
 *          <0 = Syntaxfel.
 *
 *      (C)microform ab 17/6/92 J. Kjellander
 *
 ******************************************************!*/

 {
   int   pac,lev,len;
   char  fnam[80];

/*
***Vilket paket g�ller det ?
*/
   if ( strncmp(str,"IGEPAC",6) == 0  ||
        strncmp(str,"igepac",6) == 0 )
     { pac = IGEPAC; len = 6; }
   else if ( strncmp(str,"GRAPAC",6) == 0  ||
             strncmp(str,"grapac",6) == 0 )
     { pac = GRAPAC; len = 6; }
   else if ( strncmp(str,"GEOPAC",6) == 0  ||
             strncmp(str,"geopac",6) == 0 )
     { pac = GEOPAC; len = 6; }
   else if ( strncmp(str,"GMPAC",5) == 0  ||
             strncmp(str,"gmpac",5) == 0 )
     { pac = GMPAC; len = 5; }
   else if ( strncmp(str,"EXEPAC",6) == 0  ||
             strncmp(str,"exepac",6) == 0 )
     { pac = EXEPAC; len = 6; }
   else if ( strncmp(str,"ANAPAC",6) == 0  ||
             strncmp(str,"anapac",6) == 0 )
     { pac = ANAPAC; len = 6; }
   else if ( strncmp(str,"PMPAC",5) == 0  ||
             strncmp(str,"pmpac",5) == 0 )
     { pac = PMPAC; len = 5; }
   else if ( strncmp(str,"SURPAC",6) == 0  ||
             strncmp(str,"surpac",6) == 0 )
     { pac = SURPAC; len = 6; }
   else if ( strncmp(str,"WINPAC",6) == 0  ||
             strncmp(str,"winpac",6) == 0 )
     { pac = WINPAC; len = 6; }
   else if ( strncmp(str,"MSPAC",5) == 0  ||
             strncmp(str,"mspac",5) == 0 )
     { pac = MSPAC; len = 5; }
   else return(-1);
/*
***Vilken debug-niv� ?
*/
   if ( sscanf(str+len,"%d",&lev) < 1 ) return(-1);
   dbgtab[pac].level = lev;
/*
***�ppna debug-fil.
*/
   strncpy(fnam,str,len); fnam[len] = '\0';
   strcat(fnam,".DBG");
   if ( (dbgtab[pac].fptr=fopen(fnam,"w+")) == NULL ) return(-1);
/*
***Skriv ut en info-rad.
*/
   fprintf(dbgtab[pac].fptr,"Debug-fil f�r VARKON-3D, niv� = %s\n",str);

   return(0);

 }

/*!******************************************************/
/*!******************************************************/

        int dbglev(int srcpac)

/*      Returnerar debug-niv�.
 *
 *      In: srcpac = Kod f�r vilken k�llkodskatalog som
 *                   avses, tex. GEOPAC.
 *
 *      Ut: Inget.
 *
 *      FV: Debugniv� f�r den k�llkodskatalog som avses.
 *
 *      (C)microform ab 17/6/92 J. Kjellander
 *
 ******************************************************!*/

 {
    if ( srcpac < IGEPAC  ||  srcpac > MSPAC ) return(0);
    else return(dbgtab[srcpac].level);
 }

/*!******************************************************/
/*!******************************************************/

        FILE *dbgfil(int srcpac)

/*      Returnerar debug-filpekare.
 *
 *      In: srcpac = Kod f�r vilken k�llkodskatalog som
 *                   avses, tex. GEOPAC.
 *
 *      Ut: Inget.
 *
 *      FV: Pekare till fil f�r den k�llkodskatalog som avses.
 *
 *      (C)microform ab 17/6/92 J. Kjellander
 *
 ******************************************************!*/

 {
    if ( srcpac < IGEPAC  ||  srcpac > MSPAC ) return(0);
    else return(dbgtab[srcpac].fptr);
 }

/*!******************************************************/
/*!******************************************************/

        void v3time(
        int  op,
        int  num)

/*      Hanterar tidtagning.
 *
 *      In: op  = Operation att utf�ra.
 *          num = Vilken tidtagare, 0->9
 *
 *      Ut: Inget.
 *
 *      (C)microform ab 1998-04-28 J. Kjellander
 *
 *      1998-05-25 #ifdef UNIX, J.Kjellander
 *
 ******************************************************!*/

 {
#ifdef UNIX
   FILE   *f;
   struct  tms timbuf;
   clock_t ticks;
   double  secs;
   char    path[V3PTHLEN];

   if ( num < 0  ||  num > 9 ) return;

   switch (op)
     {
     case V3_TIMER_WRITE:
     strcpy(path,jobdir);
     strcat(path,jobnam);
     strcat(path,".TIME");
     f = fopen(path,"a");
     secs = (double)timtab[num].total/sysconf(_SC_CLK_TCK);
     fprintf(f,"Timer nummer %d = %g sekunder. P�slagen %d g�nger.\n",
                                              num,secs,timtab[num].count);
     fclose(f);
     break;

     case V3_TIMER_ON:
   ++timtab[num].count;
     timtab[num].ticks = times(&timbuf);
     break;

     case V3_TIMER_OFF:
     ticks = times(&timbuf);
     timtab[num].total += ticks - timtab[num].ticks;
     break;

     case V3_TIMER_RESET:
     timtab[num].count = 0;
     timtab[num].ticks = 0;
     timtab[num].total = 0;
     break;
     }
#endif
   return;
 }

/*!******************************************************/
