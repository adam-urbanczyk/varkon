/*!******************************************************************/
/*  igpart.c                                                        */
/*  ========                                                        */
/*                                                                  */
/*  This file includes:                                             */
/*                                                                  */
/*  partpm();    Generate part... statement                         */
/*  igcpts();    Create part statement                              */
/*  igoptp();    Check for optional parameter                       */
/*  igmenp();    Check for menu parameter                           */
/*  igposp();    Check for pos parameter                            */
/*  igtypp();    Check for type parameter                           */
/*  igdefp();    Check for default parameter                        */
/*  igtstp();    Maps t-string to prompt                            */
/*  iguppt();    Uppdate Part                                       */
/*  igcptw();    Edit part, window verion                           */
/*  iggnps();    Build part call                                    */
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
#include "../../DB/include/DBintern.h"
#include "../include/IG.h"
#include "../../AN/include/AN.h"
#include "../../EX/include/EX.h"
#include "../../WP/include/WP.h"

extern pm_ptr  actmod;
extern short   actfun,v3mode,modtyp,posmod,igtrty;
extern char    jobdir[],jobnam[],actpnm[];
extern bool    tmpref,iggflg;
extern V2NAPA  defnap;
extern struct  ANSYREC sy;

/*!******************************************************/

       short partpm()

/*      Huvudrutin f�r part...
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: 0 = OK, REJECT = avsluta, GOMAIN = huvudmenyn
 *
 *      (C)microform ab 27/2/85 Mats Neslon
 *
 *      3/7/85   Sl�ck message area, B. Doverud
 *      4/9/85   Sl�ck highlight m�rke, B. Doverud
 *      23/2/86  Nytt anrop till igcpts(), J. Kjellander
 *      6/3/86   Defaultstr�ng, J. Kjellander
 *      5/10/86  GOMAIN, B. Doverud
 *
 ******************************************************!*/

  {
    char    filnam[JNLGTH+1];
    short   status;
    static  char dstr[JNLGTH+1] = "";

/*
***L�s in part-namn.
*/
    igptma(244,IG_INP);
    if ( (status=igssip(iggtts(267),filnam,dstr,JNLGTH)) < 0 )
      {
      igrsma();
      goto end;
      }
    strcpy(dstr,filnam);
    igrsma();
/*
***Generera part-satsen.
*/
    if ((status=igcpts(filnam,PART)) == REJECT || status == GOMAIN ) goto end;
    else if ( status < 0 ) errmes();
/*
***Slut.
*/
end:
    WPerhg();    
    return(status);

  }

/********************************************************/
/*!******************************************************/

       short igcpts(
       char *filnam,
       short atyp)

