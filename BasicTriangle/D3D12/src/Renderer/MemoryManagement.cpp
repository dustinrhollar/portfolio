
const u64 DEFAULT_PAGE_SIZE = _2MB;
struct UploadBuffer
{
    struct Page
    {
        ID3D12Resource           *rsrc;
        void                     *base;
        D3D12_GPU_VIRTUAL_ADDRESS gpu_ptr;
        u64                       page_size;
        u64                       offset;
        
        void Init(u64 page_size);
        void Free();
    };
    
    struct Allocation
    {
        void                     *cpu;
        D3D12_GPU_VIRTUAL_ADDRESS gpu;
    };
    
    Page *page_pool = 0;
    u32  *avail_pages = 0;
    u32   active_page;
    u64   page_size;
    
    void Init(u64 page_size = DEFAULT_PAGE_SIZE);
    void Free();
    
    void Allocate();
};



