set(BINARY_NAME ${APPLICATION_NAME})
string(REPLACE " " "" BINARY_NAME ${BINARY_NAME})
if (LINUX)
	string(TOLOWER ${BINARY_NAME} BINARY_NAME)
endif (LINUX)
ow_create_executable(${BINARY_NAME})

configure_file(buildconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/buildconfig.h)

subdirs(
	lang
)

ow_use_private_libraries(
	owphone
	owbuildid
	owqtutil
	pixertool
	webcam
	psiidle
	owtrayicon
	owbrowser
	owsoftupdater
)

if (SIPWRAPPER_BACKEND_SIPX)
	ow_add_private_definitions(
		-DSIPXWRAPPER
	)

	ow_use_private_libraries(
		sipxwrapper
	)
else (SIPWRAPPER_BACKEND_SIPX)
if (SIPWRAPPER_BACKEND_PHAPI)
	ow_add_private_definitions(
		-DPHAPIWRAPPER
	)

	ow_use_private_libraries(
		owphapiwrapper
	)
else (SIPWRAPPER_BACKEND_PHAPI)
if (IMWRAPPER_BACKEND_MULTI)
	ow_add_private_definitions(
		-DMULTIIMWRAPPER
	)

	ow_use_private_libraries(
		owmultiimwrapper
	)
endif (IMWRAPPER_BACKEND_MULTI)
endif (SIPWRAPPER_BACKEND_PHAPI)
endif (SIPWRAPPER_BACKEND_SIPX)

ow_add_private_include_dirs(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)

