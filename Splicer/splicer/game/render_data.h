#ifndef SPLICER_GAME_RENDER_DATA_H
#define SPLICER_GAME_RENDER_DATA_H


// Shader representation of a Vertex.
struct Vertex
{
    Vec3 Position;
    Vec3 Normals;
    Vec4 Color;
    Vec2 Tex0;
    
    static VkVertexInputBindingDescription GetBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
        return bindingDescription;
    }
    
    static JTuple<VkVertexInputAttributeDescription*, int> GetAttributeDescriptions() {
        // Create Attribute Descriptions
        VkVertexInputAttributeDescription *attribs = talloc<VkVertexInputAttributeDescription>(4);
        
        // Positions
        attribs[0].binding = 0;
        attribs[0].location = 0;
        attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribs[0].offset = offsetof(Vertex, Position);
        
        // Normals
        attribs[1].binding = 0;
        attribs[1].location = 1;
        attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribs[1].offset = offsetof(Vertex, Normals);
        
        // Color
        attribs[2].binding = 0;
        attribs[2].location = 2;
        attribs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attribs[2].offset = offsetof(Vertex, Color);
        
        // Texture
        attribs[3].binding = 0;
        attribs[3].location = 3;
        attribs[3].format = VK_FORMAT_R32G32_SFLOAT;
        attribs[3].offset = offsetof(Vertex, Tex0);
        
        JTuple<VkVertexInputAttributeDescription*, int> tuple(attribs, 4);
        return tuple;
    }
    
};


// Shader representation of a Vertex.
struct SkinnedVertex
{
    Vec3 Position;
    Vec3 Normals;
    Vec4 Color;
    Vec2 Tex0;
    Vec4 Weights;
    Vec4 Joints;
    
    static VkVertexInputBindingDescription GetBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(SkinnedVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
        return bindingDescription;
    }
    
    static JTuple<VkVertexInputAttributeDescription*, int> GetAttributeDescriptions() {
        // Create Attribute Descriptions
        VkVertexInputAttributeDescription *attribs = talloc<VkVertexInputAttributeDescription>(6);
        
        // Positions
        attribs[0].binding = 0;
        attribs[0].location = 0;
        attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribs[0].offset = offsetof(SkinnedVertex, Position);
        
        // Normals
        attribs[1].binding = 0;
        attribs[1].location = 1;
        attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribs[1].offset = offsetof(SkinnedVertex, Normals);
        
        // Color
        attribs[2].binding = 0;
        attribs[2].location = 2;
        attribs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attribs[2].offset = offsetof(SkinnedVertex, Color);
        
        // Texture
        attribs[3].binding = 0;
        attribs[3].location = 3;
        attribs[3].format = VK_FORMAT_R32G32_SFLOAT;
        attribs[3].offset = offsetof(SkinnedVertex, Tex0);
        
        // Weights
        attribs[4].binding = 0;
        attribs[4].location = 4;
        attribs[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attribs[4].offset = offsetof(SkinnedVertex, Weights);
        
        // Joints
        attribs[5].binding  = 0;
        attribs[5].location = 5;
        attribs[5].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
        attribs[5].offset   = offsetof(SkinnedVertex, Joints);
        
        JTuple<VkVertexInputAttributeDescription*, int> tuple(attribs, 6);
        return tuple;
    }
    
};


// For now, render components are simple
// structs. And the render system will merely
// iterate over them.
struct RenderComponent
{
    bool IndexedDraw;
    
    BufferParameters VertexBuffer;
    BufferParameters IndexBuffer;
    
    u32 tri_count;
};


#endif //RENDER_DATA_H
