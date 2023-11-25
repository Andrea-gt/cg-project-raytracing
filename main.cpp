#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <cstdlib>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>
#include <string>
#include <glm/glm.hpp>
#include <vector>
#include "color.h"
#include "intersect.h"
#include "object.h"
#include "light.h"
#include "camera.h"
#include "cube.h"
#include "imageloader.h"
#include "skybox.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float ASPECT_RATIO = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
const int MAX_RECURSION = 3;
const float BIAS = 0.0001f;
Skybox skybox("../BG/skybox01.jpg");

SDL_Renderer* renderer;
std::vector<Object*> objects;
Light light{glm::vec3(-1.0, 0.0, 0.0), 1.5f, Color(255, 255, 255)};
Camera camera(glm::vec3(0.0, 0.0, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 10.0f);


void point(glm::vec2 position, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(renderer, position.x, position.y);
}

float castShadow(const glm::vec3& shadowOrigin, const glm::vec3& lightDir, Object* hitObject) {
    for (auto& obj : objects) {
        if (obj != hitObject) {
            Intersect shadowIntersect = obj->rayIntersect(shadowOrigin, lightDir);
            if (shadowIntersect.isIntersecting && shadowIntersect.dist > 0) {
                float shadowRatio = shadowIntersect.dist / glm::length(light.position - shadowOrigin);
                shadowRatio = glm::min(1.0f, shadowRatio);
                return 1.0f - shadowRatio;
            }
        }
    }
    return 1.0f;
}

Color castRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const short recursion = 0) {
    float zBuffer = 99999;
    Object* hitObject = nullptr;
    Intersect intersect;

    for (const auto& object : objects) {
        Intersect i = object->rayIntersect(rayOrigin, rayDirection);
        if (i.isIntersecting && i.dist < zBuffer) {
            zBuffer = i.dist;
            hitObject = object;
            intersect = i;
        }
    }

    if (!intersect.isIntersecting || recursion == MAX_RECURSION) {
        return skybox.getColor(rayDirection);
    }


    glm::vec3 lightDir = glm::normalize(light.position - intersect.point);
    glm::vec3 viewDir = glm::normalize(rayOrigin - intersect.point);
    glm::vec3 reflectDir = glm::reflect(-lightDir, intersect.normal);

    float shadowIntensity = castShadow(intersect.point, lightDir, hitObject);

    float diffuseLightIntensity = std::max(0.0f, glm::dot(intersect.normal, lightDir));
    float specReflection = glm::dot(viewDir, reflectDir);
    
    Material mat = hitObject->material;

    float specLightIntensity = std::pow(std::max(0.0f, glm::dot(viewDir, reflectDir)), mat.specularCoefficient);


    Color reflectedColor(0.0f, 0.0f, 0.0f);
    if (mat.reflectivity > 0) {
        glm::vec3 origin = intersect.point + intersect.normal * BIAS;
        reflectedColor = castRay(origin, reflectDir, recursion + 1); 
    }

    Color refractedColor(0.0f, 0.0f, 0.0f);
    if (mat.transparency > 0) {
        glm::vec3 origin = intersect.point - intersect.normal * BIAS;
        glm::vec3 refractDir = glm::refract(rayDirection, intersect.normal, mat.refractionIndex);
        refractedColor = castRay(origin, refractDir, recursion + 1);
    }


    mat.diffuse = ImageLoader::getPixelColor(hitObject->material.tKey, intersect.textureCoords.x * hitObject->material.tSize,
                                             hitObject->material.tSize - (hitObject->material.tSize * intersect.textureCoords.y)) * 0.6f;
    Color diffuseLight = mat.diffuse * light.intensity * diffuseLightIntensity * mat.albedo * shadowIntensity;
    Color specularLight = light.color * light.intensity * specLightIntensity * mat.specularAlbedo * shadowIntensity;

    Color color = (diffuseLight + specularLight) * (1.0f - mat.reflectivity - mat.transparency) + reflectedColor * mat.reflectivity + refractedColor * mat.transparency;
    return color;
} 