qt4_wrap_cpp(${PROJECT_NAME}_SRCS
	QtWengoPhone.h	
	QtHttpProxyLogin.h
	QtIdle.h
	QtLanguage.h
	QtToolBar.h
	QtVoxWindowManager.h
	QtEnumWindowType.h
	QtVoxWindowInfo.h
	QtSystray.h
	QtFactory.h
	QtPresenceMenuManager.h
	QtPresenceIMMenuManager.h
	QtPresenceSIPMenuManager.h
	QtBrowserWidget.h
	QtIMAccountMonitor.h
	QtNoWengoAlert.h
	
	QtEnumPhoneType.h
	QtEnumEmailType.h
	
	dialpad/QtMiniDialpad.h
	dialpad/QtDialpad.h
	dialpad/QtSVGDialpad.h

	phoneline/QtPhoneLine.h

	phonecall/QtPhoneCall.h
	phonecall/QtVideoQt.h
	phonecall/QtContactCallListWidget.h
	phonecall/QtPhoneCallEventFilter.h
	phonecall/QtAudioSmileysWindow.h

	toaster/QtChatToaster.h
	toaster/QtCallToaster.h
	toaster/QtToaster.h

	login/ILogin.h
	login/QtLoginDialog.h
	login/QtLogAccount.h
	login/QtAddAccount.h
	login/QtAddWengoAccount.h
	login/QtAddSIPAccount.h
	login/QtAddVoxOxAccount.h
	login/QtLoginMessage.h
	login/QtVoxOxLogin.h
	login/QtVoxOxLoginMessage.h
	login/QtVoxOxLoadingLogin.h
	
	messagebox/QtVoxMessageBox.h
	messagebox/QtVoxPhoneNumberMessageBox.h
	messagebox/QtVoxEmailMessageBox.h
	messagebox/QtVoxTextMessageBox.h

	profile/QtProfileDetails.h
	profile/QtUserProfile.h
	profile/QtUserProfileHandler.h

	imcontact/QtAdvancedIMContactManager.h
	imcontact/QtAddIMContact.h
	imcontact/QtSimpleIMContactManager.h
	imcontact/QtIMContactManagerInterface.h
	imcontact/QtFastAddIMContact.h
	

	imaccount/QtIMAccountManager.h
	imaccount/QtIMAccountSettings.h
	imaccount/QtIMAccountPlugin.h
	imaccount/QtMSNSettings.h
	imaccount/QtMySpaceSettings.h
	imaccount/QtFacebookSettings.h
	imaccount/QtTwitterSettings.h
	imaccount/QtSkypeSettings.h
	imaccount/QtYahooSettings.h
	imaccount/QtAIMSettings.h
	imaccount/QtICQSettings.h
	imaccount/QtJabberSettings.h
	imaccount/QtGoogleTalkSettings.h

	contactlist/QtContactList.h
	contactlist/ContactGroupPopupMenu.h
	contactlist/QtContactWidget.h
	contactlist/QtContactTooltip.h
	contactlist/QtContactTooltipManager.h
	contactlist/QtContactActionManager.h
	contactlist/QtContactTreeMouseFilter.h
	contactlist/QtContactManager.h
	contactlist/QtContact.h
	contactlist/QtTreeViewDelegate.h
	contactlist/QtContactTreeKeyFilter.h
	contactlist/QtRenameGroup.h
	contactlist/QtConferenceAction.h
	contactlist/QtContactListStyle.h
	contactlist/VoxContactTreeWidget.h	#VOXOX - JRT - 2009.05.13
	contactlist/QtContactNetworkMenu.h

	conference/QtConferenceCallWidget.h

	chat/emoticons/QtEmoticonsWidget.h
	chat/emoticons/QtEmoticon.h
	chat/emoticons/QtEmoticonButton.h
	chat/emoticons/QtEmoticonsManager.h
	chat/chatroom/QtChatRoomInviteDlg.h
	chat/chatroom/QtChatRoomTreeWidgetItem.h
	chat/chatroom/QtChatRoomListWidgetItem.h
	chat/translation/QtTranslationManager.h
	chat/translation/QtTranslationLanguage.h
	chat/translation/QtTranslationWidget.h
	chat/translation/QtEnumTranslationMode.h
	chat/QtEnumUMMode.h
	chat/QtUMBase.h
	chat/QtUMTranslation.h
	chat/QtUMTranslationRequest.h
	chat/QtChatHandler.h
	chat/QtUMChat.h
	chat/QtUMEmail.h
	chat/QtUMSMS.h
	chat/QtUMSendFile.h
	chat/QtUMFax.h
	chat/QtContactProfileWidget.h
	chat/QtUMCall.h
	chat/QtPopUpChatDialog.h
	chat/QtPopUpChatManager.h
	chat/QtChatEditWidget.h
	chat/QtChatHistoryWidget.h
	chat/QtWengoStyleBar.h
	chat/QtChatUtils.h
	chat/QtChatToEmailSMSUtils.h
	chat/QtChatLogViewer.h
	chat/QtChatTheme.h
	chat/QtUMWindow.h
	chat/QtUMHelper.h
	chat/QtChatToEmailMessageData.h
	chat/QtChatToSMSMessageData.h
	chat/umtreewidget/QtUMItem.h 
	chat/umtreewidget/QtUMGroup.h
	chat/umtreewidget/QtUMItemList.h
	chat/umtreewidget/QtUMItemListManager.h
	chat/umtreewidget/QtUMTreeWidget.h
	chat/umtreewidget/QtUMTreeViewDelegate.h
	chat/umtreewidget/QtUMItemInfo.h
	chat/umtreewidget/QtUMItemManager.h
	chat/umtreewidget/QtUMItemListStyle.h
		
	
	webbrowser/QtWebBrowser.h
	webbrowser/QtVoxBrowser.h
	
	webservices/sms/QtSms.h
	webservices/softupdate/QtSoftUpdate.h
	webservices/softupdate/QtSoftUpdatePrompt.h			#VOXOX - JRT - 2009.10.28
	webservices/softupdate/QtBrowserSoftUpdate.h

	callbar/QtCallBar.h
	callbar/QtPhoneComboBox.h
	callbar/QtVoxOxCallBarWidget.h
	callbar/customCallBar/QtVoxOxCallBarFrame.h
	callbar/customCallBar/callBarUtilities/QtFlag.h
	callbar/customCallBar/callBarUtilities/QtFlagsListWidget.h
	callbar/customCallBar/callBarUtilities/QtFlagsManager.h

	statusbar/QtStatusBar.h

	config/QtAdvancedConfig.h
	config/QtAccountSettings.h
	config/QtAdvancedSettings.h
	config/QtAppearanceSettings.h
	config/QtPhoneSettings.h
	config/QtTextSettings.h
	config/QtMyAccountSettings.h
	config/QtAudioSettings.h
	config/QtCallForwardSettings.h
	config/QtGeneralSettings.h
	config/QtNotificationSettings.h
	config/QtEventsSettings.h
	config/QtPrivacySettings.h
	config/QtVideoSettings.h
	config/QtLanguagesSettings.h
	config/QtWengoConfigDialog.h
	config/QtVoicemailSettings.h
	config/QtSecuritySettings.h
	
	wizard/QtIWizard.h
	wizard/QtWizardDialog.h
	wizard/QtWizardProgress.h
	wizard/QtWizardAudio.h
	wizard/QtWizardVideo.h
	wizard/QtWizardSelectNetworks.h
	wizard/QtWizardIMAccountContainer.h
	wizard/QtWizardProfile.h
	wizard/QtWizardPhones.h
	wizard/QtWizardPA.h

	profilebar/QtEventWidget.h
	profilebar/QtIMProfileWidget.h
	profilebar/QtProfileBar.h
	profilebar/QtCreditWidget.h
	profilebar/QtIMAccountPresenceMenuManager.h

	history/QtHistoryWidget.h
	history/QtHistory.h

	filetransfer/QtFileTransfer.h
	filetransfer/QtFileTransferAcceptDialog.h
	filetransfer/QtFileTransferWidget.h
	filetransfer/QtFileTransferItem.h
	filetransfer/QtFileTransferDownloadItem.h
	filetransfer/QtFileTransferUploadItem.h
	filetransfer/QtFileTransferNotifyDialog.h

	webdirectory/QtWebDirectory.h
)

