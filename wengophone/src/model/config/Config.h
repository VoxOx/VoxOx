/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef OWCONFIG_H
#define OWCONFIG_H

#include <settings/CascadingSettings.h>

#include <model/wenbox/EnumWenboxStatus.h>

#include <util/String.h>
#include <util/StringList.h>

/**
 * Stores the configuration options of WengoPhone.
 *
 * Tries to make it impossible to make a mistake.
 *
 * @see CascadingSettings
 * @ingroup model
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class Config : public CascadingSettings {
public:

	static const std::string CONFIG_VERSION_KEY;
	int getConfigVersion() const;

	Config(const std::string & name);

	~Config();

	/**
	 * Gets the name associated with the Config.
	 *
	 * @return Config name
	 */
	std::string getName() const;

	/** True if SSL connection to SSO is available. */
	static const std::string NETWORK_SSO_SSL_KEY;
	bool getNetworkSSOSSL() const;

	/**
	 * Type of NAT.
	 *
	 * @see EnumNatType::NatType
	 */
	static const std::string NETWORK_NAT_TYPE_KEY;
	std::string getNetworkNatType() const;

	/** Client SIP Port to use. */
	static const std::string NETWORK_SIP_LOCAL_PORT_KEY;
	int getNetworkSipLocalPort() const;

	/** True if tunnel is SSL. */
	static const std::string NETWORK_TUNNEL_SSL_KEY;
	bool getNetworkTunnelSSL() const;

	/** Tunnel server. Empty if no tunnel is needed */
	static const std::string NETWORK_TUNNEL_SERVER_KEY;
	std::string getNetworkTunnelServer() const;

	/** True if HTTP tunnel needed. */
	static const std::string NETWORK_TUNNEL_NEEDED_KEY;
	bool getNetWorkTunnelNeeded() const;

	/** Tunnel server port. */
	static const std::string NETWORK_TUNNEL_PORT_KEY;
	int getNetworkTunnelPort() const;

	/** Is proxy detected ?. */
	static const std::string NETWORK_PROXY_DETECTED_KEY;
	bool getNetworkProxyDetected() const;

	/** Proxy server. Empty if no proxy */
	static const std::string NETWORK_PROXY_SERVER_KEY;
	std::string getNetworkProxyServer() const;

	/** Proxy server port. */
	static const std::string NETWORK_PROXY_PORT_KEY;
	int getNetworkProxyPort() const;

	/** Proxy login. Empty if not needed */
	static const std::string NETWORK_PROXY_LOGIN_KEY;
	std::string getNetworkProxyLogin() const;

	/** Proxy Password. Empty if not needed */
	static const std::string NETWORK_PROXY_PASSWORD_KEY;
	std::string getNetworkProxyPassword() const;

	/** Stun server. */
	static const std::string NETWORK_STUN_SERVER_KEY;
	std::string getNetworkStunServer() const;

	/**
	 * @name PhApi configuration Methods
	 * @{
	 */

	/** Path to codecs. */
	static const std::string CODEC_PLUGIN_PATH_KEY;
	std::string getCodecPluginPath() const;

	
	static const std::string CONTACT_LIST_STYLE_KEY;
	std::string getContactListStyle() const;


	/** Path to PhApi plugins (e.g SFP plugin). */
	static const std::string PHAPI_PLUGIN_PATH_KEY;
	std::string getPhApiPluginPath() const;

	/**
	 * @}
	 */
 
	/**
	 * @name Audio Methods
	 * @{
	 */

	/** audio codec list */
	static const std::string AUDIO_CODEC_LIST_KEY;
	StringList getAudioCodecList() const;

	/** Playback audio device name. */
	static const std::string AUDIO_OUTPUT_DEVICEID_KEY;
	StringList getAudioOutputDeviceId() const;

	/** Record audio device name. */
	static const std::string AUDIO_INPUT_DEVICEID_KEY;
	StringList getAudioInputDeviceId() const;

	/** Ringer audio device name. */
	static const std::string AUDIO_RINGER_DEVICEID_KEY;
	StringList getAudioRingerDeviceId() const;

	/** Ringing/incoming call/ringtone audio file name. */
	static const std::string AUDIO_INCOMINGCALL_FILE_KEY;
	std::string getAudioIncomingCallFile() const;

	/** Ringing/double call/ringtone audio file name. */
	static const std::string AUDIO_DOUBLECALL_FILE_KEY;
	std::string getAudioDoubleCallFile() const;

	/** Call closed audio file name. */
	static const std::string AUDIO_CALLCLOSED_FILE_KEY;
	std::string getAudioCallClosedFile() const;

	/** Incoming chat audio file name. */
	static const std::string AUDIO_INCOMINGCHAT_FILE_KEY;
	std::string getAudioIncomingChatFile() const;

	/** IM account connected audio file name. */
	static const std::string AUDIO_IMACCOUNTCONNECTED_FILE_KEY;
	std::string getAudioIMAccountConnectedFile() const;

	/** IM account disconnected audio file name. */
	static const std::string AUDIO_IMACCOUNTDISCONNECTED_FILE_KEY;
	std::string getAudioIMAccountDisconnectedFile() const;

	/** Contact online audio file name. */
	static const std::string AUDIO_CONTACTONLINE_FILE_KEY;
	std::string getAudioContactOnlineFile() const;

	/** Audio smileys directory. */
	static const std::string AUDIO_SMILEYS_DIR_KEY;
	std::string getAudioSmileysDir() const;

	/** AEC (echo canceller) enable or not. */
	static const std::string AUDIO_AEC_KEY;
	bool getAudioAEC() const;

	/** Half duplex mode enable or not. */
	static const std::string AUDIO_HALFDUPLEX_KEY;
	bool getAudioHalfDuplex() const;

	/** Ringing enable or not. */
	static const std::string AUDIO_ENABLE_RINGING_KEY;
	bool getAudioRingingEnable() const;

	/** @} */

	/**
	 * @name Wengo Specific Methods
	 * @{
	 */

	/** Wengo server hostname. */
	static const std::string WENGO_SERVER_HOSTNAME_KEY;
	std::string getWengoServerHostname() const;

	static const std::string API_HOSTNAME_KEY;
	std::string getApiHostname() const;

	static const std::string TRANSLATION_API_HOSTNAME_KEY;
	std::string getTranslationApiHostname() const;
	/** Wengo SSO path on the server. */
	static const std::string WENGO_SSO_PATH_KEY;
	std::string getWengoSSOPath() const;

	/** Wengo SMS path on the server. */
	static const std::string WENGO_SMS_PATH_KEY;
	std::string getWengoSMSPath() const;

	/** Wengo info web service path on the server. */
	static const std::string WENGO_INFO_PATH_KEY;
	std::string getWengoInfoPath() const;

	/** Wengo subscription web service path on the server. */
	static const std::string WENGO_SUBSCRIBE_PATH_KEY;
	std::string getWengoSubscribePath() const;

	/** WengoPhone update path on the Wengo server. */
	static const std::string WENGO_SOFTUPDATE_PATH_KEY;
	std::string getWengoSoftUpdatePath() const;

	static const std::string VOXOX_CONTACT_NUMBER_PATH_KEY;
	std::string getVoxoxContactNumberPath() const;

	static const std::string VOXOX_ACCOUNT_WS_PATH_KEY;				//VOXOX - JRT - 2009.05.27 
	std::string getVoxOxAccountWSPath() const;

	static const std::string VOXOX_ASK_SERVER_KEY;					//VOXOX - JRT - 2009.05.28 
	std::string getVoxOxAskServer() const;
	
	static const std::string VOXOX_SYNC_WS_PATH_KEY;				//VOXOX - JRT - 2009.05.27 
	std::string getVoxOxSyncWSPath() const;

	static const std::string VOXOX_PA_WS_PATH_KEY;				//VOXOX - CJC - 2009.06.27 
	std::string getVoxOxPAWSPath() const;
	static const std::string VOXOX_TRANSLATION_WS_PATH_KEY;		//VOXOX - CJC - 2009.06.27 
	std::string getVoxOxTranslationWSPathKey() const;

	/**
	 * Wengo cirpack web service path on the server.
	 * FIXME no cirpack information please!
	 */
	static const std::string WENGO_CIRPACK_PATH_KEY;
	std::string getWengoCirpackPath() const;

	/** Wengo directory web service path on the server. */
	static const std::string WENGO_DIRECTORY_PATH_KEY;
	std::string getWengoDirectoryPath() const;

	/** Path for current kepyad background */
	static const std::string KEYPAD_BACKGROUND_PATH_KEY; //VOXOX - SEMR - 2009.07.31 change keypad background
	std::string getKeypadBackgroundPath() const;


	/** Wengo realm. */
	static const std::string WENGO_REALM_KEY;
	std::string getWengoRealm() const;

	static const std::string WENGO_AUDIOTESTCALL_KEY;
	std::string getWengoAudioTestCall() const;

	static const std::string WENGO_VIDEOTESTCALL_KEY;
	std::string getWengoVideoTestCall() const;

	/** @} */

	/**
	 * @name Sip Methods
	 * @{
	 */
	static const std::string SIP_REGISTER_SERVER;
	std::string getSipRegisterServer() const;

	static const std::string SIP_REGISTER_PORT;
	int getSipRegisterPort() const;

	static const std::string SIP_PROXY_SERVER;
	std::string getSipProxyServer() const;

	static const std::string SIP_PROXY_PORT;
	int getSipProxyPort() const;

	static const std::string SIP_REALM;
	std::string getSipRealm() const;

	static const std::string SIP_SIMPLE_SUPPORT;
	bool getSipSimpleSupport() const;

	static const std::string SIP_AUDIOTESTCALL_KEY;
	std::string getSipAudioTestCall() const;

	static const std::string SIP_VIDEOTESTCALL_KEY;
	std::string getSipVideoTestCall() const;

	/** @} */

	/**
	 * @name Sip Options
	 * @{
	 */
	static const std::string SIP_REGISTER_TIMEOUT;
	int getSipRegisterTimeOut() const;

	static const std::string SIP_PUBLISH_TIMEOUT;
	int getSipPublishTimeOut() const;

	static const std::string SIP_USE_OPTIONS;
	bool getSipUseOptions() const;

	static const std::string SIP_P2P_PRESENCE;
	bool getSipP2pPresence() const;

	static const std::string SIP_CHAT_WITHOUT_PRESENCE;
	bool getSipChatWithoutPresence() const;
	/** @} */

	/**
	 * @name Profile Methods
	 * @{
	 */

	//VOXOX CHANGE BY ROLANDO 04-07-09
	/** Gets the name of the last dialed number by user. */
	static const std::string LAST_DIALED_NUMBER_KEY;
	std::string getLastDialedNumber() const;

	/** Gets the name of the last used UserProfile. */
	static const std::string PROFILE_LAST_USED_NAME_KEY;
	std::string getProfileLastUsedName() const;

	/** Gets the account of the last logged UserProfile. */
	static const std::string PROFILE_LAST_LOGGED_ACCOUNT_KEY;
	std::string getProfileLastLoggedAccount() const;

	/** Profile all widget enabled. */
	static const std::string PROFILE_ALL_KEY;
	bool getProfileAll() const;

	/** Profile window width. */
	static const std::string PROFILE_WIDTH_KEY;
	int getProfileWidth() const;

	/** Profile window height. */
	static const std::string PROFILE_HEIGHT_KEY;
	int getProfileHeight() const;

	/** Profile window position X. */
	static const std::string PROFILE_POSX_KEY;
	int getProfilePosX() const;

	/** Profile window position Y. */
	static const std::string PROFILE_POSY_KEY;
	int getProfilePosY() const;

	/** Chat window position X. */
	static const std::string CHAT_WINDOW_POSX_KEY;
	int getChatWindowPosX() const;

	/** Chat window position Y. */
	static const std::string CHAT_WINDOW_POSY_KEY;
	int getChatWindowPosY() const;


	//VOXOX - CJC - 2009.05.01 Add key to support last open tab
	static const std::string LAST_OPEN_TAB_INDEX;
	int getLastOpenTabIndex() const;

	/** @} */

	/**
	 * @name Call Forwarding Methods
	 * @{
	 */

	/** Call forward mode. */
	static const std::string CALL_FORWARD_MODE_KEY;
	std::string getCallForwardMode() const;

	/** Forward call phone number 1. */
	static const std::string CALL_FORWARD_PHONENUMBER1_KEY;
	std::string getCallForwardPhoneNumber1() const;

	/** Forward call phone number 2. */
	static const std::string CALL_FORWARD_PHONENUMBER2_KEY;
	std::string getCallForwardPhoneNumber2() const;

	/** Forward call phone number 3. */
	static const std::string CALL_FORWARD_PHONENUMBER3_KEY;
	std::string getCallForwardPhoneNumber3() const;

	/** Active voice mail. */
	static const std::string VOICE_MAIL_ACTIVE_KEY;
	bool getVoiceMailActive() const;

	/** @} */

	/**
	 * @name General Settings Methods
	 * @{
	 */

	/** Automatically start WengoPhone when computer starts. */
	static const std::string GENERAL_AUTOSTART_KEY;
	bool getGeneralAutoStart() const;
	//VOXOX - CJC - 2009.06.01 
	static const std::string WIZARD_AUTOSTART_KEY;
	bool getWizardAutoStart() const;

	static const std::string WIZARD_ASSISTANT_IS_MALE;
	bool getWizardAssistandIsMale() const;

	static const std::string WIZARD_STEP_COMPLETE_KEY;
	int getWizardStepsComplete() const;

	/** Start a free call if applicable. */
	static const std::string GENERAL_CLICK_START_FREECALL_KEY;
	bool getGeneralClickStartFreeCall() const;

	/** Start a chat-only when applicable. */
	static const std::string GENERAL_CLICK_START_CHAT_KEY;
	bool getGeneralClickStartChat() const;

	/** Call cell phone or land line if contact not available. */
	static const std::string GENERAL_CLICK_CALL_CELLPHONE_KEY;
	bool getGeneralClickCallCellPhone() const;

	/** Show me as away when I am inactive for. */
	static const std::string GENERAL_AWAY_TIMER_KEY;
	static const int NO_AWAY_TIMER = -1;
	int getGeneralAwayTimer() const;

	/** Show / Hide groups in the contacts list. */
	static const std::string GENERAL_SHOW_GROUPS_KEY;
	bool getShowGroups() const;

	/** Show / Hide offline contacts in the contacts list. */
	static const std::string GENERAL_SHOW_OFFLINE_CONTACTS_KEY;
	bool getShowOfflineContacts() const;

	//VOXOX CHANGE by Rolando - 2009.06.29 
	static const std::string LAST_STATUS_MESSAGE_ACCOUNT_KEY;
	std::string getLastStatusMessageAccountKey() const;

	//VOXOX CHANGE by Rolando - 2009.06.29 
	static const std::string LAST_STATUS_MESSAGE_DISPLAYED;
	std::string getLastStatusMessageDisplayed() const;

	/** show or hide Profile Bar. */
	//VOXOX CHANGE Rolando 03-20-09
	static const std::string SHOW_PROFILE_BAR_KEY;
	bool getShowProfileBar() const;

	/** show or hide Call Bar. */
	//VOXOX CHANGE Rolando 03-25-09
	static const std::string SHOW_CALL_BAR_KEY;
	bool getShowCallBar() const;

	//VOXOX CHANGE Rolando 03-25-09
	static const std::string CURRENT_FLAG_KEY;
	std::string getCurrentFlag() const;

	//VOXOX CHANGE Rolando 03-25-09
	static const std::string CURRENT_COUNTRY_DEFAULT_KEY;
	std::string getCurrentCountryDefault() const;

	//VOXOX - JRT - 2009.05.03 
	static const std::string GENERAL_CONTACT_SORT_KEY;
	int getContactSort() const;

	static const std::string GENERAL_CONTACT_GROUPING_KEY;
	int getContactGrouping() const;

	static const std::string GENERAL_GROUP_SORT_KEY;
	int getGroupSort() const;
	//End VoxOx


	/** 
	 * The protocol used to create hyperlinks: For example in
	 * 'wengo://someone', 'wengo' is the protocol.
	 */
	static const std::string GENERAL_HYPERLINKPROTOCOL_KEY;
	std::string getHyperlinkProtocol() const;

	/** The toolbar mode. See config.xml for possible values. */
	static const std::string GENERAL_TOOLBARMODE_KEY;
	std::string getToolBarMode() const;

	/** @} */

	/**
	 * @name Appearance Methods
	 * @{
	 */
	static const std::string APPEARANCE_CHATTHEME_KEY;
	std::string getChatTheme() const;

	static const std::string APPEARANCE_CHATTHEME_VARIANT_KEY;
	std::string getChatThemeVariant() const;

	static const std::string EMOTICON_CHAT_KEY;
	std::string getEmoticonPack() const;
	/** @} */


	static const std::string ASKIMVOXOXCOM_ADDRESS_KEY;
	std::string getAskIMVoxOxComAddress() const;

	static const std::string SELECTED_DICTIONARY_KEY;
	std::string getSelectedDictionary() const;

	/** ISO 639 code of the current language for translating WengoPhone. */
	static const std::string LANGUAGE_KEY;
	static const std::string LANGUAGE_AUTODETECT_KEYVALUE;
	std::string getLanguage() const;

	/**
	 * @name Notification Settings Methods
	 * @{
	 */

	/** Shows toaster for incoming calls. */
	static const std::string NOTIFICATION_SHOW_TOASTER_ON_INCOMING_CALL_KEY;
	bool getNotificationShowToasterOnIncomingCall() const;
	
	//VOXOX - CJC - 2009.05.25  play sound on incoming call
	static const std::string NOTIFICATION_PLAY_SOUND_ON_INCOMING_CALL_KEY;
	bool getNotificationPlaySoundOnIncomingCall() const;


	/** Shows toaster for incoming chats. */
	static const std::string NOTIFICATION_SHOW_TOASTER_ON_INCOMING_CHAT_KEY;
	bool getNotificationShowToasterOnIncomingChat() const;

	//VOXOX - CJC - 2009.05.25 
	static const std::string NOTIFICATION_PLAY_SOUND_ON_INCOMING_CHAT_KEY;
	bool getNotificationPlaySoundOnIncomingChat() const;

	//VOXOX - CJC - 2009.05.25 
	static const std::string NOTIFICATION_PLAY_SOUND_ON_CALL_CLOSED;
	bool getNotificationPlaySoundOnCallClosed() const;

	/** Shows toaster when a contact is turning online. */
	static const std::string NOTIFICATION_SHOW_TOASTER_ON_CONTACT_ONLINE_KEY;
	bool getNotificationShowToasterOnContactOnline() const;

	/** Do not show any toaster or window when in do not disturb mode. */
	static const std::string NOTIFICATION_DONOTDISTURB_NO_WINDOW_KEY;
	bool getNotificationDoNotDisturbNoWindow() const;

	/** Switch off audio notifications when in do not disturb mode mode. */
	static const std::string NOTIFICATION_DONOTDISTURB_NO_AUDIO_KEY;
	bool getNotificationDoNotDisturbNoAudio() const;

	/** Do not show any call toaster when in away mode. */
	static const std::string NOTIFICATION_AWAY_NO_WINDOW_KEY;
	bool getNotificationAwayNoWindow() const;

	/** Switch off audio notifications when in away mode. */
	static const std::string NOTIFICATION_AWAY_NO_AUDIO_KEY;
	bool getNotificationAwayNoAudio() const;

	/** @} */

	/**
	 * @name Privacy Methods
	 * @{
	 */

	/** Allow calls from: anyone. */
	static const std::string PRIVACY_ALLOW_CALL_FROM_ANYONE_KEY;
	bool getPrivacyAllowCallFromAnyone() const;

	/** Allow calls from: only people from my contact list. */
	static const std::string PRIVACY_ALLOW_CALL_ONLY_FROM_CONTACT_LIST_KEY;
	bool getPrivacyAllowCallOnlyFromContactList() const;

	/** Allow chats from: anyone. */
	static const std::string PRIVACY_ALLOW_CHAT_FROM_ANYONE_KEY;
	bool getPrivacyAllowChatsFromAnyone() const;

	/** Allow chats from: only people from my contact list. */
	static const std::string PRIVACY_ALLOW_CHAT_ONLY_FROM_CONTACT_LIST_KEY;
	bool getPrivacyAllowChatOnlyFromContactList() const;

	/** When I sign into Wengo: always sign in as invisible. */
	static const std::string PRIVACY_SIGN_AS_INVISIBLE_KEY;
	bool getPrivacySignAsInvisible() const;

	/** @} */

	/**
	 * @name Video Methods
	 * @{
	 */

	static const std::string VIDEO_ENABLE_KEY;
	bool getVideoEnable() const;

	static const std::string VIDEO_WEBCAM_DEVICE_KEY;
	std::string getVideoWebcamDevice() const;

	static const std::string VIDEO_QUALITY_KEY;
	std::string getVideoQuality() const;

	static const std::string VIDEO_ENABLE_XVIDEO_KEY;
	bool getXVideoEnable() const;

	static const std::string VIDEO_ENABLE_FLIP_KEY;
	bool getVideoFlipEnable() const;

	/** @} */

	/**
	 * @name Path Methods
	 * @{
	 */

	/**
	 * Location of configuration files on the hard disk. 
	 * This is not stored in the config file.
	 */
	static void setConfigDir(const std::string&);
	static std::string getConfigDir();

	/** Last location where a chat history has benn saved */
	static const std::string LAST_CHAT_HISTORY_SAVE_DIR_KEY;
	std::string getLastChatHistorySaveDir() const;

	/** @} */

	/**
	 * @name Wenbox Methods
	 * @{
	 */

	/** Wenbox support enable, disable or not connected (not found). */
	static const std::string WENBOX_ENABLE_KEY;
	std::string getWenboxEnable() const;

	/** Wenbox playback audio device name. */
	static const std::string WENBOX_AUDIO_OUTPUT_DEVICEID_KEY;
	StringList getWenboxAudioOutputDeviceId() const;

	/** Wenbox record audio device name. */
	static const std::string WENBOX_AUDIO_INPUT_DEVICEID_KEY;
	StringList getWenboxAudioInputDeviceId() const;

	/** Wenbox ringer audio device name. */
	static const std::string WENBOX_AUDIO_RINGER_DEVICEID_KEY;
	StringList getWenboxAudioRingerDeviceId() const;

	/** @} */

	/**
	 * @name Other Methods
	 * @{
	 */

	/** Enables Internet Explorer ActiveX support (relevant only under Windows). */
	static const std::string IEACTIVEX_ENABLE_KEY;
	bool getIEActiveXEnable() const;

	/**
	 * Returns the authorized domains that can pass commands to the softphone.
	 * @see CommandServer
	 */
	static const std::string CMDSERVER_AUTHORIZEDDOMAINS_KEY;
	StringList getCmdServerAuthorizedDomains() const;

	/** @} */

	/**
	 * @name File Transfer Options
	 * @{
	 */

	/** Download folder. */
	static const std::string FILETRANSFER_DOWNLOAD_FOLDER_KEY;
	std::string getFileTransferDownloadFolder() const;

	/** Path to the last uploaded file. */
	static const std::string FILETRANSFER_LASTUPLOADEDFILE_FOLDER_KEY;
	std::string getLastUploadedFileFolder() const;

	/** @} */

	/**
	 * @name Linux Options
	 * @{
	 */

	/** Prefered browser. */
	static const std::string LINUX_PREFERED_BROWSER_KEY;
	std::string getLinuxPreferedBrowser() const;

	/** @} */
