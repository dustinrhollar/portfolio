
#define MAPLE_ASSET_META_EXTENSION    ".meta"

FORCE_INLINE
bool operator==(FILE_ID left, FILE_ID right)
{
    return left.mask == right.mask;
}

FORCE_INLINE
u32 FilePoolGetMaskInts()
{
    return memory_align(FilePool::MAX_FILE_COUNT, 64) / 64;
}

FORCE_INLINE
File* GetFileById(FileManager *manager, FILE_ID fid)
{
    return manager->file_pools[fid._p.major].handles + fid._p.minor;
}

FORCE_INLINE 
char* GetFileExt(char *filepath)
{
    char *pch  = 0;
    pch = strrchr((char*)filepath, '.');
    return pch;
}

FORCE_INLINE 
bool IsFileMeta(char *filepath)
{
    char *pch  = 0;
    pch = strrchr((char*)filepath, '.');
    
    if (!pch || strncmp(MAPLE_ASSET_META_EXTENSION, pch, strlen(MAPLE_ASSET_META_EXTENSION)) != 0)
    {
        return false;
    }
    else 
    {
        return true;
    }
}

FORCE_INLINE
File* AllocateFileFromPoolList(FilePool *pools, u32 pool_count)
{
    File *result = 0;
    for (u32 i = 0; i < pool_count; ++i)
    {
        pools[i].Alloc(&result);
        if (result) 
        {
            result->fid._p.major = i;
            break;
        }
    }
    return result;
}

FORCE_INLINE
DirectoryMount* GetMountById(STR_POOL_ID id, DirectoryMount *mounts, u32 mounts_count)
{
    DirectoryMount *result = 0;
    for (u32 i = 0; i < mounts_count; ++i)
    {
        if (StrPool::CompareStrId(mounts[i].name, id))
        {
            result = mounts + i;
            break;
        }
    }
    return result;
}

void FilePairMultimap::Init(FilePairMultimap *result)
{
    *result = {};
    result->dummy_guid = PlatformGenerateGuid();
    result->guid_to_file_pair = 0;
    result->count = 0;
    result->name_to_guid_index = 0;
}

void FilePairMultimap::Shutdown(StrPool *str_pool)
{
    // Eject the file names from the string pool, if there are any
    for (u32 i = 0; i < arrcap(guid_to_file_pair); ++i)
    {
        if (guid_to_file_pair[i].guid != dummy_guid)
        {
            str_pool->Eject(&guid_to_file_pair[i].virtual_name);
        }
    }
    
    count = 0;
    if(guid_to_file_pair) arrfree(guid_to_file_pair);
    guid_to_file_pair = 0;
    
    if (name_to_guid_index) arrfree(name_to_guid_index);
    name_to_guid_index = 0;
}

// Add the element to the guid to file pair
// Returns the index the element was added to
u64 FilePairMultimap::AddToGuid(FilePair *table, FilePair *pair)
{
    // Assume does not need to resize
    u64 result = U64_MAX;
    
    u128 *guid_hash = (u128*)(&pair->guid);
    u64 start_idx = (u64)guid_hash->lower % (u64)arrcap(table);
    for (u32 i = 0; i < arrcap(table); ++i)
    {
        u64 idx = (start_idx + i) % arrcap(table);
        FilePair *to_cmp = table + idx;
        
        if (Compare128BitCoerce(&to_cmp->guid, &dummy_guid))
        {
            // No entry at this index, go ahead and add
            *to_cmp = *pair;
            result = idx;
            break;
        }
        else if (Compare128BitCoerce(&to_cmp->guid, &pair->guid))
        {
            LogError("Duplicate id found when adding a GUID to the Mount GUID to Fid Table\n");
        }
    }
    
    return result;
}

