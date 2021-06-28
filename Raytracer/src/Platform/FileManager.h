#ifndef _FILE_MANAGER_H
#define _FILE_MANAGER_H

#include "Platform/Platform.h"

//
// A file id is a mask that can locate the file within the
// Win32FilePool data structure. An Win32FileManager instance will
// contain *at most* 256 Win32FilePools, each with the capacity
// of 256 files. This allows for up to 65,536 files for a single
// instance assetsys. 
//
// A file id is a mask for the major and minor pool index.
// A major pool index is the index in the file pool array
// whereas the minor pool index is the index into the file array
// inside a pool. The lower 8 bits of the file id are the major 
// pool index and the upper 8 bits are the minor pool index.
//
union FILE_ID
{
    struct { u8 major, minor; } _p;
    u16 mask;
    static const FILE_ID INVALID_ID;
};
const FILE_ID FILE_ID::INVALID_ID = {255,255};

enum class FileType
{
    Volume = 0,
    Directory,
    Compressed,
    File,
    
    Count,
    Unknown = Count, // used for checking if file is initialized
};

enum class FileMode
{
    Read,
    Write,
    ReadWrite,
    Append,
    
    Count,
    Unknown = Count, 
};

enum class AssetType
{
    // Assets...
    
    Count,
    Unknown = Count,
};

// NOTE(Dustin): 28 bytes, possible to shrink?
struct FilePair
{
    FILE_ID     file;
    FILE_ID     meta;
    MAPLE_GUID  guid;
    STR_POOL_ID virtual_name;
    AssetType   asset_type;
};

// NOTE(Dustin): 16 bytes, possible to shrink? 
struct FileNameToGuidIdx
{
    u64         idx;
    STR_POOL_ID key; // file virtual name
};

struct FilePairMultimap
{
    // Retrieving information by name is editor-
    // dependent functionality
    
    MAPLE_GUID         dummy_guid; // invalidates slots in the guid table
    FilePair          *guid_to_file_pair;
    u32                count;
    FileNameToGuidIdx *name_to_guid_index;
    
    static void Init(FilePairMultimap *result);
    void Shutdown(StrPool *str_pool);
    
    void Add(FilePair file_pair);
    void RemoveByGuid(MAPLE_GUID guid);
    void GetByGuid(FilePair **result, MAPLE_GUID guid);
    
    void RemoveByName(STR_POOL_ID name);
    void GetByName(FilePair **result, STR_POOL_ID name);
    
    static const r32 LOAD_FACTOR;
    
    private:
    
    // Add the element to the guid to file pair
    u64 AddToGuid(FilePair *table, FilePair *pair);
    // Add the element to the name to guid index
    EDITOR_DEP_FN(void AddToName(FileNameToGuidIdx *table, STR_POOL_ID key, u64 idx));
};
const r32 FilePairMultimap::LOAD_FACTOR = 0.75f;

struct DirectoryMount
{
    STR_POOL_ID      name; // virtual name for the mount point
    FILE_ID          fid;  // root file for the mount
    FilePairMultimap file_multimap;
    
    void AddGuidToTable(MAPLE_GUID key, FILE_ID fid);
    void RemoveGuidFromTable(MAPLE_GUID key);
    FILE_ID GetFidFromGuidTable(MAPLE_GUID key);
    
    // The next 3 functions are editor dependent
    
    EDITOR_DEP_FN(void AddVirtualNameToTable(STR_POOL_ID virtual_name, MAPLE_GUID guid));
    EDITOR_DEP_FN(void RemoveVirtualNameToTable(STR_POOL_ID virtual_name));
    // Alittle unique because there is not a unique error case for 
    // a guid. Return true on success and false on failure.
    EDITOR_DEP_FN(bool GetGuidFromVirtualNameTable(MAPLE_GUID *result, STR_POOL_ID key));
    
    void Shutdown(StrPool *str_pool)
    {
        file_multimap.Shutdown(str_pool);
    }
};

enum class FileInfoType : u8 
{
	AsFile, // file info as filepath
    AsGuid, // file info as GUID
    AsName, // file info as virtual name
    
    Count,
    Unknown = Count,
};

struct FileInfo
{
	FileInfoType type;
    
    u8 is_relative:1; // is the filename a relative path to mount? 1: yes, 0: no
    u8 is_meta:1;     // is this a meta file? 1: yes, 0: no
    u8 pad0:6;        // what to put here in the future?
    
	union {
		struct {
			const char *filename;
			const char *mount;
        } _as_file;
        
		struct {
			MAPLE_GUID  guid;
			const char *mount;
		} _as_guid;
        
		struct {
			const char *virtual_name;
			const char *mount;
		} _as_virtual;
	} _p;
};

struct File
{
    // Name information
    STR_POOL_ID physical_name; // abs path,    , ex. "C:\some_dir\maple-merchant\shaders\internal\file.vert"
    FileType    type;
    bool        is_meta;
    FILE_ID     fid;
    FILE_ID     parent;
    FILE_ID    *child_files;   //stb_array
    
    static File Invalid()
    {
        File result = {};
        result.physical_name = StrPool::INVALID;
        result.type          = FileType::Unknown;
        result.child_files   = 0;
        return result;
    }
};

// Files are allocated from a File Pool.
struct FilePool
{
    static const u16 MAX_POOL_COUNT = 256;
    static const u16 MAX_FILE_COUNT = 255;
    
    File *handles;
    u32   allocated_files;
    // The number of required u64s is dependent on the number
    // of allowed files in the Pool. For example, if we have a
    // Pool with a max of 256 files should have 4 u64s to have a
    // mask for all files. Likewise, a Pool with 250 files should
    // also have 4 u64s.
    u64   allocation_mask[memory_align(FilePool::MAX_FILE_COUNT, 64) / 64];
    
    static FilePool Init();
    void Shutdown();
    void Alloc(File **File);
    void Free(FILE_ID fid);
};

struct FileManager
{
    StrPool         str_pool;
    FilePool       *file_pools;   // stb array
    DirectoryMount *mount_points; // stb array
    STR_POOL_ID     root_path;    // NOTE(Dustin): Is this needed anymore?
    
    static FileManager Init(const char *root);
    void Shutdown();
    
    void Mount(const char *dir_path, const char *name, bool is_relative, bool is_recursive);
    void ReloadMount(const char *name);
    
    FILE_ID AddFile(FILE_ID parent, const char *parent_path, const char *filepath);
    void RemoveFile(FILE_ID fid);
    
    PlatformErrorType SaveFile(FileInfo *info, void *buf, u64 size);
    PlatformErrorType LoadFile(FileInfo *info, void **buf, u64 *size);
    
    // NOTE(Dustin): Editor dependent?
    //-------------------------------------------------------------------------------------
    MAPLE_GUID GetFileGuidByName(const char *virtual_name, const char *mount);
    // Buffers for the next function are allocated using STB_ARRAY
    void GetAllFilesOfType(AssetType desired_type, const  char *mount, FilePair **buffer, u32 *size);
    
    u32 GetVirtualName(char *buf, u32 size, MAPLE_GUID guid, const char *mount);
};

#endif //_FILE_MANAGER_H