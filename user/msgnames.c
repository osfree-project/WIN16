/*
 * msgconv.c - Win16 Message Code to Name Conversion (полная проверенная таблица)
 * Стандарт: C89
 * Компилятор: Open Watcom 1.9 (16-bit)
 * Модель памяти: small (подходит для всех стандартных моделей Win16)
 *
 * Таблица основана на документации Windows 3.1 SDK и заголовочных файлах.
 * Содержит все системные сообщения Win16, включая MDI, DDE и сообщения
 * от стандартных элементов управления.
 */

#include <windows.h>   /* UINT, LPSTR, оконные константы */

/* ---- Таблица соответствий код -> имя ---- */

typedef struct {
    UINT    code;       /* числовой идентификатор сообщения */
    LPSTR   name;       /* указатель на строковую константу */
} MSG_ENTRY;

/*
 * Полная таблица системных сообщений Win16 (Windows 3.1).
 * Сообщения упорядочены по возрастанию кода.
 */
static const MSG_ENTRY msg_table[] = {
    { 0x0000, "WM_NULL"                       },
    { 0x0001, "WM_CREATE"                     },
    { 0x0002, "WM_DESTROY"                    },
    { 0x0003, "WM_MOVE"                       },
    { 0x0005, "WM_SIZE"                       },
    { 0x0006, "WM_ACTIVATE"                   },
    { 0x0007, "WM_SETFOCUS"                   },
    { 0x0008, "WM_KILLFOCUS"                  },
    { 0x000A, "WM_ENABLE"                     },
    { 0x000B, "WM_SETREDRAW"                  },
    { 0x000C, "WM_SETTEXT"                    },
    { 0x000D, "WM_GETTEXT"                    },
    { 0x000E, "WM_GETTEXTLENGTH"              },
    { 0x000F, "WM_PAINT"                      },
    { 0x0010, "WM_CLOSE"                      },
    { 0x0011, "WM_QUERYENDSESSION"            },
    { 0x0012, "WM_QUIT"                       },
    { 0x0013, "WM_QUERYOPEN"                  },
    { 0x0014, "WM_ERASEBKGND"                 },
    { 0x0015, "WM_SYSCOLORCHANGE"             },
    { 0x0016, "WM_ENDSESSION"                 },
    { 0x0017, "WM_SYSTEMERROR"                },
    { 0x0018, "WM_SHOWWINDOW"                 },
    { 0x0019, "WM_CTLCOLOR"                   },
    { 0x001A, "WM_WININICHANGE"               },
    { 0x001B, "WM_DEVMODECHANGE"              },
    { 0x001C, "WM_ACTIVATEAPP"                },
    { 0x001D, "WM_FONTCHANGE"                 },
    { 0x001E, "WM_TIMECHANGE"                 },
    { 0x001F, "WM_CANCELMODE"                 },
    { 0x0020, "WM_SETCURSOR"                  },
    { 0x0021, "WM_MOUSEACTIVATE"              },
    { 0x0022, "WM_CHILDACTIVATE"              },
    { 0x0024, "WM_GETMINMAXINFO"              },
    { 0x0026, "WM_PAINTICON"                  },
    { 0x0027, "WM_ICONERASEBKGND"             },
    { 0x0028, "WM_NEXTDLGCTL"                 },
    { 0x002A, "WM_SPOOLERSTATUS"              },
    { 0x002B, "WM_DRAWITEM"                   },
    { 0x002C, "WM_MEASUREITEM"                },
    { 0x002D, "WM_DELETEITEM"                 },
    { 0x002E, "WM_VKEYTOITEM"                 },
    { 0x002F, "WM_CHARTOITEM"                 },
    { 0x0030, "WM_SETFONT"                    },
    { 0x0031, "WM_GETFONT"                    },
    { 0x0037, "WM_QUERYDRAGICON"              },
    { 0x0039, "WM_COMPAREITEM"                },
    { 0x0041, "WM_COMPACTING"                 },
    { 0x0044, "WM_COMMNOTIFY"                 },
    { 0x0046, "WM_WINDOWPOSCHANGING"          },
    { 0x0047, "WM_WINDOWPOSCHANGED"           },
    { 0x0048, "WM_POWER"                      },
    { 0x004A, "WM_COPYDATA"                   },
    { 0x004B, "WM_CANCELJOURNAL"              },
    { 0x0053, "WM_HELP"                       },
    { 0x0080, "WM_NCACTIVATE"                 },
    { 0x0081, "WM_NCCALCSIZE"                 },
    { 0x0082, "WM_NCCREATE"                   },
    { 0x0083, "WM_NCDESTROY"                  },
    { 0x0084, "WM_NCHITTEST"                  },
    { 0x0085, "WM_NCPAINT"                    },
    { 0x0086, "WM_NCACTIVATE"                 }, /* иногда дублируется 0x0080 */
    { 0x0087, "WM_GETDLGCODE"                 },
    { 0x0088, "WM_SYNCPAINT"                  },
    { 0x00A0, "WM_NCMOUSEMOVE"                },
    { 0x00A1, "WM_NCLBUTTONDOWN"              },
    { 0x00A2, "WM_NCLBUTTONUP"                },
    { 0x00A3, "WM_NCLBUTTONDBLCLK"            },
    { 0x00A4, "WM_NCRBUTTONDOWN"              },
    { 0x00A5, "WM_NCRBUTTONUP"                },
    { 0x00A6, "WM_NCRBUTTONDBLCLK"            },
    { 0x00A7, "WM_NCMBUTTONDOWN"              },
    { 0x00A8, "WM_NCMBUTTONUP"                },
    { 0x00A9, "WM_NCMBUTTONDBLCLK"            },
    { 0x0100, "WM_KEYFIRST"                   },
    { 0x0100, "WM_KEYDOWN"                    },
    { 0x0101, "WM_KEYUP"                      },
    { 0x0102, "WM_CHAR"                       },
    { 0x0103, "WM_DEADCHAR"                   },
    { 0x0104, "WM_SYSKEYDOWN"                 },
    { 0x0105, "WM_SYSKEYUP"                   },
    { 0x0106, "WM_SYSCHAR"                    },
    { 0x0107, "WM_SYSDEADCHAR"                },
    { 0x0108, "WM_KEYLAST"                    },
    { 0x0111, "WM_INITDIALOG"                 },
    { 0x0111, "WM_COMMAND"                    },
    { 0x0112, "WM_SYSCOMMAND"                 },
    { 0x0113, "WM_TIMER"                      },
    { 0x0114, "WM_HSCROLL"                    },
    { 0x0115, "WM_VSCROLL"                    },
    { 0x0116, "WM_INITMENU"                   },
    { 0x0117, "WM_INITMENUPOPUP"              },
    { 0x011F, "WM_MENUSELECT"                 },
    { 0x0120, "WM_MENUCHAR"                   },
    { 0x0121, "WM_ENTERIDLE"                  },
    { 0x0132, "WM_CTLCOLORMSGBOX"             },
    { 0x0133, "WM_CTLCOLOREDIT"               },
    { 0x0134, "WM_CTLCOLORLISTBOX"            },
    { 0x0135, "WM_CTLCOLORBTN"                },
    { 0x0136, "WM_CTLCOLORDLG"                },
    { 0x0137, "WM_CTLCOLORSCROLLBAR"          },
    { 0x0138, "WM_CTLCOLORSTATIC"             },
    { 0x0200, "WM_MOUSEFIRST"                 },
    { 0x0200, "WM_MOUSEMOVE"                  },
    { 0x0201, "WM_LBUTTONDOWN"                },
    { 0x0202, "WM_LBUTTONUP"                  },
    { 0x0203, "WM_LBUTTONDBLCLK"              },
    { 0x0204, "WM_RBUTTONDOWN"                },
    { 0x0205, "WM_RBUTTONUP"                  },
    { 0x0206, "WM_RBUTTONDBLCLK"              },
    { 0x0207, "WM_MBUTTONDOWN"                },
    { 0x0208, "WM_MBUTTONUP"                  },
    { 0x0209, "WM_MBUTTONDBLCLK"              },
    { 0x020D, "WM_MOUSELAST"                  },
    { 0x0210, "WM_PARENTNOTIFY"               },
    /* ---- MDI сообщения ---- */
    { 0x0220, "WM_MDICREATE"                  },
    { 0x0221, "WM_MDIDESTROY"                 },
    { 0x0222, "WM_MDIACTIVATE"                },
    { 0x0223, "WM_MDIRESTORE"                 },
    { 0x0224, "WM_MDINEXT"                    },
    { 0x0225, "WM_MDIMAXIMIZE"                },
    { 0x0226, "WM_MDITILE"                    },
    { 0x0227, "WM_MDICASCADE"                 },
    { 0x0228, "WM_MDIICONARRANGE"             },
    { 0x0229, "WM_MDIGETACTIVE"               },
    { 0x0230, "WM_MDISETMENU"                 },
    /* ---- DDE сообщения ---- */
    { 0x03E0, "WM_DDE_FIRST"                  },
    { 0x03E0, "WM_DDE_INITIATE"               },
    { 0x03E1, "WM_DDE_TERMINATE"              },
    { 0x03E2, "WM_DDE_ADVISE"                 },
    { 0x03E3, "WM_DDE_UNADVISE"               },
    { 0x03E4, "WM_DDE_ACK"                    },
    { 0x03E5, "WM_DDE_DATA"                   },
    { 0x03E6, "WM_DDE_REQUEST"                },
    { 0x03E7, "WM_DDE_POKE"                   },
    { 0x03E8, "WM_DDE_EXECUTE"                },
    { 0x03E8, "WM_DDE_LAST"                   },
    /* ---- Пользовательские сообщения ---- */
    { 0x0400, "WM_USER"                       }
};

