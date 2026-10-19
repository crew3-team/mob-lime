#include <utils/File.h>

#include <stdio.h>

namespace lime
{

	File::File(const char *path, const char *mode)
	{
		ownedMemory = NULL;

		handle = SDL_RWFromFile(path, mode);

		if (!handle)
		{
			char *base = SDL_GetBasePath();

			if (base)
			{
				size_t len = SDL_strlen(base) + SDL_strlen(path) + 1;
				char *fullpath = (char *)SDL_malloc(len);

				if (fullpath)
				{
					SDL_snprintf(fullpath, len, "%s%s", base, path);

					handle = SDL_RWFromFile(fullpath, mode);

					SDL_free(fullpath);
				}

				// In SDL2, the base path must be freed by the caller
				SDL_free(base);
			}
		}
	}

	File::File(Bytes *bytes, bool copy)
	{
		ownedMemory = NULL;

		if (!bytes || !bytes->b || bytes->length == 0)
		{
			handle = NULL;
			return;
		}

		if (copy)
		{
			ownedMemory = SDL_malloc(bytes->length);

			if (ownedMemory)
			{
				SDL_memcpy(ownedMemory, bytes->b, bytes->length);

				handle = SDL_RWFromMem(ownedMemory, (int)bytes->length);
			}
			else
			{
				handle = NULL;
			}
		}
		else
		{
			handle = SDL_RWFromConstMem(bytes->b, (int)bytes->length);
		}
	}

	bool File::Close()
	{
		bool result = false;

		if (handle)
		{
			// SDL2 returns 0 on success, unlike SDL3's bool
			result = (SDL_RWclose(handle) == 0);
			handle = NULL;
		}

		if (ownedMemory)
		{
			SDL_free(ownedMemory);
			ownedMemory = NULL;
		}

		return result;
	}

	bool File::Flush()
	{
		if (!handle)
		{
			return false;
		}

		// SDL2 has no SDL_RWflush; flush the underlying native handle where possible
		switch (handle->type)
		{
#if defined(__WIN32__) || defined(_WIN32)
			case SDL_RWOPS_WINFILE:
				// Windows RWops use SDL's own buffering layer, which writes through
				// on SDL_RWwrite, so there is nothing pending to flush here.
				return true;
#endif
			case SDL_RWOPS_STDFILE:
				return handle->hidden.stdio.fp ? (fflush(handle->hidden.stdio.fp) == 0) : false;

			default:
				// Memory streams etc. have nothing to flush
				return true;
		}
	}

	size_t File::Read(void *ptr, size_t size)
	{
		return handle ? SDL_RWread(handle, ptr, 1, size) : -1;
	}

	int64_t File::Seek(int64_t offset, int whence)
	{
		if (!handle)
		{
			return -1;
		}

		int sdlWhence;

		switch (whence)
		{
			case SEEK_SET:
				sdlWhence = RW_SEEK_SET;
				break;
			case SEEK_CUR:
				sdlWhence = RW_SEEK_CUR;
				break;
			case SEEK_END:
				sdlWhence = RW_SEEK_END;
				break;
			default:
				return -1;
		}

		return SDL_RWseek(handle, offset, sdlWhence);
	}

	int64_t File::Tell()
	{
		return handle ? SDL_RWtell(handle) : -1;
	}

	size_t File::Write(const void *ptr, size_t size)
	{
		return handle ? SDL_RWwrite(handle, ptr, 1, size) : -1;
	}

} // namespace lime
