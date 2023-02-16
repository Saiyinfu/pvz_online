#include "pch.h"

#include "Communication.h"
#include "Utils.h"
#include "json.hpp"

#include <sstream>
#include <iostream>
#include <iomanip>
#include "Functions.h"
#include "Values.h"

#pragma comment(lib, "ixwebsocket.lib")
#pragma comment(lib, "crypt32.lib")


using json = nlohmann::json;
using wss = std::wstringstream;

ix::WebSocket Communication::m_webSocket;
std::atomic_bool Communication::m_isConnected = false;
int32_t Communication::m_playerId = 0;

BOOL Communication::PlacePlant(int line, int row, int type) {
	json j = {
		{"type", "place_plant" },
		{"line", line },
		{"row", row },
		{"plant_type", type }
	};
	SendPackage(j);
	return TRUE;
}

BOOL Communication::GenerateZombie(int line, int type) {
	return TRUE;
}

BOOL Communication::SendLog(std::wstring str, std::string level) noexcept {
	json j = {
		{"type", "log" },
		{"level",  level },
		{"content", wstring2utf8(str) }
	};
	SendPackage(j);
	return TRUE;
}

BOOL Communication::SendPackage(json j) {
	std::string user_name;
	Values::GetPvzUserName(user_name);
	json head = {
		{ "id", m_playerId },
		{ "user_name",  user_name }, 
	};
	for (auto& i : head.items()) {
		j += {i.key(), i.value()};
	}
	if (m_isConnected) {
		FormatOutputDebugStringW(utf82wstring(j.dump()));
		m_webSocket.send(j.dump());
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void Communication::MessageCallback(const ix::WebSocketMessagePtr& msg) {
	if (msg->type == ix::WebSocketMessageType::Message) {
		if (msg->binary) {
			std::wstringstream log_str;
			log_str << L"Binary: ";
			for (auto i : msg->str) {
				log_str << std::setw(2) << std::setfill(L'0') << std::hex << (int)((unsigned char)i) << L" ";
			}
			FormatOutputDebugString(L"%s", log_str.str().c_str());
		}
		try {
			json json_data = json::parse(msg->str);
			if (!json_data["type"].is_string()) throw std::runtime_error("type is bad");
			std::wstring type = utf82wstring(json_data["type"]);
			StreamOutputDebugStringW(wss() << L"type = " << type);
			//json类型选择器
			if (type.compare(L"place_plant") == 0) {
				int line = json_data["line"];
				int row = json_data["row"];
				int plant_type = json_data["plant_type"];
				PlacePlantCallback(line, row, plant_type);
			}
			else if (type.compare(L"set_id") == 0) {
				m_playerId = json_data["id"];
			}
		}
		catch (const std::exception& e) {
			FormatOutputDebugString(L"WebSocket Internal Error: %s", string2wstring(e.what()).c_str());
		}
	}
	else if (msg->type == ix::WebSocketMessageType::Open) {
		FormatOutputDebugString(L"Connected!!");
		SendLog(L"Connected!!", LogLevel::INFO);
		std::thread th(
			[] {
				Sleep(500);
		        SendLog(L"Hello", LogLevel::INFO);
			}
		);
		th.detach();
		m_isConnected = true;
	}
	else if (msg->type == ix::WebSocketMessageType::Error) {
		StreamOutputDebugStringW(wss() << "Error: " << string2wstring(msg->errorInfo.reason));
	}
	else if (msg->type == ix::WebSocketMessageType::Close) {
		FormatOutputDebugString(L"Disconnected!! Reason: %s", utf82wstring(msg->closeInfo.reason).c_str());
		m_isConnected = false;
	}
}

BOOL Communication::ConnectServer(std::string url) {
	if (m_isConnected) return FALSE;
	ix::initNetSystem();
	m_webSocket.setUrl(url);
	m_webSocket.setOnMessageCallback(MessageCallback);
	m_webSocket.start();
	return TRUE;
}

BOOL Communication::Disconnect() {
	if (!m_isConnected.load()) return FALSE;
	m_webSocket.close();
	ix::uninitNetSystem();
	return TRUE;
}

bool Communication::isConnected() {
	return m_isConnected;
}

int32_t Communication::GetPlayerId() {
	return m_playerId;
}

BOOL Communication::PlacePlantCallback(int line, int row, int type) {
	Functions::PlacePlant(line, row, type);
	return TRUE;
}

BOOL Communication::GenerateZombieCallback(int line, int type) {
	return 0;
}
