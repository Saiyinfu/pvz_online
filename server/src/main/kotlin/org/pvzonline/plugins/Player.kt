package org.pvzonline.plugins

import io.ktor.websocket.*
import kotlinx.serialization.Serializable
import kotlin.random.Random

class Player(val userName: String, val session: DefaultWebSocketSession) {
    val id: Int

    init {
        id = Random.nextInt(1, Int.MAX_VALUE)
    }
    @Serializable
    data class SerializablePlayer(val id: Int, val userName: String)
    fun getSerializable(): SerializablePlayer {
        return SerializablePlayer(id, userName)
    }
}