void FilePairMultimap::Add(FilePair file_pair)
{
    // Resize the tables, if necessary
    r32 current_load = (r32)(count + 1) / (r32)arrcap(guid_to_file_pair);
    if (current_load >= LOAD_FACTOR)
    {
        u32 old_cap = (u32)arrcap(guid_to_file_pair);
        u32 new_cap = ((old_cap > 0) ? old_cap * 2 : 5);
        
        FilePair  *new_guid = 0;
        arrsetcap(new_guid, new_cap);
        
        // Register the new table with all invalid slots
        for (u32 i = 0; i < new_cap; ++i)
            new_guid[i].guid = dummy_guid;
        
        FileNameToGuidIdx *new_name = 0;
        arrsetcap(new_name, new_cap);
        
        // Register new slots as invalid
        for (u32 i = 0; i < new_cap; ++i)
            new_name[i].key = StrPool::INVALID;
        
        Assert(new_cap < U64_MAX);
        
        // Iterate through the old tables and add its slots
        // to the new tables
        for (u32 i = 0; i < old_cap; ++i)
        {
            FilePair *to_cmp = guid_to_file_pair + i;
            if (!Compare128BitCoerce(&to_cmp->guid, &dummy_guid))
            {
                // Get the insertion point into the Primary Table
                u64 chosen_idx = AddToGuid(new_guid, to_cmp);
                AddToName(new_name, to_cmp->virtual_name, chosen_idx);
            }
        }
        
        // Free the old tables and set new ones
        arrfree(guid_to_file_pair);
        guid_to_file_pair = new_guid;
        arrfree(name_to_guid_index);
        name_to_guid_index = new_name;
    }
    
    // Now add the new file pair
    
    u64 chosen_idx = AddToGuid(guid_to_file_pair, &file_pair);
    AddToName(name_to_guid_index, file_pair.virtual_name, chosen_idx);
    
    count++;
}

void FilePairMultimap::RemoveByGuid(MAPLE_GUID guid)
{
    // TODO(Dustin): Insert tombstones rather than directly removing?
    
    STR_POOL_ID name_to_remove;
    bool found = false;
    
    u128 *key_hash = (u128*)(&guid);
    u64 start_idx = key_hash->lower % arrcap(guid_to_file_pair);
    for (u32 i = 0; i < arrcap(guid_to_file_pair); ++i)
    {
        u64 idx = (start_idx + i) % arrcap(guid_to_file_pair);
        FilePair *to_cmp = guid_to_file_pair + idx;
        
        if (Compare128BitCoerce(&to_cmp->guid, &guid))
        {
            name_to_remove = to_cmp->virtual_name;
            found = true;
            to_cmp->guid = dummy_guid;
            to_cmp->virtual_name = StrPool::INVALID;
            count--;
            break;
        }
        else if (Compare128BitCoerce(&to_cmp->guid, &dummy_guid))
        {
            // Insertion is by linear probing, so if an empty cell was
            // reached, then this name does not exist and give up
            break;
        }
    }
    
    if (found)
    {
        u64 start_idx = name_to_remove.v % arrcap(name_to_guid_index);
        for (u32 i = 0; i < arrcap(name_to_guid_index); ++i)
        {
            u64 idx = (start_idx + i) % arrcap(name_to_guid_index);
            FileNameToGuidIdx *to_cmp = name_to_guid_index + idx;
            
            if (StrPool::CompareStrId(to_cmp->key, name_to_remove))
            {
                to_cmp->key = StrPool::INVALID;
                to_cmp->idx = U64_MAX;
                break;
            }
            else if (StrPool::CompareStrId(to_cmp->key, StrPool::INVALID))
            {
                // Insertion is by linear probing, so if an empty cell was
                // reached, then this name does not exist and give up
                break;
            }
        }
    }
}

