#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <stdexcept>
#include <map>
#include <string>

#include "color.h"

class ImageLoader {
private:
    static std::map<std::string, SDL_Surface*> imageSurfaces;
public:
    // Initialize SDL_image
    static void init() {
        int imgFlags = IMG_INIT_PNG; // or IMG_INIT_JPG, depending on your needs
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            throw std::runtime_error("SDL_image could not initialize! SDL_image Error: " + std::string(IMG_GetError()));
        }
    }

    // Load an image from a given path and store with a key
    static void loadImage(const std::string& key, const char* path) {
        SDL_Surface* newSurface = IMG_Load(path);
        if (!newSurface) {
            throw std::runtime_error("Unable to load image! SDL_image Error: " + std::string(IMG_GetError()));
        }
        imageSurfaces[key] = newSurface;
    }

    // Get the color of the pixel at (x, y) from an image with a specific key
    static Color getPixelColor(const std::string& key, int x, int y) {
        auto it = imageSurfaces.find(key);
        if (it == imageSurfaces.end()) {
            throw std::runtime_error("Image key not found!");
        }

        SDL_Surface* targetSurface = it->second;
        int bpp = targetSurface->format->BytesPerPixel;
        Uint8 *p = (Uint8 *)targetSurface->pixels + y * targetSurface->pitch + x * bpp;

        Uint32 pixelColor;
        switch (bpp) {
            case 1:
                pixelColor = *p;
                break;
            case 2:
                pixelColor = *(Uint16 *)p;
                break;
            case 3:
                if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                    pixelColor = p[0] << 16 | p[1] << 8 | p[2];
                } else {
                    pixelColor = p[0] | p[1] << 8 | p[2] << 16;
                }
                break;
            case 4:
                pixelColor = *(Uint32 *)p;
                break;
            default:
                throw std::runtime_error("Unknown format!");
        }
        
        SDL_Color color;
        SDL_GetRGBA(pixelColor, targetSurface->format, &color.r, &color.g, &color.b, &color.a);
        return Color{color.r, color.g, color.b, color.a};
    }
};

std::map<std::string, SDL_Surface*> ImageLoader::imageSurfaces;

