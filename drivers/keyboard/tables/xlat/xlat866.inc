; 866 <> 1251

CODEPAGE	DW	866	; DOS Cyrillic Russian

;https://en.wikipedia.org/wiki/Windows-1251
;https://en.wikipedia.org/wiki/Code_page_866

; ANSI to OEM translation table
; =============================
;                      OEM	; ANSI

; 0x00-0x1f Control codes translation ANSI to OEM

		DB	00	; 00
		DB	01	; 01
		DB	02	; 02
		DB	03	; 03
		DB	04	; 04
		DB	05	; 05
		DB	06	; 06
		DB	07	; 07
		DB	08	; 08
		DB	09	; 09
		DB	10	; 0a
		DB	11	; 0b
		DB	12	; 0c
		DB	13	; 0d
		DB	14	; 0e
		DB	15	; 0f
		DB	16	; 10
		DB	17	; 11
		DB	18	; 12
		DB	19	; 13
		DB	20	; 14
		DB	21	; 15
		DB	22	; 16
		DB	23	; 17
		DB	24	; 18
		DB	25	; 19
		DB	26	; 1a
		DB	27	; 1b
		DB	28	; 1c
		DB	29	; 1d
		DB	30	; 1e
		DB	31	; 1f

; 0x80-0x9f Codes translation ANSI to OEM

; OEM to ANSI translation table
; =============================
;                      ANSI	; OEM