void setUp() {

    Material cherryLeaves = {
            Color(255, 255, 255),   // diffuse
            0.9, // Matte or glossy
            0.1, // Lower values do not reflect light
            10.0f, //
            0.0f, // Reflections
            0.0f, // Transparency
            0,
            128,
            "cherryLeaves"
    };

    Material cherryPlanks = {
            Color(255, 255, 255),   // diffuse
            0.9, // Matte or glossy
            0.1, // Lower values do not reflect light
            10.0f, //
            0.0f, // Reflections
            0.0f, // Transparency
            0,
            128,
            "cherryPlanks"
    };

    Material oakLog = {
            Color(255, 255, 255),   // diffuse
            0.9, // Matte or glossy
            0.1, // Lower values do not reflect light
            10.0f, //
            0.0f, // Reflections
            0.0f, // Transparency
            0,
            128,
            "oakLog"
    };

    Material cherryPlankStair = {
            Color(255, 255, 255),   // diffuse
            0.9, // Matte or glossy
            0.1, // Lower values do not reflect light
            10.0f, //
            0.0f, // Reflections
            0.0f, // Transparency
            0,
            64,
            "cherryPlanks"
    };


    Material cherryDoorT = {
            Color(255, 255, 255),   // diffuse
            0.9, // Matte or glossy
            0.1, // Lower values do not reflect light
            10.0f, //
            0.0f, // Reflections
            0.0f, // Transparency
            0,
            128,
            "cherryDoorT"
    };

    Material cherryDoorB = {
            Color(255, 255, 255),   // diffuse
            0.9, // Matte or glossy
            0.1, // Lower values do not reflect light
            10.0f, //
            0.0f, // Reflections
            0.0f, // Transparency
            0,
            128,
            "cherryDoorB"
    };

    Material acaciaLeaves = {
            Color(255, 255, 255),   // diffuse
            0.9, // Matte or glossy
            0.1, // Lower values do not reflect light
            10.0f, //
            0.0f, // Reflections
            0.0f, // Transparency
            0,
            128,
            "acaciaLeaves"
    };

    Material redStoneLamp = {
            Color(255, 255, 255),   // diffuse
            0.9, // Matte or glossy
            0.1, // Lower values do not reflect light
            10.0f, //
            0.0f, // Reflections
            0.0f, // Transparency
            0,
            128,
            "redStoneLamp"
    };

    Material Basalt = {
            Color(255, 255, 255),   // diffuse
            0.9, // Matte or glossy
            0.1, // Lower values do not reflect light
            10.0f, //
            0.0f, // Reflections
            0.0f, // Transparency
            0,
            128,
            "basalt"
    };

    const int gridWidth = 6;
    const float yOffset = 1.0f;

    // Plank floor & pillars
    for (int i = -gridWidth / 2; i < gridWidth / 2; ++i) {
        for (int j = -gridWidth / 2; j < gridWidth / 2; ++j) {
            float xPos = static_cast<float>(i);
            float zPos = static_cast<float>(j);

            if ((i == -gridWidth / 2 || i == gridWidth / 2 - 1) && (j == -gridWidth / 2 || j == gridWidth / 2 - 1)) {
                objects.push_back(new Cube(glm::vec3(xPos, 0, zPos), 1.0f, oakLog));
                objects.push_back(new Cube(glm::vec3(xPos, yOffset, zPos), 1.0f, oakLog));
                objects.push_back(new Cube(glm::vec3(xPos, yOffset * 2.0f, zPos), 1.0f, oakLog));
                objects.push_back(new Cube(glm::vec3(xPos, yOffset * 3.0f, zPos), 1.0f, oakLog));
            } else {
                objects.push_back(new Cube(glm::vec3(xPos, 0, zPos), 1.0f, cherryPlanks));
            }
        }
    }

    //Doors
    const int doorPosX[] = { 0, -1 };  // X positions for the doors
    const int doorPosZ = 2;  // Z position for the doors

    for (int i = 0; i < 2; ++i) {
        objects.push_back(new Cube(glm::vec3(doorPosX[i], yOffset, doorPosZ), 1.0f, cherryDoorB));
        objects.push_back(new Cube(glm::vec3(doorPosX[i], yOffset * 2, doorPosZ), 1.0f, cherryDoorT));
    }

    // Front planks
    objects.push_back(new Cube(glm::vec3(0, 0, 3.0f), 1.0f, cherryPlanks));
    objects.push_back(new Cube(glm::vec3(-1.0f, 0, 3.0f), 1.0f, cherryPlanks));
    objects.push_back(new Cube(glm::vec3(1.0f, 0, 3.0f), 1.0f, cherryPlanks));
    objects.push_back(new Cube(glm::vec3(-2.0f, 0, 3.0f), 1.0f, cherryPlanks));

    // Stairs
    objects.push_back(new Cube(glm::vec3(-0.20, 0, 4.0f), 0.7f, cherryPlankStair));
    objects.push_back(new Cube(glm::vec3(-0.8f, 0, 4.0f), 0.7f, cherryPlankStair));

    // Front leaves
    objects.push_back(new Cube(glm::vec3(1.0f, yOffset, 3.0f), 1.0f, acaciaLeaves));
    objects.push_back(new Cube(glm::vec3(-2.0f, yOffset, 3.0f), 1.0f, acaciaLeaves));

    objects.push_back(new Cube(glm::vec3(1.0f, yOffset * 2.0f, 3.0f), 1.0f, acaciaLeaves));
    objects.push_back(new Cube(glm::vec3(-2.0f, yOffset * 2.0f, 3.0f), 1.0f, acaciaLeaves));

    objects.push_back(new Cube(glm::vec3(1.0f, 0, 4.0f), 1.0f, acaciaLeaves));
    objects.push_back(new Cube(glm::vec3(-2.0f, 0, 4.0f), 1.0f, acaciaLeaves));

    objects.push_back(new Cube(glm::vec3(2.0f, 0, 3.0f), 1.0f, acaciaLeaves));
    objects.push_back(new Cube(glm::vec3(-3.0f, 0, 3.0f), 1.0f, acaciaLeaves));

    for (int z = 2; z >= -3; --z) {
        objects.push_back(new Cube(glm::vec3(3.0f, 0, static_cast<float>(z)), 1.0f, acaciaLeaves));
        objects.push_back(new Cube(glm::vec3(-4.0f, 0, static_cast<float>(z)), 1.0f, acaciaLeaves));
    }

    // Redstone lamps
    objects.push_back(new Cube(glm::vec3(2.0f, yOffset * 2.0f, 3.0f), 0.5f, redStoneLamp));
    objects.push_back(new Cube(glm::vec3(-3.0f, yOffset * 2.0f, 3.0f), 0.5f, redStoneLamp));

    // Top planks
    objects.push_back(new Cube(glm::vec3(0, yOffset * 3, 2.0f), 1.0f, cherryPlanks));
    objects.push_back(new Cube(glm::vec3(-1, yOffset * 3, 2.0f), 1.0f, cherryPlanks));

    objects.push_back(new Cube(glm::vec3(0, yOffset * 4, 2.0f), 1.0f, cherryPlanks));
    objects.push_back(new Cube(glm::vec3(-1, yOffset * 4, 2.0f), 1.0f, cherryPlanks));

    objects.push_back(new Cube(glm::vec3(1, yOffset * 3, 2.0f), 1.0f, cherryPlanks));
    objects.push_back(new Cube(glm::vec3(-2, yOffset * 3, 2.0f), 1.0f, cherryPlanks));

    // Roof;
    for (int z = 3; z >= -4; --z) {
        objects.push_back(new Cube(glm::vec3(0, yOffset * 5, static_cast<float>(z)), 1.0f, cherryLeaves));
        objects.push_back(new Cube(glm::vec3(-1, yOffset * 5, static_cast<float>(z)), 1.0f, cherryLeaves));
    }
    for (int z = 3; z >= -4; --z) {
        objects.push_back(new Cube(glm::vec3(1, yOffset * 4, static_cast<float>(z)), 1.0f, cherryLeaves));
        objects.push_back(new Cube(glm::vec3(-2, yOffset * 4, static_cast<float>(z)), 1.0f, cherryLeaves));
    }

    for (int z = 1; z >= -2; --z) {
        objects.push_back(new Cube(glm::vec3(2, yOffset * 3, static_cast<float>(z)), 1.0f, cherryLeaves));
        objects.push_back(new Cube(glm::vec3(-3, yOffset * 3, static_cast<float>(z)), 1.0f, cherryLeaves));
    }

    objects.push_back(new Cube(glm::vec3(2, yOffset * 3, 3.0f), 1.0f, cherryLeaves));
    objects.push_back(new Cube(glm::vec3(-3, yOffset * 3, 3.0f), 1.0f, cherryLeaves));

    objects.push_back(new Cube(glm::vec3(2, yOffset * 3, -4.0f), 1.0f, cherryLeaves));
    objects.push_back(new Cube(glm::vec3(-3, yOffset * 3, -4.0f), 1.0f, cherryLeaves));

    // Window walls
    objects.push_back(new Cube(glm::vec3(2, yOffset, 1.0f), 1.0f, cherryPlanks));
    objects.push_back(new Cube(glm::vec3(-3, yOffset, 1.0f), 1.0f, cherryPlanks));

    objects.push_back(new Cube(glm::vec3(2, yOffset*2.0f, 1.0f), 1.0f, cherryPlanks));
    objects.push_back(new Cube(glm::vec3(-3, yOffset*2.0f, 1.0f), 1.0f, cherryPlanks));

    objects.push_back(new Cube(glm::vec3(2, yOffset, -2.0f), 1.0f, cherryPlanks));
    objects.push_back(new Cube(glm::vec3(-3, yOffset, -2.0f), 1.0f, cherryPlanks));

    objects.push_back(new Cube(glm::vec3(2, yOffset*2.0f, -2.0f), 1.0f, cherryPlanks));
    objects.push_back(new Cube(glm::vec3(-3, yOffset*2.0f, -2.0f), 1.0f, cherryPlanks));

    // Path
    for (int z = 3; z <= 5; ++z) {
        for (int x = 0; x < 2; ++x) {
            objects.push_back(new Cube(glm::vec3(x * -1.0f, -yOffset, static_cast<float>(z)), 1.0f, Basalt));
        }
    }

}