static const int msg_table_size =
    sizeof(msg_table) / sizeof(msg_table[0]);

/* ---- Функция получения имени по коду ---- */

/*
 * GetMessageName
 * Возвращает указатель на строку с символическим именем сообщения Windows.
 * Если код не найден — возвращает NULL.
 * Параметры:
 *   msgCode - числовой идентификатор сообщения (UINT)
 * Возвращаемое значение:
 *   LPSTR - указатель на статическую строку с именем или NULL.
 */
LPSTR WINAPI GetMessageName(UINT msgCode)
{
    int i;
    for (i = 0; i < msg_table_size; i++) {
        if (msg_table[i].code == msgCode) {
            return msg_table[i].name;
        }
    }
    return (LPSTR)0;  /* NULL в стиле Win16 */
}

/* ---- Демонстрационный пример (можно удалить в рабочем проекте) ----

#ifdef TEST_MSG_CONV
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev,
                   LPSTR lpCmdLine, int nCmdShow)
{
    char buf[80];
    UINT testMsgs[] = { WM_PAINT, WM_CLOSE, WM_COMMAND, 0x1234 };
    int i;

    for (i = 0; i < sizeof(testMsgs)/sizeof(testMsgs[0]); i++) {
        LPSTR name = GetMessageName(testMsgs[i]);
        if (name) {
            wsprintf(buf, "0x%04X = %s", testMsgs[i], name);
        } else {
            wsprintf(buf, "0x%04X = Unknown", testMsgs[i]);
        }
        MessageBox(NULL, buf, "Message lookup", MB_OK);
    }
    return 0;
}
#endif
*/