ow_add_sources(
	stdafx.cpp			#VOXOX - JRT - 2009.04.02 for Precompiled Headers
	../main.cpp
	QtWengoPhone.cpp	
	QtAbout.cpp
	QtHttpProxyLogin.cpp
	QtIdle.cpp
#	QtEnumIMProtocol.cpp		#VOXOX - JRT - moved to imwrapper
	QtLanguage.cpp
	QtToolBar.cpp
	QtVoxWindowManager.cpp
	QtVoxWindowInfo.cpp
	QtEnumWindowType.cpp
	QtSystray.cpp
	QtFactory.cpp
	QtPresenceMenuManager.cpp
	QtPresenceIMMenuManager.cpp
	QtPresenceSIPMenuManager.cpp
	QtWengoStyle.cpp
	QtBrowserWidget.cpp
	QtIMAccountMonitor.cpp
	QtPresencePixmapHelper.cpp
	QtIMAccountHelper.cpp
	QtNoWengoAlert.cpp
	
	QtEnumPhoneType.cpp #VOXOX - CJC - For mapping types
	QtEnumEmailType.cpp #VOXOX - CJC - For mapping types
	
	dialpad/QtMiniDialpad.cpp
	dialpad/QtDialpad.cpp
	dialpad/QtSVGDialpad.cpp

	phoneline/QtPhoneLine.cpp

	phonecall/QtPhoneCall.cpp
	phonecall/QtVideo.cpp
	phonecall/QtVideoQt.cpp
	phonecall/QtContactCallListWidget.cpp
	phonecall/QtPhoneCallEventFilter.cpp
	phonecall/QtAudioSmileysWindow.cpp

	toaster/QtChatToaster.cpp
	toaster/QtCallToaster.cpp
	toaster/QtToaster.cpp

	login/ILogin.cpp
	login/QtLoginDialog.cpp
	login/QtLogAccount.cpp
	login/QtAddAccount.cpp
	login/QtAddWengoAccount.cpp
	login/QtAddSIPAccount.cpp
	login/QtAddAccountUtils.cpp
	login/QtAddVoxOxAccount.cpp
	login/QtLoginMessage.cpp
	login/QtVoxOxLogin.cpp
	login/QtVoxOxLoginMessage.cpp
	login/QtVoxOxLoadingLogin.cpp
	
	messagebox/QtVoxMessageBox.cpp
	messagebox/QtVoxPhoneNumberMessageBox.cpp
	messagebox/QtVoxEmailMessageBox.cpp
	messagebox/QtVoxTextMessageBox.cpp

	imcontact/QtAdvancedIMContactManager.cpp
	imcontact/QtAddIMContact.cpp
	imcontact/QtIMContactItem.cpp
	imcontact/QtSimpleIMContactManager.cpp
	imcontact/QtIMContactManagerInterface.cpp
	imcontact/QtFastAddIMContact.cpp
	

	profile/QtProfileDetails.cpp
	profile/QtUserProfile.cpp
	profile/QtUserProfileHandler.cpp

	imaccount/QtIMAccountManager.cpp
	imaccount/QtIMAccountSettings.cpp
	imaccount/QtIMAccountPlugin.cpp
	imaccount/QtMSNSettings.cpp
	imaccount/QtFacebookSettings.cpp
	imaccount/QtTwitterSettings.cpp
	imaccount/QtSkypeSettings.cpp
	imaccount/QtMySpaceSettings.cpp
	imaccount/QtYahooSettings.cpp
	imaccount/QtAIMSettings.cpp
	imaccount/QtICQSettings.cpp
	imaccount/QtJabberSettings.cpp
	imaccount/QtGoogleTalkSettings.cpp

	wenbox/QtWenboxPlugin.cpp

	contactlist/QtContactList.cpp
	contactlist/ContactGroupPopupMenu.cpp
	contactlist/QtContactWidget.cpp
	contactlist/QtContactTooltip.cpp
	contactlist/QtContactTooltipManager.cpp
	contactlist/QtContactActionManager.cpp
	contactlist/QtContactTreeMouseFilter.cpp
	contactlist/QtContactManager.cpp
	contactlist/QtContactListManager.cpp
	contactlist/QtContact.cpp
	contactlist/QtTreeViewDelegate.cpp
	contactlist/QtContactPixmap.cpp
	contactlist/QtContactInfo.cpp
	contactlist/QtContactTreeKeyFilter.cpp
	contactlist/QtRenameGroup.cpp
	contactlist/QtConferenceAction.cpp
	contactlist/QtContactMenu.cpp
#	contactlist/QtContactMenu2.cpp			#VOXOX - JRT - 2009.05.04
	contactlist/QtContactListStyle.cpp
	contactlist/VoxContactTreeWidget.cpp		#VOXOX - JRT - 2009.05.13
	contactlist/QtContactNetworkMenu.cpp

	conference/QtConferenceCallWidget.cpp

	chat/emoticons/QtEmoticonsWidget.cpp
	chat/emoticons/QtEmoticon.cpp
	chat/emoticons/QtEmoticonButton.cpp
	chat/emoticons/QtEmoticonsManager.cpp
	chat/chatroom/QtChatRoomInviteDlg.cpp
	chat/chatroom/QtChatRoomTreeWidgetItem.cpp
	chat/chatroom/QtChatRoomListWidgetItem.cpp
	chat/translation/QtTranslationManager.cpp
	chat/translation/QtTranslationLanguage.cpp
	chat/translation/QtTranslationWidget.cpp
	chat/translation/QtEnumTranslationMode.cpp
	chat/QtEnumUMMode.cpp
	chat/QtUMBase.cpp
	chat/QtUMTranslation.cpp
	chat/QtUMTranslationRequest.cpp
	chat/QtChatHandler.cpp
	chat/QtUMChat.cpp
	chat/QtUMEmail.cpp
	chat/QtUMSMS.cpp
	chat/QtUMSendFile.cpp
	chat/QtUMFax.cpp
	chat/QtContactProfileWidget.cpp
	chat/QtUMCall.cpp
	chat/QtPopUpChatDialog.cpp
	chat/QtPopUpChatManager.cpp
	chat/QtChatEditWidget.cpp
	chat/QtChatHistoryWidget.cpp
	chat/QtWengoStyleBar.cpp
	chat/QtChatUtils.cpp
	chat/QtChatToEmailSMSUtils.cpp
	chat/QtChatLogViewer.cpp
	chat/QtChatTheme.cpp
	chat/QtUMWindow.cpp
	chat/QtUMHelper.cpp
	chat/umtreewidget/QtUMItem.cpp
	chat/umtreewidget/QtUMGroup.cpp
	chat/umtreewidget/QtUMItemList.cpp
	chat/umtreewidget/QtUMItemListManager.cpp
	chat/umtreewidget/QtUMTreeWidget.cpp
	chat/umtreewidget/QtUMTreeViewDelegate.cpp
	chat/umtreewidget/QtUMItemInfo.cpp
	chat/umtreewidget/QtUMItemManager.cpp
	chat/umtreewidget/QtUMItemListStyle.cpp
	
	webbrowser/QtWebBrowser.cpp
	webbrowser/QtVoxBrowser.cpp

	webservices/sms/QtSms.cpp
	webservices/softupdate/QtSoftUpdate.cpp
	webservices/softupdate/QtSoftUpdatePrompt.cpp		#VOXOX - JRT - 2009.10.28
	webservices/softupdate/QtBrowserSoftUpdate.cpp

	callbar/QtCallBar.cpp
	callbar/QtPhoneComboBox.cpp
	callbar/QtVoxOxCallBarWidget.cpp
	callbar/customCallBar/QtVoxOxCallBarFrame.cpp
	callbar/customCallBar/callBarUtilities/QtFlag.cpp
	callbar/customCallBar/callBarUtilities/QtFlagsListWidget.cpp
	callbar/customCallBar/callBarUtilities/QtFlagsManager.cpp

	statusbar/QtStatusBar.cpp

	
	config/QtPhoneSettings.cpp
	config/QtTextSettings.cpp
	config/QtMyAccountSettings.cpp
	
	config/QtAdvancedConfig.cpp
	config/QtAccountSettings.cpp
	config/QtAdvancedSettings.cpp
	config/QtAppearanceSettings.cpp
	config/QtAudioSettings.cpp
	config/QtCallForwardSettings.cpp
	config/QtGeneralSettings.cpp
	config/QtNotificationSettings.cpp
	config/QtEventsSettings.cpp
	config/QtPrivacySettings.cpp
	config/QtVideoSettings.cpp
	config/QtLanguagesSettings.cpp
	config/QtWengoConfigDialog.cpp
	config/QtVoicemailSettings.cpp
	config/QtSecuritySettings.cpp
	
	wizard/QtWizardDialog.cpp
	wizard/QtWizardProgress.cpp
	wizard/QtWizardAudio.cpp
	wizard/QtWizardVideo.cpp
	wizard/QtWizardSelectNetworks.cpp
	wizard/QtWizardIMAccountContainer.cpp
	wizard/QtWizardProfile.cpp
	wizard/QtWizardPhones.cpp
	wizard/QtWizardPA.cpp
	

	profilebar/QtEventWidget.cpp
	profilebar/QtIMProfileWidget.cpp
	profilebar/QtCreditWidget.cpp
	profilebar/QtProfileBar.cpp
	profilebar/QtIMAccountPresenceMenuManager.cpp

	history/QtHistoryWidget.cpp
	history/QtHistory.cpp

	filetransfer/QtFileTransfer.cpp
	filetransfer/QtFileTransferAcceptDialog.cpp
	filetransfer/QtFileTransferWidget.cpp
	filetransfer/QtFileTransferItem.cpp
	filetransfer/QtFileTransferDownloadItem.cpp
	filetransfer/QtFileTransferUploadItem.cpp
	filetransfer/QtFileTransferNotifyDialog.cpp

	webdirectory/QtWebDirectory.cpp
)