//VoxOx Jabber Server Key
	static const std::string VOXOX_JABBER_SERVER_KEY;
	std::string getJabberVoxoxServer() const;

	/**
	 * @name Security Options
	 * @{
	 */

	/** Call encryption mode. */
	static const std::string CALL_ENCRYPTION_MODE_KEY;
	bool getCallEncryptionMode() const;

	/** @} */


	/**
	 * @name Urls
	 * @{
	 */
	static const std::string URL_ACCOUNTCREATION_KEY;
	std::string getAccountCreationUrl() const;

	static const std::string URL_FORUM_KEY;
	std::string getForumUrl() const;

	static const std::string URL_CALLOUT_KEY;
	std::string getCalloutUrl() const;

	static const std::string URL_SMS_KEY;
	std::string getSmsUrl() const;

	static const std::string URL_VOICEMAIL_KEY;
	std::string getVoiceMailUrl() const;

	static const std::string URL_EXTERNALDIRECTORY_KEY;
	std::string getExternalDirectoryUrl() const;

	static const std::string URL_INTERNALDIRECTORY_KEY;
	std::string getInternalDirectoryUrl() const;

	static const std::string URL_FAQ_KEY;
	std::string getFaqUrl() const;

	static const std::string URL_ACCOUNT_KEY;
	std::string getAccountUrl() const;

	static const std::string URL_BUYWENGOS_KEY;
	std::string getBuyWengosUrl() const;

	static const std::string URL_DOWNLOAD_KEY;
	std::string getDownloadUrl() const;

	static const std::string URL_PHONENUMBER_KEY;
	std::string getPhoneNumberUrl() const;

	static const std::string URL_WIKI_KEY;
	std::string getWikiUrl() const;

	//VOXOX - SEMR - 2009.05.22 HELP MENU
	static const std::string URL_BUGREPORT_KEY;
	std::string getBugReportUrl() const;

	//VOXOX - SEMR - 2009.05.22 HELP MENU
	static const std::string URL_SUGGESTIONS_KEY;
	std::string getSuggestionsUrl() const;

	//VOXOX - SEMR - 2009.05.22 HELP MENU
	static const std::string URL_BILLING_KEY;
	std::string getBillingUrl() const;

	//VOXOX CHANGES by Rolando 04-02-09
	static const std::string URL_RATES_KEY;
	std::string getRatesUrl() const;

	//VOXOX - SEMR - 2009.07.08 Developer Platform
	static const std::string URL_DEVELOPER_PLATFORM;
	std::string getDeveloperPlatformUrl() const;

	static const std::string URL_LOSTPASSWORD_KEY;
	std::string getLostPasswordUrl() const;

	static const std::string URL_LAUNCHPAD_KEY;
	std::string getLaunchpadUrl() const;

	static const std::string URL_HOME_HTTP_KEY;
	std::string getHttpHomeUrl() const;

	static const std::string URL_HOME_HTTPS_KEY;
	std::string getHttpsHomeUrl() const;

	static const std::string URL_COMPANYWEBSITE_KEY;
	std::string getCompanyWebSiteUrl() const;

	//VOXOX CHANGE CJC PHONE SETTINGS FUNTIONALITY ON CONFIG
	static const std::string URL_CONFIGPHONESETTINGS_KEY;
	std::string getConfigPhoneSettingsUrl() const;

	//VOXOX CHANGE CJC PHONE SETTINGS FUNTIONALITY ON CONFIG
	static const std::string URL_CONFIGTEXTSETTINGS_KEY;
	std::string getConfigTextSettingsUrl() const;

	static const std::string URL_CONFIGNOTIFICATIONSETTINGS_KEY;//VOXOX - CJC - 2009.06.28 
	std::string getConfigNotificationSettingsUrl() const;//VOXOX - CJC - 2009.06.28 


	//VOXOX CHANGE CJC PHONE SETTINGS FUNTIONALITY ON CONFIG
	static const std::string URL_SENDFILES_KEY;
	std::string getSendFileUrl() const;
	//VOXOX - CJC - 2009.06.23 
	static const std::string URL_SENDFAX_KEY;
	std::string getSendFaxUrl() const;

	//VOXOX CHANGE CJC PHONE SETTINGS FUNTIONALITY ON CONFIG
	static const std::string URL_CONTACTPROFILE_KEY;
	std::string getContactProfileUrl() const;

	//VOXOX - CJC - 2009.06.15 
	static const std::string URL_MYACCOUNT_KEY;
	std::string getMyAccountUrl() const;

	static const std::string URL_CHANGEPASSWORD_KEY;//VOXOX - CJC - 2009.07.03 
	std::string getChangePasswordUrl() const;

	static const std::string URL_CONTACT_MANAGER_KEY;	//VOXOX - JRT - 2009.07.05 
	std::string getContactManagerUrl() const;

	/** @} */

	/**
	 * @name Feature Methods
	 * @{
	 */
	static const std::string FEATURE_SMS_KEY;
	bool getSmsFeatureEnabled() const;

	static const std::string FEATURE_AUDIOCONFERENCE_KEY;
	bool getAudioConferenceFeatureEnabled() const;

	static const std::string SIP_USE_TYPING_STATE;
	bool getSipUseTypingState() const;
	/** @} */

	//VOXOX -ASV- 2009.12.04
	//VOXOX - JRT - 2009.12.10 - Should be a user level, not in config.xml
