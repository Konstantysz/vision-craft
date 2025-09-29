#pragma once

#include <glad/glad.h>

namespace Core
{
    /**
     * @brief RAII-managed OpenGL texture.
     *
     * Provides automatic resource management for OpenGL textures, ensuring
     * proper cleanup even in the presence of exceptions. Follows RAII principles
     * with move semantics for efficient resource transfer.
     */
    class Texture
    {
    public:
        /**
         * @brief Default constructor - creates an invalid texture.
         */
        Texture() = default;

        /**
         * @brief Constructor that takes ownership of an existing texture.
         * @param textureId OpenGL texture ID to manage (0 for invalid texture)
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
         * @brief Move constructor - transfers ownership.
         * @param other Source wrapper to move from
         */
        Texture(Texture &&other) noexcept;

        /**
         * @brief Move assignment - transfers ownership.
         * @param other Source wrapper to move from
         * @return Reference to this wrapper
         */
        Texture &operator=(Texture &&other) noexcept;

        /**
         * @brief Creates a new OpenGL texture and takes ownership.
         * @return True if texture creation succeeded, false otherwise
         */
        bool Create();

        /**
         * @brief Releases the current texture and takes ownership of a new one.
         * @param textureId New texture ID to manage
         */
        void Reset(GLuint textureId = 0);

        /**
         * @brief Releases ownership of the texture without deleting it.
         * @return The texture ID that was being managed
         */
        GLuint Release();

        /**
         * @brief Gets the OpenGL texture ID.
         * @return Texture ID (0 if invalid)
         */
        [[nodiscard]] GLuint Get() const
        {
            return textureId_;
        }

        /**
         * @brief Checks if the wrapper holds a valid texture.
         * @return True if texture is valid (non-zero), false otherwise
         */
        [[nodiscard]] bool IsValid() const
        {
            return textureId_ != 0;
        }

        /**
         * @brief Implicit conversion to GLuint for OpenGL calls.
         * @return Texture ID
         */
        operator GLuint() const
        {
            return textureId_;
        }

        /**
         * @brief Boolean conversion for validity checking.
         * @return True if texture is valid, false otherwise
         */
        explicit operator bool() const
        {
            return IsValid();
        }

    private:
        GLuint textureId_ = 0; ///< OpenGL texture ID (0 = invalid)

        /**
         * @brief Internal cleanup helper.
         */
        void Cleanup();
    };

} // namespace Core