ow_create_static_library(owsettings)

ow_use_public_libraries(
	owserialization
	owthread
)

ow_use_private_libraries(
	tinyxml
)

ow_add_sources(
	src/Settings.cpp
	src/CascadingSettings.cpp
	src/StringListXMLSerializer.cpp
	src/SettingsXMLSerializer.cpp
	src/CascadingSettingsXMLSerializer.cpp
)

ow_create_project_binary()

subdirs(
	tests
	)
