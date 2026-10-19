#include <climits>
#include <cstring>
#include <graphics/format/PNG.h>
#include <graphics/ImageBuffer.h>
#include <graphics/PixelFormat.h>
#include <SDL.h>
#include <SDL_image.h>
#include <system/System.h>
#include <utils/DynamicMemIO.h>
#include <utils/File.h>

namespace lime
{

	bool PNG::Decode(Resource *resource, ImageBuffer *imageBuffer)
	{
		File file = resource->path ? File(resource->path, "rb") : File(resource->data, false);

		if (!file.handle)
		{
			return false;
		}

		if (!IMG_isPNG(file.handle))
		{
			file.Close();

			return false;
		}

		SDL_Surface *surface = IMG_LoadPNG_RW(file.handle);

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

	bool PNG::Encode(ImageBuffer *imageBuffer, Bytes *bytes)
	{
		SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(imageBuffer->data->buffer->b, imageBuffer->width, imageBuffer->height, 32, imageBuffer->Stride(), SDL_PIXELFORMAT_RGBA32);

		if (!surface)
		{
			return false;
		}

		SDL_RWops *dst = DynamicMemIO_Create();

		if (!dst)
		{
			SDL_FreeSurface(surface);

			return false;
		}

		bool success = false;

		if (bytes)
		{
			// Returns 0 on success; freedst = 0 so we close it ourselves
			success = (IMG_SavePNG_RW(surface, dst, 0) == 0);

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
		SDL_FreeSurface(surface);

		return success;
	}

} // namespace lime