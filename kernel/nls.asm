	include ascii.inc

public errstr2
public errstr3
public szDOSstr
public szTerm	
public szErr31    
public szErr311   
public szErr32    
public szErr33    
public szErr34    
public szErr35    
public szErr36    
public szErr37    
public szErr23    
public es23hdl    
public szErr22    
public szErr21    
public errstr19   
public errstr20   
public errstr17   
public errstr16   
public errstr15   
public errstr14   
public errstr13   
public szNotaNE   
public errstr11   
public errstr10   
public errstr9    
public errstr7    
public errstr6    
public errstr5    
public int0Berr1  
public exc0derr   
public errstr26  
public szNotFnd   
public nohandles  
public szLoadErr  
public szModule   
public szSegment  
public SegNo   	
public errstr24	
public LENTERR		
public errstr25	
public modtext		
public szExtErr	
public szExtErrCod	
public szExtErrCls	
public szExtErrAct	
public szExtErrLoc	
public szDpmiErr	
public dpmifunc	
public dpmicaller	
public szLF		
public szEntryErr	
public szEntryCode	
public szLibName
public szPathConst 
public szWEP	
public nullstr	
public errstr41   
public errstr42   
public errstr43   
public szNoDPMI
CCONST segment


szTerm	   db 'Application will be terminated',lf,00
szErr31    db 'memory allocation (DPMI) for segment load failed',lf,00
szErr311   db 'memory realloc failed',lf,00
szErr32    db 'Freeing memory failed. ',0
szErr33    db 'DPMI 0000: Allocate descriptor',lf,00
szErr34    db 'DPMI 0007: Set segment base address',lf,00
szErr35    db 'DPMI 0008: Set segment limit',lf,00
szErr36    db 'DPMI 0009: Set descriptor access rights',lf,00
szErr37    db 'DPMI 000B: Get descriptor',lf,00
szErr23    db "Invalid module handle "
es23hdl    db "    ",lf,00
szErr22    db "file is corrupt",lf,00
szErr21    db "stack segment of parent destroyed",lf,00
errstr19   db "Can't create PSP (insufficient DOS memory)",lf,00
errstr20   db "Can't load a 2. instance of an app",lf,00
errstr17   db 'Stack segment is readonly',lf,00
errstr16   db 'Module has no stack',lf,00
errstr15   db 'DLL initialization error',lf,00
errstr14   db 'Relocatable code has zero relocations',lf,00
errstr13   db 'Invalid FixUp Type in relocation table',lf,00
if ?32BIT
szNotaNE	label byte
           db 'File is no 32-bit NE application',lf
           db 00
else
szNotaNE   db 'File is no 16-bit NE application',lf,00
endif
errstr11   db 'Too many segments in EXE',lf,00
errstr10   db 'Program has no valid start address',lf,00
errstr9    db 'Cannot allocate required memory',lf,00
errstr7    db 'Error while loading segment',lf,00
errstr6    db 'File read error',lf,00
errstr5    db 'Inconsistent module table size',lf,00
int0Berr1  db 'Invalid not present exception',lf,00
exc0derr   db lf,'protection exception occured',lf,0
errstr26   db "can't load ",00
szNotFnd   db 'File not found error',lf,00
nohandles  db 'Out of file handles error',lf,00
szLoadErr  db "Load error. ",0

szModule    db "Module ",0
szSegment   db ", segment "
SegNo   	db "00.",0

errstr24	db 'Entry 0x'
LENTERR		db 0,0,0,0,' not found in module ',00
errstr25	db ' not found in name tables of module ',00

modtext		db 'Module: ',00


szExtErr	db 'Last DOS Error:'
			db lf,9,'Extended Error Code '
szExtErrCod	db 4 dup (0)
			db ', Error Class '
szExtErrCls	db 2 dup (0)
			db lf,9,'Suggested Action '
szExtErrAct	db 2 dup (0)
			db ', Locus '
szExtErrLoc	db 2 dup (0)
			db lf,0

szDpmiErr	db 'DPMI function 0x'
dpmifunc	db 0,0,0,0
			db ' failed at 0x'
dpmicaller	db 0,0,0,0
szLF		db lf,0

szEntryErr	db 'Error code 0x'
szEntryCode	db 4 dup (0)
			db ' from LibEntry '
szLibName	db 40h dup (0)

szPathConst db 'PATH=',0
if 0;e ?32BIT
szCmdline	db 'CMDLINE=',0
endif
szWEP		db 'WEP'
nullstr		db 00

errstr41   db 'PMtoRMCallTHUNK: Cannot convert Selector',lf,00
errstr42   db 'PMtoRMCallTHUNK: Cannot call real mode procedure',lf,00

szNoDPMI   db 'No DPMI server available',lf,00
;szAPIerr	db "DOS API translation not supported",lf,00
szDOSstr	db "MS-DOS",00
errstr2    db 'Error allocating memory for DPMI server',lf,00
errstr3    db 'Error switching to protected mode',lf,00
errstr43   db 'PMtoRMCallTHUNK: Invalid THUNK instruction',lf,00

CCONST ends


	end