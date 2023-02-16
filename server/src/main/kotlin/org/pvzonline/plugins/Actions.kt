package org.pvzonline.plugins

import io.ktor.websocket.*
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json

suspend fun placePlant(id: Int, line: Int, row: Int, type: Int){
    val jsonString = Json.encodeToString(Json.encodeToString(PlacePlantStruct("place_plant", id, null, type, line, row)))
    playerList.find { player -> player.id == id }?.session?.send(jsonString)
}