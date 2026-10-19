#include <climits>
#include <cstring>
#include <graphics/format/JPEG.h>
#include <graphics/ImageBuffer.h>
#include <graphics/PixelFormat.h>
#include <SDL.h>
#include <SDL_image.h>
#include <system/System.h>
#include <utils/DynamicMemIO.h>
#include <utils/File.h>

namespace lime
{

	bool JPEG::Decode(Resource *resource, ImageBuffer *imageBuffer)
	{
		File file = resource->path ? File(resource->path, "rb") : File(resource->data, false);

		if (!file.handle)
		{
			return false;
		}

		if (!IMG_isJPG(file.handle))
		{
			file.Close();

			return false;
		}

		SDL_Surface *surface = IMG_LoadJPG_RW(file.handle);

		if (!surface)
		{
			file.Close();

			return false;
		}

		if (surface->format->format != SDL_PIXELFORMAT_RGBA32)
		{
			SDL_Surface *old_surface = surface;
			surface = SDL_ConvertSurfaceFormat(old_surface, SDL_PIXELFORMAT_RGBA32, 0);
			SDL_FreeSurface(old_surface);
		}

		if (!surface)
		{
			file.Close();

			return false;
		}

		imageBuffer->Resize(surface->w, surface->h, 32);

		memcpy(imageBuffer->data->buffer->b, surface->pixels, surface->h * surface->pitch);

		SDL_FreeSurface(surface);

		file.Close();

		return true;
	}

	bool JPEG::Encode(ImageBuffer *imageBuffer, Bytes *bytes, int quality)
	{
		SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(imageBuffer->data->buffer->b, imageBuffer->width, imageBuffer->height, 32, imageBuffer->Stride(), SDL_PIXELFORMAT_RGBA32);

		if (!surface)
		{
			return false;
		}

		SDL_Surface *rgb = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGB24, 0);

		SDL_FreeSurface(surface);

		if (!rgb)
		{
			return false;
		}

		SDL_RWops *dst = DynamicMemIO_Create();

		if (!dst)
		{
			SDL_FreeSurface(rgb);

			return false;
		}

		bool success = false;

		if (bytes)
		{
			// Returns 0 on success; freedst = 0 so we close it ourselves
			success = (IMG_SaveJPG_RW(rgb, dst, 0, quality) == 0);

			if (success)
			{
				DynamicMemIO *mem = DynamicMemIO_Get(dst);

				if (mem->size == 0 || mem->size > INT_MAX)
				{
					success = false;
				}
				else
				{
					bytes->Resize((int)mem->size);
					memcpy(bytes->b, mem->data, mem->size);
				}
			}
		}

		SDL_RWclose(dst);
		SDL_FreeSurface(rgb);

		return success;
	}

} // namespace lime