void FilePairMultimap::GetByGuid(FilePair **result, MAPLE_GUID guid)
{
    *result = 0;
    
    // early out
    if (!guid_to_file_pair) return;
    
    u128 *key_hash = (u128*)(&guid);
    u64 start_idx = key_hash->lower % arrcap(guid_to_file_pair);
    for (u32 i = 0; i < arrcap(guid_to_file_pair); ++i)
    {
        u64 idx = (start_idx + i) % arrcap(guid_to_file_pair);
        FilePair *to_cmp = guid_to_file_pair + idx;
        
        if (Compare128BitCoerce(&to_cmp->guid, &guid))
        {
            *result = guid_to_file_pair + idx;
            break;
        }
        else if (Compare128BitCoerce(&to_cmp->guid, &dummy_guid))
        {
            // Insertion is by linear probing, so if an empty cell was
            // reached, then this name does not exist and give up
            break;
        }
    }
}

// Add the element to the name to guid index
void FilePairMultimap::AddToName(FileNameToGuidIdx *table, STR_POOL_ID key, u64 value)
{
    // Assume does not need to resize
    u64 name_hash = key.v;
    u64 start_idx = name_hash % (u64)arrcap(table);
    for (u32 i = 0; i < arrcap(table); ++i)
    {
        u64 idx = (start_idx + i) % arrcap(table);
        FileNameToGuidIdx *to_cmp = table + idx;
        
        if (StrPool::CompareStrId(to_cmp->key, StrPool::INVALID))
        {
            // No entry at this index, go ahead and add
            to_cmp->idx = value;
            to_cmp->key = { name_hash };
            break;
        }
        else if (StrPool::CompareStrId(to_cmp->key, { name_hash }))
        {
            LogError("Duplicate id found when adding a GUID to the Mount GUID to Fid Table\n");
        }
    }
}

void FilePairMultimap::RemoveByName(STR_POOL_ID name)
{
    u64 chosen_idx = U64_MAX;
    u64 start_idx = name.v % arrcap(name_to_guid_index);
    for (u32 i = 0; i < arrcap(name_to_guid_index); ++i)
    {
        u64 idx = (start_idx + i) % arrcap(name_to_guid_index);
        FileNameToGuidIdx *to_cmp = name_to_guid_index + idx;
        
        if (StrPool::CompareStrId(to_cmp->key, name))
        {
            chosen_idx = to_cmp->idx;
            to_cmp->idx = U64_MAX;
            to_cmp->key = StrPool::INVALID;
            count--;
            break;
        }
        else if (StrPool::CompareStrId(to_cmp->key, StrPool::INVALID))
        {
            // Insertion is by linear probing, so if an empty cell was
            // reached, then this name does not exist and give up
            break;
        }
    }
    
    if (chosen_idx != U64_MAX)
    {
        guid_to_file_pair[chosen_idx].guid = dummy_guid;
        guid_to_file_pair[chosen_idx].virtual_name = StrPool::INVALID;
    }
}

void FilePairMultimap::GetByName(FilePair **result, STR_POOL_ID name)
{
    *result = 0;
    u64 chosen_idx = U64_MAX;
    u64 start_idx = name.v % arrcap(name_to_guid_index);
    for (u32 i = 0; i < arrcap(name_to_guid_index); ++i)
    {
        u64 idx = (start_idx + i) % arrcap(name_to_guid_index);
        FileNameToGuidIdx *to_cmp = name_to_guid_index + idx;
        
        if (StrPool::CompareStrId(to_cmp->key, name))
        {
            chosen_idx = to_cmp->idx;
            break;
        }
        else if (StrPool::CompareStrId(to_cmp->key, StrPool::INVALID))
        {
            // Insertion is by linear probing, so if an empty cell was
            // reached, then this name does not exist and give up
            break;
        }
    }
    
    if (chosen_idx != U64_MAX)
    {
        *result = guid_to_file_pair + chosen_idx;
    }
}

