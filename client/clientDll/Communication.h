#pragma once

#include "pch.h"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>

#include "json.hpp"

namespace LogLevel {
	const std::string VERBOSE = u8"verbose";
	const std::string INFO = u8"info";
	const std::string WARN = u8"warn";
	const std::string ERR = u8"error";
};

class Communication
{
public:
	
	BOOL PlacePlant(int line, int row, int type);
	BOOL GenerateZombie(int line, int type);
	static BOOL SendLog(std::wstring str, std::string level) noexcept;
	static BOOL ConnectServer(std::string url);
	static BOOL Disconnect();
	static bool isConnected();
	static int32_t GetPlayerId();

private:
	static ix::WebSocket m_webSocket;
	static std::atomic_bool m_isConnected;
	static int32_t m_playerId;
	static BOOL SendPackage(nlohmann::json j);
	static void MessageCallback(const ix::WebSocketMessagePtr& msg);
	static BOOL PlacePlantCallback(int line, int row, int type);
	static BOOL GenerateZombieCallback(int line, int type);
};

