//
// Created by Dustin Hollar on 10/15/18.
// Documentation located in the header file
//

//#include <OpenSimplexNoise.h>
OpenSimplexNoise openSimplex;

// --------------------------------------------------------------------------------------------- //
// Find the Biome that exists at the passed height value.
// --------------------------------------------------------------------------------------------- //
int Biome::biome( float height ) {
    
    if      ( height < 0.1f ) return WATER;
    else if ( height < 0.2f ) return BEACH;
    else if ( height < 0.3f ) return FOREST;
    else if ( height < 0.5f ) return JUNGLE;
    else if ( height < 0.7f ) return SAVANNAH;
    else if ( height < 0.9f ) return DESERT;
    else return SNOW;
    //return -1;
    
}

// --------------------------------------------------------------------------------------------- //
// Open Simplex Noise and Erosion Algorithms
// --------------------------------------------------------------------------------------------- //
// Make sure we are not trying to access outside of the array
inline bool
checkBounds( int i, int j, int pos, int width, int height )
{
    switch( pos )
    {
        case 0:  return j + 1 <= width - 1;
        case 1:  return j - 1 >= 0;
        case 2:  return i + 1 <= height - 1;
        case 3:  return i - 1 >= 0;
        default: return false;
    }
}

// Find the minimum of two values
inline float
min( float x, float y )
{
    return ( x < y ) ? x : y;
}

// Find the maximum of two values
inline float
max( float x, float y )
{
    return ( x > y ) ? x : y;
}

// Given an x, y, and z coordinated noise octaves are computed and added together to adapt a noise
// map. If the passed struct has a dimension of TWODIMENSION, then the 2D noise function is called.
// Othersise, call the 3D noise function.
static float octave( SimplexNoise::NoiseOctaveSimulation& octaveInfo, float x, float y, float z ) {
    
    float maxAmp    = 0.0f;  // intital maximum amplitude
    float amp       = 1.0f;  // initial amplitude
    float freq      = 1.0f;  // initial frequency
    float noiseCell = 0.0f;  // initial noiseCell
    //noiseCell += openSimplex.eval(x, y);
    //
    // add successively smaller, higher-frequency terms
    for( int i = 0; i < octaveInfo.numOctaves; ++i ) {
        
        // Determine which dimension to run the algorithm with
        switch( octaveInfo.dim ) {
            case SimplexNoise::NoiseOctaveSimulation::TWODIMENSION:
            {
                noiseCell += openSimplex.eval(x * freq, y * freq) * amp;
            } break;
            case SimplexNoise::NoiseOctaveSimulation::THREEDIMENSION:
            {
                noiseCell += openSimplex.eval(x * freq, y * freq, z * freq) * amp;
            } break;
            default: return 0.0;
        }
        
        maxAmp += amp;
        amp *= octaveInfo.persistence;
        freq *= 2;
    }
    //
    // take the average value of the iterations
    noiseCell /= maxAmp;
    
    // normalize the result
    noiseCell = noiseCell * ( octaveInfo.high - octaveInfo.low) * 0.5f +
        ( octaveInfo.high + octaveInfo.low) * 0.5f;
    return noiseCell;
}

// Runs a noise simulation using the passed struct. The size of the returned float array
// are the dimensions passed in the struct: width : height.
float* SimplexNoise::simulateNoise( NoiseOctaveSimulation& octaveInfo) {
    
    float nx, ny, nz, noise = 0.f;
    
    auto *noisemap = new float[ octaveInfo.width * octaveInfo.height ]{ 0 };
    
    for ( int i = 0; i < octaveInfo.height; i++ ) {
        for (int j = 0; j < octaveInfo.width; j++) {
            //
            // Get the pixel coordinated from 0..1. Offset the value by -0.5
            // (you get problems if you don't do this)
            switch (octaveInfo.dim) {
                // TODO Implement nz (?)
                case SimplexNoise::NoiseOctaveSimulation::TWODIMENSION:
                {
                    nx = (static_cast<float>(i) / (static_cast<float>(octaveInfo.width) / 2)) - 0.5f;
                    ny = (static_cast<float>(j) / (static_cast<float>(octaveInfo.height) / 2)) - 0.5f;
                    // Run octaves over the noise
                    //noisemap[(i * octaveInfo.width) + j] = openSimplex.eval(nx, ny);
                    noise = octave( octaveInfo, nx, ny, 0 );
                } break;
                
                case SimplexNoise::NoiseOctaveSimulation::THREEDIMENSION:
                nx = (static_cast<float>(i) / (static_cast<float>(octaveInfo.width) / 2)) - 0.5f;
                nz = (static_cast<float>(j) / (static_cast<float>(octaveInfo.height) / 2)) - 0.5f;
                // Run octaves over the noise
                noise = octave(octaveInfo, nx, 0, nz);
                default:
                break;
            }
            
            noisemap[(i * octaveInfo.width) + j] = pow( noise, octaveInfo.exp );
            //
        }
    }
    
    return noisemap;
}

