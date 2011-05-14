ow_create_static_library(owpurplewrapper)

ow_use_public_libraries(
	owimwrapper
	libpurple
)

ow_use_private_libraries(
	glib2
	owcutil
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

if (UNIX)
	ow_add_private_definitions(
		-DENABLE_NLS
	)
endif (UNIX)

ow_add_sources(
	PurpleIMFactory.cpp
	PurpleIMChat.cpp
	PurpleIMConnect.cpp
	PurpleIMPresence.cpp
	PurpleIMContactList.cpp
	PurpleChatMngr.cpp
	PurpleConnectMngr.cpp
	PurplePresenceMngr.cpp
	PurpleAccountMngr.cpp
	PurpleContactListMngr.cpp
	PurpleEnumPresenceState.cpp
	PurpleEnumIMProtocol.cpp
)

ow_create_project_binary()