void render() {
    float fov = 3.1415/3;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {

            float screenX = (2.0f * (x + 0.5f)) / SCREEN_WIDTH - 1.0f;
            float screenY = -(2.0f * (y + 0.5f)) / SCREEN_HEIGHT + 1.0f;
            screenX *= ASPECT_RATIO;
            screenX *= tan(fov/2.0f);
            screenY *= tan(fov/2.0f);


            glm::vec3 cameraDir = glm::normalize(camera.target - camera.position);

            glm::vec3 cameraX = glm::normalize(glm::cross(cameraDir, camera.up));
            glm::vec3 cameraY = glm::normalize(glm::cross(cameraX, cameraDir));
            glm::vec3 rayDirection = glm::normalize(
                cameraDir + cameraX * screenX + cameraY * screenY
            );

            Color pixelColor = castRay(camera.position, rayDirection);
            /* Color pixelColor = castRay(glm::vec3(0,0,20), glm::normalize(glm::vec3(screenX, screenY, -1.0f))); */

            point(glm::vec2(x, y), pixelColor);
        }
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    ImageLoader::init(); // Imageloader for textures
    ImageLoader::loadImage("cherryLeaves", "../textures/cherry_leaves.png");
    ImageLoader::loadImage("cherryPlanks", "../textures/cherry_planks.png");
    ImageLoader::loadImage("oakLog", "../textures/oak_log_s.png");
    ImageLoader::loadImage("cherryDoorB", "../textures/cherry_door_bottom.png");
    ImageLoader::loadImage("cherryDoorT", "../textures/cherry_door_top.png");
    ImageLoader::loadImage("acaciaLeaves", "../textures/azalea_leaves.png");
    ImageLoader::loadImage("redStoneLamp", "../textures/redstone_lamp.png");
    ImageLoader::loadImage("basalt", "../textures/basalt.png");


    // Create a window
    SDL_Window* window = SDL_CreateWindow("Hello World - FPS: 0", 
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                          SCREEN_WIDTH, SCREEN_HEIGHT, 
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    int frameCount = 0;
    Uint32 startTime = SDL_GetTicks();
    Uint32 currentTime = startTime;
    
    setUp();

    while (running) {
        light.position = camera.position;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                        camera.move(-1.0f);
                        break;
                    case SDLK_DOWN:
                        camera.move(1.0f);
                        break;
                    case SDLK_a:
                        camera.rotate(-1.0f, 0.0f);
                        break;
                    case SDLK_d:
                        camera.rotate(1.0f, 0.0f);
                    case SDLK_w:
                        camera.rotate(1.0f, -1.0f);
                 }
            }


        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render();

        // Present the renderer
        SDL_RenderPresent(renderer);

        frameCount++;

        // Calculate and display FPS
        if (SDL_GetTicks() - currentTime >= 1000) {
            currentTime = SDL_GetTicks();
            std::string title = "Hello World - FPS: " + std::to_string(frameCount);
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
        }
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

