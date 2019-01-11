#pragma once
#include "../Shared/D2Shared.hpp"
#include "../Shared/D2Packets.hpp"
#include "../Libraries/sdl/SDL.h"

/////////////////////////////////////////////////////////
//
//	Structures

enum D2CommandType
{
	CMD_BOOLEAN,
	CMD_DWORD,
	CMD_STRING,
	CMD_BYTE,
	CMD_WORD
};

/*
 *	The structure containing information about parsing commandline arguments
 *	@author	Necrolis
 */
struct D2CmdArgStrc
{
	char szSection[16];		// +00
	char szKeyName[16];		// +10
	char szCmdName[16];		// +20
	DWORD dwType;			// +30 ( 0 use GetProfInt - write bool, 1 DWORD , 2 string, 3 BYTE, 4 WORD)
	int nOffset;			// +34
	DWORD dwDefault;		// +38
};							// +3C

/*
 *	The structure containing information about the system running the game
 */
struct D2SystemInfoStrc
{
	char szComputerName[64];
	char szOSName[128];
	char szProcessorVendor[32];
	char szProcessorModel[64];
	char szProcessorSpeed[64];	// maybe not present on linux?
	char szProcessorIdentifier[64];
	char szRAMPhysical[64];
	char szRAMVirtual[64];
	char szRAMPaging[64];
	char szWorkingDirectory[MAX_D2PATH_ABSOLUTE];
};

/*
 *	Bitstreams are used for both DCCs and networking.
 *	Based partially on id Tech 4's bitstreams.
 *	@author	eezstreet
 */
class Bitstream
{
public:
	Bitstream();
	~Bitstream();

	void LoadStream(BYTE* pNewStream, size_t dwStreamSizeBytes);
	void SplitFrom(Bitstream* pSplitStream, size_t dwSplitStreamSizeBits);

	void SetCurrentPosition(DWORD dwPosition, DWORD dwBitOffset = 0);

	void ReadByte(BYTE& outByte);
	void ReadWord(WORD& outWord);
	void ReadDWord(DWORD& outWord);
	void ReadBits(BYTE& outBits, int bitCount);
	void ReadBits(WORD& outBits, int bitCount);
	void ReadBits(DWORD& outBits, int bitCount);
	void ReadByte(BYTE* outByte);
	void ReadWord(WORD* outWord);
	void ReadDWord(DWORD* outWord);
	void ReadBits(BYTE* outBits, int bitCount);
	void ReadBits(WORD* outBits, int bitCount);
	void ReadBits(DWORD* outBits, int bitCount);
	void ReadBits(void* outBits, size_t outSize, int bitCount);
	void ReadData(void* data, size_t outSize);
	
	// Helper function - convert from unsigned to 2C
	void ConvertFormat(long* dwOutBits, int bitCount);

	void Rewind();

	size_t GetRemainingReadBits();

	BYTE* GetHeldData(size_t& outSize);

private:
	int ReadBits(int bitsCount);
	void FreeInternalStreamSource();

	bool bExternalStorage;
	BYTE* pStream;
	size_t dwStreamStartByte;
	size_t dwStreamStartBit;
	size_t dwTotalStreamSizeBytes;
	size_t dwTotalStreamSizeBits;
	size_t dwCurrentByte;
	size_t dwReadBit;
};

/*
 *	EVERYTHING TO DO WITH MPQ FILES
 *	The structure containing data about MPQ files
 *	Pieced together with code from Tom Amigo, Paul Siramy, and Zezula
 *	@author	eezstreet
 */

 // Compression types for multiple compressions
#define MPQ_COMPRESSION_HUFFMANN          0x01  // Huffmann compression (used on WAVE files only)
#define MPQ_COMPRESSION_PKWARE            0x08  // PKWARE DCL compression
#define MPQ_COMPRESSION_ADPCM_MONO        0x40  // IMA ADPCM compression (mono)
#define MPQ_COMPRESSION_ADPCM_STEREO      0x80  // IMA ADPCM compression (stereo)
#define MPQ_COMPRESSION_NEXT_SAME   0xFFFFFFFF  // Same compression

