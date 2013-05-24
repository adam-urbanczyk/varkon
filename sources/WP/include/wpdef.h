/**********************************************************************
*
*    wpdef.h
*    =======
*
*    This file is part of the VARKON WindowPac Library.
*    URL: http://www.varkon.com
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

#ifdef V3_OPENGL
#include <GL/glx.h>
#else
#define GLXContext int
#endif

/*
***Max number of windows in wpwtab.
*/
#define WTABSIZ 100

/*
***Window types.
*/
#define TYP_UNDEF   0          /* Odefinierad typ */
#define TYP_IWIN    1          /* Input-F�nster */
#define TYP_EDIT    2          /* Edit */
#define TYP_ICON    3          /* Icon */
#define TYP_BUTTON  4          /* Button */
#define TYP_LWIN    5          /* List-f�nster */
#define TYP_GWIN    6          /* Grafiskt f�nster */
#define TYP_RWIN    7          /* Renderings-f�nster (OpenGL) */

/*
***Colors.
*/
#define WP_NPENS 256             /* Antal normala pennor */
#define WP_SPENS 5               /* Antal systempennor */
#define WP_BGND  WP_NPENS        /* Bakgrund */
#define WP_FGND  WP_NPENS+1      /* F�rgrund */
#define WP_TOPS  WP_NPENS+2      /* Toppskugga */
#define WP_BOTS  WP_NPENS+3      /* Bottenskugga */
#define WP_NOTI  WP_NPENS+4      /* Notify */

/*
***Rubberband modes.
*/
#define WP_RUB_NONE    0         /* Ingen figur */
#define WP_RUB_RECT    1         /* Rektangel */
#define WP_RUB_LINE    2         /* R�t linje */
#define WP_RUB_ARROW   3         /* Pil och ring */
/*
**Window-ID.
*/
typedef DBint  wpw_id;          /* Ett f�nster-ID �r en int */

typedef struct
{
wpw_id     w_id;               /* Eget ID, dvs. entry i "�garens wintab" */
wpw_id     p_id;               /* �garens ID */
Window     x_id;               /* X-window ID */
} WPWID;

/*
***Window geometry.
*/
typedef struct
{
short      x;                  /* L�ge X-koordinat */
short      y;                  /* L�ge Y-koordinat */
short      dx;                 /* Storlek i X-led */
short      dy;                 /* Storlek i Y-led */
short      bw;                 /* Ramens tjocklek */
double     psiz_x;             /* Pixelstorlek i X-led i mm. */
double     psiz_y;             /* Pixelstorlek i X-led i mm. */
} WPWGEO;

/*
***Window colors.
*/
typedef struct
{
short      bckgnd;             /* Bakgrundsf�rg */
short      forgnd;             /* F�rgrundsf�rg */
} WPWCOL;

/*
***A window.
*/
typedef struct
{
char       typ;                /* Typ av f�nster, tex. TYP_IWIN */
char      *ptr;                /* C-pekare till en f�nster-structure */
} WPWIN;

/*
***A graphical coordinate.
*/
typedef struct
{
double     x;                  /* X-koordinat */
double     y;                  /* Y-koordinat */
double     z;                  /* Z-koordinat */
char       a;                  /* T�nd/sl�ck + start/slut */
} WPGRPT;

/*
***The model/screen window of a graphical view.
*/
typedef struct
{
double     xmin,xmax;          /* Gr�nser i X-led */
double     ymin,ymax;          /* Gr�nser i Y-led */
} VYWIN;

/*
***A view transformation.
*/
typedef struct
{
double     k11,k12,k13;        /* 1:a raden */
double     k21,k22,k23;        /* 2:a raden */
double     k31,k32,k33;        /* 3:e raden */
} VYTRF;

/*
***A graphical view.
*/
typedef struct
{
char       vynamn[GPVNLN+1];   /* Vynamn */
bool       valid;              /* FALSE = Ej vettiga data */
bool       vy_3D;              /* TRUE om 3D-vy */
double     vydist;             /* Perspektivavst�nd */
double     k1x,k2x,k1y,k2y;    /* 2D vy-transformation */
VYWIN      scrwin;             /* Sk�rmf�nster */
VYWIN      modwin;             /* Modellf�nster */
VYTRF      vymat;              /* 3D vy-transformation */
} WPVY;

