#pragma once
#ifndef _CHROMAIMPL_H_
#define _CHROMAIMPL_H_

#define NO_DEVICES          0
#define KEYBOARD_DEVICES    1
#define MOUSEMAT_DEVICES    2
#define MOUSE_DEVICES       3
#define MOUSE_DEVICES2      4
#define HEADSET_DEVICES     5
#define KEYPAD_DEVICES      6
#define ALL_DEVICES			7

const unsigned int MAX_UNIVERSE = 8;
const unsigned int MAX_PROPERTY = 512;

class Chroma {
public:
	Chroma();
	~Chroma();
	int initialize();
	void startThread();
	void chromaThread();
	void effectThread();

	//void resetEffects(UINT DeviceType);
	int command(e131_packet_t* packet);
	int print(e131_packet_t* packet);

	//BOOL isDeviceConnected(RZDEVICEID DeviceId);
	std::set<unsigned int> universeSet;
	e131_packet_t packet;

private:
	int mapPixels();
	void updateColor(unsigned int colorComponent, unsigned int color, unsigned long* led);
	typedef struct {
		unsigned int deviceType;
		unsigned int row;
		unsigned int col;
		unsigned int color;
		const char* name;
	} mapping_t;
	typedef std::vector<mapping_t> propertySet_t;
	std::map<std::pair<unsigned int, unsigned int>, propertySet_t> propertyNumMap;

	ChromaSDK::CUSTOM_EFFECT_TYPE all_effect = {};

	ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE keyboard_effect = {};
	ChromaSDK::Mouse::CUSTOM_EFFECT_TYPE mouse_effect = {};
	ChromaSDK::Mouse::CUSTOM_EFFECT_TYPE2 mouse_effect2 = {};
	ChromaSDK::Mousepad::CUSTOM_EFFECT_TYPE mousemat_effect = {};
	ChromaSDK::Headset::CUSTOM_EFFECT_TYPE headset_effect = {};
	ChromaSDK::Keypad::CUSTOM_EFFECT_TYPE keypad_effect = {};
};

extern int bDebug;

#endif
