#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * files
 */
int             System_CreateFile(const char *name);
int             System_OpenFile(const char *name);
int             System_CloseFile(int file_handle);
int             System_SeekFileTo(int file_handle, int position);
int             System_TellFile(int file_handle);
int             System_ReadFile(int file_handle, char *buffer, int bytes);
int             System_WriteFile(int file_handle, const char *buffer,
                                 int bytes);
bool            System_TruncateFileTo(int file_handle, int length);
int             System_FileSize(int file_handle);
bool            System_FlushFile(int file_handle);
char          **System_ListDir(int *num);
void            System_FreeListDir(char **lst, int num);
bool            System_RenameFile(const char *old_name, const char *new_name);
bool            System_DeleteFile(const char *file_name);
const int       System_LastError();
const char     *System_ErrorText(int code);

/*
 * stdin
 */
int             System_ReadStdin();
const char     *System_GetStdinLine();

bool            System_Init();
void            System_Quit();

void            System_ProcessEvents();
void            System_CheckScheduledSemaphore();
bool            System_QuitSignalled();
void            System_UpdateScreen();
int             System_GetStdinInputSemaphore();
int             System_GetInputSemaphore();

/*
 * interpreter calls
 */
void            System_SetInputSemaphore(int semaphore);
void            System_SetStdinInputSemaphore(int semaphore);
uint32_t        System_GetSmalltalkEpochTime();
uint32_t        System_GetMsClock();
void            System_SignalSemaphoreAt(int semaphore, uint32_t msClockTime);
void            System_SetCursorImage(uint16_t * image);
void            System_SetCursorLocation(int x, int y);
void            System_GetCursorLocation(int *x, int *y);
void            System_SetLinkCursor(bool link);
bool            System_SetDisplaySize(int width, int height);
void            System_DisplayChanged(int x, int y, int width, int height);
bool            System_NextInputQueueWord(uint16_t * word);
void            System_Error(const char *message);
void            System_SignalQuit();
void            System_ExitToDebugger();
const char     *System_GetImageName();
void            System_SetImageName(const char *new_name);


#endif                          /* _SYSTEM_H_ */