FilePool FilePool::Init()
{
    FilePool result = {};
    
    result.handles = 0;
    arrsetlen(result.handles, MAX_FILE_COUNT);
    result.allocated_files      = 0;
    
    for (u32 i = 0; i < arrlen(result.handles); ++i)
        result.handles[i] = File::Invalid();
    
    u32 mask_ints = FilePoolGetMaskInts();
    for (u32 i = 0; i < mask_ints; ++i)
        result.allocation_mask[i] = 0;
    
    return result;
}

void FilePool::Shutdown()
{
    arrfree(handles);
    allocated_files = 0;
}

void FilePool::Alloc(File **file)
{
    *file = 0;
    
    // Early out: don't bother allocating a file if the pool is full
    if (allocated_files + 1 >= arrlen(handles)) return;
    
    u32 mask_ints = FilePoolGetMaskInts();
    u64 bitset;
    for (u32 i = 0; i < mask_ints; ++i)
    {
        // Negate the particular bitset. Ctz will find the first
        // 1 starting from the the least significant digit. However,
        // for the purposes of this bitlist, a 1 means an allocation,
        // but we want to find unallocated blocks. Negating the bitset will
        // set all allocated blocks to 0 and unallocated blocks to 1.
        // The found idx is the index of the block we want to retrieve
        // for the allocation.
        bitset = ~allocation_mask[i];
        
        // Ctz() is undefined when the number == 0
        // If the negated bitset == 0, then there are
        // no available allocations in the block.
        // If there are no allocations, then go to the
        // next bitset.
        u32 idx = 0;
        u32 bit = 0;
        if (bitset)
        {
            bit = PlatformCtzl(bitset);
            idx = i * 64 + bit;
            BIT64_TOGGLE_1(allocation_mask[i], bit);
            
            *file = handles + idx;
            (*file)->fid._p.minor = idx;
            ++allocated_files;
            
            break;
        }
    }
}

void FilePool::Free(FILE_ID fid)
{
    handles[fid._p.minor] = File::Invalid();
    
    u32 bitlist_idx = fid._p.minor / 64;
    u32 bit_idx = fid._p.minor % 64;
    BIT64_TOGGLE_0(allocation_mask[bitlist_idx], bit_idx);
}

FileManager FileManager::Init(const char *root)
{
    FileManager result = {};
    
    result.file_pools = 0;
    arrsetlen(result.file_pools, 1);
    result.file_pools[0] = FilePool::Init();
    
    // NOTE(Dustin): I am deciding _64KB as the default size
    // This might have to be expanded if there is an absurd 
    // amount of files
    result.str_pool = StrPool::Init(_64KB);
    
    //Str exe_path = PlatformGetFullExecutablePath();
    result.root_path = result.str_pool.Inject(root, (u32)strlen(root));
    
    return result;
}

void FileManager::Shutdown()
{
    for (u32 i = 0; i < arrlen(mount_points); ++i)
    {
        RemoveFile(mount_points[i].fid);
        str_pool.Eject(&mount_points[i].name);
        mount_points[i].Shutdown(&str_pool);
    }
    arrfree(mount_points);
    
    for (u32 i = 0; i < arrlen(file_pools); ++i)
        file_pools[i].Shutdown();
    arrfree(file_pools);
    
    str_pool.Eject(&root_path);
    str_pool.Shutdown();
}