if (WIN32)
	qt4_wrap_cpp(${PROJECT_NAME}_SRCS
		WinApp.h
	)

	# Create a 4 digit, comma separated, version of the APPLICATION_VERSION
	# variable. That is, from x.y.z to x,y,z,0.
	string(REPLACE "." "," APPLICATION_VERSION_4DIGITS ${APPLICATION_VERSION})
	set(APPLICATION_VERSION_4DIGITS "${APPLICATION_VERSION_4DIGITS},0,0,0")
	string(REGEX REPLACE "([0-9]+,[0-9]+,[0-9]+,[0-9]+).*" "\\1" APPLICATION_VERSION_4DIGITS "${APPLICATION_VERSION_4DIGITS}")

	configure_file(
		win32/application.rc
		${BUILD_DIR}/application.rc
		)

	ow_add_sources(
		${BUILD_DIR}/application.rc
		WinApp.cpp
	)
	ow_add_private_libraries(
		qtmain.lib
	)
endif (WIN32)

if (UNIX AND NOT WIN32)
	if (APPLE)
		set_source_files_properties(macosx/QtMacApplication.cpp
			PROPERTIES
				COMPILE_FLAGS "-x objective-c++"
		)

		qt4_wrap_cpp(${PROJECT_NAME}_SRCS
			macosx/QtMacApplication.h
		)

		ow_add_sources(
			macosx/QtMacApplication.cpp
		)
	else (APPLE)
		if (WENGOPHONE_XV_SUPPORT)
			if (X11_XV_FOUND)
				ow_use_private_libraries(
					x11
				)

				ow_add_private_definitions(
					-DXV_HWACCEL
				)

				qt4_wrap_cpp(${PROJECT_NAME}_SRCS
					phonecall/XVWindow.h
					phonecall/QtVideoXV.h
				)

				ow_add_sources(
					phonecall/XVWindow.cpp
					phonecall/QtVideoXV.cpp
				)
			endif (X11_XV_FOUND)
		endif (WENGOPHONE_XV_SUPPORT)
	endif (APPLE)
