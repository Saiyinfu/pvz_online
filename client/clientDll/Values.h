#pragma once
#include <string>

class Values
{
public:
	// Sun
	static BOOL GetSunValue(int32_t& sun);
	static BOOL SetSunValue(int32_t sun);

	static BOOL GetGameObjAddr(PVOID& obj);

	// Zombies
	static BOOL GetZombieNum(int32_t& nZombie);
	static BOOL GetZombieId(int32_t index, int32_t& id);

	// Plants
	static BOOL GetPlantNum(int32_t& nPlant);

	// Archives
	static BOOL GetPvzUserName(LPSTR userName, size_t len);
	static BOOL GetPvzUserName(std::string& name);
	static BOOL GetPvzUserNameLenth(size_t& lenth);
	static BOOL GetNowLevel(int& level);
	static BOOL SetNowLevel(int level);
};