#define MPQ_FILE_IMPLODE            0x00000100  // Implode method (By PKWARE Data Compression Library)
#define MPQ_FILE_COMPRESS           0x00000200  // Compress methods (By multiple methods)
#define MPQ_FILE_ENCRYPTED          0x00010000  // Indicates whether file is encrypted 
#define MPQ_FILE_FIX_KEY            0x00020000  // File decryption key has to be fixed
#define MPQ_FILE_PATCH_FILE         0x00100000  // The file is a patch file. Raw file data begin with TPatchInfo structure
#define MPQ_FILE_SINGLE_UNIT        0x01000000  // File is stored as a single unit, not sectors
#define MPQ_FILE_DELETE_MARKER      0x02000000  // File is a deletion marker.
#define MPQ_FILE_SECTOR_CRC         0x04000000  // File has checksums for each sector.
 // Ignored if file is not compressed or imploded.
#define MPQ_FILE_SIGNATURE          0x10000000  // Present on STANDARD.SNP\(signature). The only occurence ever observed
#define MPQ_FILE_EXISTS             0x80000000  // Set if file exists, reset when the file was deleted
#define MPQ_FILE_REPLACEEXISTING    0x80000000  // Replace when the file exist (SFileAddFile)

#define MPQ_FILE_COMPRESS_MASK      0x0000FF00  // Mask for a file being compressed

#define MPQ_FILE_DEFAULT_INTERNAL   0xFFFFFFFF  // Use default flags for internal files

#define MPQ_FILE_VALID_FLAGS     (MPQ_FILE_IMPLODE       |  \
                                  MPQ_FILE_COMPRESS      |  \
                                  MPQ_FILE_ENCRYPTED     |  \
                                  MPQ_FILE_FIX_KEY       |  \
                                  MPQ_FILE_PATCH_FILE    |  \
                                  MPQ_FILE_SINGLE_UNIT   |  \
                                  MPQ_FILE_DELETE_MARKER |  \
                                  MPQ_FILE_SECTOR_CRC    |  \
                                  MPQ_FILE_SIGNATURE     |  \
                                  MPQ_FILE_EXISTS)

 // Decryption keys for MPQ tables - thanks Zezula
#define MPQ_KEY_HASH_TABLE          0xC3AF3770  // Obtained by HashString("(hash table)", MPQ_HASH_FILE_KEY)
#define MPQ_KEY_BLOCK_TABLE         0xEC83B3A3  // Obtained by HashString("(block table)", MPQ_HASH_FILE_KEY)

#pragma pack(push,enter_include)
#pragma pack(1)
/*
 *	@author Zezula
 */
struct MPQHash
{
	DWORD	dwMethodA;			// Hash (method A)
	DWORD	dwMethodB;			// Hash (method B)
	WORD	wLocale;
	BYTE	nPlatform;
	BYTE	nReserved;
	DWORD	dwBlockEntry;
};

/*
 *	@author	Zezula
 */
struct MPQBlock
{
	DWORD dwFilePos;	// Offset from the beginning of the file
	DWORD dwCSize;		// Compressed file size
	DWORD dwFSize;		// Uncompressed file size (0 if not a file)
	DWORD dwFlags;		// File flags
};
#pragma pack(pop,enter_include)

// @author eezstreet
typedef char MPQName[MAX_D2PATH];

/*
 *	@author eezstreet
 */
struct D2MPQArchive
{
	bool			bOpen;					// Whether the MPQ archive is currently open / has no errors
	DWORD			dwDataOffset;			// Offset to MPQ file data
	DWORD			dwHashOffset;			// Offset to MPQ hash table
	DWORD			dwBlockOffset;			// Offset to MPQ block table
	DWORD			dwArchiveSize;			// Length of MPQ file data
	DWORD			dwNumHashEntries;		// Length of MPQ hash table
	DWORD			dwNumBlockEntries;		// Length of MPQ block table
	MPQHash*		pHashTable;				// Pointer to MPQ hash table
	MPQBlock*		pBlockTable;			// Pointer to MPQ block table
	DWORD			dwFileCount;			// Number of files in MPQ
	WORD			wSectorSize;			// The size of MPQ sectors (always 0x200)
	DWORD			dwSlackSpace[0x500];
	DWORD*			pSectorOffsets;
	DWORD			dwSectorCount;

	MPQName*		pNameTable;				// Names of each file in the MPQ. Only filled as used.
	fs_handle		f;						// FS file handle
};

/*
 *	ALL OTHER FILE FORMATS
 */

/*
 *	String Archive TBLs
 *	Based on code from QTblEditor by kambala
 *	@author eezstreet
 */

#define MAX_TBL_FILE_HANDLE	16
#define MAX_TBL_KEY_SIZE	64

#pragma pack(push,enter_include)
#pragma pack(1)