endif (UNIX AND NOT WIN32)

qt4_wrap_ui(${PROJECT_NAME}_SRCS
	WengoPhoneWindow.ui
	AboutWindow.ui
	HttpProxyLoginWindow.ui	
	NoWengoAlert.ui
	
	dialpad/MiniDialpadWidget.ui
	dialpad/DialpadWidget.ui

	login/LogAccount.ui
	login/AddAccount.ui
	login/AddWengoAccount.ui
	login/AddSIPAccount.ui
	login/AddVoxOxAccount.ui
	login/LoginMessage.ui
	login/VoxOxLoadingLogin.ui
	
	messagebox/VoxTextMessageBox.ui
	messagebox/VoxPhoneNumberMessageBox.ui
	messagebox/VoxEmailMessageBox.ui

	imaccount/IMAccountTemplate.ui
	imaccount/AIMSettings.ui
	imaccount/ICQSettings.ui
	imaccount/GoogleTalkSettings.ui
	imaccount/IMAccountManager.ui
	imaccount/JabberSettings.ui
	imaccount/MSNSettings.ui
	imaccount/MySpaceSettings.ui
	imaccount/FacebookSettings.ui
	imaccount/TwitterSettings.ui
	imaccount/SkypeSettings.ui
	imaccount/YahooSettings.ui

	profile/ProfileDetails.ui

	imcontact/IMContactManager.ui
	imcontact/AddIMContact.ui
	imcontact/SimpleIMContactManager.ui
	imcontact/FastAddIMContact.ui
	
	webbrowser/WebBrowser.ui
	
	webservices/sms/SmsWindow.ui
	webservices/softupdate/SoftUpdateWindow.ui
	webservices/softupdate/SoftUpdatePromptWindow.ui		#VOXOX - JRT - 2009.10.28
	webservices/softupdate/BrowserSoftUpdateWindow.ui
	
	callbar/VoxOxCallBarWidget.ui
	callbar/customCallBar/VoxOxCallBarFrame.ui

	conference/ConferenceCallWidget.ui

	config/AccountSettings.ui
	config/AdvancedConfigWindow.ui
	config/GeneralSettings.ui
	config/AdvancedSettings.ui
	config/AppearanceSettings.ui
	config/PhoneSettings.ui
	config/TextSettings.ui
	config/MyAccountSettings.ui
	config/AudioSettings.ui
	config/CallForwardSettings.ui
	config/LanguagesSettings.ui
	config/ManageBlockedUsersSettings.ui
	config/NotificationSettings.ui
	config/EventsSettings.ui
	config/NotificationSettingsItem.ui
	config/PrivacySettings.ui
	config/VideoSettings.ui
	config/WengoConfigDialog.ui
	config/VoicemailSettings.ui
	config/SecuritySettings.ui
	
	wizard/WizardDialog.ui
	wizard/WizardProgress.ui
	wizard/WizardAudio.ui
	wizard/WizardVideo.ui
	wizard/WizardSelectNetworks.ui
	wizard/WizardIMAccountContainer.ui
	wizard/WizardProfile.ui
	wizard/WizardPhones.ui
	wizard/WizardPA.ui


	chat/PopUpChatDialog.ui
	chat/UMChat.ui
	chat/UMEmail.ui
	chat/UMSMS.ui
	chat/UMSendFile.ui
	chat/UMFax.ui
	chat/ContactProfileWidget.ui
	chat/UserFrame.ui
	chat/ChatLogViewer.ui
	chat/UMWindow.ui
	chat/umtreewidget/UMItemList.ui
	chat/chatroom/ChatRoomInviteWidget.ui
	chat/translation/TranslationWidget.ui

	contactlist/RenameGroupDialog.ui
	contactlist/ContactWidget.ui
	contactlist/ContactTooltip.ui
	contactlist/ContactList.ui

	profilebar/CreditWidget.ui
	profilebar/EventWidget.ui
	profilebar/IMProfileWidget.ui

	toaster/QtChatToaster.ui
	toaster/QtCallToaster.ui

	phonecall/PhoneCallWidget.ui
	phonecall/ContactCallListWidget.ui	
	phonecall/VideoWindow.ui

	filetransfer/AcceptDialog.ui
	filetransfer/FileTransferDialog.ui
	filetransfer/FileTransferItem.ui
	filetransfer/NotifyDialog.ui

	history/HistoryWidget.ui

	statusbar/StatusBarWidget.ui
)

