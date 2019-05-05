#pragma once
#include "D2Widget.hpp"
#include "D2Widget_Scrollbar.hpp"

class D2Widget_CharSelectList : public D2Widget
{
private:
	struct CharacterSaveData
	{
		D2SaveHeader header;
		char16_t name[16];	// The name gets converted into UTF-16 when we load it
		char path[MAX_D2PATH_ABSOLUTE];
		token_handle token; // The token
		anim_handle tokenInstance;	// The token anim instance for this save so we can draw it
		CharacterSaveData* pNext;
	};
	CharacterSaveData* pCharacterData;
	D2Widget_Scrollbar*		pScrollBar;
	int nCurrentScroll;
	int nCurrentSelection;
	int nNumberSaves;
	tex_handle	frameHandle;
	tex_handle	greyFrameHandle;
	const int nSlotWidth = 272;
	const int nSlotHeight = 93;

	void DrawSaveSlot(CharacterSaveData* pCharacterData, int nSlot);
	void Clicked(DWORD dwX, DWORD dwY);
public:
	D2Widget_CharSelectList(int x, int y, int w, int h);
	virtual ~D2Widget_CharSelectList();

	void AddSave(D2SaveHeader& header, char* path);
	char16_t* GetSelectedCharacterName();
	void LoadSave();

	virtual void OnWidgetAdded();
	virtual void Draw();
	virtual bool HandleMouseDown(DWORD dwX, DWORD dwY);
	virtual bool HandleMouseClick(DWORD dwX, DWORD dwY);

	void Selected(int nNewSelection);
};