#ifndef RENDER_H
#define RENDER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "structs.h"
#include "helper.h"

// TEXTURE1 = CURRENT TEXTURE FOR RENDER()
// TEXTURE2 = CURRENT TEXTURE FOR RENDERWEAPON() (DEPRECATED)
// TEXTURE3 = SKYBOX TEXTURE FOR RENDERSKYBOX()
// TEXTURE4 = CURRENT TEXTURE FOR RENDERGBUFFER()
// TEXTURE5 = G_POSITION
// TEXTURE6 = G_NORMAL
// TEXTURE7 = G_ALBEDO
// TEXTURE8 = G_DEPTH
// TEXTURE9 = TEXTURE COLOUR BUFFER 2
// TEXTURE9-16 = FREE (7 FREE TEXTURES)
// TEXTURE17-23 = DYNAMIC SHADOWS
// TEXTURE24-30 = STATIC SHADOWS
// TEXTURE31 = TEXTURE COLOUR BUFFER

float ambientintensity = 0.0f;

GLfloat backgroundColor[4] = {0.2f, 0.3f, 0.3f, 1.0f};

glm::mat4 proj;
extern glm::mat4 view;
glm::mat4 model;

extern unsigned int quadVAO, quadVBO;

unsigned int skyboxVAO;
unsigned int skyboxVBO;

extern unsigned int textureColorbuffer;
extern unsigned int framebuffer;
extern unsigned int rbo;

extern unsigned int gBuffer;
extern unsigned int gPosition, gNormal, gAlbedo, gMaterial;
extern unsigned int grbo;

const unsigned int RENDER_MAX_SHADOWS = 6;

extern unsigned int depthMapFBOs[RENDER_MAX_SHADOWS];
extern unsigned int depthCubeMaps[RENDER_MAX_SHADOWS];
extern unsigned int depthDynamicMapFBOs[RENDER_MAX_SHADOWS];
extern unsigned int depthDynamicCubeMaps[RENDER_MAX_SHADOWS];

extern glm::vec3 cameraPos;

extern int currentShader;

extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;
extern int SHADOW_RESOLUTION;

extern float fov;
extern float yaw;
extern float pitch;

extern player playerInstance;

extern std::vector<glm::vec3> lightPos;

extern std::vector<texture> textureArray;

extern bool shadowsEnabled;
extern bool shadowDebug;

extern int currentWidth;
extern int currentHeight;

float near_plane = 1.0f, far_plane = 100.0f;

// Shaders
extern Shader lightShader;
extern Shader regularShader;
extern Shader screenShader;
extern Shader depthShader;
extern Shader skyboxShader;
extern Shader gBufferShader;
extern Shader billboardShader;


// Camera
extern glm::vec3 cameraPos;
extern glm::vec3 cameraTarget;
extern glm::vec3 cameraDirection;

extern glm::vec3 up;
extern glm::vec3 cameraRight;
extern glm::vec3 cameraUp;
extern glm::vec3 cameraFront;

extern std::vector<object> objects;

float tempValue;

int getShadowAmount();

void render()
{
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, gMaterial);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, grbo);

    regularShader.setInt("gPosition", 5);
    regularShader.setInt("gNormal", 6);
    regularShader.setInt("gAlbedo", 7);
    regularShader.setInt("gMaterial", 8);
    regularShader.setInt("gDepth", 9);

    proj = glm::perspective(glm::radians(fov), (float)currentWidth / (float)currentHeight, 0.1f, 1000.0f);

    regularShader.setMatrix4fv("view", 1, GL_FALSE, glm::value_ptr(view));
    regularShader.setMatrix4fv("projection", 1, GL_FALSE, glm::value_ptr(proj));


    for (int i = 0; i < lightArray.size(); i++) // Ah yes, peak, run a for loop every single frame multiple times, truly the pythonic way.
    {
        lightArray[i].pos = objects[lightArray[i].id].transform.pos;
        std::string uniformName = "pointLights[" + std::to_string(i) + "]";
        regularShader.setFloat(uniformName + ".castShadow", lightArray[i].castShadow);
        if (lightArray[i].castShadow == true)
        {
            regularShader.setInt(uniformName + ".shadowID", lightArray[i].shadowID);
            regularShader.setFloat3(uniformName + ".position", lightArray[i].pos.x, lightArray[i].pos.y, lightArray[i].pos.z);
            regularShader.setBool(uniformName + ".enabled", lightArray[i].enabled);
            regularShader.setFloat3(uniformName + ".color", lightArray[i].color[0], lightArray[i].color[1], lightArray[i].color[2]); // For some reason red
            regularShader.setFloat(uniformName + ".strength", lightArray[i].strength);
        }
    }

    regularShader.setBool("shadowsEnabled", shadowsEnabled);

    regularShader.setInt("lightAmount", lightArray.size()); // Me when no access to regular shader :moyai:
    regularShader.setFloat("ambientStrength", ambientintensity);
    regularShader.setFloat3("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);
    regularShader.setInt("skybox", 3);

    for (unsigned int i = 0; i < objects.size(); i++)
    {
        if (objects[i].enabled == false || objects[i].visible == false || objects[i].objectType != REGULAR)
        {
            continue;
        }
        const auto &obj = objects[i];

        // Textures
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureArray[findTextureByName(obj.textureName)].id);

        regularShader.setBool("selected", obj.selected);
        regularShader.setInt("currentTexture", 1);
        regularShader.setFloat("reflectancy", obj.reflectance);
        regularShader.setFloat("tempValue", obj.id / 10.0f);

        glBindVertexArray(objects[i].VAO);
        
        if (obj.objectType == REGULAR)
        {
            glDrawArrays(GL_TRIANGLES, 0, obj.temp_data.size());
        }
    }
}