if (ENABLE_CRASHREPORT)
    ow_use_private_libraries(
            owmemorydump
    )
    ow_add_private_definitions(
            -DENABLE_CRASHREPORT
    )
endif (ENABLE_CRASHREPORT)

if (CUSTOM_ACCOUNT)
		ow_add_private_definitions(
			-DCUSTOM_ACCOUNT
		)
		message(STATUS "Custom account activated")
endif (CUSTOM_ACCOUNT)

if (DISABLE_CALL_FORWARD)
		ow_add_private_definitions(
			-DDISABLE_CALL_FORWARD
		)
		message(STATUS "Call forward deactivated")
endif (DISABLE_CALL_FORWARD)

if (DISABLE_SMS)
		ow_add_private_definitions(
			-DDISABLE_SMS
		)
		message(STATUS "Sms deactivated")
endif (DISABLE_SMS)

if (DISABLE_VOICE_MAIL)
		ow_add_private_definitions(
			-DDISABLE_VOICE_MAIL
		)
		message(STATUS "Voice mail deactivated")
endif (DISABLE_VOICE_MAIL)

if (APPLE)
	ow_use_private_frameworks(
		Cocoa
		Carbon
		Security
		Growl-WithInstaller
	)
	ow_add_private_include_dirs(
		/Developer/Headers/FlatCarbon/
	)
	ow_use_private_libraries(
		glib2 #comes from gaimimwrapper <- gaim
	)
	
	include_directories (
		../../../../libs/3rdparty/smartcrashreports/include
	)

	add_executable(${BINARY_NAME} ../../../../libs/3rdparty/smartcrashreports/binary/SmartCrashReportsInstall.o)
	add_executable(${BINARY_NAME} ../../../../libs/3rdparty/smartcrashreports/binary/SmartCrashReportsAPI.o)