// --------------------------------------------------------------------------------------------- //
// Thermal Erosion Simulation
// --------------------------------------------------------------------------------------------- //
// Run a thermal erosion simulation over a heightmap
void Erosion::simulateThermalErosion( ThermalErosionSimlation& thermalSim )
{
    // Exit condition
    if ( thermalSim.numberIterations <= 0 )
        return;
    
    // talos angle : orig 4 / ...
    float T = 4.0f / static_cast<float>(thermalSim.width);
    // amount of material to "move" from the cell in question
    //float c = 0.5f;
    
    // Array for hashing
    int indexHash[] = { 1, -1, thermalSim.width, -thermalSim.width };
    
    for( int i = 0; i < thermalSim.height; ++i )
    {
        for( int j = 0; j < thermalSim.width; ++j )
        {
            
            float dmax = 0.0f;
            //float dtotal = 0.0f;
            
            float h = thermalSim.noiseMap[ i * thermalSim.width + j ];
            float d[] = { 0, 0, 0, 0 };
            // TODO variable not originally here
            int hli = -1;
            //float hl = 1.0f;
            
            // If there are multiple occurrences of of needed to apply erosion, find a proportion
            // based on the surrounding cells
            for( int k = 0; k < 4; ++k )
            {
                // do not check this neighbor if it is not within bounds
                if( !checkBounds( i, j, k, thermalSim.width, thermalSim.height ) )
                { d[ k ] = -1; continue; }
                
                // Calculate the height difference
                d[ k ] = h - thermalSim.noiseMap[ (i * thermalSim.width) + j + indexHash[ k ] ];
                // TODO new implementation
                if ( d[k] > dmax )
                {
                    dmax = d[k];
                    hli = (i * thermalSim.width) + j + indexHash[k];
                }
                // TODO original implementation
                //                if( d[ k ] > T && d[ k ] > 0 )
                //                {
                //                    dtotal += d[ k ];
                //                    if( d[ k ] > dmax ) {
                //                        dmax = d[k];
                //                        // TODO if statement not originally here
                //                        if ( d[k] < hl ) {
                //                            hli = (i * thermalSim.width) + j + indexHash[k];
                //                            hl = d[k];
                //                        }
                //                    }
                //                }
                else
                    d[ k ] = -1.0f;
            }
            
            // Now do it all again, but this time with feeling
            // Calculate the amount of material to move to each cell
            //            for( int k = 0; k < 4; ++k )
            //            {
            //                // If di = -1 then erosion should not happen here
            //                if( d[ k ] == -1 ) continue;
            //                // TODO originally no if statement
            //                if ( dmax > 0 && dmax > T ) {
            //                    thermalSim.noiseMap[(i * thermalSim.width) + j + indexHash[k]] +=
            //                            c * (dmax - T) * (d[k] / dtotal);
            //            }
            
            // TODO added implementation
            if ( hli >= 0 && dmax > 0 && dmax > T ) {
                float dh = 0.5f * dmax;
                thermalSim.noiseMap[ (i * thermalSim.width) + j ] -= dh;
                thermalSim.noiseMap[ hli ] += dh;
            }
        }
    }
    
    // Recursively call thermal erosion until it finishes the number of iterations
    --thermalSim.numberIterations;
    simulateThermalErosion( thermalSim );
}

