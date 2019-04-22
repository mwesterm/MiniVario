// DisplayObjects.h
//

#include <Arduino.h>
#include "Widget.h"
#include "PrefItem.h"
#include "MenuItem.h"

#define MAX_WIDGETS				(16)
#define MAX_PREFITEMS			(16)
#define MAX_MEUITEMS			(16)

#define GET_COMMAND(x)			((x) & 0xFFFF0000)
#define GET_PARAM(x)			((x) & 0x0000FFFF)

#define MAKE_LRESULT(c, p)		((c) | (p))

#define CMD_SHOW_NEXT_PAGE		(0x00010000)
#define CMD_SHOW_PREV_PAGE		(0x00020000)
#define CMD_SHOW_TOP_MENU		(0x00030000)

#define CMD_LEAVE_TOPMENU		(0x00040000)
#define CMD_LEAVE_PREFERENCE	(0x00050000)

#define CMD_SHUTDOWN			(0xFFFE0000)

typedef enum _DispObjectType
{
	DispObject_Undef,
	DispObject_Screen,
	DispObject_Preference,
	DispObject_PopupMenu,
	DispObject_PopupMessageBox,
	DispObject_PopupListBox,
	DispObject_PopupCheckBox,
	DispObject_PopupRadioBox
	
} DispObjectType;


////////////////////////////////////////////////////////////////////////////////////////////////
// class DisplayObject

class DisplayObject
{
public:
	DisplayObject(DispObjectType type) : objType(type) { }
	
public:
	virtual uint32_t	processKey(uint8_t key) { return 0; }
	
public:
	DispObjectType		objType;
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioScreen

class VarioScreen : public DisplayObject
{
public:
	VarioScreen();
	
public:
	virtual uint32_t	processKey(uint8_t key);
	
public:
	Widget				widget[MAX_WIDGETS];
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioPreference

class VarioPreference : public DisplayObject
{
public:
	VarioPreference();
	
public:
	PrefItem			prefs[MAX_PREFITEMS];
	
	volatile int16_t	itemCount;
	volatile int16_t	itemTop;
	volatile int16_t	itemSelect;
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioPopup

class VarioPopup : public DisplayObject
{
public:
	VarioPopup(DispObjectType type) : DisplayObject(type) { }
	
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupMenu

class PopupMenu : public VarioPopup
{
public:
	PopupMenu();
	
public:
	int16_t				addItem(uint16_t itemId, uint16_t strId);
	
	virtual uint32_t	processKey(uint8_t key);
	
public:
	MenuItem			items[MAX_MEUITEMS];
	
	volatile int16_t	itemCount;
	volatile int16_t	itemTop;
	volatile int16_t	itemSelect;
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupMessageBox

class PopupMessageBox : public VarioPopup
{
public:
	PopupMessageBox(uint32_t type, const char * msg);

public:
	virtual uint32_t	processKey(uint8_t key);

protected:
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupListBox

class PopupListBox : public VarioPopup
{
public:
	PopupListBox();
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupCheckBox

class PopupCheckBox : public VarioPopup
{
public:
	PopupCheckBox();
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupRadioBox

class PopupRadioBox : public VarioPopup
{
public:
	PopupRadioBox();
};

