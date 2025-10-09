// #include "gop.h"
// #include "../kernel.h"

// #define DEFAULT_COLOR 0x0F

// static UINTN cursorX = 0;
// static UINTN cursorY = 0;
// static EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

// // Функция для инициализации GOP
// void GOP_Init() {
//     cursorX = 0;
//     cursorY = 0;

//     EFI_STATUS status;
//     EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

//     // Локализуем протокол GOP
//     status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void **)&gop);
//     if (EFI_ERROR(status)) {
//         //I should smth here, like RSOF call
//     }
// }

// // Функция для очистки экрана
// void GOP_Clear() {
//     GOP_ClearColor(DEFAULT_COLOR);
// }

// // Функция для очистки с указанным цветом
// void GOP_ClearColor(unsigned char color) {
//     EFI_GRAPHICS_OUTPUT_BLT_PIXEL fillColor = {color, color, color, 0};

//     // Заполняем экран текущим цветом
//     uefi_call_wrapper(gop->Blt, 7, gop, &fillColor, EfiBltVideoFill, 0, 0,
//                       0, 0, gop->Mode->Info->HorizontalResolution,
//                       gop->Mode->Info->VerticalResolution);
    
//     cursorX = 0;
//     cursorY = 0;
// }

// // Функция для перехода на новую строку
// void GOP_NewLine() {
//     cursorY++;
//     if (cursorY >= gop->Mode->Info->VerticalResolution / 16) {  // 16 - высота символа
//         cursorY = gop->Mode->Info->VerticalResolution / 16 - 1;
//     }
//     cursorX = 0;
// }

// // Функция для печати строки с цветом
// void GOP_PrintColor(const char* str, unsigned char color) {
//     EFI_GRAPHICS_OUTPUT_BLT_PIXEL textColor = {color, color, color, 0};

//     while (*str) {
//         if (*str == '\n') { 
//             GOP_NewLine(); 
//             str++; 
//             continue; 
//         }

//         // Печать символа (должно быть реализовано создание битмапа символа)
//         // Код для рисования символа, основываясь на текущих координатах cursorX, cursorY.
//         // Например, используйте uefi_call_wrapper для вызова графического вывода.

//         cursorX++;
//         if (cursorX >= gop->Mode->Info->HorizontalResolution / 8) {  // 8 - ширина символа
//             GOP_NewLine();  // если переполнено, переносим строку
//         }
//         str++;
//     }
// }

// // Функция для печати строки с умолчаниями
// void GOP_Print(const char* str) {
//     GOP_PrintColor(str, DEFAULT_COLOR);
// }