//	static const std::string SOFTUPDATE_SHOWRECOMMENDED;
//	bool getSoftUpdateShowRecommended() const;
	
	//VOXOX - JRT - 2009.11.05 - Debugging options which allow QA to alter behavior without having to rebuild the app.
	static const std::string DEBUG_ENABLED;
	bool isDebugEnabled() const;

	static const std::string DEBUG_SOFTUPDATE_UPDATE;
	bool getDebugSoftUpdateUpdate() const;

	static const std::string DEBUG_SOFTUPDATE_TYPE;
	std::string getDebugSoftUpdateType() const;

	static const std::string DEBUG_SOFTUPDATE_DOWNLOAD_URL;
	std::string getDebugSoftUpdateDownloadUrl() const;

	static const std::string DEBUG_SOFTUPDATE_DESCRIPTION;
	std::string getDebugSoftUpdateDescription() const;

	static const std::string DEBUG_SOFTUPDATE_VERSION;
	std::string getDebugSoftUpdateVersion() const;

	static const std::string DEBUG_SOFTUPDATE_CMDLINE_PARMS;
	std::string getDebugSoftUpdateCmdLineParms() const;

	static const std::string DEBUG_SOFTUPDATE_DOWNLOAD_NOW;
	bool getDebugSoftUpdateDownloadNow() const;

private:

	/** Name of this configuration. */
	std::string _name;

	static std::string _configDir;
};

#endif	//OWCONFIG_H
