package org.pvzonline.plugins

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

@Serializable
data class BaseStruct(
    val type: String,
    val id: Int,
    @SerialName("user_name") val userName: String
)

@Serializable
data class LogStruct(
    val type: String,
    val id: Int,
    @SerialName("user_name") val userName: String,
    val content: String,
    val level: String
)

@Serializable
data class PlacePlantStruct(
    val type: String,
    val id: Int,
    @SerialName("user_name") val userName: String?,
    @SerialName("plant_type") val plantType: Int,
    val line: Int,
    val row: Int
)

@Serializable
data class SetIdStruct(
    val type: String,
    val id: Int
)