FILE_ID FileManager::AddFile(FILE_ID parent, const char *parent_path, const char *filepath)
{
    File *file = AllocateFileFromPoolList(file_pools, (u32)arrlen(file_pools));
    file->parent = parent;
    
    FILE_ID result = file->fid;
    
    char path[2048];
    int ch = 0;
    
    if (parent_path)
    {
        ch = snprintf(path, 2048, "%s/%s", parent_path, filepath);
    }
    else
    {
        ch = snprintf(path, 2048, "%s", filepath);
    }
    
    path[ch] = 0;
    
    BY_HANDLE_FILE_INFORMATION file_info;
    BOOL Err = GetFileAttributesEx(path,
                                   GetFileExInfoStandard,
                                   &file_info);
    if (!Err) LogError("Error getting file attribute when initializing file %s!\n", path);
    else
    {
        if (file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            file->type = FileType::Directory;
        }
        else if (file_info.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)
        {
            file->type = FileType::Compressed;
        }
        else
        {
            file->type = FileType::File;
        }
    }
    
    
    file->is_meta = IsFileMeta((char*)filepath);
    file->physical_name = str_pool.Inject(path, (u32)strlen(path));
    file->child_files = 0;
    
    if (file->type == FileType::Directory)
    {
        char child_path[2048];
        
        ch = snprintf(child_path, 2048, "%s/*", path);
        child_path[ch] = 0;
        
        WIN32_FIND_DATA find_file_data;
        HANDLE handle = FindFirstFileEx(child_path, FindExInfoStandard, &find_file_data,
                                        FindExSearchNameMatch, NULL, 0);
        
        if (handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (strcmp(find_file_data.cFileName, filepath) != 0 &&
                    strcmp(find_file_data.cFileName, "..") != 0 &&
                    find_file_data.cFileName[0] != '.' ) // don't allow hidden files or folders
                {
                    arrput(file->child_files, AddFile(file->fid, path, find_file_data.cFileName));
                }
            }
            while (FindNextFile(handle, &find_file_data) != 0);
            
            
            FindClose(handle);
        }
    }
    
    return result;
}

void FileManager::RemoveFile(FILE_ID fid)
{
    File *file = file_pools[fid._p.major].handles + fid._p.minor;
    str_pool.Eject(&file->physical_name);
    
    for (u32 i = 0; i < arrlen(file->child_files); ++i)
    {
        RemoveFile(file->child_files[i]);
    }
    if (file->child_files) arrfree(file->child_files);
    
    file_pools[fid._p.major].Free(fid);
}

file_internal void JoinFilesWithMeta(FileManager *manager, DirectoryMount *mount)
{
    char meta_path[MAX_PATH];
    
    // Add all child files to a file lookup table
    File *mount_file = GetFileById(manager, mount->fid);
    FILE_ID *file_queue = 0;
    for (u32 i = 0; i < arrlen(mount_file->child_files); ++i)
    {
        arrput(file_queue, mount_file->child_files[i]);
    }
    
    while (arrlen(file_queue) > 0)
    {
        FILE_ID fid = arrpop(file_queue);
        File *file = GetFileById(manager, fid);
        
        for (u32 i = 0; i < arrlen(file->child_files); ++i)
        {
            arrput(file_queue, file->child_files[i]);
        }
        
        if (file->type == FileType::File && !file->is_meta)
        {
            u32 len = manager->str_pool.StrIdToStr(file->physical_name, meta_path, MAX_PATH);
            File *parent = GetFileById(manager, file->parent);
            
            meta_path[len+0] = '.';
            meta_path[len+1] = 'm';
            meta_path[len+2] = 'e';
            meta_path[len+3] = 't';
            meta_path[len+4] = 'a';
            meta_path[len+5] = 0;
            
            void *meta_buf;
            u32 meta_buf_size = 0;
            PlatformReadFileToBuffer(meta_path, (u8**)&meta_buf, &meta_buf_size);
            
            if (!meta_buf)
            {
                // TODO(Dustin): Rather than generating a new file here...the asset
                // manager should write the meta file to disk and a directory watcher
                // will issue the appropriate commands to load the meta file
                LogError("Meta file does not exist: %s\n", meta_path);
                continue;
            }
            
            // Find Meta file...
            STR_POOL_ID meta_str_id = StrPool::GetStrId(meta_path, len+4);
            FILE_ID meta_file_id = FILE_ID::INVALID_ID;
            
            for (u32 i = 0; i < arrlen(parent->child_files); i++)
            {
                File *meta_file = GetFileById(manager, parent->child_files[i]);
                if (StrPool::CompareStrId(meta_file->physical_name, meta_str_id))
                {
                    meta_file_id = parent->child_files[i];
                    break;
                }
            }
            
            // TODO(Dustin): Check to make sure the meta file was actually loaded into
            // the manager. If not, maybe update the directory?
            if (meta_file_id == FILE_ID::INVALID_ID)
            {
                LogError("Could not find meta file in the directory of the asset: %s\n", meta_path);
            }
            
            FilePair file_pair = {};
            file_pair.file = fid;
            file_pair.meta = meta_file_id;
            
            // Load the meta file and get the virtual name + guid from it
            
            // TODO(Dustin): Re-implement
#if 0
            AssetMeta meta_data = {};
            AssetManager::DeserializeMetaFile(&meta_data, (const char*)meta_buf);
            
            file_pair.guid = meta_data.guid;
            file_pair.virtual_name = manager->str_pool.Inject(meta_data.virtual_name.ptr, 
                                                              (u32)meta_data.virtual_name.size);
            file_pair.asset_type = meta_data.type;
            
            mount->file_multimap.Add(file_pair);
            MemFree(meta_buf);
#endif
        }
        
    }
    
    arrfree(file_queue);
}

