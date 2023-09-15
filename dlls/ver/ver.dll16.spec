#1 DLLENTRYPOINT
2 pascal GetFileResourceSize(str str str ptr) GetFileResourceSize16
3 pascal GetFileResource(str str str long long ptr) GetFileResource16
6 pascal GetFileVersionInfoSize(str ptr) GetFileVersionInfoSize16
7 pascal GetFileVersionInfo(str long long ptr) GetFileVersionInfo16
8 pascal VerFindFile(word str str str ptr ptr ptr ptr) VerFindFile16
9 pascal VerInstallFile(word str str str str str ptr ptr) VerInstallFile16
10 pascal VerLanguageName(word ptr word) VerLanguageName16
11 pascal VerQueryValue(segptr str ptr ptr) VerQueryValue16



GETFILERESOURCESIZE.2 seg 0003 off 0010 parm 0000 EXPORTED|SHAREDATA
GETFILERESOURCE.3 seg 0003 off 017E parm 0000 EXPORTED|SHAREDATA
GETFILEVERSIONINFOSIZE.6 seg 0003 off 086E parm 0000 EXPORTED|SHAREDATA
GETFILEVERSIONINFO.7 seg 0003 off 0906 parm 0000 EXPORTED|SHAREDATA
VERFINDFILE.8 seg 0003 off 1315 parm 0000 EXPORTED|SHAREDATA
VERINSTALLFILE.9 seg 0003 off 0C9A parm 0000 EXPORTED|SHAREDATA
VERLANGUAGENAME.10 seg 0003 off 1599 parm 0000 EXPORTED|SHAREDATA
VERQUERYVALUE.11 seg 0003 off 0943 parm 0000 EXPORTED|SHAREDATA
