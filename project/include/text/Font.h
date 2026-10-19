#ifndef LIME_TEXT_FONT_H
#define LIME_TEXT_FONT_H


#include <graphics/ImageBuffer.h>
#include <system/CFFI.h>
#include <system/System.h>
#include <utils/Resource.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H
#include FT_TRUETYPE_TABLES_H

#ifdef HX_WINDOWS
#undef GetGlyphIndices
#endif


namespace lime {


	typedef struct {

		unsigned long codepoint;
		size_t size;
		int index;
		int height;

	} GlyphInfo;


	typedef struct {

		uint32_t index;
		uint32_t width;
		uint32_t height;
		uint32_t x;
		uint32_t y;
		unsigned char data;

	} GlyphImage;


	class Font {


		public:

			static void InitializeLibrary ();
			static void ShutdownLibrary ();

			Font (Resource *resource, int faceIndex = 0);
			~Font ();

			int GetAscender ();
			int GetDescender ();
			wchar_t *GetFamilyName ();
			int GetGlyphIndex (const char* character);
			void* GetGlyphIndices (const char* characters);
			void* GetGlyphMetrics (int index);
			void* GetKerning (int leftIndex, int rightIndex);
			int GetHeight ();
			int GetNumGlyphs ();
			int GetUnderlinePosition ();
			int GetUnderlineThickness ();
			int GetStrikethroughPosition ();
			int GetStrikethroughThickness ();
			int GetUnitsPerEM ();
			int RenderGlyph (int index, Bytes *bytes, int offset = 0, int flags = 0);
			int RenderGlyphs (int *indices, int numIndices, Bytes *bytes, int flags = 0);
			void SetSize (size_t size);

			FT_Library library;
			FT_Face face;
			bool ownedLibrary;

			static FT_Library sharedLibrary;

	};


}


#endif