endif (APPLE)

if (LINUX)
	# Define QT_TRANSLATION_DIR, this is used on Linux to load Qt translations
	EXEC_PROGRAM(${QT_QMAKE_EXECUTABLE}
		ARGS "-query QT_INSTALL_PREFIX"
		OUTPUT_VARIABLE QT_INSTALL_PREFIX)
	ow_add_private_definitions(
		-DQT_TRANSLATION_DIR='"${QT_INSTALL_PREFIX}/translations"'
	)
endif (LINUX)

if (WIN32)
   if (ENABLE_CONSOLE)
      ow_create_project_binary()
   else(ENABLE_CONSOLE)
      ow_create_project_binary(WIN32)

   endif(ENABLE_CONSOLE)
  
else (WIN32)
      ow_create_project_binary(MACOSX_BUNDLE)
endif (WIN32)

ow_locale_release(
	${CMAKE_CURRENT_SOURCE_DIR}/lang
)

add_dependencies(${BINARY_NAME} lrelease)

# Bugfix, spaces don't work with install target
string(REPLACE " " "CMAKEBUGWITHSPACES" TMP_BUILD_DIR ${BUILD_DIR})
string(REPLACE " " "CMAKEBUGWITHSPACES" TMP_SOURCE_DIR ${CMAKE_SOURCE_DIR})

