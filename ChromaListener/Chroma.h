#pragma once
#ifndef _CHROMASDKIMPL_H_
#define _CHROMASDKIMPL_H_

#ifndef DLL_COMPILED
#define DLL_INTERNAL __declspec( dllexport )
#endif 

#define NO_DEVICES          0
#define KEYBOARD_DEVICES    1
#define MOUSEMAT_DEVICES    2
#define MOUSE_DEVICES       3
#define HEADSET_DEVICES     4
#define KEYPAD_DEVICES      5
#define ALL_DEVICES         6

const UINT MAX_UNIVERSE = 8;
const UINT MAX_PROPERTY = 512;

class Chroma {
public:
	Chroma();
	~Chroma();
	BOOL initialize();

	//void resetEffects(UINT DeviceType);
	BOOL command(e131_packet_t* packet);
	BOOL print(e131_packet_t* packet);

	//BOOL isDeviceConnected(RZDEVICEID DeviceId);
	std::set<UINT> universeSet;

private:
	HMODULE m_ChromaSDKModule;
	BOOL mapPixels();
	void updateColor(UINT colorComponent, UINT color, COLORREF* led);
	typedef struct {
		UINT deviceType;
		UINT row;
		UINT col;
		UINT count;
		UINT color;
		const char* name;
	} mapping_t;
	typedef std::vector<mapping_t> propertySet_t;
	std::map<std::pair<UINT,UINT>, propertySet_t> propertyNumMap;

	ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE keyboard_effect = {};
	ChromaSDK::Mouse::CUSTOM_EFFECT_TYPE mouse_effect = {};
	ChromaSDK::Mousepad::CUSTOM_EFFECT_TYPE mousemat_effect = {};
	ChromaSDK::Headset::CUSTOM_EFFECT_TYPE headset_effect = {};
	ChromaSDK::Keypad::CUSTOM_EFFECT_TYPE keypad_effect = {};
};

#endif