void renderOverlay()
{
    lightShader.use();
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, grbo);

    lightShader.setInt("gPosition", 5);
    lightShader.setInt("gNormal", 6);
    lightShader.setInt("gAlbedo", 7);
    lightShader.setInt("gDepth", 8);

    proj = glm::perspective(glm::radians(fov), (float)currentWidth / (float)currentHeight, 0.1f, 1000.0f);

    lightShader.use();
    lightShader.setMatrix4fv("view", 1, GL_FALSE, glm::value_ptr(view));

    for (unsigned int i = 0; i < objects.size(); i++)
    {
        if (objects[i].enabled == false)
        {
            continue;
        }
        const auto &obj = objects[i];

        // Textures
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureArray[findTextureByName(obj.textureName)].id);

        // Transformations
        model = glm::mat4(1.0f);
        model = glm::translate(model, obj.transform.pos);
        model = glm::scale(model, glm::vec3(obj.transform.scale));
        glm::vec3 angle = obj.transform.rot;
        model = glm::rotate(model, glm::radians(angle.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(angle.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(angle.z), glm::vec3(0.0f, 0.0f, 1.0f));

        lightShader.use();

        for (int j = 0; j < lightArray.size(); j++) // Ah yes, peak, run a for loop every single frame multiple times, truly the pythonic way.
        {
            
            lightArray[j].color = objects[lightArray[j].id].objectColor;
            if (lightArray[j].id == obj.id)
            {
                lightShader.setFloat3("lightColor", lightArray[j].color[0], lightArray[j].color[1], lightArray[j].color[2]);
                break;
            }
        }

        lightShader.setMatrix4fv("model", 1, GL_FALSE, glm::value_ptr(model));
        lightShader.setMatrix4fv("view", 1, GL_FALSE, glm::value_ptr(view));
        lightShader.setMatrix4fv("projection", 1, GL_FALSE, glm::value_ptr(proj));

        lightShader.setInt("currentTexture", 1);

        glBindVertexArray(objects[i].VAO);
        
        if (obj.objectType != REGULAR && obj.visible == true)
        {
            glDrawArrays(GL_TRIANGLES, 0, obj.temp_data.size());
        }
        glDisable(GL_DEPTH_TEST);
        if (obj.iconTextureName != "NULL")
        {
            billboardShader.use();

            model = glm::mat4(1.0f);
            model = glm::translate(model, obj.transform.pos);

            glm::vec3 cameraRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
            glm::vec3 cameraUp = glm::vec3(view[0][1], view[1][1], view[2][1]);
            glm::vec3 cameraForward = glm::vec3(view[0][2], view[1][2], view[2][2]);

            glm::mat4 rotation = glm::mat4(1.0f);
            rotation[0] = glm::vec4(cameraRight, 0.0f);
            rotation[1] = glm::vec4(cameraUp, 0.0f);
            rotation[2] = glm::vec4(-cameraFront, 0.0f);
            model *= rotation;

            billboardShader.setMatrix4fv("model", 1, GL_FALSE, glm::value_ptr(model));
            billboardShader.setMatrix4fv("view", 1, GL_FALSE, glm::value_ptr(view));
            billboardShader.setMatrix4fv("projection", 1, GL_FALSE, glm::value_ptr(proj));

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureArray[findTextureByName(obj.iconTextureName)].id);

            billboardShader.setInt("currentTexture", 2);

            if (obj.objectType == LIGHT)
            {
                billboardShader.setFloat3("colour", obj.objectColor[0], obj.objectColor[1], obj.objectColor[2]);
            }
            else
            {
                billboardShader.setFloat3("colour", 1.0f, 1.0f, 1.0f);
            }

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
        glEnable(GL_DEPTH_TEST);
        
    }

    // Needs to be here for some reason or light vertex shader will stop working?????
    // Edit: This was so stupid, how did I not know that I needed to set the model, view and proj??? Also, how does this even work, it only runs once per frame????


    

}

