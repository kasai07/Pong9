#include "fs.h"
#include "draw.h"

#include "i2c.h"
#include "fatfs/ff.h"
#include "fatfs/sdmmc.h"

static bool fs_ok = false;
static FATFS fs;
static FIL file;
static DIR dir;

/* Volume - Partition resolution table (default table) */
PARTITION VolToPart[] = {
    {0, 1},    /* Logical drive 0 ==> Physical drive 0, 1st partition */
    {0, 2},    /* Logical drive 1 ==> Physical drive 0, 2nd partition */
    {1, 0}     /* Logical drive 2 ==> Physical drive 1, auto detection */
};


bool InitFS()
{
    bool ret = fs_ok = (f_mount(&fs, "0:", 1) == FR_OK);
    if (ret)
        f_chdir(GetWorkDir());

    return ret;
}

void DeinitFS()
{
    fs_ok = false;
    f_mount(NULL, "0:", 1);
}

bool CheckSD()
{
    return fs_ok;
}

const char* GetWorkDir()
{
    const char* root = "/";
    const char* work_dirs[] = { WORK_DIRS };
    u32 n_dirs = sizeof(work_dirs) / sizeof(char*);
    
    if (!fs_ok)
        return NULL;
    
    u32 i;
    for (i = 0; i < n_dirs; i++) {
        FILINFO fno = { .lfname = NULL };
        if ((f_stat(work_dirs[i], &fno) == FR_OK) && (fno.fattrib & AM_DIR))
            break;
    }
    
    return ((i >= n_dirs) ? root : work_dirs[i]);
}

bool DebugCheckFreeSpace(size_t required)
{
	if (!fs_ok)
        return false;
    if (required > RemainingStorageSpace()) {
        Debug("Not enough space left on SD card");
        return false;
    }
    
    return true;
}

bool FileOpen(const char* path)
{
    if (!fs_ok)
        return false;
    unsigned flags = FA_READ | FA_WRITE | FA_OPEN_EXISTING;
    unsigned flags_ro = FA_READ | FA_OPEN_EXISTING;
    bool ret = (f_open(&file, path, flags) == FR_OK) ||
        (f_open(&file, path, flags_ro) == FR_OK);
    f_lseek(&file, 0);
    f_sync(&file);
    return ret;
}

bool DebugFileOpen(const char* path)
{
    Debug("Opening %s ...", path);
    if (!FileOpen(path)) {
        Debug("Could not open %s!", path);
        return false;
    }
    
    return true;
}

bool FileCreate(const char* path, bool truncate)
{
    if (!fs_ok)
        return false;
    unsigned flags = FA_READ | FA_WRITE;
    flags |= truncate ? FA_CREATE_ALWAYS : FA_OPEN_ALWAYS;
    bool ret = (f_open(&file, path, flags) == FR_OK);
    f_lseek(&file, 0);
    f_sync(&file);
    return ret;
}

bool DebugFileCreate(const char* path, bool truncate) {
    Debug("Creating %s ...", path);
    if (!FileCreate(path, truncate)) {
        Debug("Could not create %s!", path);
        return false;
    }

    return true;
}

size_t FileRead(void* buf, size_t size, size_t foffset)
{
    UINT bytes_read = 0;
    f_lseek(&file, foffset);
    f_read(&file, buf, size, &bytes_read);
    return bytes_read;
}

bool DebugFileRead(void* buf, size_t size, size_t foffset) {
    size_t bytesRead = FileRead(buf, size, foffset);
    if(bytesRead != size) {
        Debug("ERROR, file is too small!");
        return false;
    }
    
    return true;
}

size_t FileWrite(void* buf, size_t size, size_t foffset)
{
    UINT bytes_written = 0;
    f_lseek(&file, foffset);
    f_write(&file, buf, size, &bytes_written);
    f_sync(&file);
    return bytes_written;
}

bool DebugFileWrite(void* buf, size_t size, size_t foffset)
{
    size_t bytesWritten = FileWrite(buf, size, foffset);
    if(bytesWritten != size) {
        Debug("SD failure or SD full");
        return false;
    }
    
    return true;
}

size_t FileGetSize()
{
    return f_size(&file);
}

void FileClose()
{
    f_close(&file);
}

bool DirMake(const char* path)
{
    if (!fs_ok)
        return false;
    FRESULT res = f_mkdir(path);
    bool ret = (res == FR_OK) || (res == FR_EXIST);
    return ret;
}

bool DebugDirMake(const char* path)
{
    Debug("Creating dir %s ...", path);
    if (!DirMake(path)) {
        Debug("Could not create %s!", path);
        return false;
    }
    
    return true;
}

bool DirOpen(const char* path)
{
    if (!fs_ok)
        return false;
    return (f_opendir(&dir, path) == FR_OK);
}

