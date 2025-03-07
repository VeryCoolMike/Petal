#ifndef STRUCTS_H
#define STRUCTS_H

// Oh my flibbity

#include <string.h>
#include <stdio.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <sstream>

#include <vector>

struct player // Add other things later such as health and more
{
    int weaponID = 0;
};

struct gui
{
    int id; // The same as the object its representing
    bool visible;
};

extern std::vector<gui> guisVisible;

struct vertices
{
    std::vector<glm::vec3> position;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normal;
    std::string id;
    std::string name;
};

struct texture
{
    GLuint id;
    std::string path;
    std::string name;
};

std::vector<texture> textureArray;

enum objectTypes
{
    REGULAR, // A regular object such as a wall or a floor
    STATIC,  // A static object that is not destroyed/replaced upon load
    LIGHT,    // A light
    CAMERA, // A camera object
};

struct light // This truly is an ECS
{
    glm::vec3 pos;
    glm::vec3 color;
    int id; // The same as the object its representing
    bool enabled = true;
    bool selected = false;
    float strength = 1.0f;

    // Shadows
    bool castShadow = true;
    int shadowID;
};



struct transform
{
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
};

std::vector<light> lightArray;

class object; // Stupid compiler can't work this out on its own
extern std::vector<object> objects;

extern int currentIDNumber;
extern int currentLightID;

class object
{
public:
    int id;
    std::string name;
    vertices vertexData;
    transform transform;
    enum objectTypes objectType;
    glm::vec3 objectColor = glm::vec3(1.0f, 1.0f, 1.0f);
    unsigned int shader;
    std::string textureName;
    float reflectance = 0.0f;
    bool enabled = true; // Not very efficient way of cleaning things up but we won't be deleting too many objects dynamically
    bool visible = true; // Make the object completely visible or invisible (rendered / not rendered)
    bool selected = false;
    bool canCollide = true;
    bool dynamic = false;
    std::vector<float> temp_data; // Used for OpenGL to correctly parse my custom way of saving objects
    unsigned int VAO;
    unsigned int VBO;
    int parent; // Can't make it object because of things like the workspace

    std::string iconTextureName;

    object(std::string nameGiven, vertices vertexDataGiven, enum objectTypes objectTypeGiven)
    {
        id = currentIDNumber;
        name = nameGiven;
        vertexData = vertexDataGiven;
        transform.pos = glm::vec3(0.0f, 0.0f, 0.0f);
        transform.rot = glm::vec3(0.0f, 0.0f, 0.0f);
        transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
        objectType = objectTypeGiven;
        textureName = "placeholder";
        iconTextureName = "NULL";

        unsigned int tempVAO, tempVBO;
        glGenVertexArrays(1, &tempVAO);
        glGenBuffers(1, &tempVBO);

        glBindVertexArray(tempVAO);
        glBindBuffer(GL_ARRAY_BUFFER, tempVBO);

        VAO = tempVAO;
        VBO = tempVBO;


        std::vector<float> tempTempData = convertGLMToOpenGLFLoat(vertexDataGiven); // Seems correct

        temp_data = tempTempData;

        glBufferData(GL_ARRAY_BUFFER, temp_data.size() * sizeof(float), temp_data.data(), GL_STATIC_DRAW);
        int vertexsize = 8; // I hate doing this manually

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexsize * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexsize * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vertexsize * sizeof(float), (void *)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Custom types
        if (objectType == LIGHT)
        {
            iconTextureName = "light";
            visible = false;

            struct light newLight;
            newLight.pos = transform.pos;
            newLight.id = currentIDNumber;
            newLight.color = glm::vec3(1.0f, 1.0f, 1.0f);

            lightArray.push_back(newLight);
            currentLightID += 1;
        }
        else if (objectType == CAMERA)
        {
            visible = false;
            iconTextureName = "light";
        }

        for (int i = 0; i < guisVisible.size(); i++)
        {
            guisVisible[i].visible = false;
        }

        gui newGui;
        newGui.id = currentIDNumber;
        newGui.visible = false;
        guisVisible.push_back(newGui);

        currentIDNumber++;

        objects.push_back(*this);
    }

    void remove()
    {
        enabled = false;
        for (int i = 0; i < lightArray.size(); i++)
        {
            if (lightArray[i].id == id)
            {
                lightArray[i].enabled = false;
            }
        }
    }

    void updateVertices(vertices new_vertices)
    {
        std::vector<float> tempTempData = convertGLMToOpenGLFLoat(new_vertices);

        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, tempTempData.size() * sizeof(float), tempTempData.data(), GL_STATIC_DRAW);
    }

    std::vector<float> convertGLMToOpenGLFLoat(vertices vertexDataGiven)
    {
        std::vector<float> tempTempData;

        for (int v = 0; v < vertexDataGiven.position.size(); v++)
        {
            // Positions
            tempTempData.push_back(vertexDataGiven.position[v].x);
            tempTempData.push_back(vertexDataGiven.position[v].y);
            tempTempData.push_back(vertexDataGiven.position[v].z);

            // Texture Coordinates
            tempTempData.push_back(vertexDataGiven.texCoords[v][0]);
            tempTempData.push_back(vertexDataGiven.texCoords[v][1]);

            // Normals
            tempTempData.push_back(vertexDataGiven.normal[v][0]);
            tempTempData.push_back(vertexDataGiven.normal[v][1]);
            tempTempData.push_back(vertexDataGiven.normal[v][2]);
        }

        return tempTempData;
    }
};

/*
struct LuaObject
{
    int id; // The same as the object its representing
    object *obj;
};
*/

struct weapon // Later add ammo, and other customizations
{
    vertices vertices;
    std::vector<float> temp_data;
    unsigned int texture = 1;
    struct
    {
        glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    } transform;

    struct
    {
        glm::vec3 pos = glm::vec3(1.0f, -1.5f, 0.25f);
        glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f);
    } offset;

    glm::vec3 objectColor = glm::vec3(1.0f, 1.0f, 1.0f);
    std::string name = "Placeholder";
    unsigned int VAO;
    unsigned int VBO;

    // Stats

    // Shotgun
    bool shotgun = false;
    int pellets = 10;
    float spread = 7.5f; // Spread (radius of circle, not diameter)

    // Grenade launcher
    bool grenadeLauncher = false;
    float velocityStrength = 3.0f;

    // General
    bool explosive = false;
};

std::vector<weapon> weapons;

#endif // STRUCTS_H