struct TBLHeader // header of string *.tbl file
{
	WORD  CRC;             // +0x00 - CRC value for string table
	WORD  NodesNumber;     // +0x02 - size of Indices array
	DWORD HashTableSize;   // +0x04 - size of TblHashNode array
	BYTE  Version;         // +0x08 - file version, either 0 or 1, doesn't matter
	DWORD DataStartOffset; // +0x09 - string table start offset
	DWORD HashMaxTries;    // +0x0D - max number of collisions for string key search based on its hash value
	DWORD FileSize;        // +0x11 - size of the file
};

struct TBLHashNode // node of the hash table in string *.tbl file
{
	BYTE  Active;          // +0x00 - shows if the entry is used. if 0, then it has been "deleted" from the table
	WORD  Index;           // +0x01 - index in Indices array
	DWORD HashValue;       // +0x03 - hash value of the current string key
	DWORD StringKeyOffset; // +0x07 - offset of the current string key
	DWORD StringValOffset; // +0x0B - offset of the current string value
	WORD  StringValLength; // +0x0F - length of the current string value
};

struct TBLDataNode
{
	BYTE		bActive;
	char16_t	key[MAX_TBL_KEY_SIZE];
	char16_t*	str;
};

struct TBLFile
{
	TBLHeader header;
	TBLHashNode* pHashNodes;
	TBLDataNode* pDataNodes;
	WORD* pIndices;

	char szHandle[MAX_TBL_FILE_HANDLE];
	D2MPQArchive* pArchive;	// the archive that this file was found in
	fs_handle archiveHandle;	// the file inside the archive

	size_t dwFileSize; // the size of the whole TBL file
	char16_t* szStringTable; // a table that contains all of the string data
};

#pragma pack(pop, enter_include)

/*
 *	Font TBLs
 *	Based on structures provided by Necrolis
 *	@author	eezstreet
 */

#pragma pack(push,enter_include)
#pragma pack(1)

// FIXME: there's a lot of unknowns here, they need to be properly investigated with a debugger
struct TBLFontGlyph
{
	WORD			wChar;		// Unicode character code
	BYTE			nUnknown1;
	BYTE			nWidth;
	BYTE			nHeight;
	BYTE			nUnknown2;
	WORD			wUnknown3;
	BYTE			nImageIndex;
	BYTE			nChar;		// ANSI character code
	DWORD			dwUnknown4;	// NOTE: on SDL renderer this gets set to the X offset of the glyph on atlas
};

struct TBLFontFile
{
	BYTE			magic[4];	// always "Woo!", this exclamation appears in the save file format too
	WORD			wVersion;
	DWORD			dwLocale;	// 1 = english, 5 = russian, ...?
	BYTE			nHeight;	// interline spacing
	BYTE			nWidth;		// cap height (?)
	TBLFontGlyph	glyphs[256];
};

#pragma pack(pop, enter_include)

/*
 *	DC6 Files
 *	Pieced together from code by Paul Siramy and SVR.
 *	@author eezstreet
 */
#define MAX_DC6_CELL_SIZE	256

#pragma pack(push,enter_include)
#pragma pack(1)
struct DC6Frame
{
	struct DC6FrameHeader
	{
		DWORD	dwFlip;				// If true, it's encoded top to bottom instead of bottom to top
		DWORD	dwWidth;			// Width of this frame
		DWORD	dwHeight;			// Height of this frame
		DWORD	dwOffsetX;			// X offset
		DWORD	dwOffsetY;			// Y offset
		DWORD	dwUnknown;
		DWORD	dwNextBlock;
		DWORD	dwLength;			// Number of blocks to decode
	};
	DC6FrameHeader fh;

	DWORD dwDeltaY; // delta from previous frame (0 on first frame in direction)
};

struct DC6ImageHeader
{
	DWORD	dwVersion;			// Version; always '6'
	DWORD	dwUnk1;				// Unknown; always '1'
	DWORD	dwUnk2;				// Unknown; always '0'
	DWORD	dwTermination;		// Termination code - usually 0xEEEEEEEE or 0xCDCDCDCD
	DWORD	dwDirections;		// Number of directions
	DWORD	dwFrames;			// Number of frames per direction
};
#pragma pack(pop, enter_include)

struct DC6Image
{
	void*			mpq;			// The MPQ we found it in
	fs_handle		f;				// The file handle from within the MPQ
	DC6ImageHeader	header;
	DC6Frame*		pFrames;
	BYTE*			pPixels;
	DWORD			dwDirectionHeights[32];
	DWORD			dwTotalWidth;
	DWORD			dwTotalHeight;
	bool			bPixelsFreed;
};

