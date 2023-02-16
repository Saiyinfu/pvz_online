package org.pvzonline.plugins

import io.ktor.server.websocket.*
import io.ktor.websocket.*
import java.time.Duration
import io.ktor.server.application.*
import io.ktor.server.routing.*
import kotlinx.coroutines.channels.ClosedReceiveChannelException
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import org.pvzonline.logI
import java.util.Date

val logSessions = mutableListOf<DefaultWebSocketSession>()
val logList = mutableListOf<String>()
val playerList = mutableListOf<Player>()

suspend fun sendWebLog(msg: String, level: String, userName: String? = null, id: Int? = null){
    val logText = "[${Date()}][${userName?:"No user name"}][${id?:"No id"}][${level}] " + msg
    logList += logText
    logSessions.forEach {
        it.send(logText)
    }
}

fun Application.configureSockets() {
    install(WebSockets) {
        pingPeriod = Duration.ofSeconds(15)
        timeout = Duration.ofSeconds(15)
        maxFrameSize = Long.MAX_VALUE
        masking = false
    }
    routing {
        webSocket("/pvzonline") {
            var player: Player? = null
            try {
                for (frame in incoming){
                    when(frame) {
                        is Frame.Text -> {
                            val text = frame.readText()
                            logI(text)
                            val json = Json {
                                ignoreUnknownKeys = true
                            }
                            when(json.decodeFromString<BaseStruct>(text).type) {
                                "log" -> {
                                    val logStruct = Json.decodeFromString<LogStruct>(text)
                                    sendWebLog(logStruct.content, logStruct.level, logStruct.userName, logStruct.id)
                                }
                            }
                            if (json.decodeFromString<BaseStruct>(text).id == 0 || playerList.find { player1 -> player1.id == json.decodeFromString<BaseStruct>(text).id } == null) { //新玩家，分配Id
                                player = Player(json.decodeFromString<BaseStruct>(text).userName, this)
                                playerList += player
                                send(Json.encodeToString(SetIdStruct("set_id", player.id)))
                                sendWebLog("玩家<${player.userName}>加入, 已分配id:${player.id}", "info", "Server")
                            }

                        }
                        is Frame.Binary -> {

                        }
                        else -> {

                        }
                    }
                }
            } catch (e: ClosedReceiveChannelException) {
                println("Closed!!!")
            } catch (e: Throwable) {
                System.err.println("Error: ${e.message}")
                e.printStackTrace()
                //throw e
            }
            sendWebLog("玩家<${player?.userName}>断开连接, id:${player?.id}", "info", "Server")
        }
        webSocket("/games_log") {
            logSessions += this
            try {
                for (frame in incoming){
                    when(frame) {
                        is Frame.Text -> {
                            val text = frame.readText()
                            if (text == "refresh") {
                                logList.forEach {
                                    send(it)
                                }
                            }
                        }
                        else -> {

                        }
                    }
                }
            } catch (e: ClosedReceiveChannelException) {
                println("Closed!!!")
                logSessions.remove(this)
            } catch (e: Throwable) {
                System.err.println("Error :$e")
            }
        }
    }
}
