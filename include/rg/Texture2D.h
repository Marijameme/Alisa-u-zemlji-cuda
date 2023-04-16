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
   unsigned int texture;
public:
    Texture2D(std::string path, GLenum sampling, GLenum filtering){
        glGenTextures(1, &texture);
        //load image
        int width, height, nChannel;
        unsigned char *data = stbi_load(FileSystem::getPath(path).c_str()
                , &width, &height, &nChannel, 0);

        if(data){
            std::cout << "Teksura je uspesno ucitana\n";
            GLenum format;
            if (nChannel == 1)
                format = GL_RED;
            else if (nChannel == 3)
                format = GL_RGB;
            else if (nChannel == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            //wrap
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : sampling);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : sampling);

            //filter
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
        }
        else{
            ASSERT(false, "Failed to load texture!\n");
        }
        stbi_image_free(data);
    }

    void bind() {
        glBindTexture(GL_TEXTURE_2D, texture);
    }
};

#endif //PROJECT_BASE_TEXTURE2D_H