void FileManager::Mount(const char *dir_path, const char *name, bool is_relative, bool is_recursive)
{
    // Recursively add the mount file and any children it has
    char root[MAX_PATH];
    u32 root_len = str_pool.StrIdToStr(root_path, root, MAX_PATH);
    root[root_len] = 0;
    
    FILE_ID mounted_file;
    if (is_relative)
        mounted_file = AddFile(FILE_ID::INVALID_ID, root, dir_path);
    else
        mounted_file = AddFile(FILE_ID::INVALID_ID, NULL, dir_path);
    
    // Initialize the mount point
    DirectoryMount mount = {};
    mount.name = str_pool.Inject(name, (u32)strlen(name));
    mount.fid  = mounted_file;
    FilePairMultimap::Init(&mount.file_multimap);
    
    JoinFilesWithMeta(this, &mount);
    
    // Add the mount point to the manager
    arrput(mount_points, mount);
}

void FileManager::ReloadMount(const char *name)
{
    STR_POOL_ID mount_id = StrPool::GetStrId(name, (u32)strlen(name));
    DirectoryMount *mount = GetMountById(mount_id, mount_points, (u32)arrlen(mount_points));
    AssertCustom(mount, "Attempted to reload mount from mount that does not exist!\n");
    
    File *file = GetFileById(this, mount->fid);
    
    // Save the physical path
    char phys_path[MAX_PATH];
    u32 len = str_pool.StrIdToStr(file->physical_name, phys_path, MAX_PATH);
    phys_path[len] = 0;
    
    RemoveFile(mount->fid);
    //str_pool.Eject(&mount_points[i].name);
    mount->Shutdown(&str_pool);
    
    mount->fid = AddFile(FILE_ID::INVALID_ID, NULL, phys_path);
    FilePairMultimap::Init(&mount->file_multimap);
    JoinFilesWithMeta(this, mount);
}