# Passes variables to the install script
install(
	CODE "
	set(SOURCE_DIR ${TMP_SOURCE_DIR})
	set(APPLICATION_NAME ${APPLICATION_NAME})
	set(APPLICATION_VERSION ${APPLICATION_VERSION})
	set(APPLICATION_VERSION_1 ${APPLICATION_VERSION_1})
	set(APPLICATION_VERSION_2 ${APPLICATION_VERSION_2})
	set(APPLICATION_VERSION_3 ${APPLICATION_VERSION_3})
	set(APPLICATION_VERSION_4 ${APPLICATION_VERSION_4})
	set(BUILD_DIR ${TMP_BUILD_DIR})
	set(BUILD_TYPE ${CMAKE_BUILD_TYPE})
	set(DATA_INSTALL_DIR ${DATA_INSTALL_DIR})
	set(INSTALLER_WITH_PDB ${INSTALLER_WITH_PDB})
	set(SVN_REVISION ${SVN_REVISION})
	set(BINARY_NAME ${BINARY_NAME})
	set(QT_LIBRARY_DIR ${QT_LIBRARY_DIR})
	set(QT_PLUGINS_DIR ${QT_PLUGINS_DIR})
	"
)

#VOXOX CHANGE by Alexander 02-12-09, Qutecom used ow_declare_install_script() to create an install script but it didn't work. I changed it
# so that when you run make install the cmake script located in CMakeLists-install-macosx.txt is executed and creates the Mac OS bundle.
if (APPLE)
	set_target_properties(${PROJECT_NAME}
		PROPERTIES
			COMPILE_FLAGS "-x objective-c++"
		)
	install(SCRIPT install/CMakeLists-install-macosx.txt)
endif (APPLE)

if (LINUX)
	include(CMakeLists-install-linux.txt)
endif (LINUX)

if (WIN32)
	# NSIS installer script
	install(SCRIPT ../../../nsis/CMakeLists.txt)
endif (WIN32)


# zip target: zip ${BUILD_DIR}
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
set(ZIP_NAME
	${CMAKE_BINARY_DIR}/${APPLICATION_NAME}-${APPLICATION_VERSION}-${BUILD_TYPE}
)

# Because BuildBot cannot handle random zip names
# Generates 2 zip: one with the -revnumber.tar.bz2 and one with -${SVN_REVISION}.tar.bz2
set(ZIP_NAME_REVNUMBER
	${CMAKE_BINARY_DIR}/${APPLICATION_NAME}-${APPLICATION_VERSION}-${BUILD_TYPE}-revnumber
)
if (NOT SVN_REVISION OR SVN_REVISION EQUAL 0)
	set(ZIP_NAME ${ZIP_NAME}-norev)
else (NOT SVN_REVISION OR SVN_REVISION EQUAL 0)
	set(ZIP_NAME ${ZIP_NAME}-${SVN_REVISION})
endif (NOT SVN_REVISION OR SVN_REVISION EQUAL 0)

set(ZIP_DIR_NAME
	${CMAKE_BINARY_DIR}/${APPLICATION_NAME}-${APPLICATION_VERSION}-${BUILD_TYPE}
)

if (NOT WIN32)
configure_file(
	${CMAKE_SOURCE_DIR}/scripts/wengophone.sh
	${BUILD_DIR}/${BINARY_NAME}.sh
	@ONLY
)
endif (NOT WIN32)

add_custom_target(zip
	COMMAND
		"${CMAKE_COMMAND}"
		-DSOURCE_DIR="${CMAKE_SOURCE_DIR}"
		-DBUILD_DIR="${BUILD_DIR}"
		-DZIP_NAME="${ZIP_NAME}"
		-DZIP_NAME_REVNUMBER="${ZIP_NAME_REVNUMBER}"
		-DZIP_DIR_NAME="${ZIP_DIR_NAME}"
		-DBINARY_NAME="${BINARY_NAME}"
		-P "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists-zip.txt"
	WORKING_DIRECTORY
		${CMAKE_BINARY_DIR}
)