// --------------------------------------------------------------------------------------------- //
// Inverse Thermal Erosion Simulation
// --------------------------------------------------------------------------------------------- //
// Run a thermal erosion simulation over a heightmap
void Erosion::simulateInverseThermalErosion( ThermalErosionSimlation& thermalSim )
{
    // Exit condition
    if ( thermalSim.numberIterations <= 0 )
        return;
    
    // talos angle: Orig 4 / ..
    float T = 10.0f / static_cast<float>(thermalSim.width);
    // amount of material to "move" from the cell in question
    //float c = 0.5f;
    
    // Array for hashing
    int indexHash[] = { 1, -1, thermalSim.width, -thermalSim.width };
    
    for( int i = 0; i < thermalSim.height; ++i )
    {
        for( int j = 0; j < thermalSim.width; ++j )
        {
            
            float dmax = 0.0f;
            //float height = 0.0f;
            
            float h = thermalSim.noiseMap[ i * thermalSim.width + j ];
            float d[] = { 0, 0, 0, 0 };
            // TODO variable not originally here
            int hli = -1;
            //float hl = 1.0f;
            
            // If there are multiple occurrences of of needed to apply erosion, find a proportion
            // based on the surrounding cells
            for( int k = 0; k < 4; ++k )
            {
                // do not check this neighbor if it is not within bounds
                if( !checkBounds( i, j, k, thermalSim.width, thermalSim.height ) )
                { d[ k ] = -1; continue; }
                
                // Calculate the height difference
                d[ k ] = h - thermalSim.noiseMap[ (i * thermalSim.width) + j + indexHash[ k ] ];
                // TODO new implementation
                if ( d[k] > dmax )
                {
                    dmax = d[k];
                    hli = (i * thermalSim.width) + j + indexHash[k];
                }
                // TODO original implementation
                //                if( d[ k ] > T && d[ k ] > 0 )
                //                {
                //                    dtotal += d[ k ];
                //                    if( d[ k ] > dmax ) {
                //                        dmax = d[k];
                //                        // TODO if statement not originally here
                //                        if ( d[k] < hl ) {
                //                            hli = (i * thermalSim.width) + j + indexHash[k];
                //                            hl = d[k];
                //                        }
                //                    }
                //                }
                else
                    d[ k ] = -1.0f;
            }
            
            // Now do it all again, but this time with feeling
            // Calculate the amount of material to move to each cell
            //            for( int k = 0; k < 4; ++k )
            //            {
            //                // If di = -1 then erosion should not happen here
            //                if( d[ k ] == -1 ) continue;
            //                // TODO originally no if statement
            //                if ( dmax > 0 && dmax > T ) {
            //                    thermalSim.noiseMap[(i * thermalSim.width) + j + indexHash[k]] +=
            //                            c * (dmax - T) * (d[k] / dtotal);
            //            }
            
            // TODO added implementation
            if ( hli >= 0 && dmax > 0 && dmax <= T ) {
                float dh = 0.5f * dmax;
                thermalSim.noiseMap[ (i * thermalSim.width) + j ] -= dh;
                thermalSim.noiseMap[ hli ] += dh;
            }
        }
    }
    
    // Recursively call thermal erosion until it finishes the number of iterations
    --thermalSim.numberIterations;
    simulateInverseThermalErosion( thermalSim );
}

// --------------------------------------------------------------------------------------------- //
// Hydraulic Erosion
// --------------------------------------------------------------------------------------------- //
// Reset the simulation for next use
// coefficients remain persistent across simulation
static void reset( Erosion::HydraulicErosionSimulation &erosionStruct, float* waterMap )
{
    delete[] waterMap;
    
    erosionStruct.noiseMap = nullptr;
}