bool DebugDirOpen(const char* path)
{
    Debug("Opening %s ...", path);
    if (!DirOpen(path)) {
        Debug("Could not open %s!", path);
        return false;
    }
    
    return true;
}

bool DirRead(char* fname, int fsize)
{
    FILINFO fno;
    fno.lfname = fname;
    fno.lfsize = fsize;
    bool ret = false;
    
	while (f_readdir(&dir, &fno) == FR_OK) {
        
		if (fno.fname[0] == 0) break;
        
		if ((fno.fname[0] != '.') && !(fno.fattrib & AM_DIR)) 
		{
           
			if (fname[0] == 0)
			{
				
				strcpy(fname, fno.fname);
				
			}
            
			ret = true;
            ;
			break;
        }
    }
    return ret;
}

void DirClose()
{
    f_closedir(&dir);
}

bool GetFileListWorker(char** list, int* lsize, char* fpath, int fsize, bool recursive)
{
    DIR pdir;
    FILINFO fno;
    char* fname = fpath + strnlen(fpath, fsize - 1);
    bool ret = false;
    
    if (f_opendir(&pdir, fpath) != FR_OK) return false;
    (fname++)[0] = '/';
    fno.lfname = fname;
    fno.lfsize = fsize - (fname - fpath);
    
    while (f_readdir(&pdir, &fno) == FR_OK) {
        if (fno.fname[0] == '.') continue;
        if (fname[0] == 0)
            strcpy(fname, fno.fname);
        if (fno.fname[0] == 0) {
            ret = true;
            break;
        } else if (fno.fattrib & AM_DIR) {
            if (recursive && !GetFileListWorker(list, lsize, fpath, fsize, recursive))
                break;
        } else {
            snprintf(*list, *lsize, "%s\n", fpath);
            for(;(*list)[0] != '\0' && (*lsize) > 1; (*list)++, (*lsize)--); 
            if ((*lsize) <= 1) break;
        }
    }
    f_closedir(&pdir);
    
    return ret;
}

bool GetFileList(const char* path, char* list, int lsize, bool recursive)
{
    if (!fs_ok)
        return false;
    char fpath[256];
    strncpy(fpath, path, 256);
    return GetFileListWorker(&list, &lsize, fpath, 256, recursive);
}

size_t FileGetData(const char* path, void* buf, size_t size, size_t foffset)
{
    unsigned flags = FA_READ | FA_OPEN_EXISTING;
    FIL tmp_file;
    if (!fs_ok)
        return 0;
    if (f_open(&tmp_file, path, flags) == FR_OK) {
        UINT bytes_read = 0;
        bool res = false;
        f_lseek(&tmp_file, foffset);
        f_sync(&tmp_file);
        res = (f_read(&tmp_file, buf, size, &bytes_read) == FR_OK);
        f_close(&tmp_file);
        return (res) ? bytes_read : 0;
    }
    
    return 0;
}

size_t FileDumpData(const char* path, void* buf, size_t size)
{
    unsigned flags = FA_WRITE | FA_CREATE_ALWAYS;
    FIL tmp_file;
    UINT bytes_written = 0;;
    bool res = false;
    if (!fs_ok)
        return 0;
    if (f_open(&tmp_file, path, flags) != FR_OK)
        return 0;
    f_lseek(&tmp_file, 0);
    f_sync(&tmp_file);
    res = (f_write(&tmp_file, buf, size, &bytes_written) == FR_OK);
    f_close(&tmp_file);
    
    return (res) ? bytes_written : 0;
}

bool PartitionFormat(const char* label)
{
    UINT p_size_gb = ((uint64_t) getMMCDevice(1)->total_size * 512) / (1000 * 1000 * 1000);
    UINT c_size = (p_size_gb < 4) ? 0 : (p_size_gb < 15) ? 32768 : 65536;
    bool ret = (f_mkfs("0:", 0, c_size) == FR_OK);
    if (ret && label) {
        char label0[16];
        snprintf(label0, 16, "0:%-11.11s", label);
        f_setlabel(label0);
    }
    return ret;
}

static uint64_t ClustersToBytes(FATFS* fs, DWORD clusters)
{
    uint64_t sectors = clusters * fs->csize;
    #if _MAX_SS != _MIN_SS
    return sectors * fs->ssize;
    #else
    return sectors * _MAX_SS;
    #endif
}

uint64_t RemainingStorageSpace()
{
    if (!fs_ok)
        return -1;
    DWORD free_clusters;
    FATFS *fs2;
    FRESULT res = f_getfree("0:", &free_clusters, &fs2);
    if (res != FR_OK)
        return -1;

    return ClustersToBytes(&fs, free_clusters);
}

uint64_t TotalStorageSpace()
{
    if (!fs_ok)
        return -1;
    return ClustersToBytes(&fs, fs.n_fatent - 2);
}
void Reboot()
{
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 2);
    while(true);
}


void PowerOff()
{
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0);
    while (true);
}



