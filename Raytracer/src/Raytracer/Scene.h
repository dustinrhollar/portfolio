#ifndef _RAYTRACER_SCENE_H
#define _RAYTRACER_SCENE_H

struct Scene 
{
    struct Primitive *primitives;
    u32               primitives_count;
    u32               primitives_cap;
    
    // Optional
    struct Primitive *bvh_tree; 
};

file_internal void scene_init(Scene *scene, u32 cap);
file_internal void scene_free(Scene *scene);
file_internal void scene_add(Scene *scene, struct Primitive *primitive);
file_internal void build_random_scene(Scene *scene, b8 motion_blur);


#endif //_RAYTRACER_SCENE_H
