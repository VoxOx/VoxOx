ow_create_static_library(owserializer)

ow_use_public_libraries(
	owserialization
)

ow_use_private_libraries(
	tinyxml
	json					#VOXOX - JRT - 2009.06.26
)

ow_add_sources(
	src/DateXMLSerializer.cpp
	src/TimeXMLSerializer.cpp
	src/VoxXMLSerializerBase.cpp		#VOXOX - JRT - 2009.03.31
	src/VoxJSONSerializerBase.cpp		#VOXOX - JRT - 2009.06.26
	src/VoxQueryResultXMLSerializer.cpp	#VOXOX - JRT - 2009.06.01
)

ow_create_project_binary()
