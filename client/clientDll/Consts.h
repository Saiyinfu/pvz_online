#pragma once
#include <Windows.h>

const PVOID BASE_ADDR = (PVOID)0x6a9ec0;
const size_t ZOMBIE_OBJ_SIZE = 0x15c;
// TODO
const LPCSTR ZOMBIE_NAMES[] = {
	"普通僵尸",
	"旗帜僵尸",
	"路障僵尸",
	"铁桶僵尸",
	"报纸僵尸",
	"铁门僵尸",
	"橄榄僵尸",
	"舞王僵尸",
	"伴舞僵尸",
	"鸭子僵尸",
	"潜水僵尸",
	"冰车僵尸"
};

//Functions

const PVOID fpPlacePlant = (PVOID)0x40d130;
const PVOID fpGenerateZombie = (PVOID)0x40DDD0;
