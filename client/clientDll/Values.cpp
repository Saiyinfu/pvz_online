#include "pch.h"
#include "Values.h"
#include "Utils.h"
#include "Consts.h"

BOOL Values::GetSunValue(int32_t& sun)
{
	int _sun;
	PVOID offsets[] = { (PVOID)0x768, (PVOID)0x5560 };
	BOOL r = ReadOffsetsPointerMemoryArr(_sun, (PVOID)0x6a9ec0, offsets);
	if (r) {
		sun = _sun;
	}
	return r;
}

BOOL Values::GetGameObjAddr(PVOID& obj) {
	PVOID _obj;
	PVOID offsets[] = { (PVOID)0x768 };
	BOOL r = ReadOffsetsPointerMemoryArr(_obj, (PVOID)0x6a9ec0, offsets);
	if (r) {
		obj = _obj;
	}
	return r;
}

BOOL Values::GetZombieNum(int32_t& nZombie) {
	int zombie_num;
	PVOID offsets[] = { (PVOID)0x768, (PVOID)0xa0 };
	BOOL r = ReadOffsetsPointerMemoryArr(zombie_num, BASE_ADDR, offsets);
	if (r) {
		nZombie = zombie_num;
	}
	return r;
}

BOOL Values::SetSunValue(int32_t sun) {
	PVOID offsets[] = { (PVOID)0x768, (PVOID)0x5560 };
	BOOL r = WriteOffsetsPointerMemoryArr(sun, (PVOID)0x6a9ec0, offsets);
	return r;
}

BOOL Values::GetPlantNum(int32_t& nPlant) {
	int num;
	PVOID offsets[] = { (PVOID)0x768, (PVOID)0xbc };
	BOOL r = ReadOffsetsPointerMemoryArr(num, BASE_ADDR, offsets);
	if (r) {
		nPlant = num;
	}
	return r;
}


BOOL Values::GetZombieId(int32_t index, int32_t& id) {
	int _id;
	PVOID offsets[] = { (PVOID)0x768, (PVOID)0x90, (PVOID)(0x158 + index * ZOMBIE_OBJ_SIZE) };
	BOOL r = ReadOffsetsPointerMemoryArr(_id, BASE_ADDR, offsets);
	if (r) {
		id = _id;
	}
	return r;
}

BOOL Values::GetNowLevel(int& level) {
	int _level;
	PVOID offsets[] = { (PVOID)0x82c, (PVOID)0x24 };
	BOOL r = ReadOffsetsPointerMemoryArr(_level, BASE_ADDR, offsets);
	if (r) {
		level = _level;
	}
	return r;
}

BOOL Values::SetNowLevel(int level) {
	PVOID offsets[] = { (PVOID)0x82c, (PVOID)0x14 };
	BOOL r = WriteOffsetsPointerMemoryArr(level, BASE_ADDR, offsets);
	return r;
}

BOOL Values::GetPvzUserNameLenth(size_t& lenth) {
	int len;
	PVOID offsets[] = { (PVOID)0x82c, (PVOID)0x14 };
	BOOL r = ReadOffsetsPointerMemoryArr(len, BASE_ADDR, offsets);
	if (r) {
		lenth = len;
	}
	return r;
}

BOOL Values::GetPvzUserName(LPSTR userName, size_t len) {
	char* name = (char*)_malloca(len+1);
	PVOID offsets[] = { (PVOID)0x82c, (PVOID)0x4 };
	//BOOL r = ReadOffsetsPointerMemoryArr(name, BASE_ADDR, offsets);
	BOOL r = ReadOffsetsPointerMemoryArr(name, len + 1, BASE_ADDR, 2, offsets);
	if (r && name != NULL) {
		strcpy_s(userName, len + 1, name);
	}
	_freea(name);
	return r;
}

BOOL Values::GetPvzUserName(std::string& name) {
	size_t len;
	if (!GetPvzUserNameLenth(len)) {
		return FALSE;
	}

	char* cname = (char*)_malloca(len + 1);
	if (!cname || !GetPvzUserName(cname, len)) {
		return FALSE;
	}

	name = cname;
	_freea(cname);
	return TRUE;
}

