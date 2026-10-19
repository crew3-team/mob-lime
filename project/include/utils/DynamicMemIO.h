#pragma once

#include <SDL.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// SDL2 replacement for SDL3's SDL_IOFromDynamicMem

namespace lime
{

	struct DynamicMemIO
	{
		uint8_t *data;
		size_t size;
		size_t capacity;
		size_t position;
	};

	static inline DynamicMemIO *DynamicMemIO_Get(SDL_RWops *ctx)
	{
		return (DynamicMemIO *)ctx->hidden.unknown.data1;
	}

	static inline Sint64 DynamicMemIO_Size(SDL_RWops *ctx)
	{
		return (Sint64)DynamicMemIO_Get(ctx)->size;
	}

	static inline Sint64 DynamicMemIO_Seek(SDL_RWops *ctx, Sint64 offset, int whence)
	{
		DynamicMemIO *mem = DynamicMemIO_Get(ctx);
		Sint64 base;

		switch (whence)
		{
			case RW_SEEK_SET:
				base = 0;
				break;
			case RW_SEEK_CUR:
				base = (Sint64)mem->position;
				break;
			case RW_SEEK_END:
				base = (Sint64)mem->size;
				break;
			default:
				return SDL_SetError("Unknown value for 'whence'");
		}

		Sint64 target = base + offset;

		if (target < 0)
		{
			return SDL_SetError("Cannot seek before the start of the stream");
		}

		mem->position = (size_t)target;
		return target;
	}

	static inline size_t DynamicMemIO_Read(SDL_RWops *ctx, void *ptr, size_t size, size_t maxnum)
	{
		DynamicMemIO *mem = DynamicMemIO_Get(ctx);

		if (size == 0 || mem->position >= mem->size)
		{
			return 0;
		}

		size_t count = (mem->size - mem->position) / size;

		if (count > maxnum)
		{
			count = maxnum;
		}

		memcpy(ptr, mem->data + mem->position, count * size);
		mem->position += count * size;

		return count;
	}

	static inline size_t DynamicMemIO_Write(SDL_RWops *ctx, const void *ptr, size_t size, size_t num)
	{
		DynamicMemIO *mem = DynamicMemIO_Get(ctx);
		size_t total = size * num;

		if (total == 0)
		{
			return 0;
		}

		size_t required = mem->position + total;

		if (required > mem->capacity)
		{
			size_t newCapacity = mem->capacity ? mem->capacity : 4096;

			while (newCapacity < required)
			{
				newCapacity *= 2;
			}

			uint8_t *grown = (uint8_t *)realloc(mem->data, newCapacity);

			if (!grown)
			{
				SDL_OutOfMemory();
				return 0;
			}

			mem->data = grown;
			mem->capacity = newCapacity;
		}

		// If a seek went past the end, zero-fill the gap
		if (mem->position > mem->size)
		{
			memset(mem->data + mem->size, 0, mem->position - mem->size);
		}

		memcpy(mem->data + mem->position, ptr, total);
		mem->position = required;

		if (required > mem->size)
		{
			mem->size = required;
		}

		return num;
	}

	static inline int DynamicMemIO_Close(SDL_RWops *ctx)
	{
		if (ctx)
		{
			DynamicMemIO *mem = DynamicMemIO_Get(ctx);

			if (mem)
			{
				free(mem->data);
				free(mem);
			}

			SDL_FreeRW(ctx);
		}

		return 0;
	}

	static inline SDL_RWops *DynamicMemIO_Create()
	{
		SDL_RWops *ctx = SDL_AllocRW();

		if (!ctx)
		{
			return NULL;
		}

		DynamicMemIO *mem = (DynamicMemIO *)calloc(1, sizeof(DynamicMemIO));

		if (!mem)
		{
			SDL_FreeRW(ctx);
			return NULL;
		}

		ctx->size = DynamicMemIO_Size;
		ctx->seek = DynamicMemIO_Seek;
		ctx->read = DynamicMemIO_Read;
		ctx->write = DynamicMemIO_Write;
		ctx->close = DynamicMemIO_Close;
		ctx->type = SDL_RWOPS_UNKNOWN;
		ctx->hidden.unknown.data1 = mem;
		ctx->hidden.unknown.data2 = NULL;

		return ctx;
	}

} // namespace lime