/*
 *	DCC Files
 *	Pieced together with code from Necrolis, SVR, and Paul Siramy
 */
#pragma pack(push,enter_include)
#pragma pack(1)
struct DCCHeader
{
	BYTE			nSignature;
	BYTE			nVersion;
	BYTE			nNumberDirections;
	DWORD			dwFramesPerDirection;
	DWORD			dwTag;
	DWORD			dwFinalDC6Size;
	DWORD			dwDirectionOffset[MAX_DIRECTIONS];
};

struct DCCFrame
{
	DWORD			dwVariable0;
	DWORD			dwWidth;
	DWORD			dwHeight;
	long			nXOffset;
	long			nYOffset;
	DWORD			dwOptionalBytes;
	DWORD			dwCodedBytes;
	DWORD			dwFlipped;

	//////////////////////////////////
	// Not actually in the file, these are calculated
	long			nMinX, nMaxX;
	long			nMinY, nMaxY;
	DWORD			dwCellW, dwCellH;
	//////////////////////////////////

	BYTE*			pOptionalByteData;
};

#define MAX_DCC_PIXEL_BUFFER	300000
#define MAX_DCC_FRAMES			200
struct DCCPixelBuffer
{
	BYTE			pixel[4];
	int				frame;
	int				frameCellIndex;
};

struct DCCDirection
{
	DWORD			dwOutsizeCoded;
	BYTE			nCompressionFlag;
	BYTE			nVar0Bits;
	BYTE			nWidthBits;
	BYTE			nHeightBits;
	BYTE			nXOffsetBits;
	BYTE			nYOffsetBits;
	BYTE			nOptionalBytesBits;
	BYTE			nCodedBytesBits;

	DCCFrame		frames[MAX_FRAMES];

	//////////////////////////////////
	// Not actually in the file, these are calculated
	long			nMinX, nMaxX;
	long			nMinY, nMaxY;
	//////////////////////////////////

	DWORD			dwEqualCellStreamSize;	// only present when nCompressionFlag & 0x02
	DWORD			dwPixelMaskStreamSize;
	DWORD			dwEncodingStreamSize;	// only present when nCompressionFlag & 0x01
	DWORD			dwRawPixelStreamSize;	// only present when nCompressionFlag & 0x01
	BYTE			nPixelValues[256];

	//////////////////////////////////
	//	Allocated when the DCC direction is created
	Bitstream*		EqualCellStream;
	Bitstream*		PixelMaskStream;
	Bitstream*		EncodingTypeStream;
	Bitstream*		RawPixelStream;
	Bitstream*		PixelCodeDisplacementStream;

	//////////////////////////////////
	//	Functions to make our lives a bit easier
	void RewindAllStreams()
	{
		if (EqualCellStream) { EqualCellStream->Rewind(); }
		if (PixelMaskStream) { PixelMaskStream->Rewind(); }
		if (EncodingTypeStream) { EncodingTypeStream->Rewind(); }
		if (RawPixelStream) { RawPixelStream->Rewind(); }
		if (PixelCodeDisplacementStream) { PixelCodeDisplacementStream->Rewind(); }
	}
};

struct DCCFile
{
	// Part of the file structure
	DCCHeader		header;
	DCCDirection	directions[MAX_DIRECTIONS];

	// Other stuff used by OpenD2
	DWORD			dwFileSize;
	BYTE*			pFileBytes;
};

// Each frame in the DCC is composed of cells.
// Cells are (roughly) 4x4 blocks of pixels.
// The size is flexible; sometimes you can have a 5x4, etc.
struct DCCCell
{
	int nX, nY, nW, nH;
	BYTE clrmap[4];
};

#pragma pack(pop, enter_include)

/*
 *	COF (component object files)
 *	COF consists of three key elements: a header, a series of layers, and a series of keyframes.
 *	The layer count and keyframe count are included as part of the header.
 *	Each COF has exactly seven characters in its name (not including the extension), and is formatted thusly:
 *	<token name> <animation> <weapon set>
 *	So for example, the Amazon's first attack animation, using a staff, would be:
 *	AMA1STF.cof (AM for the AMazon token, A1 for the first attack animation, STF for the STaFf.
 *	For optimization (?), some COF metadata is stored in animdata.d2, however we don't need this for our purposes.
 *	These structures are provided by Paul Siramy.
 */

#pragma pack(push,enter_include)
#pragma pack(1)
enum COFKeyframe
{
	COFKEY_NONE,
	COFKEY_ATTACK,
	COFKEY_MISSILE,
	COFKEY_SOUND,
	COFKEY_SKILL,
	COFKEY_MAX,
};

