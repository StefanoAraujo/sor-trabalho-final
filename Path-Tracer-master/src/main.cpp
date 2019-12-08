/*
*  Simple monte carlo path tracer in C++
*
*  Compilation:
*      $ mkdir build
*      $ cd build
*      $ cmake ..
*      $ make
*
*  Usage:
*      $ ./pathtracer <number of samples>
*/

#include <stdio.h>
#include <stdlib.h>
#include "time.h"
#include <vector>
#include <iostream>

#include "vector.h"
#include "material.h"
#include "objects.h"
#include "camera.h"
#include "scene.h"
#include <pthread.h>
#include "../lib/lodepng/lodepng.h"


#define N_THREADS 8

int samples = 128; // Default samples per pixel

typedef struct {
  Scene *m_scene;
  Camera *m_camera;
  Vec *m_pixel_buffer;
} renderer;

renderer teste; //objeto renderer

// Clamp double to min/max of 0/1
inline double clamp(double x){ return x<0 ? 0 : x>1 ? 1 : x; }
// Clamp to between 0-255
inline int toInt(double x){ return int(clamp(x)*255+.5); }

void Rend(Scene *scene, Camera *camera) {
    teste.m_scene = scene;
    teste.m_camera = camera;
    teste.m_pixel_buffer = new Vec[teste.m_camera->get_width()*teste.m_camera->get_height()];
}
void *render(void *tid) {
    int width = teste.m_camera->get_width();
    int height = teste.m_camera->get_height();

    int t = *((int*)(&tid));

    int blocksize = height/N_THREADS;

    double samples_recp = 1./samples;

    // Main Loop
    for (int y=(t)*blocksize; y<(t+1)*blocksize; y++){
        unsigned short Xi[3]={0,0,y*y*y};               // Stores seed for erand48

        fprintf(stderr, "\rThread %i -> Rendering (%i samples and %i threads): %.2f%%",      // Prints
                        t, samples, N_THREADS, (double)(y-t*blocksize)/(blocksize)*100);                   // progress

        for (int x=0; x<width; x++){
            Vec col = Vec();

            for (int a=0; a<samples; a++){
                Ray ray = teste.m_camera->get_ray(x, y, a>0, Xi);
                col = col + teste.m_scene->trace_ray(ray,0,Xi);
            }

            teste.m_pixel_buffer[(y)*width + x] = col * samples_recp;
        }
    }

    pthread_exit(NULL);

}


void save_image(const char *file_path) {
    int width = teste.m_camera->get_width();
    int height = teste.m_camera->get_height();

    std::vector<unsigned char> pixel_buffer;

    int pixel_count = width*height;

    for (int i=0; i<pixel_count; i++) {
        pixel_buffer.push_back(toInt(teste.m_pixel_buffer[i].x));
        pixel_buffer.push_back(toInt(teste.m_pixel_buffer[i].y));
        pixel_buffer.push_back(toInt(teste.m_pixel_buffer[i].z));
        pixel_buffer.push_back(255);
    }

    //Encode the image
    unsigned error = lodepng::encode(file_path, pixel_buffer, width, height);
    //if there's an error, display it
    if(error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;

    pixel_buffer.clear();
}


int main(int argc, char *argv[]) {

    time_t start, stop;
    time(&start);               // Start execution timer

    Camera camera = Camera(Vec(0, -5, 2.5), Vec(0,0,1), 4096, 2160);     // Create camera
    Scene scene = Scene();                                              // Create scene

    // Add objects to scene
    scene.add( dynamic_cast<Object*>(new Sphere(Vec(0,0,-1000), 1000, Material())) );
    scene.add( dynamic_cast<Object*>(new Sphere(Vec(-1004,0,0), 1000, Material(DIFF, Vec(0.85,0.4,0.4)))) );
    scene.add( dynamic_cast<Object*>(new Sphere(Vec(1004,0,0), 1000, Material(DIFF, Vec(0.4,0.4,0.85)))) );
    scene.add( dynamic_cast<Object*>(new Sphere(Vec(0,1006,0), 1000, Material())) );
    scene.add( dynamic_cast<Object*>(new Sphere(Vec(0,0,110), 100, Material(EMIT, Vec(1,1,1), Vec(2.2,2.2,2.2)))) );
    scene.add( dynamic_cast<Object*>(new Mesh(Vec(), "../obj/dragon2.obj", Material(DIFF, Vec(0.9, 0.9, 0.9)))) );


    Rend(&scene, &camera);  // Create renderer with our scene and camera
    int status;
    pthread_t threads[N_THREADS];

    //int i = *((int *) i);

    ///////////////////////////////////////////////////////////
    for(int i = 0 ; i < N_THREADS ; i ++)
  	{
  		status = pthread_create(&threads[i], NULL, render, (void*)i);
  		if(status != 0)
  		{
  			printf("Cannot create thread\n");
  			exit(1);
  		}
  	}

  	for(int i = 0 ; i < N_THREADS ; i ++)
  	{
  		pthread_join(threads[i],NULL);
  	}


    //renderer.render(samples);                       // Render image to pixel buffer

    ////////////////////////////////////////////////////////////
    save_image("render.png");              // Save image

    // Print duration information
    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff/3600;
    int mins = ((int)diff/60)-(hrs*60);
    int secs = (int)diff-(hrs*3600)-(mins*60);
    printf("\nRendering (%i samples and %i threads): Complete!\nTime Taken: %i hrs, %i mins, %i secs\n\n", samples, N_THREADS, hrs, mins, secs);
    return 0;
}