PlatformErrorType FileManager::SaveFile(FileInfo *info, void *buf, u64 size)
{
    PlatformErrorType result = PlatformError_Success;
    char path[MAX_PATH];
    
    switch (info->type)
    {
        case FileInfoType::AsFile:
        {
            result = PlatformWriteBufferToFile(info->_p._as_file.filename, (u8*)buf, size);
        } break;
        
        case FileInfoType::AsGuid:
        {
            STR_POOL_ID mount_id = StrPool::GetStrId(info->_p._as_guid.mount, 
                                                     (u32)strlen(info->_p._as_guid.mount));
            DirectoryMount *mount = GetMountById(mount_id, mount_points, (u32)arrlen(mount_points));
            AssertCustom(mount, "Attempted to get a file from mount that does not exist!\n");
            
            FilePair *file_pair = 0;
            mount->file_multimap.GetByGuid(&file_pair, info->_p._as_guid.guid);
            AssertCustom(file_pair, "Attempted to save to file as GUID not located in File Manager!\n");
            
            File *file = 0;
            if (info->is_meta)
                file = GetFileById(this, file_pair->meta);
            else
                file = GetFileById(this, file_pair->file);
            
            u32 res = str_pool.StrIdToStr(file->physical_name, path, MAX_PATH);
            path[res] = 0;
            
            result = PlatformWriteBufferToFile(path, (u8*)buf, size);
        } break;
        
        case FileInfoType::AsName:
        {
            STR_POOL_ID mount_id = StrPool::GetStrId(info->_p._as_virtual.mount, 
                                                     (u32)strlen(info->_p._as_virtual.mount));
            STR_POOL_ID name_id = StrPool::GetStrId(info->_p._as_virtual.virtual_name, 
                                                    (u32)strlen(info->_p._as_virtual.virtual_name));
            
            DirectoryMount *mount = GetMountById(mount_id, mount_points, (u32)arrlen(mount_points));
            AssertCustom(mount, "Attempted to get a file from mount that does not exist!\n");
            
            FilePair *file_pair = 0;
            mount->file_multimap.GetByName(&file_pair, name_id);
            AssertCustom(file_pair, "Attempted to save to file as GUID not located in File Manager!\n");
            
            File *file = 0;
            if (info->is_meta)
                file = GetFileById(this, file_pair->meta);
            else
                file = GetFileById(this, file_pair->file);
            
            u32 res = str_pool.StrIdToStr(file->physical_name, path, MAX_PATH);
            path[res] = 0;
            
            result = PlatformWriteBufferToFile(path, (u8*)buf, size);
        } break;
    }
    
    return result;
}

PlatformErrorType FileManager::LoadFile(FileInfo *info, void **buf, u64 *size)
{
    PlatformErrorType result = PlatformError_Success;
    char path[MAX_PATH];
    
    switch (info->type)
    {
        case FileInfoType::AsFile:
        {
            result = PlatformReadFileToBuffer(info->_p._as_file.filename, (u8**)buf, (u32*)size);
        } break;
        
        case FileInfoType::AsGuid:
        {
            STR_POOL_ID mount_id = StrPool::GetStrId(info->_p._as_guid.mount, 
                                                     (u32)strlen(info->_p._as_guid.mount));
            DirectoryMount *mount = GetMountById(mount_id, mount_points, (u32)arrlen(mount_points));
            AssertCustom(mount, "Attempted to get a file from mount that does not exist!\n");
            
            FilePair *file_pair = 0;
            mount->file_multimap.GetByGuid(&file_pair, info->_p._as_guid.guid);
            AssertCustom(file_pair, "Attempted to save to file as GUID not located in File Manager!\n");
            
            File *file = 0;
            if (info->is_meta)
                file = GetFileById(this, file_pair->meta);
            else
                file = GetFileById(this, file_pair->file);
            
            u32 res = str_pool.StrIdToStr(file->physical_name, path, MAX_PATH);
            path[res] = 0;
            
            result = PlatformReadFileToBuffer(path, (u8**)buf, (u32*)size);
        } break;
        
        case FileInfoType::AsName:
        {
            STR_POOL_ID mount_id = StrPool::GetStrId(info->_p._as_virtual.mount, 
                                                     (u32)strlen(info->_p._as_virtual.mount));
            STR_POOL_ID name_id = StrPool::GetStrId(info->_p._as_virtual.virtual_name, 
                                                    (u32)strlen(info->_p._as_virtual.virtual_name));
            
            DirectoryMount *mount = GetMountById(mount_id, mount_points, (u32)arrlen(mount_points));
            AssertCustom(mount, "Attempted to get a file from mount that does not exist!\n");
            
            FilePair *file_pair = 0;
            mount->file_multimap.GetByName(&file_pair, name_id);
            AssertCustom(file_pair, "Attempted to save to file as GUID not located in File Manager!\n");
            
            File *file = 0;
            if (info->is_meta)
                file = GetFileById(this, file_pair->meta);
            else
                file = GetFileById(this, file_pair->file);
            
            u32 res = str_pool.StrIdToStr(file->physical_name, path, MAX_PATH);
            path[res] = 0;
            
            result = PlatformReadFileToBuffer(path, (u8**)buf, (u32*)size);
        } break;
    }
    
    return result;
}

