#include "../EFI/efi.h"
#include "../EFI/protocol/efi-gop.h"

#define DEFAULT_COLOR 0xFFFFFFFF
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

static EFI_GRAPHICS_OUTPUT_PROTOCOL *gGop;
static UINTN cursorX = 0;
static UINTN cursorY = 0;

static void GOP_DisableCursor() {
    // GOP does not manage hardware cursors; this is a placeholder.
}

EFI_STATUS GOP_Init(EFI_HANDLE ImageHandle) {
    EFI_STATUS Status;
    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&gGop);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    GOP_DisableCursor();
    cursorX = 0;
    cursorY = 0;

    return EFI_SUCCESS;
}

void GOP_Clear() {
    GOP_ClearColor(DEFAULT_COLOR);
}

void GOP_ClearColor(UINT32 Color) {
    UINTN Size = gGop->Mode->Info->HorizontalResolution * gGop->Mode->Info->VerticalResolution * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer = AllocatePool(Size);
    
    for (UINTN i = 0; i < Size / sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL); i++) {
        BltBuffer[i].Blue = (Color & 0xFF0000) >> 16;
        BltBuffer[i].Green = (Color & 0x00FF00) >> 8;
        BltBuffer[i].Red = (Color & 0x0000FF);
        BltBuffer[i].Reserved = 0;
    }

    gGop->Blt(gGop, BltBuffer, EfiBltVideoFill, 0, 0, 0, 0,
              gGop->Mode->Info->HorizontalResolution, gGop->Mode->Info->VerticalResolution, 0);
    
    FreePool(BltBuffer);
    cursorX = 0;
    cursorY = 0;
}

void GOP_NewLine() {
    cursorY++;
    if (cursorY >= SCREEN_HEIGHT) {
        cursorY = SCREEN_HEIGHT - 1;
    }
    cursorX = 0;
}

void GOP_PrintColor(const CHAR8* str, UINT32 Color) {
    while (*str) {
        if (*str == '\n') {
            GOP_NewLine();
            str++;
            continue;
        }

        if (cursorX < SCREEN_WIDTH && cursorY < SCREEN_HEIGHT) {
            EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixel;
            pixel.Blue = (Color & 0xFF0000) >> 16;
            pixel.Green = (Color & 0x00FF00) >> 8;
            pixel.Red = (Color & 0x0000FF);
            pixel.Reserved = 0;

            gGop->Blt(gGop, &pixel, EfiBltVideoFill, 0, 0, cursorX, cursorY, 1, 1, 0);
        }

        cursorX++;
        if (cursorX >= SCREEN_WIDTH) {
            GOP_NewLine();
        }
        str++;
    }
}

void GOP_Print(const CHAR8* str) {
    GOP_PrintColor(str, DEFAULT_COLOR);
}
