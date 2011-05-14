ow_create_static_library(owgaimwrapper)

ow_use_public_libraries(
	owimwrapper
	gaim
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
	GaimIMFactory.cpp
	GaimIMChat.cpp
	GaimIMConnect.cpp
	GaimIMPresence.cpp
	GaimIMContactList.cpp
	GaimChatMngr.cpp
	GaimConnectMngr.cpp
	GaimPresenceMngr.cpp
	GaimAccountMngr.cpp
	GaimContactListMngr.cpp
	GaimEnumPresenceState.cpp
	GaimEnumIMProtocol.cpp
)

ow_create_project_binary()
