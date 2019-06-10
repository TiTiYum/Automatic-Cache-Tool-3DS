#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <3ds.h>

#define ENTRY_SHARED_COUNT 0x200
#define ENTRY_HOMEMENU_COUNT 0x168
#define ENTRY_LIBRARY_START 0xC3510
#define ENTRY_LIBRARY_COUNT 0x100
#define ENTRY_HISTORY_COUNT 0x11D28

Handle ptmSysmHandle;

typedef struct {
    u16 shortDescription[0x40];
    u16 longDescription[0x80];
    u16 publisher[0x40];
} SMDH_META;

typedef struct {
    u8 unknown[0x20];
    SMDH_META titles[0x10];
    u8 smallIcon[0x480];
    u8 largeIcon[0x1200];
} SMDH_SHARED;

typedef struct {
    char magic[0x04];
    u16 version;
    u16 reserved1;
    SMDH_META titles[0x10];
    u8 ratings[0x10];
    u32 region;
    u32 matchMakerId;
    u64 matchMakerBitId;
    u32 flags;
    u16 eulaVersion;
    u16 reserved;
    u32 optimalBannerFrame;
    u32 streetpassId;
    u64 reserved2;
    u8 smallIcon[0x480];
    u8 largeIcon[0x1200];
} SMDH_HOMEMENU;

typedef struct {
    u8 version;
    bool animated;
    u16 crc16[4];
    u8 reserved[0x16];
    u8 mainIconBitmap[0x200];
    u16 mainIconPalette[0x10];
    u16 titles[16][0x80];
    u8 animatedFrameBitmaps[8][0x200];
    u16 animatedFramePalettes[8][0x10];
    u16 animationSequence[0x40];
} SMDH_TWL;

typedef struct {
    u64 unknown;
    u64 titleid;
} ENTRY_DATA;

typedef struct {
    u64 titleid;
    u32 totalPlayed;
    u16 timesPlayed;
    u16 flags;
    u16 firstPlayed;
    u16 lastPlayed;
    u32 padding;
} ENTRY_LIBRARY;

typedef struct {
    u64 titleid;
    u32 timestamp;
} ENTRY_HISTORY;

Result PTMSYSM_ClearStepHistory(void)
{
    Result ret;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x805,0,0); // 0x8050000

    if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

    return (Result)cmdbuf[1];
}

Result PTMSYSM_ClearPlayHistory(void)
{
    Result ret;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x80A,0,0); // 0x80A0000

    if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

    return (Result)cmdbuf[1];
}
void gfxEndFrame() {gfxFlushBuffers() ;gfxSwapBuffers() ;gspWaitForVBlank();}

void promptError(const char* title, const char* message) {
    consoleClear();
    printf("\x1b[1;0H\x1b[30;47m%-50s", " ");
    printf("\x1b[1;%uH%s\x1b[0;0m", (25 - (strlen(title) >> 1)), title);
    printf("\x1b[14;%uH%s", (25 - (strlen(message) >> 1)), message);
}

FS_Archive openSystemSavedata(u32* UniqueID) {
    Result res;
    u8 region = 0;

    res = CFGU_SecureInfoGetRegion(&region);
    printf("Retrieving console's region... %s %#lx.\n", R_FAILED(res) ? "ERROR" : "OK", res);

    FS_Archive archive;
    u32 low = UniqueID[region];
    u32 archpath[2] = {MEDIATYPE_NAND, low};

    FS_Path fspath = {PATH_BINARY, 8, archpath};

    res = FSUSER_OpenArchive(&archive, ARCHIVE_SYSTEM_SAVEDATA, fspath);
    printf("Opening archive %#lx... %s %#lx.\n", low, R_FAILED(res) ? "ERROR" : "OK", res);

    return archive;
}
void clearPlayHistory(bool wait = true) {
    printf("Auto Delete Cache By: Yum");
    Result res = PTMSYSM_ClearPlayHistory();
    if (R_FAILED(res)) promptError("Clear Step History", "Failed to clear play history.");
    printf("Clearing play history... %s %#lx.\n", R_FAILED(res) ? "ERROR" : "OK", res);
    
}

void clearStepHistory(bool wait = true) {
    printf("Auto Delete Cache By: Yum");
    Result res = PTMSYSM_ClearStepHistory();
    if (R_FAILED(res)) promptError("Clear Step History", "Failed to clear step history.");
    printf("Clearing step history... %s %#lx.\n", R_FAILED(res) ? "ERROR" : "OK", res);
}

void clearSoftwareLibrary(bool wait = true) {
    printf("Auto Delete Cache By: Yum");
    Result res;

    u32 activitylogID[] = {0x00020202, 0x00020212, 0x00020222, 0x00020222, 0x00020262, 0x00020272, 0x00020282};
    FS_Archive syssave = openSystemSavedata(activitylogID);

    res = FSUSER_DeleteFile(syssave, (FS_Path)fsMakePath(PATH_ASCII, "/pld.dat"));

    if (R_FAILED(res)) promptError("Clear Software Library", "Failed to delete software library data.");
    printf("Deleting file \"pld.dat\"... %s %#lx.\n", R_FAILED(res) ? "ERROR" : "OK", res);

    FSUSER_ControlArchive(syssave, ARCHIVE_ACTION_COMMIT_SAVE_DATA, NULL, 0, NULL, 0);
    FSUSER_CloseArchive(syssave);
}

void resetDemoPlayCount(bool wait = true) {
    Result res = AM_DeleteAllDemoLaunchInfos();
    printf("Auto Delete Cache By: Yum");
}

int main() {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    if (R_FAILED(srvGetServiceHandle(&ptmSysmHandle, "ptm:sysm"))) {
    }
    amInit();
    cfguInit();
    fsInit();
printf("Auto Delete Cache By: Yum");

clearPlayHistory();
clearSoftwareLibrary();
resetDemoPlayCount();
consoleClear();
APT_HardwareResetAsync(); //Desactivar si no va
gfxEndFrame();
    fsExit();
    cfguExit();
    amExit();
    svcCloseHandle(ptmSysmHandle);
    gfxExit();

    return 0;
}