void renderGBuffer()
{

    proj = glm::perspective(glm::radians(fov), (float)currentWidth / (float)currentHeight, 0.1f, 1000.0f);

    gBufferShader.use();
    gBufferShader.setMatrix4fv("view", 1, GL_FALSE, glm::value_ptr(view));
    gBufferShader.setMatrix4fv("projection", 1, GL_FALSE, glm::value_ptr(proj));

    for (unsigned int i = 0; i < objects.size(); i++)
    {
        if (objects[i].enabled == false || objects[i].visible == false)
        {
            continue;
        }
        const auto &obj = objects[i];

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, textureArray[findTextureByName(obj.textureName)].id);

        gBufferShader.setInt("albedoTexture", 4);

        gBufferShader.setFloat3("objectColor", obj.objectColor[0], obj.objectColor[1], obj.objectColor[2]);
        gBufferShader.setFloat("reflectancy", obj.reflectance);

        glBindVertexArray(objects[i].VAO);


        // Transformations
        model = glm::mat4(1.0f);
        model = glm::translate(model, obj.transform.pos);
        model = glm::scale(model, glm::vec3(obj.transform.scale));
        glm::vec3 angle = obj.transform.rot;
        model = glm::rotate(model, glm::radians(angle.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(angle.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(angle.z), glm::vec3(0.0f, 0.0f, 1.0f));

        gBufferShader.setMatrix4fv("model", 1, GL_FALSE, glm::value_ptr(model));

        if (obj.objectType == REGULAR)
        {
            glDrawArrays(GL_TRIANGLES, 0, obj.temp_data.size());
        }
    }
}

void renderDepth(int currentMap, bool dynamic = false)
{
    // First pass
    if (dynamic == true)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, depthDynamicMapFBOs[currentMap]);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBOs[currentMap]);
    }
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers

    proj = glm::perspective(glm::radians(fov), (float)SHADOW_RESOLUTION / (float)SHADOW_RESOLUTION, 0.1f, 1000.0f);

    

    depthShader.use();

    for (unsigned int i = 0; i < objects.size(); i++)
    {
        if (objects[i].enabled == false || objects[i].visible == false)
        {
            continue;
        }
        
        
        if (dynamic == true)
        {
            if (objects[i].dynamic == false)
            {
                continue;
            }
        }
        else
        {
            if (objects[i].dynamic == true)
            {
                continue;
            }
        }
        
        
        
        const auto &obj = objects[i];

        // Transformations
        model = glm::mat4(1.0f);
        model = glm::translate(model, obj.transform.pos);
        model = glm::scale(model, glm::vec3(obj.transform.scale));
        glm::vec3 angle = obj.transform.rot;
        model = glm::rotate(model, glm::radians(angle.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(angle.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(angle.z), glm::vec3(0.0f, 0.0f, 1.0f));

        depthShader.setMatrix4fv("model", 1, GL_FALSE, glm::value_ptr(model));
        
        glBindVertexArray(objects[i].VAO);

        glDrawArrays(GL_TRIANGLES, 0, obj.temp_data.size());


    }
}

void updateStaticShadows()
{
    
    regularShader.use();

    regularShader.setFloat("far_plane", far_plane);

    glEnable(GL_CULL_FACE);
    
    for (int i = 0; i < getShadowAmount(); i++) // 5 is RENDER_MAX_SHADOWS
    {
        // Rendering to depth map
        
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_RESOLUTION/(float)SHADOW_RESOLUTION, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(lightPos[i], lightPos[i] + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
        shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(lightPos[i], lightPos[i] + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
        shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(lightPos[i], lightPos[i] + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
        shadowTransforms.push_back(shadowProj *
                    glm::lookAt(lightPos[i], lightPos[i] + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)));
        shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(lightPos[i], lightPos[i] + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)));
        shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(lightPos[i], lightPos[i] + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)));

        // Rendering to depth cubemap
        glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBOs[i]);
        glClear(GL_DEPTH_BUFFER_BIT);
        depthShader.use();
        for (int j = 0; j < 6; j++)
        {
            depthShader.setMatrix4fv("shadowMatrices[" + std::to_string(i) + "]" + "[" + std::to_string(j) + "]", 1, GL_FALSE, glm::value_ptr(shadowTransforms[j]));
        }
        depthShader.setFloat("far_plane", far_plane);
        depthShader.setInt("shadow", i);
        depthShader.setFloat3("lightPos["+ std::to_string(i) + "]", lightPos[i].x, lightPos[i].y, lightPos[i].z);
        depthShader.setBool("dynamic", false);
        regularShader.use();
        regularShader.setFloat3("lightPos["+ std::to_string(i) + "]", lightPos[i].x, lightPos[i].y, lightPos[i].z);
        regularShader.setInt("shadowMap[" + std::to_string(i) + "]", 30 - i);
        glActiveTexture(GL_TEXTURE30-i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMaps[i]);

        renderDepth(i, false);
    }

    glDisable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, currentWidth, currentHeight);
}


