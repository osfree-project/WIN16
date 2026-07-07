/*
 *  intl_date.c Ц диалог формата даты
 */
#include "intl.h"

/* ---------- вспомогательные функции (локальные) ---------- */
static int FarStrnicmp(const char FAR *s1, const char FAR *s2, int n) {
    while (n-- > 0) { char c1=*s1++,c2=*s2++;
        if(c1>='A'&&c1<='Z') c1+='a'-'A'; if(c2>='A'&&c2<='Z') c2+='a'-'A';
        if(c1!=c2) return (unsigned char)c1-(unsigned char)c2;
        if(c1=='\0') return 0;
    }
    return 0;
}
static int DayOfWeek(int y, int m, int d) { static int t[]={0,3,2,5,0,3,5,1,4,6,2,4}; y-=m<3; return (y+y/4-y/100+y/400+t[m-1]+d)%7; }
static void FAR GetLocalDateTime(WORD FAR *y, WORD FAR *mo, WORD FAR *d, WORD FAR *h, WORD FAR *mi, WORD FAR *s) {
    static struct dosdate_t date; static struct dostime_t time;
    _dos_getdate(&date); _dos_gettime(&time);
    *y=date.year; *mo=date.month; *d=date.day;
    *h=time.hour; *mi=time.minute; *s=time.second;
}

/* ---------- статические данные дл€ диалога даты ---------- */
static const char FAR * MonthTokens[] = { "M","MM","MMM","MMMM" };
static const char FAR * DayTokens[]   = { "d","dd" };
static const char FAR * YearTokens[]  = { "yy","yyyy" };
static const char FAR * WeekTokens[]  = { "ddd","dddd" };
static HWND g_hwndAll[7]; static int g_width[7],g_height[7],g_gap[6],g_rowX,g_rowY;

static void FAR SaveOriginalPositions(HWND hDlg) {
    HWND hwndItems[7]; int i,j; RECT rc; POINT pt;
    hwndItems[0]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1); hwndItems[1]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT1);
    hwndItems[2]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2); hwndItems[3]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT2);
    hwndItems[4]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3); hwndItems[5]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT3);
    hwndItems[6]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4);
    for (i=0;i<7;i++) for (j=i+1;j<7;j++) { RECT r1,r2; GetWindowRect(hwndItems[i],&r1); GetWindowRect(hwndItems[j],&r2); if (r1.left>r2.left) { HWND t=hwndItems[i]; hwndItems[i]=hwndItems[j]; hwndItems[j]=t; } }
    GetWindowRect(hwndItems[0],&rc); pt.x=rc.left; pt.y=rc.top; ScreenToClient(hDlg,&pt); g_rowX=pt.x; g_rowY=pt.y;
    for (i=0;i<7;i++) { g_hwndAll[i]=hwndItems[i]; GetWindowRect(hwndItems[i],&rc); g_width[i]=rc.right-rc.left; g_height[i]=rc.bottom-rc.top; }
    for (i=0;i<6;i++) { RECT rL,rR; GetWindowRect(hwndItems[i],&rL); GetWindowRect(hwndItems[i+1],&rR); g_gap[i]=rR.left-rL.right; }
}

static void FAR ArrangeLongDateOrder(HWND hDlg, int order) {
    HWND hwndRow[7]; int i;
    if (order==0) { hwndRow[0]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4); hwndRow[1]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT1); hwndRow[2]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1); hwndRow[3]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT2); hwndRow[4]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2); hwndRow[5]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT3); hwndRow[6]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3); }
    else if (order==1) { hwndRow[0]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4); hwndRow[1]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT1); hwndRow[2]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2); hwndRow[3]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT2); hwndRow[4]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1); hwndRow[5]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT3); hwndRow[6]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3); }
    else { hwndRow[0]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3); hwndRow[1]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT1); hwndRow[2]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1); hwndRow[3]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT2); hwndRow[4]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2); hwndRow[5]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT3); hwndRow[6]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4); }
    { int x=g_rowX; for (i=0;i<7;i++) { int k; for (k=0;k<7;k++) if (g_hwndAll[k]==hwndRow[i]) break; SetWindowPos(hwndRow[i],NULL,x,g_rowY,g_width[k],g_height[k],SWP_NOZORDER|SWP_NOACTIVATE); if (i<6) x+=g_width[k]+g_gap[i]; } }
}