struct COFHeader
{
	BYTE	nLayers;
	BYTE	nFrames;
	BYTE	nDirs;
	BYTE	nVersion;
	DWORD	dwUnk1;
	int		nXMin;
	int		nXMax;
	int		nYMin;
	int		nYMax;
	WORD	nFPS;
	/*BYTE	nFPS;
	BYTE	nArmType;*/
	WORD	wUnk2;
};

struct COFLayer
{
	BYTE	nComponent;
	BYTE	bShadow;
	BYTE	bSelectable;
	BYTE	nOverrideTranslvl;
	BYTE	nNewTranslvl;
	BYTE	szWeaponClass[4];
};

struct COFFile
{
	COFHeader	header;
	COFLayer*	layers;
	BYTE*		keyframes;

	// The following aren't in the file. They are just loaded for my purposes.
	DWORD		dwLayersPresent;
};
#pragma pack(pop, enter_include)

/*
 *	Tokens.
 *	Tokens are a collection of COFs.
 *	@author	eezstreet
 */
struct AnimToken
{
	D2TokenType	tokenType;
	DWORD		dwCOFsPresent;

	union
	{
		cof_handle	plrCof[PLRMODE_MAX];
		cof_handle	monCof[MONMODE_MAX];
		cof_handle	objCof[OBJMODE_MAX];
	};
};

/*
 *	Token instances.
 *	When you want a monster, object or character to be shown on the screen, you should use a token instance.
 *	A token instance is a single rendering of one of those objects and is animated separately.
 *	(Missiles and overlays do not use token instances. Instead, they are rendered as raw DCCs.)
 *	@author	eezstreet
 */
struct AnimTokenInstance
{
	token_handle	currentHandle;
	int				tokenType;
	int				currentMode;
	int				currentFrame;
	int				currentDirection;
	char			components[COMP_MAX][4];
	bool			bInUse;
	bool			bActive;

	anim_handle		componentAnims[XXXMODE_MAX][COMP_MAX];
};

/*
 *	DS1 files.
 *	They are an arranged collection of tiles that have been created either by Blizzard or through Paul Siramy's win_ds1edit tool.
 */
#define MAX_DS1_OBJECTS			300
#define MAX_DS1_WALL_LAYERS		4
#define MAX_DS1_FLOOR_LAYERS	4
#define MAX_DS1_SHADOW_LAYERS	1
#define MAX_DS1_PATHS			100
typedef char DS1Path[MAX_D2PATH];
#pragma pack(push,enter_include)
#pragma pack(1)
struct DS1File
{
	struct DS1FileHeader {
		DWORD dwX1;					// Not known.
		DWORD dwWidth;				// Number of tiles wide that this DS1 is.
		DWORD dwHeight;				// Number of tiles tall that this DS1 is.
		DWORD dwAct;				// Which palette this DS1 uses.
		DWORD bUnknownLayer;		// If true, there exists an unknown layer in the file (after wall, floor and shadow layers)
		DWORD dwExtraFilesCount;	// Vestigial.
	};

	struct DS1LayerHeader {
		struct DS1LoadedLayerHeader {
			DWORD dwWallLayers;			// Number of wall layers to use. Usually 4.
			DWORD dwFloorLayers;		// Number of floor layers to use. Usually 2.
		} loaded;
		DWORD dwShadowLayers;		// Number of shadow layers to use. Always 1.
	};

	struct DS1ObjectHeader {
		DWORD dwNumObjects;			// Number of objects to have in this DS1.
	};

	struct DS1Object {
		struct DS1ObjectInfo {
			DWORD dwType;
			DWORD dwId;
			DWORD dwX;
			DWORD dwY;
			DWORD dwUnused;
		} info;
	};

	struct DS1Path {
		DWORD dwPath;
	};

	struct DS1Cell {
		BYTE prop1;
		BYTE prop2;
		BYTE prop3;
		BYTE prop4;
		BYTE orientation; // only present on wall tiles
		DWORD tileIndex;
		BYTE flags;
	};

	DS1FileHeader fileHeader;
	DS1Path* optionalFileList;		// Vestigial. This is most likely used by an internal tool to "know" which tiles to load, because it isn't used in-game.
									// It's pretty obvious that this is the case when everything up to Act4 tiles uses .tgl files, but Act5 uses .dt1 references.
	DS1LayerHeader layerHeader;
	BYTE* pUnknownLayer;
	DS1ObjectHeader objectHeader;
	DS1Object objects[MAX_DS1_OBJECTS];

