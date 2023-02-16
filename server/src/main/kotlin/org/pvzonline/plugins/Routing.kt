package org.pvzonline.plugins

import io.ktor.http.*
import io.ktor.server.application.*
import io.ktor.server.response.*
import io.ktor.server.routing.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json

fun Application.configureRouting() {
    routing {
        get("/") {
            call.respondText("欢迎来到PVZ在线服务器")
        }
        get("/log") {
            call.respondBytes(withContext(Dispatchers.IO) {
                javaClass.classLoader.getResourceAsStream("static/log_page.html")?.readAllBytes() ?: byteArrayOf()
            })
        }
        route("api"){
            get("get_player_list") {
                call.respondText(Json.encodeToString(playerList.map { it.getSerializable() }))
            }
            post("place_plant") {
                try {
                    val params = call.request.queryParameters
                    val line = params["line"]!!.toInt()
                    val row = params["row"]!!.toInt()
                    val type = params["type"]!!.toInt()
                    val id = params["id"]!!.toInt()
                    placePlant(id, line, row, type)
                } catch (e: Exception){
                    call.respondText(e.stackTraceToString(), status = HttpStatusCode.BadRequest)
                }

            }
        }
    }
}