static void FAR FillComboWithDateValues(HWND hDlg, WORD year, WORD month, WORD day) {
    char buf[32]; HWND hCombo; int dow=DayOfWeek(year,month,day); LCTYPE lcFull, lcAbbr;
    hCombo=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1); SendMessage(hCombo,CB_RESETCONTENT,0,0);
    wsprintf(buf,"%u",month); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    wsprintf(buf,"%02u",month); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    lcFull=LOCALE_SMONTHNAME1+(month-1); lcAbbr=LOCALE_SABBREVMONTHNAME1+(month-1);
    GetLocaleInfo(LOCALE_USER_DEFAULT,lcAbbr,buf,sizeof(buf)); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    GetLocaleInfo(LOCALE_USER_DEFAULT,lcFull,buf,sizeof(buf)); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    hCombo=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2); SendMessage(hCombo,CB_RESETCONTENT,0,0);
    wsprintf(buf,"%u",day); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    wsprintf(buf,"%02u",day); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    hCombo=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3); SendMessage(hCombo,CB_RESETCONTENT,0,0);
    wsprintf(buf,"%02u",year%100); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    wsprintf(buf,"%04u",year); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    hCombo=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4); SendMessage(hCombo,CB_RESETCONTENT,0,0);
    lcFull=LOCALE_SDAYNAME1+dow; lcAbbr=LOCALE_SABBREVDAYNAME1+dow;
    GetLocaleInfo(LOCALE_USER_DEFAULT,lcAbbr,buf,sizeof(buf)); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    GetLocaleInfo(LOCALE_USER_DEFAULT,lcFull,buf,sizeof(buf)); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
}
static int FindMonthIndex(const char* fmt) { int i; for (i=0;i<lstrlen(fmt);i++) { if(FarStrnicmp(fmt+i,"MMMM",4)==0) return 3; if(FarStrnicmp(fmt+i,"MMM",3)==0) return 2; if(FarStrnicmp(fmt+i,"MM",2)==0) return 1; if(*(fmt+i)=='M') return 0; } return 3; }
static int FindDayIndex(const char* fmt) { int i; for (i=0;i<lstrlen(fmt);i++) { if(FarStrnicmp(fmt+i,"dd",2)==0) return 1; if(*(fmt+i)=='d') return 0; } return 1; }
static int FindYearIndex(const char* fmt) { int i; for (i=0;i<lstrlen(fmt);i++) { if(FarStrnicmp(fmt+i,"yyyy",4)==0) return 1; if(FarStrnicmp(fmt+i,"yy",2)==0) return 0; if(*(fmt+i)=='y') return 0; } return 1; }
static int FindWeekIndex(const char* fmt) { int i; for (i=0;i<lstrlen(fmt);i++) { if(FarStrnicmp(fmt+i,"dddd",4)==0) return 1; if(FarStrnicmp(fmt+i,"ddd",3)==0) return 0; } return 1; }
static void FAR GetCurrentFormatString(HWND hDlg, char* outFmt) {
    HWND hwndElements[7]; int i,j;
    hwndElements[0]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1); hwndElements[1]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT1);
    hwndElements[2]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2); hwndElements[3]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT2);
    hwndElements[4]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3); hwndElements[5]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT3);
    hwndElements[6]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4);
    for (i=0;i<7;i++) for (j=i+1;j<7;j++) { RECT r1,r2; GetWindowRect(hwndElements[i],&r1); GetWindowRect(hwndElements[j],&r2); if (r1.left>r2.left) { HWND t=hwndElements[i]; hwndElements[i]=hwndElements[j]; hwndElements[j]=t; } }
    outFmt[0]='\0';
    for (i=0;i<7;i++) { HWND hwnd=hwndElements[i]; char szClass[16]; GetClassName(hwnd,szClass,sizeof(szClass));
        if (lstrcmpi(szClass,"ComboBox")==0) { LRESULT sel=SendMessage(hwnd,CB_GETCURSEL,0,0); if (sel!=CB_ERR && sel>=0) { if (hwnd==GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1)) lstrcat(outFmt,MonthTokens[sel]); else if (hwnd==GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2)) lstrcat(outFmt,DayTokens[sel]); else if (hwnd==GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3)) lstrcat(outFmt,YearTokens[sel]); else if (hwnd==GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4)) lstrcat(outFmt,WeekTokens[sel]); } }
        else if (lstrcmpi(szClass,"Edit")==0) { char sep[20]; GetWindowText(hwnd,sep,sizeof(sep)); lstrcat(outFmt,sep); } }
}
static void FAR ExtractSeparators(HWND hDlg, const char* fmt) {
    const char* p=fmt; char sep[20]; int sepIdx=0, i;
    for (i=0;i<3;i++) SetDlgItemText(hDlg,IDC_DATEFMT_L_EDIT1+i,"");
    while (*p && sepIdx<3) {
        int isToken=0;
        if (FarStrnicmp(p,"MMMM",4)==0)      { isToken=1; p+=4; }
        else if (FarStrnicmp(p,"MMM",3)==0)  { isToken=1; p+=3; }
        else if (FarStrnicmp(p,"MM",2)==0)   { isToken=1; p+=2; }
        else if (*p=='M')                     { isToken=1; p++; }
        else if (FarStrnicmp(p,"dddd",4)==0) { isToken=1; p+=4; }
        else if (FarStrnicmp(p,"ddd",3)==0)  { isToken=1; p+=3; }
        else if (FarStrnicmp(p,"dd",2)==0)   { isToken=1; p+=2; }
        else if (*p=='d')                     { isToken=1; p++; }
        else if (FarStrnicmp(p,"yyyy",4)==0) { isToken=1; p+=4; }
        else if (FarStrnicmp(p,"yy",2)==0)   { isToken=1; p+=2; }
        else if (*p=='y')                     { isToken=1; p++; }
        else {
            int t=0;
            while (*p && t<19 && !isToken) {
                if (FarStrnicmp(p,"MMMM",4)==0||FarStrnicmp(p,"MMM",3)==0||FarStrnicmp(p,"MM",2)==0||*p=='M'||
                    FarStrnicmp(p,"dddd",4)==0||FarStrnicmp(p,"ddd",3)==0||FarStrnicmp(p,"dd",2)==0||*p=='d'||
                    FarStrnicmp(p,"yyyy",4)==0||FarStrnicmp(p,"yy",2)==0||*p=='y') break;
                sep[t++]=*p++;
            }
            sep[t]='\0'; SetDlgItemText(hDlg,IDC_DATEFMT_L_EDIT1+sepIdx,sep); sepIdx++;
        }
    }
}
static void FAR UpdateLongDateSample(HWND hDlg, const char* fmt) {
    char buf[80];
    GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, fmt, buf, sizeof(buf));
    SetDlgItemText(hDlg, IDC_DATEFMT_L_SAMPLE, buf);
    InvalidateRect(GetDlgItem(hDlg, IDC_DATEFMT_L_SAMPLE), NULL, TRUE);
}