void updateDynamicShadows()
{
    glEnable(GL_CULL_FACE);
    regularShader.use();

    regularShader.setFloat("far_plane", far_plane);

    glEnable(GL_CULL_FACE);
    
    for (int i = 0; i < getShadowAmount(); i++) // 5 is RENDER_MAX_SHADOWS
    {

        // Rendering to depth map
        
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_RESOLUTION/(float)SHADOW_RESOLUTION, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(lightPos[i], lightPos[i] + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
        shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(lightPos[i], lightPos[i] + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
        shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(lightPos[i], lightPos[i] + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
        shadowTransforms.push_back(shadowProj *
                    glm::lookAt(lightPos[i], lightPos[i] + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)));
        shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(lightPos[i], lightPos[i] + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)));
        shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(lightPos[i], lightPos[i] + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)));

        // Rendering to depth cubemap
        glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
        glBindFramebuffer(GL_FRAMEBUFFER, depthDynamicMapFBOs[i]);
        glClear(GL_DEPTH_BUFFER_BIT);
        depthShader.use();
        for (int j = 0; j < 6; j++)
        {
            depthShader.setMatrix4fv("shadowMatrices[" + std::to_string(i) + "]" + "[" + std::to_string(j) + "]", 1, GL_FALSE, glm::value_ptr(shadowTransforms[j]));
        }
        depthShader.setFloat("far_plane", far_plane);
        depthShader.setInt("shadow", i);
        depthShader.setFloat3("lightPos["+ std::to_string(i) + "]", lightPos[i].x, lightPos[i].y, lightPos[i].z);
        regularShader.use();
        regularShader.setFloat3("lightPos["+ std::to_string(i) + "]", lightPos[i].x, lightPos[i].y, lightPos[i].z);
        regularShader.setInt("dynamicShadowMap[" + std::to_string(i) + "]", 30 - (RENDER_MAX_SHADOWS+1) - i);
        glActiveTexture(GL_TEXTURE30-(RENDER_MAX_SHADOWS+1)-i); // - RENDER_MAX_SHADOWS to not intrude on the static shadows, i for the shadow id itself and finally - 1 because 30-5-0 is 25 and 30-5 is 25 so they intrude
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthDynamicCubeMaps[i]);

        renderDepth(i, true);

    }

    glDisable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, currentWidth, currentHeight);

}

void createSkybox()
{
    std::vector<float> skyboxVertices = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);

    glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(float), skyboxVertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
}

void renderSkybox(unsigned int cubeMapTexture)
{
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    skyboxShader.use();
    skyboxShader.setMatrix4fv("view", 1, GL_FALSE, glm::value_ptr(glm::mat4(glm::mat3(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp))))); // Oh the misery
    skyboxShader.setMatrix4fv("projection", 1, GL_FALSE, glm::value_ptr(proj));

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
}

int getShadowAmount()
{
    int counter = 0;
    for (int i = 0; i < lightArray.size(); i++)
    {
        if (lightArray[i].enabled == true && lightArray[i].castShadow == true)
        {
            counter++;
        }
    }
    if (counter >= RENDER_MAX_SHADOWS)
    {
        counter = RENDER_MAX_SHADOWS - 1;
    }

    return counter;
}

#endif // RENDER_H