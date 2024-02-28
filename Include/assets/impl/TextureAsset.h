#pragma once
#include "assets/Asset.h"
#include "opengl/TextureComponent.h"
#include "FreeImage/Dist/x64/FreeImage.h"

namespace asset
{

    void GuessTextureFormat(FIBITMAP* texture, graphics::ExternalTextureFormat* dataFormat, graphics::DataType* pixelType)
    {
        FREE_IMAGE_COLOR_TYPE colorType = FreeImage_GetColorType(texture);
        unsigned bpp = FreeImage_GetBPP(texture); // Bits per pixel
        FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(texture);

        switch (colorType) {
        case FIC_MINISBLACK:
        case FIC_MINISWHITE:
            *dataFormat = graphics::ExternalTextureFormat::Red;
            break;
        case FIC_RGB:
            *dataFormat = graphics::ExternalTextureFormat::RGB;
            break;
        case FIC_RGBALPHA:
            *dataFormat = graphics::ExternalTextureFormat::RGBA;
            break;
        case FIC_PALETTE:
        case FIC_CMYK:
        default:
            *dataFormat = graphics::ExternalTextureFormat::RGB;
            break;
        }

        switch (imageType)
        {
        case FIT_BITMAP:
            // Typical for FIT_BITMAP, but check bitsPerPixel for safety
            if (bpp == 8) {
                *pixelType = graphics::DataType::uint8_t;
            }
            else if (bpp == 16) {
                *pixelType = graphics::DataType::uint16_t;
            }
            else if (bpp == 24 || bpp == 32) {
                // 24 & 32 are usually for RGB and RGBA respectively
                *pixelType = graphics::DataType::uint8_t;
            }
            else {
                // Handle other unexpected bitsPerPixel or throw/assert error
            }
            break;
        case FIT_UINT16:
            assert(bpp == 16);
            *pixelType = graphics::DataType::uint16_t;
            break;
        case FIT_INT16:
            assert(bpp == 16);
            *pixelType = graphics::DataType::int16_t;
            break;
        case FIT_UINT32:
            assert(bpp == 32);
            *pixelType = graphics::DataType::uint32_t;
            break;
        case FIT_INT32:
            assert(bpp == 32);
            *pixelType = graphics::DataType::int32_t;
            break;
        case FIT_FLOAT:
            // Assuming 32-bit float. If your context supports 16-bit floats, check for 16.
            assert(bpp == 32);
            *pixelType = graphics::DataType::float32_t;
            break;
        case FIT_DOUBLE:
            // Assuming 64-bit double
            assert(bpp == 64);
            // Determine appropriate action or set pixel tokenType for double
            break;
        case FIT_COMPLEX:
            // Complex types typically contain 2 floats (for real and imaginary parts)
            assert(bpp == 64); // Assuming each part is a 32-bit float
            // Determine appropriate action for complex tokenType
            break;
        case FIT_RGB16:
            assert(bpp == 48); // 16 bits per channel
            *pixelType = graphics::DataType::uint16_t;
            *dataFormat = graphics::ExternalTextureFormat::RGB;
            break;
        case FIT_RGBA16:
            assert(bpp == 64); // 16 bits per channel
            *pixelType = graphics::DataType::uint16_t;
            *dataFormat = graphics::ExternalTextureFormat::RGBA;
            break;
        case FIT_RGBF:
            assert(bpp == 96); // 32 bits (float) per channel
            *pixelType = graphics::DataType::float32_t;
            *dataFormat = graphics::ExternalTextureFormat::RGB;
            break;
        case FIT_RGBAF:
            assert(bpp == 128); // 32 bits (float) per channel
            *dataFormat = graphics::ExternalTextureFormat::RGBA;
            *pixelType = graphics::DataType::float32_t;
            break;
        }
    }
    struct LoadedTexture {
        FIBITMAP* bitmap = nullptr;
        BYTE* pixels = nullptr;
        std::string fullPath;
        graphics::ExternalTextureFormat pixelFormat = graphics::ExternalTextureFormat::RGBA;
        graphics::DataType pixelType = graphics::DataType::uint8_t;
        glm::ivec2 wh = glm::ivec2(0);
        glm::ivec2 tiling = glm::ivec2(1);
        bool seamless = false;
    	int32_t bitsPerPixel = 0;


        LoadedTexture() = default;

        utility::span<void> getPixels()
        {
            return { pixels, static_cast<size_t>(bitsPerPixel * wh.x * wh.y / 8) };
        }
        glm::ivec3 getDim()const
        {
            return {wh.x /tiling.x,wh.y/tiling.y, tiling.x*tiling.y };
        }

        void Load(std::string path) {
            Dispose();
            path::Shortcuts::Apply(path);
            fullPath = path;

            FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(path.c_str(), 0);
            if (fif == FIF_UNKNOWN) {
                fif = FreeImage_GetFIFFromFilename(path.c_str());
            }
            if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
                bitmap = FreeImage_Load(fif, path.c_str());
                if (bitmap) {
                    pixels = FreeImage_GetBits(bitmap);
                    wh.x = FreeImage_GetWidth(bitmap);
                    wh.y = FreeImage_GetHeight(bitmap);
                    bitsPerPixel = FreeImage_GetBPP(bitmap);
                    tiling = glm::ivec2(1,1);
                    seamless = false;
                }
            }
            std::cout << "++\n";
            GuessTextureFormat(bitmap, &pixelFormat, &pixelType);
        }

