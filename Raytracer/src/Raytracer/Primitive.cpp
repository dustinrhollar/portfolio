
file_internal void 
make_sphere(Primitive *prim, v3 origin, r32 radius,  Material material)
{
    prim->type   = Primitive_Sphere;
    prim->sphere.origin   = origin;
    prim->sphere.radius   = radius;
    prim->sphere.material = material;
}

file_internal void 
make_dynamic_sphere(Primitive *prim,
                    v3 center0, v3 center1,
                    r32 time0, r32 time1,
                    r32 radius,
                    Material material)
{
    prim->type = Primitive_DynamicSphere;
    prim->dyn_sphere.c0       = center0;
    prim->dyn_sphere.c1       = center1;
    prim->dyn_sphere.t0       = time0;
    prim->dyn_sphere.t1       = time1;
    prim->dyn_sphere.radius   = radius;
    prim->dyn_sphere.material = material;
}

typedef int (*bvh_compar)(const void *, const void*);

int 
box_x_compare(const void *l, const void *r)
{
    Aabb left, right;
    build_aabb_primitive(&left, (Primitive*)l, 0, 0);
    build_aabb_primitive(&right, (Primitive*)r, 0, 0);
    return left.min.p[0] < right.min.p[0];
}

int 
box_y_compare(const void *l, const void *r)
{
    Aabb left, right;
    build_aabb_primitive(&left, (Primitive*)l, 0, 0);
    build_aabb_primitive(&right, (Primitive*)r, 0, 0);
    return left.min.p[1] < right.min.p[1];
}

int 
box_z_compare(const void *l, const void *r)
{
    Aabb left, right;
    build_aabb_primitive(&left, (Primitive*)l, 0, 0);
    build_aabb_primitive(&right, (Primitive*)r, 0, 0);
    return left.min.p[2] < right.min.p[2];
}

file_internal void 
make_bvh_node(Primitive *result, Primitive *primitives, u32 start, u32 end, r32 t0, r32 t1)
{
    result->type  = Primitive_BvhNode;
    int axis = random_int_clamped(0, 3);
    int span = end - start;
    
    bvh_compar comparator = (axis == 0) ? box_x_compare : (axis == 1) ? box_y_compare : box_z_compare;
    
    if (span == 1)
    {
        result->bvh_node.left = result->bvh_node.right = &primitives[start];
    }
    else if (span == 2)
    {
        if (comparator(&primitives[start], &primitives[start+1]))
        {
            result->bvh_node.left  = &primitives[start];
            result->bvh_node.right = &primitives[start+1];
        }
        else
        {
            result->bvh_node.left  = &primitives[start+1];
            result->bvh_node.right = &primitives[start];
        }
    }
    else
    {
        qsort(primitives + start, span, sizeof(Primitive), comparator);
        
        result->bvh_node.left  = (Primitive*)malloc(sizeof(Primitive));
        result->bvh_node.right = (Primitive*)malloc(sizeof(Primitive));
        
        int mid = start + span / 2;
        make_bvh_node(result->bvh_node.left,  primitives, start, mid, t0, t1);
        make_bvh_node(result->bvh_node.right, primitives, mid, end, t0, t1);
    }
    
    Aabb left, right;
    build_aabb_primitive(&left, result->bvh_node.left, t0, t1);
    build_aabb_primitive(&right, result->bvh_node.right, t0, t1);
    build_surrounding_box(&result->bvh_node.aabb, &left, &right);
    int a = 0;
}