	// TODO: add NPC pathing

	DS1Cell* pFloorCells;
	DS1Cell* pWallCells;
	DS1Cell* pShadowCells;
};
#pragma pack(pop, enter_include)

/*
 *	Renderer related structures
 *	@author	eezstreet
 */
struct D2Renderer
{
	//
	void		(*RF_Init)(D2GameConfigStrc* pConfig, OpenD2ConfigStrc* pOpenConfig, SDL_Window* pWindow);
	void		(*RF_Shutdown)();
	void		(*RF_Present)();

	//
	tex_handle	(*RF_TextureFromStitchedDC6)(char* szDc6Path, char* szHandle, DWORD dwStart, DWORD dwEnd, int nPalette);
	tex_handle	(*RF_TextureFromAnimatedDC6)(char* szDc6Path, char* szHandle, int nPalette);
	void		(*RF_DrawTexture)(tex_handle texture, int x, int y, int w, int h, int u, int v);
	void		(*RF_DrawTextureFrames)(tex_handle texture, int x, int y, DWORD dwStartFrame, DWORD dwEndFrame);
	void		(*RF_DrawTextureFrame)(tex_handle texture, int x, int y, DWORD dwFrame);
	void		(*RF_DeregisterTexture)(char* handleName, tex_handle texture);
	void		(*RF_SetTextureBlendMode)(tex_handle texture, D2ColorBlending blendMode);
	void		(*RF_PollTexture)(tex_handle texture, DWORD* dwWidth, DWORD* dwHeight);
	bool		(*RF_PixelPerfectDetect)(anim_handle anim, int nSrcX, int nSrcY, int nDrawX, int nDrawY, bool bAllowAlpha);

	anim_handle	(*RF_RegisterDCCAnimation)(tex_handle texture, char* szHandleName, DWORD dwStartingFrame);
	void		(*RF_DeregisterAnimation)(anim_handle anim);
	void		(*RF_Animate)(anim_handle anim, DWORD dwFramerate, int x, int y);
	void		(*RF_SetAnimFrame)(anim_handle anim, DWORD dwFrame);
	DWORD		(*RF_GetAnimFrame)(anim_handle anim);
	void		(*RF_AddAnimKeyframe)(anim_handle anim, int nFrame, AnimKeyframeCallback callback, int nExtraInt);
	void		(*RF_RemoveAnimKeyframe)(anim_handle anim);
	DWORD		(*RF_GetAnimFrameCount)(anim_handle anim);

	font_handle	(*RF_RegisterFont)(char* szFontName);
	void		(*RF_DeregisterFont)(font_handle font);
	void		(*RF_DrawText)(font_handle font, char16_t* text, int x, int y, int w, int h,
		D2TextAlignment alignHorz, D2TextAlignment alignVert);

	void		(*RF_AlphaModTexture)(tex_handle texture, int nAlpha);
	void		(*RF_ColorModTexture)(tex_handle texture, int nRed, int nGreen, int nBlue);
	void		(*RF_AlphaModFont)(font_handle font, int nAlpha);
	void		(*RF_ColorModFont)(font_handle font, int nRed, int nGreen, int nBlue);

	void		(*RF_DrawRectangle)(int x, int y, int w, int h, int r, int g, int b, int a);

	void		(*RF_DrawTokenInstance)(anim_handle instance, int x, int y, int translvl, int palette);

	void		(*RF_Clear)();
};

extern D2Renderer* RenderTarget;	// nullptr if there isn't a render target

/////////////////////////////////////////////////////////
//
//	Functions

// Audio.cpp
namespace Audio
{
	void Init(OpenD2ConfigStrc* openconfig);
	void Shutdown();
	sfx_handle RegisterSound(char* szAudioFile);
	mus_handle RegisterMusic(char* szAudioFile);
	void PlaySound(sfx_handle handle, int loops);
	void SetMasterVolume(float fNewVolume);
	void SetMusicVolume(float fNewVolume);
	void SetSoundVolume(float fNewVolume);
}

// COF.cpp
namespace COF
{
	cof_handle Register(char* type, char* token, char* animation, char* hitclass);
	void Deregister(cof_handle cof);
	void DeregisterType(char* type);
	void DeregisterAll();
	bool LayerPresent(cof_handle cof, int layer);
	COFFile* GetFileData(cof_handle handle);
};

