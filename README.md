# 3D-Rendering

## Overview
Two 3D rendering implementations, one being a basic ray tracing program based off of [Scratchapixel's raytracer](https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-ray-tracing//raytracing-algorithm-in-a-nutshell.html) and the other being an implementation of [TinyRenderer](https://haqr.eu/tinyrenderer/).

## Usage
### Raytracer
Run the executable ray_tracer.out and send the output to a .ppm file (e.g. `./ray_tracer.out > img.ppm`). Alternatively compile ray_tracer.cpp first using clang or g++ and the execute the compiled file. 

The sphere objects can be modified in the main function of ray_tracer.cpp before compiling for different scenes.

### 3D_Renderer
Executables are found under the bin folder of 3D_Renderer. Each one demonstrates various steps along the way of creating the 3D renderer. Each executable creates a .tga image file as output as well as a lightbuffer and depthbuffer .tga image. Example .obj files are in the objects file (demon model is taken from TinyRenderer, the floor and terrible looking puffin head were scrappily made by me).

bresenham.out -> Uses bresenham's line drawing algorithm to rasterize lines. Usage: `./bresenham.out`

wireframe.out -> Uses bresenham's line drawing for wireframe rendering of an .obj file. Usage: `./wireframe.out path/to/.obj`

triangle_rasterization.out -> Rasterizes triangles by drawing lines between left and right sides of triangle. Usage: `./triangle_rasterization.out`

bounding_box_rasterization.out -> Rasterizes triangles by checking every pixel within the triangle's bounding box. Usage: `./bounding_box_rasterization.out`

basic_model_rendering.out -> Renders an .obj file with random colours, using the bounding box rasterization. Usage: `./basic_model_rendering.out path/to/.obj`

matrix_refactor.out -> Same as basic_model_rendering but uses matrices for transformations, such as between worldspace to cameraspace to screenspace. Usage: `./matrix_refactor.out path/to/.obj`

phong.out -> Basic phong shading. Usage `./phong.out path/to/1.obj path.to/2.obj ...`

phong_nm.out -> Phong shading with smoothing using vertex normals, uv mapping for diffuse, glow and specular maps and basic shadows. Usage `./phong_nm.out path/to/1.obj path.to/2.obj ...`

toon_shading.out -> Basic toon shading with sobel edge detection. `./toon_shading.out path/to/1.obj path.to/2.obj ...`

Each binary was compiled using clang under the C++17 standard