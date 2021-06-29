//
// A set of functions used used to generate a LOD mesh
//

#include "LODMesh.h"
#include <math.h>
#include <iostream>

#define PI 3.14159

//----------------------------------------------------------------------------------------------//
// SET OF INLINE FUNCTIONS TO SPEED UP THE MATH
//----------------------------------------------------------------------------------------------//
// floor a double
static inline
int32_t fastfloor( double x )
{
    auto xi = static_cast<int32_t>(x);
    return ( x < xi ) ? ( xi - 1 ) : xi;
}

// cast a double up
static inline
int32_t fastceiling( double x )
{
    auto xi = static_cast<int32_t>(x);
    return ( x > xi ) ? ( xi + 1 ) : xi;
}

// convert an angle from degrees to radians
static inline
void convertRadians( float& angleInDegrees ) {
    angleInDegrees = ( angleInDegrees / 180.0f ) * (float)PI;
}

// determien if a number is within a threshhold
static inline
bool threshhold( float num ) {
    return ( 1.0f - ( (float)fastfloor( (double) num ) / num ) ) > 0.01;
}

// Calcualte the bound for an angle and distance
static inline
int calculateBound( float angle, int i ) {
    // Find the number of required columns
    float bound = (float)(i) * tanf( angle );
    
    return !threshhold( (float)(i) * tanf( angle ) ) ? fastfloor( bound ) : fastceiling( bound );
}

//----------------------------------------------------------------------------------------------//
// GENERATE THE MESH
//----------------------------------------------------------------------------------------------//

// Generate a quadrant of the mesh, currently cannot handle a grid larger than 32x32
LODMesh* generateFullMesh( uint width, uint height, uint patch_size ) {
    
    LODMesh* mesh = new LODMesh;
	// 8x8 = 64
	// patch_size / width = 64 / 16 = 4 patches total
	// 16 * t_patches = 16 * 4 = 64
	uint extra_verts_per_row = width / (patch_size / 4);
	uint new_width = width + extra_verts_per_row;
    mesh->num_vert = static_cast<uint>( patch_size * ( ( (new_width * new_width) / patch_size ) ) );
    mesh->size_vert = 2 * mesh->num_vert * sizeof( float );
    
    // memory block init
    mesh->memory_block = new char[ mesh->size_vert ];
    mesh->verts = (float*)mesh->memory_block;
    
    // generate each vertex
    int pos_v = 0;
	int patch = patch_size / 4;
    for( int j = 0; j < height - 3; j += patch - 1 ) {
        for( int i = 0; i < width - 3; i += patch - 1 ) {
			int max_k = i + patch - 1;
			int max_w = j + patch - 1;
			for (int w = j; w <= max_w; ++w) {
				for (int k = i; k <= max_k; ++k) {
					mesh->verts[pos_v++] = k;
					mesh->verts[pos_v++] = w;
				}
			}
        }
    }
    
    return mesh;
}

// Generate a portion of the grid that lies within an arc. User can specify if the angle is passed in degrees or radians
LODMesh* generateSlice( uint rows, uint patch, Angle type, float lowerAngle, float upperAngle ) {
    
	// Convert to radians if necessary
    if ( type != RADIANS ) {
        convertRadians(lowerAngle);
        convertRadians(upperAngle);
    }
    
	// Determine the total number of patches that will be in the grid
	const uint patch_width = (patch / 4) - 1;
	uint patch_count = 1;
	for (int i = 0; i < rows; i += patch_width) {
		// Find the number of required columns
		int lower = calculateBound(lowerAngle, i);
		int upper = calculateBound(upperAngle, i);
        
		for (auto k = lower; k <= upper; k += patch_width) {
			++patch_count;
		}
	}
    
	// Allocate the memory
	LODMesh* mesh = new LODMesh;
	// Each patch is 16 verts: vert_count = patch_count * patch
	mesh->num_vert = static_cast<uint>(patch_count * patch);
	mesh->size_vert = 2 * mesh->num_vert * sizeof(float);
    
	// memory block init
	mesh->memory_block = new char[mesh->size_vert];
	mesh->verts = (float*)mesh->memory_block;
    
    
	// fill in first patch
	uint postion = 0;
	for (int i = 0; i < patch_width + 1; ++i) {
		for (int j = 0; j < patch_width + 1; ++j) {
			mesh->verts[postion++] = j;
			mesh->verts[postion++] = i;
		}
	}
    
	patch_count = 0;
	// fill in the memory block
	for (int i = patch_width; i < rows; i += patch_width) {
		// Find the number of required columns
		int lower = calculateBound(lowerAngle, i);
		int upper = calculateBound(upperAngle, i);
        
		if (lower > upper) continue;
        
		for (auto k = lower; k <= upper; k += patch_width) {
			// y pos
			for (int j = 0; j < patch_width + 1; ++j) {
				// x pos
				for (int w = 0; w < patch_width + 1; ++w) {
					mesh->verts[postion++] = k + w;
					mesh->verts[postion++] = j + i;
				}
			}
		}
	}
    
    return mesh;
}