// DC6.cpp
namespace DC6
{
	void LoadImage(char* szPath, DC6Image* pImage);
	void UnloadImage(DC6Image* pImage);
	BYTE* GetPixelsAtFrame(DC6Image* pImage, int nDirection, int nFrame, size_t* pNumPixels);
	void PollFrame(DC6Image* pImage, DWORD nDirection, DWORD nFrame,
		DWORD* dwWidth, DWORD* dwHeight, DWORD* dwOffsetX, DWORD* dwOffsetY);
	void StitchStats(DC6Image* pImage,
		DWORD dwStart, DWORD dwEnd, DWORD* pWidth, DWORD* pHeight, DWORD* pTotalWidth, DWORD* pTotalHeight);
	void FreePixels(DC6Image* pImage);
};

// DCC.cpp
namespace DCC
{
	void GlobalInit();
	void GlobalShutdown();
	anim_handle Load(char* szPath, char* szName);
	void IncrementUseCount(anim_handle dccHandle, int amount);
	DCCFile* GetContents(anim_handle dccHandle);
	void FreeHandle(anim_handle dcc);
	void FreeIfInactive(anim_handle handle);
	void FreeInactive();
	void FreeByName(char* name);
	void FreeAll();
	DWORD GetCellCount(int pos, int& sz);
};

// Diablo2.cpp
int InitGame(int argc, char** argv);
DWORD GetMilliseconds();
bool ServerProcessPacket(D2Packet* pPacket);
bool ClientProcessPacket(D2Packet* pPacket);

// FileSystem.cpp
namespace FS
{
	void Init(OpenD2ConfigStrc* pConfig);
	void Shutdown();
	void LogSearchPaths();
	size_t Open(char* filename, fs_handle* f, OpenD2FileModes mode, bool bBinary = false);
	size_t Read(fs_handle f, void* buffer, size_t dwBufferLen = 4, size_t dwCount = 1);
	size_t Write(fs_handle f, void* buffer, size_t dwBufferLen = 1, size_t dwCount = 1);
	size_t WritePlaintext(fs_handle f, char* text);
	void CloseFile(fs_handle f);
	void Seek(fs_handle f, size_t offset, int nSeekType);
	size_t Tell(fs_handle f);
	bool Find(char* szFileName, char* szBuffer, size_t dwBufferLen);
	char** ListFilesInDirectory(char* szDirectory, char* szExtensionFilter, int *nFiles);
	void FreeFileList(char** pszFileList, int nNumFiles);
	void CreateSubdirectory(char* szSubdirectory);
};

// FileSystem_MPQ.cpp
namespace FSMPQ
{
	void Init();
	void Shutdown();
	D2MPQArchive* AddSearchPath(char* szMPQName, char* szMPQPath);
	fs_handle FindFile(char* szFileName, char* szMPQName, D2MPQArchive** pArchiveOut);
}

// INI.cpp
namespace INI
{
	void WriteConfig(fs_handle* f, D2GameConfigStrc* pGameConfig, OpenD2ConfigStrc* pOpenConfig);
	void ReadConfig(fs_handle* f, D2GameConfigStrc* pGameConfig, OpenD2ConfigStrc* pOpenConfig);
}

// Input.cpp
namespace IN
{
	void PumpEvents(OpenD2ConfigStrc* pOpenConfig);
}

// Logging.cpp
#define Log_WarnAssert(x) if(!(x)) { Log::Warning(__FILE__, __LINE__, "" #x); }
#define Log_ErrorAssert(x) if(!(x)) { Log::Error(__FILE__, __LINE__, "" #x); }

#define Log_WarnAssertReturn(x, y) if(!(x)) { Log::Warning(__FILE__, __LINE__, "" #x); return y; }
#define Log_ErrorAssertReturn(x, y) if(!(x)) { Log::Error(__FILE__, __LINE__, "" #x); return y; }

#define Log_WarnAssertVoidReturn(x) if(!(x)) { Log::Warning(__FILE__, __LINE__, "" #x); return; }
#define Log_ErrorAssertVoidReturn(x) if(!(x)) { Log::Error(__FILE__, __LINE__, "" #x); return; }

namespace Log
{
	void InitSystem(const char* szLogHeader, const char* szGameName, OpenD2ConfigStrc* pOpenConfig);
	void Shutdown();
	void Print(OpenD2LogFlags nPriority, char* szFormat, ...);
	void Warning(char* szFile, int nLine, char* szCondition);
	void Error(char* szFile, int nLine, char* szCondition);
}