/*
***A Button window.
*/
typedef struct
{
WPWID      id;                 /* ID */
WPWGEO     geo;                /* Geometri */
WPWCOL     color;              /* F�rger */
short      font;               /* Fontnummer */
char       stron[81];          /* Text  f�r knapp p� */
char       stroff[81];         /* Text  f�r knapp av */
bool       status;             /* Logiskt tillst�nd, default = FALSE */
short      acttyp;             /* Aktionstyp, MENU/FUNC etc. */
short      actnum;             /* Aktionens nummer */
bool       hlight;             /* Highlight, TRUE/FALSE */
} WPBUTT;

/*
***A graphical window.
*/

#define GWEM_NORM    (ButtonPressMask      | \
                      ExposureMask         | \
                      StructureNotifyMask  | \
                      KeyPressMask         | \
                      FocusChangeMask )

#define GWEM_RUB     (ButtonPressMask      | \
                      ButtonReleaseMask    | \
                      ButtonMotionMask     | \
                      PointerMotionHintMask)

#define WP_GWMAX      25       /* Max antal WPGWIN-f�nster */
#define WP_GWSMAX     100      /* Max antal sub-f�nster i ett WPGWIN */
#define WP_NIVANT     2000     /* Max antal niv�er/f�nster */
#define WP_NTSIZ      250      /* Niv�tabellens storlek i bytes */
#define WP_PMKMAX     10       /* Max antal pekm�rken */

typedef struct
{
WPWID         id;                 /* WP-ID */
WPWGEO        geo;                /* X-Geometri */
WPWIN         wintab[WP_GWSMAX];  /* Subf�nster f�r snabbval */
Pixmap        savmap;             /* Pixmap f�r 'save-under' */
GC            win_gc;             /* Normalt grafiskt GC */
GC            rub_gc;             /* Rubber-band GC */
WPVY          vy;                 /* Aktiv grafisk vy */
WPVY          old_vy;             /* F�reg�ende grafisk vy */
DFPOST       *df_adr;             /* 1:a posten i DF eller NULL */
DBint         df_all;             /* Antal allokerade DF-poster */
DBint         df_ptr;             /* Sista anv�nda posten i DF */
DBint         df_cur;             /* Aktuell DF-post */
unsigned char nivtab[WP_NTSIZ];   /* Niv�tabell */
DBint         pmkant;             /* Antal aktiva pekm�rken */
XPoint        pmktab[WP_PMKMAX];  /* Pekm�rkestabellen */
bool          reprnt;             /* Bytt f�r�lder TRUE/FALSE */
short         wmandx;             /* Flyttad i X-led f�re f�r�ldrabyte */
short         wmandy;             /* Flyttad i Y-led f�re f�r�ldrabyte */
WPBUTT       *mesbpt;             /* Pekare till meddelanderaden eller NULL */
double        linwdt;             /* Aktiv linjebredd */
} WPGWIN;

/*
***An OpenGL Dynamic Rendering window.
*/
#define RWEM_NORM    (ButtonPressMask       | \
                      ButtonReleaseMask     | \
                      ExposureMask          | \
                      StructureNotifyMask   | \
                      KeyPressMask          | \
                      ButtonMotionMask      | \
                      PointerMotionHintMask | \
                      FocusChangeMask )

#define WP_RWSMAX     100      /* Max antal sub-f�nster i ett WPRWIN */

