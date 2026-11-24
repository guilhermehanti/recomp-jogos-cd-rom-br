#include <windows.h>

static HFONT WINAPI fontA(int h,int w,int e,int o,int weight,DWORD i,DWORD u,DWORD s,
                         DWORD charset,DWORD precision,DWORD clip,DWORD quality,DWORD pitch,LPCSTR face)
{
    return CreateFontA(h,w,e,o,weight,i,u,s,charset,precision,clip,NONANTIALIASED_QUALITY,pitch,face);
}
static HFONT WINAPI fontIndirect(const LOGFONTA *font)
{
    if (!font) return CreateFontIndirectA(font);
    LOGFONTA copy = *font;
    copy.lfQuality = NONANTIALIASED_QUALITY;
    return CreateFontIndirectA(&copy);
}
static BOOL setSlot(void **slot, void *target)
{
    DWORD old;
    if (!VirtualProtect(slot, sizeof *slot, PAGE_READWRITE, &old)) return FALSE;
    *slot = target;
    VirtualProtect(slot, sizeof *slot, old, &old);
    return TRUE;
}
BOOL WINAPI DllMain(HINSTANCE dll, DWORD reason, LPVOID reserved)
{
    if (reason != DLL_PROCESS_ATTACH) return TRUE;
    BYTE *base = (BYTE *)GetModuleHandleA(NULL);
    IMAGE_NT_HEADERS *pe = (IMAGE_NT_HEADERS *)(base + ((IMAGE_DOS_HEADER *)base)->e_lfanew);
    if (pe->FileHeader.TimeDateStamp != 0x350d5801 || pe->OptionalHeader.SizeOfImage != 401408) return FALSE;
    if (!setSlot((void **)(base + 0x584a0), fontA) ||
        !setSlot((void **)(base + 0x584d0), fontIndirect)) return FALSE;
    return TRUE;
}

#include <winspool.h>
BOOL WINAPI diagOpen(LPSTR n, LPHANDLE h, LPPRINTER_DEFAULTSA p) {return OpenPrinterA(n,h,p);}
BOOL WINAPI diagClose(HANDLE h) {return ClosePrinter(h);}
LONG WINAPI diagProps(HWND w,HANDLE h,LPSTR n,PDEVMODEA o,PDEVMODEA i,DWORD f){return DocumentPropertiesA(w,h,n,o,i,f);}