        void SaveBitmap(std::string path)const
        {
            path::Shortcuts::Apply(path);

            FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(path.c_str());
            if (format == FIF_UNKNOWN) {
                std::cerr << "Unknown or unsupported file format." << std::endl;
                return;
            }

            if (!FreeImage_FIFSupportsWriting(format) || !FreeImage_FIFSupportsExportBPP(format, FreeImage_GetBPP(bitmap))) {
                std::cerr << "Bitmap cannot be saved in the determined format." << std::endl;
                return;
            }

            if (!FreeImage_Save(format, bitmap, path.c_str())) {
                std::cerr << "Failed to save the bitmap." << std::endl;
            }
            else {
                std::cout << "Bitmap saved successfully to " << path << std::endl;
            }
        }


        LoadedTexture AllocateNew(int32_t w, int32_t h)const {
            LoadedTexture newTexture;
            if (bitmap) {
                FREE_IMAGE_TYPE type = FreeImage_GetImageType(bitmap);

                int bpp = FreeImage_GetBPP(bitmap);
                FIBITMAP* newBitmap = FreeImage_AllocateT(type, w, h, bpp);
                std::cout << "++\n";

                if (newBitmap) {
                    newTexture.bitmap = newBitmap;
                    newTexture.pixels = FreeImage_GetBits(newBitmap);
                    newTexture.wh = glm::ivec2(w, h);
                    newTexture.bitsPerPixel = bitsPerPixel;
                    newTexture.tiling = tiling;
                    newTexture.seamless = seamless;

                    newTexture.fullPath = fullPath;
                    newTexture.pixelFormat = pixelFormat;
                    newTexture.pixelType = pixelType;
                }
            }

            return newTexture;
        }



        LoadedTexture resize(int32_t w, int32_t h) const {
            LoadedTexture resizedTexture;
            if (bitmap) {
                FIBITMAP* resizedBitmap = FreeImage_Rescale(bitmap, w, h);
                std::cout << "++\n";

                if (resizedBitmap) {
                    resizedTexture.bitmap = resizedBitmap;
                    resizedTexture.pixels = FreeImage_GetBits(resizedBitmap);

                    resizedTexture.wh = glm::ivec2(w, h);
                    resizedTexture.bitsPerPixel = bitsPerPixel; 
                    resizedTexture.tiling = tiling;
                    resizedTexture.seamless = seamless;

                    resizedTexture.fullPath = fullPath;
                    resizedTexture.pixelFormat = pixelFormat;
                    resizedTexture.pixelType = pixelType;
                }
            }

            return resizedTexture;
        }
       

        LoadedTexture tile(int32_t outTileWidth, glm::ivec2 tile, bool seamless)
        {
            const int32_t inTileWidth = outTileWidth - seamless;
            const glm::ivec2 inTextureSize = inTileWidth * tile;
            const glm::ivec2 outTextureSize = outTileWidth * tile;
            const int32_t bytesPerPixel = bitsPerPixel / 8;
            const int32_t tiles = tile.x * tile.y;

            LoadedTexture local;
            bool need_resize = inTextureSize != wh;
            if(need_resize)
            {
                local = resize(inTextureSize.x, inTextureSize.y);
            }
            LoadedTexture& in = need_resize ? local : *this;
            LoadedTexture out = AllocateNew(outTextureSize.x, outTextureSize.y);
            out.tiling = tile;
            out.seamless = seamless;
        

            for (int32_t i = 0; i < tiles; i++)
            {
                int32_t tile_x = i % tile.x;
                int32_t tile_y = i / tile.x;

                int32_t in_x = tile_x * inTileWidth;

                int32_t in_y = tile_y * inTileWidth;
                int32_t out_xy = i * outTileWidth * outTileWidth;


                int32_t right = (in_x + outTileWidth) >= inTextureSize.x;
                int32_t down = (in_y + outTileWidth) >= inTextureSize.y;
                int32_t cpylength = (outTileWidth - right)*bytesPerPixel;

                for (int32_t y = 0; y < outTileWidth - down; y++)
                {
                    int32_t in_offset = (in_x + (in_y + y) * inTextureSize.x) * bytesPerPixel;
                    int32_t out_offset = (out_xy + y * outTileWidth) * bytesPerPixel;

                    std::memcpy(out.pixels + out_offset, in.pixels + in_offset, cpylength);
                }
            }

            return out;
        }


        LoadedTexture(LoadedTexture&& other) noexcept
            : bitmap(other.bitmap), pixels(other.pixels), bitsPerPixel(other.bitsPerPixel),
            fullPath(std::move(other.fullPath)), pixelFormat(other.pixelFormat), pixelType(other.pixelType), wh(other.wh), tiling(other.tiling), seamless(other.seamless) {
            other.bitmap = nullptr;
            other.pixels = nullptr;
        }

        LoadedTexture& operator=(LoadedTexture&& other) noexcept {
            if (this != &other) {
                Dispose();

                bitmap = other.bitmap;
                pixels = other.pixels;
                bitsPerPixel = other.bitsPerPixel;
                fullPath = std::move(other.fullPath);
                pixelFormat = other.pixelFormat;
                pixelType = other.pixelType;
                wh = other.wh;
                tiling = other.tiling;
				seamless = other.seamless;

                other.bitmap = nullptr;
                other.pixels = nullptr;
            }
            return *this;
        }
        LoadedTexture(const LoadedTexture&) = delete;
        LoadedTexture& operator=(const LoadedTexture&) = delete;

		
        void Dispose()
        {
	        if(bitmap)
	        {
                std::cout<<"--\n";
                FreeImage_Unload(bitmap);
                bitmap = nullptr;
                pixels = nullptr;
	        }
        }


        ~LoadedTexture() {
            Dispose();
        }
       
    };
}
