
#define MESH_VERTEX_POSITION  BIT(0)
#define MESH_VERTEX_COLOR     BIT(2)
#define MESH_VERTEX_NORMAL    BIT(3)
#define MESH_VERTEX_UV0       BIT(4)

v3 v3_calc_normal(v3 vert0, v3 vert1, v3 vert2);
void mesh_gen_simple(
        RenderComponent *render_comp,
        u8               vertex_mask,
        u32              width,
        u32              height);

void mesh_gen_clever(
        RenderComponent *render_comp,
        u32              width,
        u32              height);