// Runs a sungle iteration of the hydraulic erosion algorithm
static void runSimulation( Erosion::HydraulicErosionSimulation &erosionStruct,
                          float* waterMap )
{
    int size = erosionStruct.width * erosionStruct.height;
    
    float* sedimentMap = new float[ size ]{ 0 };
    
    int indexHash[] = { 1, -1, erosionStruct.width, -erosionStruct.width };
    
    /*
     * Kr is added to each cell every iteration to simulate rain
     *     w(i,j) = w(i,j) + Kr    <- constant amount of water
     *
     * Amount of the height value proportional to the amount of water present in the same
     * cell is converted to sediment.
     *    h(i,j) = h(i,j) − Ks * w(i,j)    <- Height map, solubility constant of the terrain
     *    m(i,j) = m(i,j) + Ks * w(i,j)    <- Sediment map
     */
    for( int i = 0; i < size; ++i )
    {
        waterMap   [ i ] += erosionStruct.erosionCoeffStruct->Kr;
        erosionStruct.noiseMap   [ i ] -= erosionStruct.erosionCoeffStruct->Ks * waterMap[ i ];
        sedimentMap[ i ] += erosionStruct.erosionCoeffStruct->Ks * waterMap[ i ];
    }
    
    // Set of variables for the set part of calculations. Their explanation is above
    float atot = 0.f, dtotal = 0.f, mmax, dm, wi;
    
    float acc;
    
    // Update the percentage of water evaportated
    
    for( int i = 0; i < erosionStruct.height; ++i )
    {
        for( int k = 0; k < erosionStruct.width; ++k ) {
            
            int acount = 0;
            float d[] = { -1.f, -1.f, -1.f, -1.f };
            float a[] = { -1.f, -1.f, -1.f, -1.f };
            
            // height at the current cell
            float h = erosionStruct.noiseMap[ (i * erosionStruct.width) + k ];
            acc = waterMap[ (i * erosionStruct.width) + k ];
            
            // Get the total height of the neighboring cells, if they are lower than the current cell
            for (int j = 0; j < 4; ++j) {
                
                if (!checkBounds(i, k, j, erosionStruct.width, erosionStruct.height ) ) { continue; }
                
                // Do not run erosion if a neighboring cell is taller than the current one
                if ( erosionStruct.noiseMap[ (i * erosionStruct.width) + k + indexHash[ j ] ] > h) continue;
                
                ++acount;
                a[j] = erosionStruct.noiseMap[ (i * erosionStruct.width) + k + indexHash[ j ] ]
                    + waterMap[ (i * erosionStruct.width) + k + indexHash[ j ] ];
                atot += a[ j ];
                
                d[ j ] = acc - a[ j ];
                if (d[j] > 0)
                    dtotal += d[ j ];
            }
            
            // ∆a = a−a ̄ is the total height of the current cell minus the average total height of the
            // cells involved in the distribution
            // a[j] - (atot/acount)
            
            // move the sediment downhill
            for (int j = 0; j < 4; ++j) {
                
                if (!checkBounds(i, k, j, erosionStruct.width, erosionStruct.height ) ) { continue; }
                
                if (d[j] <= 0 || a[ j ] <= 0 )
                    continue;
                
                wi = min( waterMap[ (i * erosionStruct.width) + k + indexHash[ j ] ],
                         a[j] - (atot/acount) ) * ( d[ j ] / dtotal );
                
                erosionStruct.noiseMap[ (i * erosionStruct.width) + k + indexHash[ j ] ] =
                    sedimentMap[ (i * erosionStruct.width) + k ] *
                    ( wi / waterMap[ (i * erosionStruct.width) + k ] );
            }
            
            // Reset the current cell's height based on the amount of erosion
            waterMap[ (i * erosionStruct.width) + k ] *= ( 1 - erosionStruct.erosionCoeffStruct->Ke );
            mmax = erosionStruct.erosionCoeffStruct->Kc * waterMap[ (i * erosionStruct.width) + k ];
            dm = max( 0.f, sedimentMap[ (i * erosionStruct.width) + k ] - mmax );
            waterMap[ (i * erosionStruct.width) + k ] -= dm;
            erosionStruct.noiseMap[ (i * erosionStruct.width) + k ] += dm;
        }
    }
    
    delete[] sedimentMap;
}

// This is done for encapsulation. Don't worry about it. Starts the hydraulic erosion simulation
void Erosion::simulateHydraulicErosion( HydraulicErosionSimulation &erosionStruct )
{
    // Initialize the variables for the simulation
    float* waterMap = new float[ erosionStruct.width * erosionStruct.height ]{ 0 };
    
    // Run the erosion simulation
    for( int i = 0; i < erosionStruct.numberIterations; ++i )
        runSimulation( erosionStruct, waterMap );
    
    // call reset
    reset( erosionStruct, waterMap );
}

// Allocates memory for a normal map and runs a vonNuemann neighborhood check to calculate Normals
float* generateHeighmapNormals( float* heightmap, unsigned int width, unsigned int height )
{
    // TODO I might have realized why normals are wrong. Currently loading an xyz into the array and then using
    // this map as a 2D texture. Well...when I sample that texture, I am just getting a x,y, OR z (i think).
    // I think I need to take an average of the results for the normal at the corresponding height. ???
    
    auto* normalMap = new float[ width * height * 3 ];
    
    float top, bottom, left, right;
    int pos = 0;
    for( unsigned int i = 0; i < height; ++i )
    {
        for( unsigned int j = 0; j < width; ++j )
        {
            // TODO: need to verify, but for now when at edge, treat it as a height of 0
            // TODO: Generating normals for the y-direction as up, this is because terrain lays flat. Will need to add tangent and bi-tangent maps
            
            // X normal: 0.5 * ( right - left )
            // Check right
            if ( checkBounds(i, j + 1, 0, width, height ) ) {
                right = heightmap[i * width + j + 1];
            }
            else
                right = 0;
            if ( checkBounds(i, j - 1, 1, width, height ) ) {
                left = heightmap[i * width + j - 1];
            }
            else
                left = 0;
            
            normalMap[ pos++ ] = 0.5f * ( right - left );
            
            // Y normal: -1
            normalMap[ pos++ ] = -1.0f;
            
            // Z normal: 0.5 * ( bottom - top )
            if ( checkBounds(i + 1 , j, 2, width, height ) ) {
                bottom = heightmap[i * width + j + width];
            }
            else
                bottom = 0;
            if ( checkBounds(i - 1, j, 3, width, height ) ) {
                top = heightmap[i * width + j - width];
            }
            else
                top = 0;
            
            normalMap[ pos++ ] = 0.5f * ( bottom - top );
        }
    }
    
    return normalMap;
}