//
// Created by matf-rg on 30.10.20..
//

#ifndef PROJECT_BASE_TEXTURE2D_H
#define PROJECT_BASE_TEXTURE2D_H
#include <glad/glad.h>
#include <stb_image.h>
#include <rg/Error.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <learnopengl/filesystem.h>

#include <iostream>

class Texture2D{
    unsigned int m_id;
    unsigned int texture;
public:
    Texture2D(std::string path, GLenum sampling, GLenum filtering){
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        //wrap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        //filter
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //load image
        int width, height, nChannel;
        unsigned char *data = stbi_load(FileSystem::getPath("resources/textures/grass_texture.jpg").c_str()
                , &width, &height, &nChannel, 0);
        if(data){
            std::cout << "Teksura je uspesno ucitana\n";
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else{
            ASSERT(false, "Failed to load grass texture!\n");
        }
        stbi_image_free(data);
    }

    void bind() {
        glBindTexture(GL_TEXTURE_2D, texture);
    }
};

#endif //PROJECT_BASE_TEXTURE2D_H