MAPLE_GUID FileManager::GetFileGuidByName(const char *virtual_name, const char *mount_name)
{
    STR_POOL_ID mount_id = StrPool::GetStrId(mount_name, 
                                             (u32)strlen(mount_name));
    STR_POOL_ID name_id = StrPool::GetStrId(virtual_name, 
                                            (u32)strlen(virtual_name));
    
    DirectoryMount *mount = GetMountById(mount_id, mount_points, (u32)arrlen(mount_points));
    AssertCustom(mount, "Attempted to get a file from mount that does not exist!\n");
    
    FilePair *file_pair = 0;
    mount->file_multimap.GetByName(&file_pair, name_id);
    AssertCustom(file_pair, "Attempted to get file as NAME not located in File Manager!\n");
    
    return file_pair->guid;
}

void FileManager::GetAllFilesOfType(AssetType desired_type, const  char *mount_name, FilePair **buffer, u32 *size)
{
    STR_POOL_ID mount_id = StrPool::GetStrId(mount_name, 
                                             (u32)strlen(mount_name));
    DirectoryMount *mount = GetMountById(mount_id, mount_points, (u32)arrlen(mount_points));
    AssertCustom(mount, "Attempted to get a file from mount that does not exist!\n");
    
    // TODO(Dustin): Which is better here?
    // A.) Iterate through the multimap to find the file
    // B.) Iterate through the file tree and then lookup in the multimap
    //
    // For a minimal file system, (A) is probably faster because of fewer
    // indirections. (B) might be faster for large file systems. Will need
    // to do some testing in the future. For now, go with option (A).
    //
    // Does it even matter? This is editor dependent code anyways. It
    // only runs when the editor is active or on startup.
    
    FilePair *result = 0;
    FilePairMultimap *map = &mount->file_multimap;
    u32 cap = (u32)arrcap(map->guid_to_file_pair);
    
    if (desired_type == AssetType::Unknown)
    {
        // Get all files rather than a specific type
        for (u32 i = 0; i < cap; ++i)
        {
            FilePair *potential = map->guid_to_file_pair + i;
            if (potential->guid != map->dummy_guid)
            {
                arrput(result, *potential);
            }
        }
    }
    else
    {
        for (u32 i = 0; i < cap; ++i)
        {
            FilePair *potential = map->guid_to_file_pair + i;
            if (potential->guid != map->dummy_guid &&
                potential->asset_type == desired_type)
            {
                arrput(result, *potential);
            }
        }
    }
    
    *buffer = result;
    *size = (u32)arrlen(result);
}

u32 FileManager::GetVirtualName(char *buf, u32 size, MAPLE_GUID guid, const char *mount_name)
{
    STR_POOL_ID mount_id = StrPool::GetStrId(mount_name, 
                                             (u32)strlen(mount_name));
    
    DirectoryMount *mount = GetMountById(mount_id, mount_points, (u32)arrlen(mount_points));
    AssertCustom(mount, "Attempted to get a file from mount that does not exist!\n");
    
    FilePair *file_pair = 0;
    mount->file_multimap.GetByGuid(&file_pair, guid);
    AssertCustom(file_pair, "Attempted get file as GUID not located in File Manager!\n");
    
    return str_pool.StrIdToStr(file_pair->virtual_name, buf, size);
}