// MPQ.cpp
namespace MPQ
{
	void OpenMPQ(char* szMPQPath, const char* szMPQName, D2MPQArchive* pMPQ);
	void CloseMPQ(D2MPQArchive* pMPQ);
	fs_handle FetchHandle(D2MPQArchive* pMPQ, char* szFileName);
	size_t FileSize(D2MPQArchive* pMPQ, fs_handle fFile);
	size_t ReadFile(D2MPQArchive* pMPQ, fs_handle fFile, BYTE* buffer, DWORD dwBufferLen);
	void Cleanup();
}

// Network.cpp
namespace Network
{
	void SendServerPacket(int nClientMask, D2Packet* pPacket);
	void SendClientPacket(D2Packet* pPacket);
	void SetMaxPlayerCount(DWORD dwNewPlayerCount);
	DWORD ReadClientPackets(DWORD dwTimeout);
	DWORD ReadServerPackets(DWORD dwTimeout);
	bool ConnectToServer(char* szServerAddress, DWORD dwPort);
	void DisconnectFromServer();
	void StartListen(DWORD dwPort);
	void StopListening();
	void Init();
	void Shutdown();
};

// Palette.cpp
namespace Pal
{
	bool Init();
	D2Palette* GetPalette(int nIndex);
}

// Platform_*.cpp
namespace Sys
{
	void GetWorkingDirectory(char* szBuffer, size_t dwBufferLen);
	void DefaultHomepath(char* szBuffer, size_t dwBufferLen);
	void GetSystemInfo(D2SystemInfoStrc* pInfo);
	bool CreateDirectory(char* szPath);
	void ListFilesInDirectory(char* szPath, char* szExtensionFilter, char* szOriginalPath, int* nFiles, char(*szList)[MAX_FILE_LIST_SIZE][MAX_D2PATH_ABSOLUTE]);
	D2ModuleExportStrc* OpenModule(OpenD2Modules nModule, D2ModuleImportStrc* pImports);
	void CloseModule(OpenD2Modules nModule);
	char16_t* GetAdapterIP();
}

// Renderer.cpp
namespace Renderer
{
	void Init(D2GameConfigStrc* pConfig, OpenD2ConfigStrc* pOpenConfig, SDL_Window* pWindow);
	void MapRenderTargetExports(D2ModuleImportStrc* pExport);
}

// TBL_Font.cpp
namespace TBLFont
{
	tbl_handle RegisterFont(char* szFontName);
	TBLFontFile* GetPointerFromHandle(tbl_handle handle);
}

// TBL_Text.cpp
namespace TBL
{
	tbl_handle Register(char* szTblFile);
	char16_t* FindStringFromIndex(DWORD dwIndex);
	tbl_handle FindStringIndexFromKey(tbl_handle tbl, char16_t* szReference);
	char16_t* FindStringText(char16_t* szReference);
	void Init();
	void Cleanup();
}

// Threadpool.cpp
namespace Threadpool
{
	void WaitUntilCompletion();
	void SpawnJob(D2AsyncTask job, void* pData);
	void Init();
	void Shutdown();
}

// Token.cpp - Should maybe move this to gamecode?
namespace Token
{
	token_handle RegisterToken(D2TokenType type, char* tokenName, char* szWeaponClass);
	void DeregisterToken(token_handle token);
	AnimToken* GetAnimData(token_handle token);
	cof_handle GetCOFData(token_handle token, int mode);
}

namespace TokenInstance
{
	anim_handle CreateTokenAnimInstance(token_handle token);
	void SwapTokenAnimToken(anim_handle handle, token_handle newhandle);
	void DestroyTokenInstance(anim_handle handle);
	void SetTokenInstanceComponent(anim_handle handle, int componentNum, char* componentName);
	char* GetTokenInstanceComponent(anim_handle handle, int component);
	void SetTokenInstanceFrame(anim_handle handle, int frameNum);
	int GetTokenInstanceFrame(anim_handle handle);
	char* GetTokenInstanceWeaponClass(anim_handle handle);
	void SetInstanceActive(anim_handle handle, bool bNewActive);
	AnimTokenInstance* GetTokenInstanceData(anim_handle handle);
	void SetTokenInstanceMode(anim_handle handle, int modeNum);
	void SetTokenInstanceDirection(anim_handle handle, int dirNum);
}

// Window.cpp
namespace Window
{
	void InitSDL(D2GameConfigStrc* pConfig, OpenD2ConfigStrc* pOpenConfig);
	void ShutdownSDL();
	void ShowMessageBox(int nMessageBoxType, char* szTitle, char* szMessage);
	bool InFocus(DWORD nWindowID);
}