typedef struct
{
WPWID         id;                 /* WP-ID */
WPWGEO        geo;                /* X-Geometri */
WPWIN         wintab[WP_GWSMAX];  /* Subf�nster f�r snabbval */
Pixmap        savmap;             /* Pixmap f�r 'save-under' */
GC            win_gc;             /* Normalt grafiskt GC */
GC            rub_gc;             /* Rubber-band GC */
GLXContext    rc;                 /* OpenGL rendering context */
XVisualInfo  *pvinfo;             /* Visual */
Colormap      colmap;             /* Colormap */
unsigned long bgnd;               /* Pixelv�rde f�r knappbakgrund */
unsigned long fgnd;               /* Dito f�rgrund */
unsigned long tops;               /* Dito topskugga */
unsigned long bots;               /* Dito bottenskugga */
unsigned long noti;               /* Dito notify */
WPVY          vy;                 /* Aktiv grafisk vy */
bool          rgb_mode;           /* RGB eller Colorindex */
bool          double_buffer;      /* Double buffer eller single */
double        xmin,xmax,ymin,ymax,
              zmin,zmax;          /* Vy-box */
DBint         musmod;             /* Aktuell mosmode */
double        rotx,roty,rotz;     /* Aktuell rotation */
double        movx,movy;          /* Aktuell translation */
double        scale;              /* Aktuell skalfaktor */
double        light;              /* Aktuell ljusstyrka 0 -> 100 */
double        pfactor;            /* Perspektivfaktor 0 -> 100 */
double        ambient[3];         /* Ambient ljusfaktor */
double        diffuse[3];         /* Diffuse ljusfaktor */
double        specular[3];        /* Specualar ljusfaktor */
bool          zclip;              /* Z-klipp p�/av */
double        zfactor;            /* Aktuell Z-klippfaktor 0 -> 100 */
} WPRWIN;

/*
***An Input window.
*/
#define WP_IWSMAX    500       /* Max antal sub-f�nster i ett WPIWIN */

typedef struct
{
WPWID      id;                 /* ID */
WPWGEO     geo;                /* Geometri */
WPWIN      wintab[WP_IWSMAX];  /* Subf�nster */
bool       mapped;             /* Mappat i X, ja/nej */
} WPIWIN;

/*
***A List window.
*/
#define WP_LWSMAX     3        /* Max antal sub-f�nster i ett WPIWIN */

typedef struct
{
WPWID      id;                 /* ID */
WPWGEO     geo;                /* Geometri */
WPWIN      wintab[WP_LWSMAX];  /* Subf�nster, scroll och save */
FILE      *filpek;             /* Pekare till list-fil */
char       filnam[V3PTHLEN+1]; /* Listfilens namn (inkl. path) */
char       rubrik[V3STRLEN+1]; /* Listans rubrik, dvs. lst_ini(rubrik) */
DBint      maxrln;             /* Max radl�ngd */
DBint      rstart;             /* Radb�rjan */
DBint      trant;              /* Totalt antal rader */
DBint      frant;              /* Antal rader i f�nstret */
bool       sscrol;             /* TRUE=Sid-scroll, FALSE=Rad-scroll */
} WPLWIN;

/*
***An Edit window.
*/
#define    FIRST_EDIT    1     /* 1:a edit-f�nstret */
#define    NEXT_EDIT     2     /* N�sta edit-f�nster */
#define    PREV_EDIT     3     /* F�reg�ende edit-f�nster */
#define    FOCUS_EDIT    4     /* Aktiva edit-f�nstret */
#define    LAST_EDIT     5     /* Sista edit-f�nstret */

typedef struct
{
WPWID      id;                 /* ID */
WPWGEO     geo;                /* Geometri */
char       str[V3STRLEN+1];    /* Edit-str�ng */
char       dstr[V3STRLEN+1];   /* Default-str�ng */
DBint      tknmax;             /* Max antal tecken */
DBint      scroll;             /* Scroll-position */
DBint      curpos;             /* Cursor-position */
bool       fuse;               /* Default-s�kring */
bool       focus;              /* TRUE = Input focus */
int        symbol;             /* Senast inmatade symbol */
} WPEDIT;

/*
***An Icon window.
*/

typedef struct
{
WPWID      id;                 /* ID */
WPWGEO     geo;                /* Geometri */
WPWCOL     color;              /* F�rger */
Pixmap     bitmap;             /* Bitmap */
short      acttyp;             /* Aktionstyp, MENU/FUNC etc. */
short      actnum;             /* Aktionens nummer */
} WPICON;