/*      Skapar part-sats.
 *
 *      In: filnam => Pekare till modulfilnamn.
 *          atyp   => PART/RUN/MFUNC
 *
 *      Ut: Inget.
 *
 *      FV:      0 = OK.
 *          REJECT = Operationen avbruten.
 *          GOMAIN = Huvudmenyn.
 *
 *
 *      Felkod: IG5272   => Kan ej ladda modulen %s
 *              IG5283   => Kan ej l�sa modulens parametrar
 *              IG5023   => Kan ej skapa PART sats
 *              IG5222   => Fel vid interpretering av PART-sats
 *              IG5043   => Fel vid l�nkning
 *              IG5352   => MACRO anropas som PART
 *
 *      (C)microform ab 9/9/85 J. Kjellander efter Mats "partpm"
 *
 *      30/10/85 �nde och sida, J. Kjellander
 *      23/2/86  Link, J. Kjellander
 *      6/3/86   Defaultv�rden, J. Kjellander
 *      23/3/86  genpos(pnr,  B. Doverud
 *      24/3/86  Felutg�ng B. Doverud
 *      14/4/86  pmmark(), J. Kjellander
 *      26/6/86  Nytt anrop till pmrpap(), J. Kjellander
 *      26/6/86  St�d f�r ref, J. Kjellander
 *      6/10/86  GOMAIN, J. Kjellander
 *      13/10/86 Help, J. Kjellander
 *      20/10/86 tmpref, J. Kjellander
 *      25/4/87  MFUNC, J. Kjellander
 *      15/3/88  Ritpaketet, J. Kjellander
 *      10/11/88 Optionella parametrar, J. Kjellander
 *      11/11/88 Meny-parametrar, J. Kjellander
 *       1/12/91 Default-parametrar, J. Kjellander
 *      13/2/92  Macro, J. Kjellander
 *      15/8/93  Nytt anrop till pmcpas(), J. Kjellander
 *      16/8/93  MACRO f�r inte anropas som PART, J. Kjellander
 *      9/11/94  GLOBAL_REF, J. Kjellander
 *      1996-05-30 t-str�ng i promt, J.Kjellander
 *      1998-09-08 V�nta.. �ven f�r MACRO:n, J.Kjellander
 *
 ******************************************************!*/

  {
    pm_ptr  oblparam;            /* obligatoriska parametrar */
    pm_ptr  parlst;              /* soft parameter list */
    pm_ptr  exnpt;               /* pekare till expr. node */
    pm_ptr  retla;     
    pm_ptr  oldmod;              /* base adress of caller */
    pm_ptr  newmod;              /* base adress of called module */
    pm_ptr  dummy;
    short   status,mnum,posalt,oldpmd;
    DBetype   typmsk;
    bool    end,right;
    pm_ptr  panola;              /* PM-pointer to param. node */
    char    name[80];            /* parameter name string buffer */
    char    prompt[80];          /* parameter prompt string buffer */
    PMLITVA defval;              /* literal value structure */
    PMMONO *mnpnt;               /* pointer to module node */
    char    dstr[V3STRLEN+1];    /* Defaultstr�ng f�r param.v�rde */
    char    istr[V3STRLEN+1];    /* Inputstr�ng f�r param.v�rde */
    short   tmpafu;              /* Aktiv funktion */
    bool    oldtrf;              /* Aktiv tmpref */
    short   tmphit;              /* Tempor�r hit */
    short   oldpen,oldlev;       /* Aktiv penna och niv� */
    short   prtid;               /* Partens sekvensnummer */
    bool    optflg,optpar;       /* Optionella parametrar */
    bool    menpar=FALSE;        /* Meny-parameter */
    bool    pospar=FALSE;        /* Pos-parameter */
    PMREFVA prtref;              /* Partens identitet */
    pm_ptr  ref,arglst;          /* F�r GLOABL_REF */
    stidcl  kind;                /* F�r GLOABL_REF */

/*
***Om "K�r namngiven modul" eller MFUNC aktivera tempor�r 
***referens och hit = FALSE.
*/
     optflg = FALSE;
     oldtrf = tmpref;
     tmphit = (short)defnap.hit;
     oldpen = (short)defnap.pen;
     oldlev = (short)defnap.level;

     if ( atyp != PART )
       {
       tmpref = TRUE;
       defnap.hit = 0;
       }
/*
***Om MFUNC, st�ng av save.
*/
     if ( atyp == MFUNC ) defnap.save = 0;
/*
***Lagra undan aktiv funktion, s�tt den = -2 och lagra
***filnamnet i actpnm f�r hj�lpsystemet.
*/
    tmpafu = actfun;
    actfun = -2;
    strcpy(actpnm,filnam);
/*
***S�tt aktuell pm-pekare.
*/
    pmmark();
/*
***Ladda in modulen.
*/
    oldmod = pmgbla();
    if ( pmgeba(filnam,&newmod) != 0 ) 
      {
      status = erpush("IG5272",filnam);
      goto exit;
      }
/*
***L�s modulens huvud.
*/
    pmsbla(newmod);                 /* set new base adress */
    pmgmod((pm_ptr)0,&mnpnt);       /* get c-pointer to module node */
    pmsbla(oldmod);                 /* reset base adress */
/*
***MACRO f�r inte anropas som en part.
*/
    if ( (mnpnt->moat_ == MACRO)  && (atyp != MFUNC) )
      {
      status = erpush("IG5352",filnam);
      goto exit;
      }
/*
***Ta reda p� modulens attribut LOCAL/GLOBAL.
***Skapa listan med obligatoriska parametrar, ref.
*/
    oblparam = (pm_ptr)NULL;

    if ( mnpnt->moat_ == LOCAL )
        {
        typmsk = CSYTYP;
        if ( (status=genref(271,&typmsk,&exnpt,&end,&right)) < 0 ) goto rject1;
        pmtcon(exnpt,(pm_ptr)NULL,&oblparam,&dummy);
        }
/*
***Skapa listan med modulens parametrar.
*/
    parlst = (pm_ptr)NULL;
    pmsbla(newmod);                          /* set base adress */
    if ( pmrpap((pm_ptr)0) != 0 ) goto error1;

    for(;;)
        {
        if ( pmgpad(&panola) != 0 ) goto error1;
        if ( panola == (pm_ptr)NULL )      /* no more params. ? */
            {
            pmsbla(oldmod);
            break;
            }
        if ( pmrpar(panola,name,prompt,&defval) != 0 ) goto error1;
        pmsbla(oldmod);            /* reset base adress */
/*
***�r det en g�md parameter vars v�rde skall h�mtas fr�n fil ?
*/
        igdefp(prompt,&defval);
/*
***Parameter med promptstr�ng.
*/
        if ( strlen(prompt) > 0 )
          {
          optpar = igoptp(prompt);
          if ( defval.lit_type == C_STR_VA )
            menpar = igmenp(prompt,&mnum);
          if ( defval.lit_type == C_VEC_VA )
            pospar = igposp(prompt,&posalt);
          if ( defval.lit_type == C_REF_VA &&
            !igtypp(prompt,&typmsk) ) typmsk = ALLTYP;

          igdefp(prompt,&defval);

          if ( optflg && optpar )
            {
            pmclie(&defval,&exnpt);
            }
          else
            {
            optflg = FALSE;
/*
***Mappa ev. t-str�ng i prompten till klartext.
*/    
            igtstp(prompt);
/*
***L�s in parameterv�rde.
*/ 
            switch(defval.lit_type)
              {
              case C_INT_VA:
              igplma(prompt,IG_INP);
              if ( optpar ) status = genint(0,"",istr,&exnpt);
              else
                {
                sprintf(dstr,"%d",defval.lit.int_va);
                status = genint(0,dstr,istr,&exnpt);
                }
              break;

              case C_FLO_VA: 
              igplma(prompt,IG_INP);
              if ( optpar ) status = genflt(0,"",istr,&exnpt);
              else
                {
                sprintf(dstr,"%g",defval.lit.float_va);
                status = genflt(0,dstr,istr,&exnpt);
                }
              break;

              case C_STR_VA:
              if ( optpar )
                {
                if ( menpar )
                  {
                  igplma(prompt,IG_MESS);
                  status = genstm(mnum,&exnpt);
                  }
                else
                  {
                  igplma(prompt,IG_INP);
                  status = genstr(0,"",istr,&exnpt);
                  }
                }
              else
                {
                if ( menpar )
                  {
                  igplma(prompt,IG_MESS);
                  status = genstm(mnum,&exnpt);
                  }
                else
                  {
                  igplma(prompt,IG_INP);
                  status = genstr(0,defval.lit.str_va,istr,&exnpt);
                  }
                }
              break;

              case C_VEC_VA:
              igplma(prompt,IG_MESS);
              if ( pospar )
                {
                oldpmd = posmod; posmod = posalt;
                status = genpos(0,&exnpt); posmod = oldpmd;
                }
              else { status = genpos(0,&exnpt); }
              break;

              case C_REF_VA:
              igplma(prompt,IG_MESS);
              status = genref(0,&typmsk,&exnpt,&end,&right);
              if ( iggflg )
                {
                pmtcon(exnpt,(pm_ptr)NULL,&arglst,&dummy);
                stlook("GLOBAL_REF",&kind,&ref);
                pmcfue(ref,arglst,&exnpt);
                }
              break;

              default:
              WPerhg();    
              status = erpush("IG5302",name);
              goto exit;
              }

            if ( optpar && (status == REJECT) )
              {
              optflg = TRUE;
              pmclie(&defval,&exnpt);
              }
            else if ( status < 0 ) goto rject2;

            igrsma();
            }
          }
/*
***Parameter utan promptstr�ng.
*/
        else pmclie(&defval,&exnpt);
/*
***L�nka in parametern i i parameterlistan.
*/
        pmtcon(exnpt,parlst,&parlst,&dummy);
        pmsbla(newmod);                  /* set new base adress */
        }
/*
***Skapa satsen, spara sekvensnummer i prtid.
***Nytt anrop till pmcpas 15/8/93 JK.
*/
    prtid = iggnid();

    if ( pmcpas(prtid,filnam,(pm_ptr)NULL,parlst,oblparam,
                (pm_ptr)NULL,&retla) != 0 )
      {
      status = erpush("IG5023","PART");
      goto exit;
      }
/*
***Om det �r en MACRO-modul kan den komma att skapa nya satser i PM.
***D�rf�r st�dar vi bort det nyss genererade part-anropet innan vi
***interpreterar. Under alla omst�ndigheter skall anropet st�das
***bort om det �r n�got annat �n en part eller om ritsystemet �r
***aktivt.
*/
    if ( atyp != PART ||  v3mode == RIT_MOD ) pmrele();
/*
***Prova att interpretera.
***Om exekveringen inte gick bra kan skr�p i GM beh�va
***st�das bort. Dessutom skall anropet i PM strykas.
***Med X k�r vi med v�nt-hanteringen p�slagen.
*/
#ifdef V3_X11
    WPwait(GWIN_ALL,TRUE);
#endif

    status = inssta(retla);

#ifdef V3_X11
    WPwait(GWIN_ALL,FALSE);
#endif

    if ( status < 0 )
      {
      if ( defnap.save == 1 )
        {
        prtref.seq_val = prtid; prtref.ord_val = 1;
        prtref.p_nextre = NULL; EXdel(&prtref);
        }
      if ( atyp == PART ) pmrele();
      if ( atyp == MFUNC ) status = erpush("IG5222","MACRO");
      else                 status = erpush("IG5222","PART");
      goto exit;
      }
/*
***EXIT med felmeddelande.
*/
    else if ( status == 3 )
      {
      if ( defnap.save == 1 )
        {
        prtref.seq_val = prtid; prtref.ord_val = 1;
        prtref.p_nextre = NULL; EXdel(&prtref);
        }
      status = 0;
      goto exit;
      }
/*
***EXIT utan felmeddelande.
*/
    else if ( status == 4 ) status = 0;
/*
***Interpreteringen gick bra. Om basmodulen aktiv, och det
***�r ett PART-anrop, l�nka in satsen i satslistan. 
*/
    if ( atyp == PART )
      {
      if ( v3mode & BAS_MOD  &&  pmlmst(actmod, retla) < 0 )
        {
        status = erpush("IG5043","");
        goto exit;
        }
      }
/*
***Avslutning.
*/
    status = 0;
exit:
    tmpref = oldtrf;
/*
***Om ej part, stryk satsen ur PM och �terst�ll statusarea,
***hit och save. Om det �r en part kan �nd� statusarean beh�va
***uppdateras.
*/
    if ( atyp != PART )
      {
      defnap.hit = tmphit;
      }
     
    if ( atyp == MFUNC ) defnap.save = 1;

    actfun = tmpafu;
    WPerhg();    

    return(status);
/*
***Felutg�ngar.
*/
rject2:
    igrsma();
rject1:
    pmrele();
    goto exit;

error1:
    pmsbla(oldmod);
    status = erpush("IG5283","");
    goto exit;

  }

