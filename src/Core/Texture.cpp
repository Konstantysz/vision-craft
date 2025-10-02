#include "Texture.h"

namespace Core
{
    Texture::Texture(GLuint textureId) : textureId_(textureId)
    {
    }

    Texture::~Texture()
    {
        Cleanup();
    }

    Texture::Texture(Texture &&other) noexcept : textureId_(other.textureId_)
    {
        other.textureId_ = 0;
    }

    Texture &Texture::operator=(Texture &&other) noexcept
    {
        if (this != &other)
        {
            Cleanup();
            textureId_ = other.textureId_;
            other.textureId_ = 0;
        }
        return *this;
    }

    bool Texture::Create()
    {
        Cleanup();
        glGenTextures(1, &textureId_);
        return textureId_ != 0;
    }

    void Texture::Reset(GLuint textureId)
    {
        if (textureId_ != textureId)
        {
            Cleanup();
            textureId_ = textureId;
        }
    }

    GLuint Texture::Release()
    {
        GLuint temp = textureId_;
        textureId_ = 0;
        return temp;
    }

    void Texture::Cleanup()
    {
        if (textureId_ != 0)
        {
            glDeleteTextures(1, &textureId_);
            textureId_ = 0;
        }
    }

} // namespace Core