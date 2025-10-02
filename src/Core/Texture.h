#pragma once

#include <glad/glad.h>

namespace Core
{
    /**
     * @brief RAII-managed OpenGL texture.
     */
    class Texture
    {
    public:
        /**
         * @brief Default constructor - creates an invalid texture.
         */
        Texture() = default;

        /**
         * @brief Constructs texture from ID.
         * @param textureId OpenGL texture ID
         */
        explicit Texture(GLuint textureId);

        /**
         * @brief Destructor - automatically cleans up the texture.
         */
        ~Texture();

        /**
         * @brief Deleted copy constructor - textures can't be copied.
         */
        Texture(const Texture &) = delete;

        /**
         * @brief Deleted copy assignment - textures can't be copied.
         */
        Texture &operator=(const Texture &) = delete;

        /**
         * @brief Move constructor.
         * @param other Source texture
         */
        Texture(Texture &&other) noexcept;

        /**
         * @brief Move assignment.
         * @param other Source texture
         * @return This texture
         */
        Texture &operator=(Texture &&other) noexcept;

        /**
         * @brief Creates a new OpenGL texture.
         * @return True if successful
         */
        bool Create();

        /**
         * @brief Resets to a new texture ID.
         * @param textureId New texture ID
         */
        void Reset(GLuint textureId = 0);

        /**
         * @brief Releases ownership without deleting.
         * @return Texture ID
         */
        GLuint Release();

        /**
         * @brief Returns the texture ID.
         * @return Texture ID
         */
        [[nodiscard]] GLuint Get() const
        {
            return textureId_;
        }

        /**
         * @brief Checks if texture is valid.
         * @return True if valid
         */
        [[nodiscard]] bool IsValid() const
        {
            return textureId_ != 0;
        }

        operator GLuint() const
        {
            return textureId_;
        }

        explicit operator bool() const
        {
            return IsValid();
        }

    private:
        GLuint textureId_ = 0; ///< OpenGL texture ID

        void Cleanup();
    };

} // namespace Core