/********************************************************/
/*!******************************************************/

       bool igoptp(
       char *prompt)

/*      Kollar om parameter �r optionell, dvs. om dess
 *      promptstr�ng b�rjar p� $+mellanslag eller $$.
 *
 *      In: Parameterns promptstr�ng.
 *
 *      Ut: Promtstr�ngen strippad p� inledande $.
 *
 *      FV: TRUE  = Optionell parameter.
 *          FALSE = Ej optionell.
 *
 *      (C)microform ab 19/2/88 J. Kjellander
 *
 *      11/11/88 Meny-parametrar, J. Kjellander
 *
 ******************************************************!*/

  {
  char tmp[V3STRLEN+1];

    if ( *prompt == '@'  &&  *(prompt+1) == ' ' )
      {
      strcpy(tmp,prompt);
      strcpy(prompt,tmp+2);
      return(TRUE);
      }
    else if ( *prompt == '@'  &&  *(prompt+1) == '@' )
      {
      strcpy(tmp,prompt);
      strcpy(prompt,tmp+1);
      return(TRUE);
      }
    else return(FALSE);

  }
  
/********************************************************/
/*!******************************************************/

       bool igmenp(
       char  *prompt,
       short *mnum)

/*      Kollar om parameter skall tilldelas v�rde genom
 *      val i meny, dvs. om dess promptstr�ng b�rjar p�
 *      $+m+heltal.
 *
 *      In: prompt = Parameterns promptstr�ng.
 *          mnum   = Pekare till ev. resultat.
 *
 *      Ut: Promtstr�ngen strippad p� inledande kod.
 *          *mnum = Ev. meny-nummer.
 *
 *      FV: TRUE  = Meny-parameter.
 *          FALSE = Ej meny-parameter.
 *
 *      (C)microform ab 11/11/88 J. Kjellander
 *
 ******************************************************!*/

  {
    short n,i,pl;
    char tmp[V3STRLEN+1];

/*
***St�r det @m....
*/
    if ( *prompt == '@'  &&  *(prompt+1) == 'm' )
      {
      pl = strlen(prompt);
      for ( i=2; i<pl; ++i) if ( prompt[i] == ' '  ) break;
      if ( i < pl )
        {
        n = sscanf(prompt+2,"%hd",mnum);
        if ( n == 1 )
          {
          strcpy(tmp,prompt+i+1);
          strcpy(prompt,tmp);
          return(TRUE);
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

       bool igposp(
       char  *prompt,
       short *posalt)

/*      Kollar om VECTOR-parameter skall tilldelas v�rde utan
 *      val i pos-meny, dvs. om dess promptstr�ng b�rjar p�
 *      $+a+heltal.
 *
 *      In: prompt = Parameterns promptstr�ng.
 *          posalt = Pekare till ev. resultat.
 *
 *      Ut: Promtstr�ngen strippad p� inledande kod.
 *          *posalt = Ev. pos-metod.
 *
 *      FV: TRUE  = Pos-parameter.
 *          FALSE = Ej pos-parameter.
 *
 *      (C)microform ab 15/11/88 J. Kjellander
 *
 ******************************************************!*/

  {
    short n,i,pl;
    char tmp[V3STRLEN+1];

    if ( *prompt == '@'  &&  *(prompt+1) == 'a' )
      {
      pl = strlen(prompt);
      for ( i=2; i<pl; ++i) if ( prompt[i] == ' '  ) break;
      if ( i < pl )
        {
        n = sscanf(prompt+2,"%hd",posalt);
        if ( n == 1 )
          {
          strcpy(tmp,prompt+i+1);
          strcpy(prompt,tmp);
          return(TRUE);
          }
        }
      }
    return(FALSE);
  }
  
/********************************************************/
/*!******************************************************/

       bool igtypp(
       char  *prompt,
       DBetype *typmsk)

/*      Kollar om REF-parameter skall till�tas referera till
 *      vad som helst, dvs. om dess promptstr�ng b�rjar p�
 *      $+t+heltal.
 *
 *      In: prompt = Parameterns promptstr�ng.
 *          typmsk = Pekare till ev. resultat.
 *
 *      Ut: Promtstr�ngen strippad p� inledande kod.
 *          *typmsk = Ev. typmask.
 *
 *      FV: TRUE  = Typ-parameter.
 *          FALSE = Ej typ-parameter.
 *
 *      (C)microform ab 25/11/88 J. Kjellander
 *
 ******************************************************!*/

  {
    short n,i,pl;
    int   tmpint;
    char tmp[V3STRLEN+1];

    if ( *prompt == '@'  &&  *(prompt+1) == 't' )
      {
      pl = strlen(prompt);
      for ( i=2; i<pl; ++i) if ( prompt[i] == ' '  ) break;
      if ( i < pl )
        {
        n = sscanf(prompt+2,"%d",&tmpint);
       *typmsk = tmpint;
        if ( n == 1 )
          {
          strcpy(tmp,prompt+i+1);
          strcpy(prompt,tmp);
          return(TRUE);
          }
        }
      }
    return(FALSE);
  }
  
/********************************************************/
/*!******************************************************/

       short igdefp(
       char    *prompt,
       PMLITVA *defval)

/*      Kollar om parameter skall anv�nda default
 *      default-v�rde eller om det skall h�mtas fr�n fil.
 *      @f+v�gbeskrivning.
 *
 *      In: prompt = Parameterns promptstr�ng.
 *          defval = Pekare till defaultv�rde.
 *
 *      Ut: Promtstr�ngen strippad p� inledande kod.
 *          *defval = Ev. nytt defaultv�rde.
 *
 *      FV: 0.
 *
 *      (C)microform ab 11/8/90 J. Kjellander
 *
 *      1/10/91  Strippat �n, J. Kjellander.
 *      10/11/91 G�md parameter, J. Kjellander
 *      12/2/92  Bug "iggtts(119)", J. Kjellander
 *
 ******************************************************!*/

  {
    short  n,i,pl,rn;
    int    ival;
    double fval;
    char   tmp[V3STRLEN+1];
    char   defstr[V3STRLEN+1];
    char   path[80];
    FILE   *f;

/*
***�r det 'Krullalfa + f' ?
*/
    if ( *prompt == '@'  &&  *(prompt+1) == 'f' )
      {
/*
***Packa upp filnamnet.
*/
      pl = strlen(prompt);
      for ( n=2; n<pl; ++n)
        if ( prompt[n] == ' '  ||  prompt[n] == '(' ) break;

      strncpy(path,&prompt[2],n-2);
      path[n-2] = '\0';
/*
***Om det �r "act_job", ers�tt med act_jobdir()+actjobnam().
*/
      if ( strncmp(iggtts(119),path,7) == 0 )
        {
        strcpy(tmp,jobdir);
        strcat(tmp,jobnam);
        strcat(tmp,&path[7]);
        strcpy(path,tmp);
        }
/*
***Packa upp ev. radnr.
*/
      if ( prompt[n] == '(' )
        {
        for ( i=n; i<pl; ++i) if ( prompt[i] == ')' ) break;
        strncpy(tmp,&prompt[n+1],i-n-1);
        tmp[i-n-1] = '\0';
        sscanf(tmp,"%hd",&rn);
        }
      else
        {
        rn = 1;
        i = n - 1;
        }
/*
***L�s v�rde fr�n filen.
*/
      if ( (f=fopen(path,"r")) != 0 )
        {
        for ( n=0; n<rn; ++n )
          fgets(defstr,V3STRLEN,f);
        n = strlen(defstr);
        if ( defstr[n-1] == '\n' ) defstr[n-1] = '\0';
        fclose(f);
/*
***Lagra defaultv�rdet i PMLITVA:n.
*/
        switch ( defval->lit_type )
          {
          case C_STR_VA:
          strcpy(defval->lit.str_va,defstr);
          break;

          case C_INT_VA:
          if ( sscanf(defstr,"%d",&ival) == 1 )
            defval->lit.int_va = ival;
          break;

          case C_FLO_VA:
          if ( sscanf(defstr,"%lf",&fval) == 1 )
            defval->lit.float_va = fval;
          break;
          }
        }
/*
***Strippa promten fr�n v�gbeskrivn etc.
*/
      if ( (int)strlen(prompt) > i+1 )
        {
        strcpy(tmp,prompt+i+2);
        strcpy(prompt,tmp);
        }
      else prompt[0] = '\0';
      }

    return(0);

  }
  
/********************************************************/
/*!******************************************************/

       short igtstp(char *prompt)

/*      Kollar om promten b�rjar med t samt ett nummer
 *      och �vers�tter is�fall till motsvarande t-str�ng.
 *
 *      In: Parameterns promptstr�ng.
 *
 *      Ut: Ursprunglig eller ny promtstr�ng.
 *
 *      FV: 0
 *
 *      (C)microform ab 1996-05-30 J. Kjellander
 *
 ******************************************************!*/

  {
   int  tnum;

   if ( *prompt == 't'  &&  (sscanf(prompt+1,"%d",&tnum) == 1) )
     strcpy(prompt,iggtts((short)tnum));

   return(0);
  }
  
/********************************************************/
/*!******************************************************/

        short iguppt()

/*      Varkonfunktion f�r att uppdatera en part.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: 0      = OK
 *          REJECT = avsluta
 *          GOMAIN = huvudmenyn
 *
 *      Felkod:  IG5222  => Fel vid interpr. av PART-sats
 *               IG5332  => Kan ej hitta PART-sats i PM
 *
 *      (C)microform ab 27/10/88 R. Svedin
 *
 *      1/12/88  Ritpaketet, J. Kjellander
 *      3/5/89   Bug REJECT, J. Kjellander
 *      30/3/92  Ny re-interpretering, J. Kjellander
 *      20/5/94  Bug DBread_part_attributes()-1.10, J. Kjellander
 *
 ******************************************************!*/

  {
    DBetype    typ;
    bool     end,right,dstflg;
    short    status;
    pm_ptr   retla,stlla,dummy;
    PMREFVA  idvek[MXINIV];
    DBPart    part;
    DBPdat   pdat;
    PMMONO  *np;
    V2NAPA   oldnap;

/*
***Initiering.
*/
    dstflg = FALSE;
/*
***Ta reda p� parten:s ID.
*/
    igptma(269,IG_MESS);
    typ = PRTTYP;
    status=getidt(idvek,&typ,&end,&right,(short)0);
    igrsma();
    if ( status < 0 ) goto exit;
/*
***Om ritpaket, skapa partanrop.
*/
    if ( v3mode == RIT_MOD )
      {
      if ( (status=iggnps(idvek)) < 0 ) goto error;
      else dstflg = TRUE;
      }
/*
*** H�mta PM-pekare till partsatsen.
*/
    np = (PMMONO *)pmgadr((pm_ptr)0 );
    stlla = np->pstl_;

    if ( pmlges(idvek, &stlla, &retla) < 0 || retla == (pm_ptr)NULL)
      {
      erpush("IG5332","");
      goto error;
      }

    pmglin(retla, &dummy, &retla);
/*
***K�r om anropet.
*/
    V3MOME(&defnap,&oldnap,sizeof(V2NAPA));
    EXgtpt(idvek,&part);
    DBread_part_parameters(&pdat,NULL,NULL,part.dtp_pt,part.dts_pt);
/*
***Om det handlar om en mycket gammal part, 1.10 etc. kanske den
***inte har n�gra attribut lagrade !!!
*/
    if ( part.dts_pt == sizeof(DBPdat)  &&  pdat.attp_pd > DBNULL )
      DBread_part_attributes((char *)&defnap,(int)pdat.asiz_pd,pdat.attp_pd);

    EXdraw(idvek,FALSE);
    gmmtm((DBseqnum)idvek->seq_val);
    status = inssta(retla); 
    V3MOME(&oldnap,&defnap,sizeof(V2NAPA));

    if ( status < 0 ) 
      {
      erpush("IG5222","PART");
      errmes();
      EXdraw(idvek,FALSE);
      gmumtm();
      EXdraw(idvek,TRUE);
      }
/*
***Avslutning.
*/
exit:
    if ( dstflg ) pmdlst();
    WPerhg();
    return(status);
/*
***Felutg�ng.
*/
error:
    errmes();
    goto exit;
  }

/********************************************************/
/*!******************************************************/

        short igcptw()

/*      F�nstervariant av '�ndra part' gemensam f�r X11
 *      och Windows.
 *
 *      In: Inget.
 *
 *      Ut: Inget.
 *
 *      FV: 0      = OK
 *          REJECT = avsluta
 *          GOMAIN = huvudmenyn
 *
 *      Felkod:  IG5272  => Kan ej ladda modulen %s
 *               IG5222  => Fel vid interpr. av PART-sats
 *               IG5322  => Kan ej �ndra PART-sats
 *               IG3982  => Fel vid analysering av parameter
 *               IG4072  => Parten har inga parametrar
 *
 *      (C)microform ab 2/9/92 J. Kjellander
 *
 *      20/5/94 Bug DBread_part_attributes()-1.10, J. Kjellander
 *      13/2/95 ALLTYP, J. Kjellander
 *      1996-04-30 Noll parametrar, J. Kjellander
 *      1996-12-12 Flyttad till igepac, J.Kjellander
 *      1998-09-17 t/@ i promtstr�ngar, J.Kjellander
 *      1998-09-25 actfun, J.Kjellander
 *      2004-02-21 pmmark()+pmrele(), J.Kjellander
 *
 ******************************************************!*/

  {
    int     typarr[V2MPMX];                 /* Parametertyper */
    char    oldval[V2MPMX][V3STRLEN];       /* Gamla v�rden */
    char   *osparr[V2MPMX];         
    char    newval[V2MPMX][V3STRLEN];       /* Nya v�rden */
    char   *nsparr[V2MPMX];         
    char    pmtarr[V2MPMX][V3STRLEN+1];     /* Promptar */
    char   *pmparr[V2MPMX];         
    short   maxtkn[V2MPMX];                 /* Str�ngl�ngder */
    short   pant;                           /* Antal parametrar */
    short   i,status;
    char    parnam[V3STRLEN+1];
    char    rubrik[V3STRLEN],errbuf[V3STRLEN];
    pm_ptr  pla[V2MPMX];                    /* Parametrars PM-adress */
    pm_ptr  oldmod,newmod,panola,exnpt,retla;
    DBetype   typ;
    bool    end,right,edit,ref,dstflg;
    short   oldafn;
    ANFSET  set;
    ANATTR  attr;
    DBPart   part;
    PMREFVA idvek[MXINIV];
    DBPdat  pdat;
    PMLITVA defval;
    V2NAPA  oldnap;

/*
***Denna funktion f�r bara anropas med X11 eller Windows
***som terminaltyp.
*/
    if ( igtrty != X11  &&  igtrty != MSWIN ) return(0);
/*
***Initiering.
*/
    edit = dstflg = FALSE;
    pmmark();
/*
***Ta reda p� parten:s ID.
*/
    igptma(269,IG_MESS);
    typ = PRTTYP;
    status=getidt(idvek,&typ,&end,&right,(short)0);
    igrsma();

    if ( status < 0 )
      goto exit;
    else
      {
      idvek[0].ord_val = 1;
      idvek[0].p_nextre = NULL;
      }
/*
***L�s part-posten.
*/
    if ( EXgtpt(idvek,&part) < 0 ) goto error1;
/*
***Om ritpaket, skapa partanrop.
*/
    if ( v3mode == RIT_MOD )
      {
      if ( (status=iggnps(idvek)) < 0 ) goto error1;
      else dstflg = TRUE;
      }
/*
***Notera nuvarande parametrars antal och PM-adresser.
*/
    pmgpla(idvek,&pant,pla);
/*
***Om parametrar saknas slutar vi h�r.
*/
   if ( pant == 0 )
     {
     erpush("IG4072",part.name_pt);
     goto error1;
     }
/*
***Dekompilera de aktuella parametrarnas v�rde till str�ngar
***s� att l�mpliga v�rden kan presenteras i inmatningsf�lten.
*/
    for ( i=0; i<pant; ++i )
      {
      maxtkn[i] = V3STRLEN;
      pprexs(pla[i],modtyp,oldval[i],maxtkn[i]);
      }
/*
***�r parten refererad ?
*/
    if ( v3mode & BAS_MOD  &&  pmamir(idvek) ) ref = TRUE;
    else  ref = FALSE;
/*
***Ladda in modulen.
*/
    oldmod = pmgbla();
    if ( pmgeba(part.name_pt,&newmod) != 0 )
      {
      erpush("IG5272",part.name_pt);
      goto error1;
      }
/*
***G�r den anropade modulen aktiv och plocka fram promtar
***och typer.
*/
    pmsbla(newmod);
    pmrpap((pm_ptr)0);

    for ( i=0; i<pant; ++i )
      {
      pmgpad(&panola);
/*
***Initiera pekare till str�ngar f�r senare indata till WPmsip().
*/
      osparr[i] = oldval[i];
      nsparr[i] = newval[i];
      pmparr[i] = pmtarr[i];
/*
***L�s parameterns namn, prompt och defaultv�rde.
*/
      pmrpar(panola,parnam,pmtarr[i],&defval);
/*
***Mappa ev. t-str�ng till klartext och strippa
***ev. inledande "@ " eller om det �r 2 @ strippa
***det 1:a.
*/
      if ( strlen(pmtarr[i]) > 0 )
        {
        igtstp(pmtarr[i]);
        igoptp(pmtarr[i]);
        }
/*
***Parameter utan promt �r g�md.
*/
     else strcpy(pmtarr[i],iggtts(1627));
/*
***Vilken typ har den ?
*/
      typarr[i] = defval.lit_type;
/*
***L�gg till typ och namn efter promten.
*/
      switch(defval.lit_type)
        {
        case C_INT_VA: strcat(pmtarr[i],"  (INT "); break;
        case C_FLO_VA: strcat(pmtarr[i],"  (FLOAT "); break;
        case C_STR_VA: strcat(pmtarr[i],"  (STRING "); break;
        case C_VEC_VA: strcat(pmtarr[i],"  (VECTOR "); break;
        case C_REF_VA: strcat(pmtarr[i],"  (REF "); break;
        }
      strcat(pmtarr[i],parnam);
      strcat(pmtarr[i],")");
      }

    pmsbla(oldmod);
/*
***L�t hj�lpsystemet f� veta vad vi g�r.
*/
    oldafn = actfun;
    actfun = -2;
    strcpy(actpnm,part.name_pt);
/*
***Anropa WPmsip. t1599 = "Parametrar f�r parten : "
*/
    strcpy(rubrik,iggtts(1599));
    strcat(rubrik,part.name_pt);
#ifdef V3_X11
    status = WPmsip(rubrik,pmparr,osparr,nsparr,maxtkn,typarr,pant);
#endif
#ifdef WIN32
    status = (short)msmsip(rubrik,pmparr,osparr,nsparr,maxtkn,typarr,pant);
#endif

    actfun = oldafn;

    if ( status < 0 ) goto exit;
/*
***G� igenom de nya str�ngarna och byt ut alla parametrar i
***anropet som har �ndrats.
*/
    for ( i=0; i<pant; ++i )
      {
      if ( strcmp(nsparr[i],osparr[i]) != 0 )
        {
/*
***Parametern "i" har �ndrats ! Analysera den nya MBS-str�ngen.
*/
        anlogi();
        if ( (status=asinit(nsparr[i],ANRDSTR)) < 0 ) goto exit;
        ancset(&set,NULL,0,0,0,0,0,0,0,0,0);
        anascan(&sy);
        anarex(&exnpt,&attr,&set);
        if ( (status=asexit()) < 0 ) goto exit;
/*
***Hur gick det.
*/ 
        if ( anyerr() )
          {
          sprintf(errbuf,"%d%%%s",i+1,nsparr[i]);
          erpush("IG3982",errbuf);
          goto error1;
          }
/*
***Kolla om det nya parameterv�rdet inneb�r fram�t-referenser.
*/
        if ( v3mode & BAS_MOD  &&  pmarex(idvek,exnpt) == TRUE )
          {
          erpush("IG3882",nsparr[i]);
          goto error1;
          }
/*
***Byt parametern i part-anropet.
*/
        if ( pmchpa(idvek,(short)(i+1),exnpt,&retla) < 0 || retla == (pm_ptr)NULL )
          {
          erpush("IG5322","");
          goto error1;
          }
        edit = TRUE;
        }
      }
/*
***Om refererad, fr�ga om hela modulen skall k�ras. Annars
***k�r bara om part-anropet.
*/
    if ( edit )
      {
      if ( ref && igialt(373,67,68,FALSE) ) igramo();
/*
***Reinterpretera inkrementellt.
*/
      else
        {
        V3MOME(&defnap,&oldnap,sizeof(V2NAPA));
        DBread_part_parameters(&pdat,NULL,NULL,part.dtp_pt,part.dts_pt);
        if ( part.dts_pt == sizeof(DBPdat)  &&  pdat.attp_pd > DBNULL )
          DBread_part_attributes((char *)&defnap,(int)pdat.asiz_pd,pdat.attp_pd);
        EXdraw(idvek,FALSE);
        gmmtm((DBseqnum)idvek->seq_val);
#ifdef V3_X11
        WPwait(GWIN_MAIN,TRUE);
#endif
        status = inssta(retla); 
#ifdef V3_X11
        WPwait(GWIN_MAIN,FALSE);
#endif
        V3MOME(&oldnap,&defnap,sizeof(V2NAPA));

        if ( status < 0  ||  status == 3 ) 
          {
          if ( status < 0 )
            {
            erpush("IG5222","PART");
            errmes();
            }
          EXdraw(idvek,FALSE);
          gmumtm();
          EXdraw(idvek,TRUE);
          if ( v3mode & BAS_MOD ) for ( i=0; i<pant; ++i )
            pmchpa(idvek,(short)(i+1),pla[i],&retla);
          }
        }
      }
/*
***Avslutning.
*/
exit:
    if ( dstflg ) pmdlst();
    if ( v3mode == RIT_MOD ) pmrele();
    WPerhg();
    return(status);
/*
***Felutg�ngar.
*/
error1:
    errmes();
    goto exit;
  }

/********************************************************/
/*!******************************************************/

       short iggnps(PMREFVA *id)

/*    Genererar en part-anropssats med utseende enl.
 *    part-post i GM.
 *
 *      In: id  => Partens ID.
 *
 *      Ut: Inget.
 *
 *      Felkod: IG5023 = Kan ej skapa PART sats 
 *              IG5342 = N�gon parameter �r call by reference
 *
 *      (C)microform ab 29/11/88 J. Kjellander
 *
 ******************************************************!*/

  {
    short    prtid;               /* Partens sekvensnummer */
    pm_ptr   dummy;               /* Som det l�ter */
    pm_ptr   oblpar;              /* Obligatoriska parametrar */
    pm_ptr   parlst;              /* Parameterlista */
    pm_ptr   exnpt;               /* Pekare till expr. node */
    pm_ptr   retla;               /* Pekare till den skapade satsen */
    short    status;              /* Anropade rutiners status */
    short    i;                   /* Loop-variabel */
    PMLITVA  litval;              /* Litter�rt v�rde */
    DBPart    part;                /* Part-posten */
    DBPdat   data;                /* Part-data */
    PMPATLOG typarr[V2MPMX];      /* Parametertyper */

/*
***L�s part-posten.
*/
    if ( (status=EXgtpt(id,&part)) < 0 ) return(status);
    DBread_part_parameters(&data,typarr,NULL,part.dtp_pt,part.dts_pt);
/*
***Ta reda p� modulens attribut LOCAL/GLOBAL, om LOCAL,
***skapa referens till lokalt koordinatsystem.
*/
    if ( data.matt_pd  == LOCAL )
        {
        litval.lit_type = C_REF_VA;
        DBget_id(data.csp_pd,&litval.lit.ref_va[0]);
        pmclie(&litval,&exnpt);
        pmtcon(exnpt,(pm_ptr)NULL,&oblpar,&dummy);
        }
     else oblpar = (pm_ptr)NULL;
/*
***Skapa parameterlistan.
*/
    parlst = (pm_ptr)NULL;

    for( i=0; i<data.npar_pd; ++i)
      {
      switch ( typarr[i].log_id )
        {
        case C_INT_VA:
        litval.lit_type = C_INT_VA;
        if ( (status=EXgint(id,(short)(i+1),&litval)) < 0 ) return(status);
        break;

        case C_FLO_VA: 
        litval.lit_type = C_FLO_VA;
        if ( (status=EXgflt(id,(short)(i+1),&litval)) < 0 ) return(status);
        break;

        case C_STR_VA:
        litval.lit_type = C_STR_VA;
        if ( (status=EXgstr(id,(short)(i+1),&litval)) < 0 ) return(status);
        break;

        case C_VEC_VA:
        litval.lit_type = C_VEC_VA;
        if ( (status=EXgvec(id,(short)(i+1),&litval)) < 0 ) return(status);
        break;

        case C_REF_VA:
        litval.lit_type = C_REF_VA;
        if ( (status=EXgref(id,(short)(i+1),&litval)) < 0 ) return(status);
        break;

        default:
        return(erpush("IG5342",""));
        }

      pmclie(&litval,&exnpt);
      pmtcon(exnpt,parlst,&parlst,&dummy);
      }
/*
***Skapa partsats.
*/
    prtid = id[0].seq_val;

    if ( pmcpas(prtid,part.name_pt,(pm_ptr)NULL,parlst,
                oblpar,(pm_ptr)NULL,&retla) != 0 )
      return(erpush("IG5023","PART"));
    else
      pmlmst(actmod,retla);

    return(0);

  }

/********************************************************/
