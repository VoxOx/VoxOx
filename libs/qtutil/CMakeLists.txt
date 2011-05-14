ow_create_static_library(owqtutil)

ow_use_public_libraries(
	owthread
	qt4
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Because of UIC and MOC
ow_add_private_include_dirs(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)

qt4_wrap_cpp(${PROJECT_NAME}_SRCS
	include/qtutil/EventFilter.h
	include/qtutil/QObjectThreadSafe.h
	include/qtutil/NonResizeableWidget.h
	include/qtutil/WengoStyleLabel.h
	include/qtutil/ToolTipLineEdit.h
	include/qtutil/ClickableLabel.h
	include/qtutil/Wizard.h
	include/qtutil/ThreadEventFilter.h
	include/qtutil/AnimatedButton.h
	include/qtutil/UpQComboBox.h
	include/qtutil/LinkLabel.h
	include/qtutil/VoxOxLinkLabel.h
	include/qtutil/VoxOxClickableLabel.h
	include/qtutil/VoxOxFrame.h
	include/qtutil/VoxOxToolTipLineEdit.h
	include/qtutil/VoxoxErrorMessageLabel.h
	
	include/qtutil/PictureFlow.h
	

	
	include/qtutil/ImageButton.h

	include/qtutil/ScrollFrame.h
	include/qtutil/ImageSelector.h
	include/qtutil/DropDownWindow.h
	include/qtutil/ClearLineEdit.h
	

	src/QtThumbnailManager.h
	src/QtThumbnailDirModel.h
)

if (WIN32)	
qt4_wrap_cpp(${PROJECT_NAME}_SRCS
include/qtutil/hunspell/affentry.hxx
include/qtutil/hunspell/affixmgr.hxx
include/qtutil/hunspell/atypes.hxx
include/qtutil/hunspell/baseaffix.hxx
include/qtutil/hunspell/csutil.hxx
include/qtutil/hunspell/dictmgr.hxx
include/qtutil/hunspell/filemgr.hxx
include/qtutil/hunspell/hashmgr.hxx
include/qtutil/hunspell/htypes.hxx
include/qtutil/hunspell/hunspell.hxx
include/qtutil/hunspell/hunspell.h
include/qtutil/hunspell/hunzip.hxx
include/qtutil/hunspell/langnum.hxx
include/qtutil/hunspell/phonet.hxx
include/qtutil/hunspell/suggestmgr.hxx
include/qtutil/hunspell/w_char.hxx

include/qtutil/SpellTextEdit.h
include/qtutil/highlighter.h
)
endif (WIN32)

if (APPLE)
	qt4_wrap_cpp(${PROJECT_NAME}_SRCS
		include/qtutil/MacAddressBookImporter.h
	)
endif(APPLE)

qt4_wrap_ui(${PROJECT_NAME}_SRCS
	src/Wizard.ui
	src/ImageSelector.ui
)

qt4_add_resources(${PROJECT_NAME}_SRCS
	qtutil.qrc
)

ow_add_sources(
	src/EventFilter.cpp
	src/KeyEventFilter.cpp
	src/MouseEventFilter.cpp
	src/PaintEventFilter.cpp
	src/EnabledChangeEventFilter.cpp
	src/WidgetFactory.cpp
	src/Object.cpp
	src/QObjectThreadSafe.cpp
	src/ResizeEventFilter.cpp
	src/NonResizeableWidget.cpp
	src/StringListConvert.cpp
	src/Widget.cpp
	src/WengoStyleLabel.cpp
	src/ToolTipLineEdit.cpp
	src/ClickableLabel.cpp
	src/WidgetBackgroundImage.cpp
	src/Wizard.cpp
	src/ThreadEventFilter.cpp
	src/PixmapMerging.cpp
	src/SafeConnect.cpp
	src/LanguageChangeEventFilter.cpp
	src/CloseEventFilter.cpp
	src/AnimatedButton.cpp
	src/DesktopService.cpp
	src/UpQComboBox.cpp
	src/LinkLabel.cpp
	src/VoxoxErrorMessageLabel.cpp
	src/VoxOxLinkLabel.cpp
	src/VoxOxClickableLabel.cpp
	src/VoxOxFrame.cpp
	src/VoxOxToolTipLineEdit.cpp
	src/ImageButton.cpp
	src/ScrollFrame.cpp
	src/ImageSelector.cpp
	src/ThumbnailView.cpp
	src/QtThumbnailDirModel.cpp
	src/QtThumbnailManager.cpp
	src/WidgetRatioEnforcer.cpp
	src/WidgetUtils.cpp
	src/ExpandButton.cpp
	src/OsdButton.cpp
	src/FSResourceFileEngineHandler.cpp
	src/KeywordTranslator.cpp
	src/DropDownWindow.cpp
	src/IconLineEdit.cpp
	src/ClearLineEdit.cpp
	src/PictureFlow.cpp
)

if (WIN32)
ow_add_sources(

	include/qtutil/hunspell/affentry.cxx
	include/qtutil/hunspell/affixmgr.cxx
	include/qtutil/hunspell/csutil.cxx
	include/qtutil/hunspell/dictmgr.cxx
	include/qtutil/hunspell/filemgr.cxx
	include/qtutil/hunspell/hashmgr.cxx
	include/qtutil/hunspell/hunspell.cxx
	include/qtutil/hunspell/hunzip.cxx
	include/qtutil/hunspell/phonet.cxx
	include/qtutil/hunspell/suggestmgr.cxx
	include/qtutil/hunspell/utf_info.cxx

	src/highlighter.cpp
	src/SpellTextEdit.cpp
)
endif (WIN32)


if (APPLE)
	ow_add_sources(
		src/MacDesktopService.cpp
		src/Mac_Private.mm
		src/MacSpellChecker.mm
		src/GrowlNotifier.cpp
		src/MacAddressBookImporter.cpp
	)
	qt4_wrap_cpp(${PROJECT_NAME}_SRCS
		include/qtutil/MacDesktopService.h
		include/qtutil/Mac_Private.h
		include/qtutil/MacSpellChecker.h
		include/qtutil/GrowlNotifier.h		
	)
	ow_add_private_include_dirs(
		/Developer/Headers/FlatCarbon/
	)
	ow_use_public_frameworks(
		Cocoa
		Carbon
	)
endif (APPLE)

if (WIN32)
	if (MINGW)
		ow_add_private_definitions(
			-DWINVER=0x500
			-D_WIN32_IE=0x500
		)
	endif (MINGW)

	ow_add_sources(
		src/WindowsDesktopService.cpp
	)
	qt4_wrap_cpp(${PROJECT_NAME}_SRCS
		include/qtutil/WindowsDesktopService.h
	)
endif (WIN32)

if (LINUX)
	ow_use_private_libraries(
		x11
	)
endif (LINUX)

subdirs(
	tests
)

ow_create_project_binary()