/* ========== диалогова€ процедура даты ========== */
BOOL WINAPI DateFmtDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static int s_dateFmt=0, s_longFmtOrder=0; static char s_szLongFmt[80], s_szShortFmt[80]; static BOOL s_bInit=FALSE;
    switch (msg) {
    case WM_INITDIALOG: { WORD y,m,d,h,mi,s; GetLocalDateTime(&y,&m,&d,&h,&mi,&s); s_bInit=TRUE;
        s_dateFmt=g_iniDateFormat; lstrcpy(s_szLongFmt,g_szLongDateFmt); lstrcpy(s_szShortFmt,g_szShortDateFmt);
        { int val; GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_ILDATE|LOCALE_RETURN_NUMBER,(LPSTR)&val,sizeof(val)); s_longFmtOrder=val; }
        CheckRadioButton(hDlg,IDC_DATEFMT_S_MDY,IDC_DATEFMT_S_YMD,IDC_DATEFMT_S_MDY+s_dateFmt);
        SetDlgItemText(hDlg,IDC_DATEFMT_SEPARATOR,g_szDateSep);
        { int val; GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IDAYLZERO|LOCALE_RETURN_NUMBER,(LPSTR)&val,sizeof(val)); CheckDlgButton(hDlg,IDC_DATEFMT_S_DAYLZ,val);
          GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IMONLZERO|LOCALE_RETURN_NUMBER,(LPSTR)&val,sizeof(val)); CheckDlgButton(hDlg,IDC_DATEFMT_S_MONTHLZ,val);
          GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_ICENTURY|LOCALE_RETURN_NUMBER,(LPSTR)&val,sizeof(val)); CheckDlgButton(hDlg,IDC_DATEFMT_S_CENTURY,val); }
        CheckRadioButton(hDlg,IDC_DATEFMT_L_MDY,IDC_DATEFMT_L_YMD,IDC_DATEFMT_L_MDY+s_longFmtOrder);
        FillComboWithDateValues(hDlg,y,m,d);
        SendMessage(GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1),CB_SETCURSEL,(WPARAM)FindMonthIndex(s_szLongFmt),0);
        SendMessage(GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2),CB_SETCURSEL,(WPARAM)FindDayIndex(s_szLongFmt),0);
        SendMessage(GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3),CB_SETCURSEL,(WPARAM)FindYearIndex(s_szLongFmt),0);
        SendMessage(GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4),CB_SETCURSEL,(WPARAM)FindWeekIndex(s_szLongFmt),0);
        ExtractSeparators(hDlg,s_szLongFmt); SaveOriginalPositions(hDlg);
        ArrangeLongDateOrder(hDlg,s_longFmtOrder); UpdateLongDateSample(hDlg,s_szLongFmt);
        s_bInit=FALSE; return TRUE; }
    case WM_COMMAND: { WORD id=LOWORD(wParam),code=HIWORD(lParam); if (s_bInit) return FALSE;
        if ((id>=IDC_DATEFMT_L_COMBO1&&id<=IDC_DATEFMT_L_COMBO4)||(id>=IDC_DATEFMT_L_EDIT1&&id<=IDC_DATEFMT_L_EDIT3))
            if (code==CBN_SELCHANGE||code==EN_CHANGE) { GetCurrentFormatString(hDlg,s_szLongFmt); UpdateLongDateSample(hDlg,s_szLongFmt); }
        if (code==BN_CLICKED) { switch (id) {
            case IDC_DATEFMT_S_MDY: case IDC_DATEFMT_S_DMY: case IDC_DATEFMT_S_YMD: s_dateFmt=id-IDC_DATEFMT_S_MDY; CheckRadioButton(hDlg,IDC_DATEFMT_S_MDY,IDC_DATEFMT_S_YMD,id); return TRUE;
            case IDC_DATEFMT_L_MDY: case IDC_DATEFMT_L_DMY: case IDC_DATEFMT_L_YMD: s_longFmtOrder=id-IDC_DATEFMT_L_MDY; CheckRadioButton(hDlg,IDC_DATEFMT_L_MDY,IDC_DATEFMT_L_YMD,id); ArrangeLongDateOrder(hDlg,s_longFmtOrder); GetCurrentFormatString(hDlg,s_szLongFmt); UpdateLongDateSample(hDlg,s_szLongFmt); return TRUE;
        } }
        if (id==IDOK) {
            g_iniDateFormat=s_dateFmt; GetDlgItemText(hDlg,IDC_DATEFMT_SEPARATOR,g_szDateSep,sizeof(g_szDateSep));
            { int dayLZ=IsDlgButtonChecked(hDlg,IDC_DATEFMT_S_DAYLZ), monLZ=IsDlgButtonChecked(hDlg,IDC_DATEFMT_S_MONTHLZ), century=IsDlgButtonChecked(hDlg,IDC_DATEFMT_S_CENTURY);
              char *s=s_szShortFmt; if (s_dateFmt==0) { lstrcpy(s,monLZ?"MM":"M"); lstrcat(s,g_szDateSep); lstrcat(s,dayLZ?"dd":"d"); lstrcat(s,g_szDateSep); lstrcat(s,century?"yyyy":"yy"); }
              else if (s_dateFmt==1) { lstrcpy(s,dayLZ?"dd":"d"); lstrcat(s,g_szDateSep); lstrcat(s,monLZ?"MM":"M"); lstrcat(s,g_szDateSep); lstrcat(s,century?"yyyy":"yy"); }
              else { lstrcpy(s,century?"yyyy":"yy"); lstrcat(s,g_szDateSep); lstrcat(s,monLZ?"MM":"M"); lstrcat(s,g_szDateSep); lstrcat(s,dayLZ?"dd":"d"); } }
            GetCurrentFormatString(hDlg,s_szLongFmt);
            lstrcpy(g_szLongDateFmt,s_szLongFmt); lstrcpy(g_szShortDateFmt,s_szShortFmt);
            EndDialog(hDlg,IDOK); return TRUE; }
        if (id==IDCANCEL) { EndDialog(hDlg,IDCANCEL); return TRUE; } break; } }
    return FALSE; }
