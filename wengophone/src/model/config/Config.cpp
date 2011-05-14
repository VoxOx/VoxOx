/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "Config.h"

#include <model/wenbox/EnumWenboxStatus.h>

#include <sipwrapper/EnumNatType.h>
#include <sipwrapper/EnumVideoQuality.h>

#include <PhApiCodecList.h>

#include <webcam/WebcamDriver.h>

#include <sound/AudioDeviceManager.h>

#include <util/File.h>
#include <util/Path.h>
#include <util/Logger.h>
#include <cutil/global.h>

#ifdef OS_MACOSX
	#include <CoreFoundation/CoreFoundation.h>
#endif

using namespace std;

const string Config::CONFIG_VERSION_KEY					= "config.version";

const string Config::NETWORK_SSO_SSL_KEY				= "network.sso.ssl";
const string Config::NETWORK_NAT_TYPE_KEY				= "network.nat.type";
const string Config::NETWORK_SIP_LOCAL_PORT_KEY			= "network.sip.localport";
const string Config::NETWORK_TUNNEL_NEEDED_KEY			= "network.tunnel.needed";
const string Config::NETWORK_TUNNEL_SSL_KEY				= "network.tunnel.ssl";
const string Config::NETWORK_TUNNEL_SERVER_KEY			= "network.tunnel.server";
const string Config::NETWORK_TUNNEL_PORT_KEY			= "network.tunnel.port";
const string Config::NETWORK_PROXY_DETECTED_KEY			= "network.proxy.detected";
const string Config::NETWORK_PROXY_SERVER_KEY			= "network.proxy.server";
const string Config::NETWORK_PROXY_PORT_KEY				= "network.proxy.port";
const string Config::NETWORK_PROXY_LOGIN_KEY			= "network.proxy.login";
const string Config::NETWORK_PROXY_PASSWORD_KEY			= "network.proxy.password";
const string Config::NETWORK_STUN_SERVER_KEY			= "network.stun.server";

const string Config::CODEC_PLUGIN_PATH_KEY				= "codec.plugin.path";
const string Config::PHAPI_PLUGIN_PATH_KEY				= "phapi.plugin.path";

const string Config::CONTACT_LIST_STYLE_KEY				= "contact.list.style";

const string Config::AUDIO_CODEC_LIST_KEY				= "audio.codec.list";
const string Config::AUDIO_OUTPUT_DEVICEID_KEY			= "audio.output.deviceid";
const string Config::AUDIO_INPUT_DEVICEID_KEY			= "audio.input.deviceid";
const string Config::AUDIO_RINGER_DEVICEID_KEY			= "audio.ringer.deviceid";
const string Config::AUDIO_ENABLE_RINGING_KEY			= "audio.ringing.enable";
const string Config::AUDIO_INCOMINGCALL_FILE_KEY		= "audio.incomingcall.file";
const string Config::AUDIO_DOUBLECALL_FILE_KEY			= "audio.doublecall.file";
const string Config::AUDIO_CALLCLOSED_FILE_KEY			= "audio.callclosed.file";
const string Config::AUDIO_INCOMINGCHAT_FILE_KEY		= "audio.incomingchat.file";
const string Config::AUDIO_IMACCOUNTCONNECTED_FILE_KEY	= "audio.imaccountconnected.file";
const string Config::AUDIO_IMACCOUNTDISCONNECTED_FILE_KEY = "audio.imaccountdisconnected.file";
const string Config::AUDIO_CONTACTONLINE_FILE_KEY		= "audio.contactonline.file";
const string Config::AUDIO_SMILEYS_DIR_KEY				= "audio.smileys.dir";
const string Config::AUDIO_AEC_KEY						= "audio.aec";
const string Config::AUDIO_HALFDUPLEX_KEY				= "audio.halfduplex";

const string Config::LAST_DIALED_NUMBER_KEY				= "last.dialed.number";		//VOXOX CHANGE BY ROLANDO 04-07-09
const string Config::PROFILE_LAST_USED_NAME_KEY			= "profile.last_used_name";
const string Config::PROFILE_LAST_LOGGED_ACCOUNT_KEY	= "profile.last_logged_account";
const string Config::PROFILE_WIDTH_KEY					= "profile.width";
const string Config::PROFILE_HEIGHT_KEY					= "profile.height";
const string Config::PROFILE_POSX_KEY					= "profile.posx";
const string Config::PROFILE_POSY_KEY					= "profile.posy";

const string Config::CHAT_WINDOW_POSX_KEY				= "chat.window.posx";
const string Config::CHAT_WINDOW_POSY_KEY				= "chat.window.posy";

const string Config::CALL_FORWARD_MODE_KEY				= "call.forward.mode";
const string Config::CALL_FORWARD_PHONENUMBER1_KEY		= "call.forward.phonenumber1";
const string Config::CALL_FORWARD_PHONENUMBER2_KEY		= "call.forward.phonenumber2";
const string Config::CALL_FORWARD_PHONENUMBER3_KEY		= "call.forward.phonenumber3";

const string Config::VOICE_MAIL_ACTIVE_KEY				= "voicemail.active";

const string Config::LAST_OPEN_TAB_INDEX				= "last.open.tab";				//VOXOX - CJC - 2009.05.01 

const string Config::GENERAL_AUTOSTART_KEY				= "general.autostart";
const string Config::WIZARD_AUTOSTART_KEY				= "wizard.autostart";//VOXOX - CJC - 2009.06.01 
const string Config::WIZARD_STEP_COMPLETE_KEY			= "wizard.step.complete";//VOXOX - CJC - 2009.06.01 

const string Config::WIZARD_ASSISTANT_IS_MALE			= "wizard.assistant.ismale";//VOXOX - CJC - 2009.06.01 

const string Config::GENERAL_CLICK_START_FREECALL_KEY	= "general.click.start.freecall";
const string Config::GENERAL_CLICK_START_CHAT_KEY		= "general.click.start.chat";
const string Config::GENERAL_CLICK_CALL_CELLPHONE_KEY	= "general.click.call.cellphone";
const string Config::GENERAL_AWAY_TIMER_KEY				= "general.away.timer";
const string Config::GENERAL_HYPERLINKPROTOCOL_KEY		= "general.hyperlinkprotocol";
const string Config::GENERAL_TOOLBARMODE_KEY			= "general.toolbarmode";
const string Config::GENERAL_SHOW_GROUPS_KEY			= "general.show.groups";
const string Config::GENERAL_SHOW_OFFLINE_CONTACTS_KEY	= "general.show.offline.contacts";

const string Config::GENERAL_CONTACT_SORT_KEY			= "general.contact.sort";
const string Config::GENERAL_CONTACT_GROUPING_KEY		= "general.contact.grouping";
const string Config::GENERAL_GROUP_SORT_KEY				= "general.group.sort";

const string Config::SHOW_PROFILE_BAR_KEY				= "show.profile.bar";			//VOXOX CHANGE Rolando 03-20-09
const string Config::SHOW_CALL_BAR_KEY					= "show.call.bar";				//VOXOX CHANGE Rolando 03-25-09

const string Config::CURRENT_FLAG_KEY					= "current.flag";				//VOXOX CHANGE Rolando 03-25-09
const string Config::CURRENT_COUNTRY_DEFAULT_KEY		= "current.country.default";	//VOXOX CHANGE Rolando 03-25-09

//VOXOX CHANGE by Rolando - 2009.06.29 
const string Config::LAST_STATUS_MESSAGE_ACCOUNT_KEY	= "last.status.message.account.key";//VOXOX CHANGE by Rolando - 2009.06.29
const string Config::LAST_STATUS_MESSAGE_DISPLAYED		= "last.status.message.displayed";//VOXOX CHANGE by Rolando - 2009.06.29 

const string Config::APPEARANCE_CHATTHEME_KEY			= "chat.theme";
const string Config::APPEARANCE_CHATTHEME_VARIANT_KEY	= "chat.variant";

const string Config::EMOTICON_CHAT_KEY					= "emoticon.pack";

const string Config::ASKIMVOXOXCOM_ADDRESS_KEY			= "askImVoxOxCom.address";

const string Config::LANGUAGE_AUTODETECT_KEYVALUE		= "detect";
const string Config::LANGUAGE_KEY						= "language";

const string Config::SELECTED_DICTIONARY_KEY = "selected.dictionary";
const string Config::NOTIFICATION_SHOW_TOASTER_ON_INCOMING_CALL_KEY	= "notification.show.toaster.on.incoming.call";
const string Config::NOTIFICATION_PLAY_SOUND_ON_INCOMING_CALL_KEY	= "notification.play.sound.on.incoming.call";//VOXOX - CJC - 2009.05.25 
const string Config::NOTIFICATION_SHOW_TOASTER_ON_INCOMING_CHAT_KEY	= "notification.show.toaster.on.incoming.chat";
const string Config::NOTIFICATION_PLAY_SOUND_ON_INCOMING_CHAT_KEY	= "notification.play.sound.on.incoming.chat";//VOXOX - CJC - 2009.05.25 
const string Config::NOTIFICATION_PLAY_SOUND_ON_CALL_CLOSED			= "notification.play.sound.on.call.closed";//VOXOX - CJC - 2009.05.25 
const string Config::NOTIFICATION_SHOW_TOASTER_ON_CONTACT_ONLINE_KEY = "notification.show.toaster.on.contact";
const string Config::NOTIFICATION_DONOTDISTURB_NO_WINDOW_KEY		= "notification.donotdisturb.nowindow";
const string Config::NOTIFICATION_DONOTDISTURB_NO_AUDIO_KEY			= "notification.donotdisturb.noaudio";
const string Config::NOTIFICATION_AWAY_NO_WINDOW_KEY				= "notification.away.nowindow";
const string Config::NOTIFICATION_AWAY_NO_AUDIO_KEY					= "notification.away.noaudio";

const string Config::PRIVACY_ALLOW_CALL_FROM_ANYONE_KEY				= "privacy.allow.call.from.anyone";
const string Config::PRIVACY_ALLOW_CALL_ONLY_FROM_CONTACT_LIST_KEY	= "privacy.allow.call.only.from.contact.list";
const string Config::PRIVACY_ALLOW_CHAT_FROM_ANYONE_KEY				= "privacy.allow.chat.from.anyone";
const string Config::PRIVACY_ALLOW_CHAT_ONLY_FROM_CONTACT_LIST_KEY	= "privacy.allow.chat.only.from.contact.list";
const string Config::PRIVACY_SIGN_AS_INVISIBLE_KEY					= "privacy.sign.as.invisible";

const string Config::VIDEO_ENABLE_KEY								= "video.enable";
const string Config::VIDEO_WEBCAM_DEVICE_KEY						= "video.webcam.device";
const string Config::VIDEO_QUALITY_KEY								= "video.quality";
const string Config::VIDEO_ENABLE_XVIDEO_KEY						= "video.xvideo.enable";
const string Config::VIDEO_ENABLE_FLIP_KEY							= "video.flip.enable";

const string Config::WENGO_SERVER_HOSTNAME_KEY		= "voxox.server.hostname";
const string Config::API_HOSTNAME_KEY = "api.hostname"; //VOXOX - CJC - 2009.06.27 

const string Config::TRANSLATION_API_HOSTNAME_KEY = "translation.api.hostname"; //VOXOX - CJC - 2010.01.20 
const string Config::WENGO_SMS_PATH_KEY				= "voxox.sms.path";//VOXOX - CJC - 2009.10.23 Change wengo to voxox
const string Config::WENGO_SSO_PATH_KEY				= "voxox.sso.path";//VOXOX - CJC - 2009.10.23 Change wengo to voxox
const string Config::WENGO_INFO_PATH_KEY			= "voxox.info.path";//VOXOX - CJC - 2009.10.23 Change wengo to voxox
const string Config::WENGO_SUBSCRIBE_PATH_KEY		= "voxox.subscribe.path";//VOXOX - CJC - 2009.10.23 Change wengo to voxox
const string Config::WENGO_SOFTUPDATE_PATH_KEY		= "voxox.softupdate.path";//VOXOX - CJC - 2009.10.23 Change wengo to voxox
const string Config::WENGO_CIRPACK_PATH_KEY			= "voxox.cirpack.path";//VOXOX - CJC - 2009.10.23 Change wengo to voxox
const string Config::WENGO_DIRECTORY_PATH_KEY		= "voxox.directory.path";//VOXOX - CJC - 2009.10.23 Change wengo to voxox
const string Config::WENGO_AUDIOTESTCALL_KEY		= "voxox.testcall.audio";//VOXOX - CJC - 2009.10.23 Change wengo to voxox
const string Config::WENGO_VIDEOTESTCALL_KEY		= "voxox.testcall.video";//VOXOX - CJC - 2009.10.23 Change wengo to voxox
const string Config::WENGO_REALM_KEY				= "voxox.realm";//VOXOX - CJC - 2009.10.23 Change wengo to voxox

const string Config::VOXOX_ASK_SERVER_KEY			= "voxox.ask.server";			//VOXOX - JRT - 2009.05.28 
const string Config::VOXOX_CONTACT_NUMBER_PATH_KEY	= "voxox.contactnumber.path";
const string Config::VOXOX_ACCOUNT_WS_PATH_KEY		= "voxox.accountws.path";		//VOXOX - JRT - 2009.05.27 
const string Config::VOXOX_SYNC_WS_PATH_KEY			= "voxox.syncws.path";			//VOXOX - JRT - 2009.05.28 

const string Config::VOXOX_PA_WS_PATH_KEY			= "voxox.paws.path";	//VOXOX - CJC - 2009.06.27 
const string Config::VOXOX_TRANSLATION_WS_PATH_KEY		= "voxox.translationws.path"; //VOXOX - CJC - 2010.01.08 

const string Config::KEYPAD_BACKGROUND_PATH_KEY		= "keypad.background"; //VOXOX - SEMR - 2009.07.31 change keypad background


const string Config::SIP_REGISTER_SERVER			= "sip.register.server";
const string Config::SIP_REGISTER_PORT				= "sip.register.port";
const string Config::SIP_PROXY_SERVER				= "sip.proxy.server";
const string Config::SIP_PROXY_PORT					= "sip.proxy.port";
const string Config::SIP_REALM						= "sip.realm";
const string Config::SIP_SIMPLE_SUPPORT				= "sip.simplesupport";
const string Config::SIP_AUDIOTESTCALL_KEY			= "sip.testcall.audio";
const string Config::SIP_VIDEOTESTCALL_KEY			= "sip.testcall.video";
const string Config::SIP_REGISTER_TIMEOUT			= "sip.register.timeout";
const string Config::SIP_PUBLISH_TIMEOUT			= "sip.publish.timeout";
const string Config::SIP_USE_OPTIONS				= "sip.use.options";
const string Config::SIP_P2P_PRESENCE				= "sip.p2p.presence";
const string Config::SIP_CHAT_WITHOUT_PRESENCE		= "sip.chat.without.presence";
const string Config::SIP_USE_TYPING_STATE			= "sip.use.typing_state";

const string Config::LAST_CHAT_HISTORY_SAVE_DIR_KEY = "last.chat.history.save.dir";

const string Config::WENBOX_ENABLE_KEY					= "wenbox.enable";
const string Config::WENBOX_AUDIO_OUTPUT_DEVICEID_KEY	= "wenbox.audio.output.deviceid";
const string Config::WENBOX_AUDIO_INPUT_DEVICEID_KEY	= "wenbox.audio.input.deviceid";
const string Config::WENBOX_AUDIO_RINGER_DEVICEID_KEY	= "wenbox.audio.ringer.deviceid";

const string Config::IEACTIVEX_ENABLE_KEY						= "ieactivex.enable";

const string Config::CMDSERVER_AUTHORIZEDDOMAINS_KEY			= "cmdserver.authorizeddomains";

const string Config::FILETRANSFER_DOWNLOAD_FOLDER_KEY			= "filetransfer.downloadfolder";
const string Config::FILETRANSFER_LASTUPLOADEDFILE_FOLDER_KEY	= "filetransfer.lastuploadedfilefolder";

const string Config::LINUX_PREFERED_BROWSER_KEY					= "linux.prefered.browser";
const string Config::VOXOX_JABBER_SERVER_KEY					= "voxox.jabber.server";

const string Config::CALL_ENCRYPTION_MODE_KEY					= "call.encryption.mode";


const string Config::URL_ACCOUNTCREATION_KEY	= "url.accountcreation";
const string Config::URL_FORUM_KEY				= "url.forum";
const string Config::URL_CALLOUT_KEY			= "url.callout";

const string Config::URL_SMS_KEY				= "url.sms";
const string Config::URL_VOICEMAIL_KEY			= "url.voicemail";
const string Config::URL_EXTERNALDIRECTORY_KEY	= "url.externaldirectory";
const string Config::URL_INTERNALDIRECTORY_KEY	= "url.internaldirectory";

const string Config::URL_FAQ_KEY				= "url.faq";
const string Config::URL_ACCOUNT_KEY			= "url.account";
const string Config::URL_BUYWENGOS_KEY			= "url.buyvoxox";
const string Config::URL_DOWNLOAD_KEY			= "url.download";
const string Config::URL_PHONENUMBER_KEY		= "url.phonenumber";
const string Config::URL_WIKI_KEY				= "url.wiki";
const string Config::URL_BUGREPORT_KEY			= "url.bug";//VOXOX - SEMR - 2009.05.22 HELP MENU
const string Config::URL_SUGGESTIONS_KEY		= "url.suggestions";//VOXOX - SEMR - 2009.05.22 HELP MENU
const string Config::URL_BILLING_KEY			= "url.billing";//VOXOX - SEMR - 2009.05.22 HELP MENU
const string Config::URL_RATES_KEY				= "url.rates";	//VOXOX CHANGES by Rolando 04-02-09
const string Config::URL_DEVELOPER_PLATFORM		= "url.developer.platform";	//VOXOX - SEMR - 2009.07.08 
const string Config::URL_LOSTPASSWORD_KEY		= "url.lostpassword";
const string Config::URL_LAUNCHPAD_KEY			= "url.launchpad";
const string Config::URL_HOME_HTTP_KEY			= "url.home.http";
const string Config::URL_HOME_HTTPS_KEY			= "url.home.https";
const string Config::URL_CONFIGPHONESETTINGS_KEY = "url.config.phone.settings";
const string Config::URL_CONFIGTEXTSETTINGS_KEY = "url.config.text.settings";
const string Config::URL_CONFIGNOTIFICATIONSETTINGS_KEY = "url.config.notification.settings";//VOXOX - CJC - 2009.06.28 
const string Config::URL_COMPANYWEBSITE_KEY		= "url.companywebsite";
const string Config::URL_SENDFILES_KEY			= "url.sendfile";
const string Config::URL_SENDFAX_KEY			= "url.sendfax";
const string Config::URL_CONTACTPROFILE_KEY		= "url.contact.profile";

const string Config::URL_MYACCOUNT_KEY			= "url.myaccount";

const string Config::URL_CHANGEPASSWORD_KEY		= "url.change.password";
const string Config::URL_CONTACT_MANAGER_KEY	= "url.contactmanager";

const string Config::FEATURE_SMS_KEY			 = "feature.sms";
const string Config::FEATURE_AUDIOCONFERENCE_KEY = "feature.audioconference";

//VOXOX - JRT - 2009.11.05 - Debugging options which allow QA to alter behavior without having to rebuild the app.
const string Config::DEBUG_ENABLED						= "debug.enabled";
const string Config::DEBUG_SOFTUPDATE_UPDATE			= "debug.softupdate.update";
const string Config::DEBUG_SOFTUPDATE_TYPE				= "debug.softupdate.type";
const string Config::DEBUG_SOFTUPDATE_DOWNLOAD_URL		= "debug.softupdate.download_url";
const string Config::DEBUG_SOFTUPDATE_DESCRIPTION		= "debug.softupdate.description";
const string Config::DEBUG_SOFTUPDATE_VERSION			= "debug.softupdate.version";
const string Config::DEBUG_SOFTUPDATE_CMDLINE_PARMS		= "debug.softupdate.cmdline_parms";
const string Config::DEBUG_SOFTUPDATE_DOWNLOAD_NOW		= "debug.softupdate.download_now";

//const string Config::SOFTUPDATE_SHOWRECOMMENDED			= "softupdate.showrecommended"; // VOXOX -ASV- 2009.12.04

string Config::_configDir;

Config::Config(const string & name)
	: CascadingSettings() {

	_name = name;

	// Version is set to sticky otherwise it would be impossible to detect
	// config version changes
	addStickyKey(CONFIG_VERSION_KEY);
}

Config::~Config() {
}

string Config::getName() const {
	return _name;
}

int Config::getConfigVersion() const {
	return getIntegerKeyValue(CONFIG_VERSION_KEY);
}

bool Config::getNetworkSSOSSL() const {
	return getBooleanKeyValue(NETWORK_SSO_SSL_KEY);
}

string Config::getNetworkNatType() const {
	return getStringKeyValue(NETWORK_NAT_TYPE_KEY);
}

int Config::getNetworkSipLocalPort() const {
	return getIntegerKeyValue(NETWORK_SIP_LOCAL_PORT_KEY);
}

bool Config::getNetworkTunnelSSL() const {
	return getBooleanKeyValue(NETWORK_TUNNEL_SSL_KEY);
}

string Config::getNetworkTunnelServer() const {
	return getStringKeyValue(NETWORK_TUNNEL_SERVER_KEY);
}

bool Config::getNetWorkTunnelNeeded() const {
	return getBooleanKeyValue(NETWORK_TUNNEL_NEEDED_KEY);
}

int Config::getNetworkTunnelPort() const {
	return getIntegerKeyValue(NETWORK_TUNNEL_PORT_KEY);
}

bool Config::getNetworkProxyDetected() const {
	return getBooleanKeyValue(NETWORK_PROXY_DETECTED_KEY);
}

string Config::getNetworkProxyServer() const {
	return getStringKeyValue(NETWORK_PROXY_SERVER_KEY);
}

int Config::getNetworkProxyPort() const {
	return getIntegerKeyValue(NETWORK_PROXY_PORT_KEY);
}

string Config::getNetworkProxyLogin() const {
	return getStringKeyValue(NETWORK_PROXY_LOGIN_KEY);
}

string Config::getNetworkProxyPassword() const {
	return getStringKeyValue(NETWORK_PROXY_PASSWORD_KEY);
}

string Config::getNetworkStunServer() const {
	return getStringKeyValue(NETWORK_STUN_SERVER_KEY);
}

string Config::getCodecPluginPath() const {
	return getStringKeyValue(CODEC_PLUGIN_PATH_KEY);
}


string Config::getContactListStyle() const {
	return getStringKeyValue(CONTACT_LIST_STYLE_KEY);
}


string Config::getPhApiPluginPath() const {
	return getStringKeyValue(PHAPI_PLUGIN_PATH_KEY);
}

StringList Config::getAudioCodecList() const {
	StringList res = getStringListKeyValue(AUDIO_CODEC_LIST_KEY);
	return res;
}

bool Config::getAudioAEC() const {
	return getBooleanKeyValue(AUDIO_AEC_KEY);
}

bool Config::getAudioHalfDuplex() const {
	return getBooleanKeyValue(AUDIO_HALFDUPLEX_KEY);
}

bool Config::getAudioRingingEnable() const {
	return getBooleanKeyValue(AUDIO_ENABLE_RINGING_KEY);
}

string Config::getLastChatHistorySaveDir() const {
	return getStringKeyValue(LAST_CHAT_HISTORY_SAVE_DIR_KEY);
}

string Config::getAudioSmileysDir() const {
	return getResourceKeyValue(AUDIO_SMILEYS_DIR_KEY);
}

string Config::getAudioIncomingCallFile() const {
	return getResourceKeyValue(AUDIO_INCOMINGCALL_FILE_KEY);
}

string Config::getAudioDoubleCallFile() const {
	return getResourceKeyValue(AUDIO_DOUBLECALL_FILE_KEY);
}

string Config::getAudioCallClosedFile() const {
	return getResourceKeyValue(AUDIO_CALLCLOSED_FILE_KEY);
}

string Config::getAudioIncomingChatFile() const {
	return getResourceKeyValue(AUDIO_INCOMINGCHAT_FILE_KEY);
}

string Config::getAudioIMAccountConnectedFile() const {
	return getResourceKeyValue(AUDIO_IMACCOUNTCONNECTED_FILE_KEY);
}

string Config::getAudioIMAccountDisconnectedFile() const {
	return getResourceKeyValue(AUDIO_IMACCOUNTDISCONNECTED_FILE_KEY);
}

string Config::getAudioContactOnlineFile() const {
	return getResourceKeyValue(AUDIO_CONTACTONLINE_FILE_KEY);
}

/**
 * Code factorization.
 *
 * @see Config::getAudioRingerDeviceId()
 * @see Config::getAudioInputDeviceId()
 * @see Config::getAudioOutputDeviceId()
 * @param storedDeviceData the stored audio device
 * @param currentdeviceList the actual audio devices list
 * @param currentDefaultDevice the actual default device
 */
static StringList getProperAudioDeviceId(
	const StringList & storedDeviceData,
	const std::list<AudioDevice> & currentdeviceList,
	const StringList & currentDefaultDevice) {

	StringList result;

	// if no device stored use the default one
	if (storedDeviceData.empty()) {
		result = currentDefaultDevice;

	// else check if the stored device is currently available
	// do not exec this check under Linux because when devices
	// are busy they are not in currentdeviceList
	} else {
#ifndef OS_LINUX
		bool found = false;

		std::list<AudioDevice>::const_iterator it;
		for (it = currentdeviceList.begin(); it != currentdeviceList.end(); ++it) {

			if ((*it).getData() == storedDeviceData) {
				found = true;
			}
		}

		// if the stored device has been found we use it
		if (found) {
			result = storedDeviceData;
		} else {
			result = currentDefaultDevice;
		}
#else
		result = storedDeviceData;
#endif
	}
	return result;
}

StringList Config::getAudioOutputDeviceId() const {
	EnumWenboxStatus::WenboxStatus wenboxStatus =
		EnumWenboxStatus::toWenboxStatus(getWenboxEnable());
	if (wenboxStatus == EnumWenboxStatus::WenboxStatusEnable) {
		return getWenboxAudioOutputDeviceId();
	} else {
		return getProperAudioDeviceId(
			getStringListKeyValue(AUDIO_OUTPUT_DEVICEID_KEY),
			AudioDeviceManager::getInstance().getOutputDeviceList(),
			AudioDeviceManager::getInstance().getDefaultOutputDevice().getData()
		);
	}
}

StringList Config::getAudioInputDeviceId() const {
	EnumWenboxStatus::WenboxStatus wenboxStatus =
		EnumWenboxStatus::toWenboxStatus(getWenboxEnable());
	if (wenboxStatus == EnumWenboxStatus::WenboxStatusEnable) {
		return getWenboxAudioInputDeviceId();
	} else {
		return getProperAudioDeviceId(
			getStringListKeyValue(AUDIO_INPUT_DEVICEID_KEY),
			AudioDeviceManager::getInstance().getInputDeviceList(),
			AudioDeviceManager::getInstance().getDefaultInputDevice().getData()
		);
	}
}

StringList Config::getAudioRingerDeviceId() const {
	EnumWenboxStatus::WenboxStatus wenboxStatus =
		EnumWenboxStatus::toWenboxStatus(getWenboxEnable());
	if (wenboxStatus == EnumWenboxStatus::WenboxStatusEnable) {
		return getWenboxAudioRingerDeviceId();
	} else {
		return getProperAudioDeviceId(
			getStringListKeyValue(AUDIO_RINGER_DEVICEID_KEY),
			AudioDeviceManager::getInstance().getOutputDeviceList(),
			AudioDeviceManager::getInstance().getDefaultOutputDevice().getData()
		);
	}
}

string Config::getWengoServerHostname() const {
	return getStringKeyValue(WENGO_SERVER_HOSTNAME_KEY);
}
//VOXOX - CJC - 2009.06.27 
string Config::getApiHostname() const {
	return getStringKeyValue(API_HOSTNAME_KEY);
}

string Config::getTranslationApiHostname() const {
	return getStringKeyValue(TRANSLATION_API_HOSTNAME_KEY);
}
string Config::getWengoSSOPath() const {
	return getStringKeyValue(WENGO_SSO_PATH_KEY);
}

string Config::getWengoSMSPath() const {
	return getStringKeyValue(WENGO_SMS_PATH_KEY);
}

string Config::getWengoSubscribePath() const {
	return getStringKeyValue(WENGO_SUBSCRIBE_PATH_KEY);
}

string Config::getWengoInfoPath() const {
	return getStringKeyValue(WENGO_INFO_PATH_KEY);
}

string Config::getWengoSoftUpdatePath() const {
	return getStringKeyValue(WENGO_SOFTUPDATE_PATH_KEY);
}

string Config::getWengoCirpackPath() const {
	return getStringKeyValue(WENGO_CIRPACK_PATH_KEY);
}

string Config::getWengoDirectoryPath() const {
	return getStringKeyValue(WENGO_DIRECTORY_PATH_KEY);
}

string Config::getKeypadBackgroundPath() const { //VOXOX - SEMR - 2009.07.31 change keypad  background
	return getResourceKeyValue(KEYPAD_BACKGROUND_PATH_KEY);//VOXOX - CJC - 2009.09.07 Fix functionality that was not working at all....
}

string Config::getWengoRealm() const {
	return getStringKeyValue(WENGO_REALM_KEY);
}

string Config::getWengoAudioTestCall() const {
	return getStringKeyValue(WENGO_AUDIOTESTCALL_KEY);
}

string Config::getWengoVideoTestCall() const {
	return getStringKeyValue(WENGO_VIDEOTESTCALL_KEY);
}



string Config::getVoxoxContactNumberPath() const {
	return getStringKeyValue(VOXOX_CONTACT_NUMBER_PATH_KEY);
}

string Config::getVoxOxAccountWSPath() const {						//VOXOX - JRT - 2009.05.28 
	return getStringKeyValue(VOXOX_ACCOUNT_WS_PATH_KEY);
}

std::string Config::getVoxOxSyncWSPath() const {					//VOXOX - JRT - 2009.05.28 
	return getStringKeyValue(VOXOX_SYNC_WS_PATH_KEY);	
}

string Config::getVoxOxAskServer() const {							//VOXOX - JRT - 2009.05.28 
	return getStringKeyValue(VOXOX_ASK_SERVER_KEY);	
}

std::string Config::getVoxOxPAWSPath() const {					//VOXOX - CJC - 2009.06.27 
	return getStringKeyValue(VOXOX_PA_WS_PATH_KEY);	
}

std::string Config::getVoxOxTranslationWSPathKey() const {			//VOXOX - CJC - 2010.01.08 
	return getStringKeyValue(VOXOX_TRANSLATION_WS_PATH_KEY);	
}



std::string Config::getSipRegisterServer() const {
	return getStringKeyValue(SIP_REGISTER_SERVER);
}

int Config::getSipRegisterPort() const {
	return getIntegerKeyValue(SIP_REGISTER_PORT);
}

std::string Config::getSipProxyServer() const {
	return getStringKeyValue(SIP_PROXY_SERVER);
}

int Config::getSipProxyPort() const {
	return getIntegerKeyValue(SIP_PROXY_PORT);
}

std::string Config::getSipRealm() const {
	return getStringKeyValue(SIP_REALM);
}

bool Config::getSipSimpleSupport() const {
	return getBooleanKeyValue(SIP_SIMPLE_SUPPORT);
}

string Config::getSipAudioTestCall() const {
	return getStringKeyValue(SIP_AUDIOTESTCALL_KEY);
}

string Config::getSipVideoTestCall() const {
	return getStringKeyValue(SIP_VIDEOTESTCALL_KEY);
}

int Config::getSipRegisterTimeOut() const {
	return getIntegerKeyValue(SIP_REGISTER_TIMEOUT);
}

int Config::getSipPublishTimeOut() const {
	return getIntegerKeyValue(SIP_PUBLISH_TIMEOUT);
}

bool Config::getSipUseOptions() const {
	return getBooleanKeyValue(SIP_USE_OPTIONS);
}

bool Config::getSipP2pPresence() const {
	return getBooleanKeyValue(SIP_P2P_PRESENCE);
}

bool Config::getSipChatWithoutPresence() const {
	return getBooleanKeyValue(SIP_CHAT_WITHOUT_PRESENCE);
}

//VOXOX CHANGE BY ROLANDO 04-07-09
string Config::getLastDialedNumber() const {
	return getStringKeyValue(LAST_DIALED_NUMBER_KEY);
}

string Config::getProfileLastUsedName() const {
	return getStringKeyValue(PROFILE_LAST_USED_NAME_KEY);
}

string Config::getProfileLastLoggedAccount() const {
	return getStringKeyValue(PROFILE_LAST_LOGGED_ACCOUNT_KEY);
}

int Config::getProfileWidth() const {
	return getIntegerKeyValue(PROFILE_WIDTH_KEY);
}

int Config::getProfileHeight() const {
	return getIntegerKeyValue(PROFILE_HEIGHT_KEY);
}

int Config::getProfilePosX() const {
	return getIntegerKeyValue(PROFILE_POSX_KEY);
}

int Config::getProfilePosY() const {
	return getIntegerKeyValue(PROFILE_POSY_KEY);
}

int Config::getChatWindowPosX() const {
	return getIntegerKeyValue(CHAT_WINDOW_POSX_KEY);
}

int Config::getChatWindowPosY() const {
	return getIntegerKeyValue(CHAT_WINDOW_POSY_KEY);
}

int Config::getLastOpenTabIndex() const {
	return getIntegerKeyValue(LAST_OPEN_TAB_INDEX);
}
string Config::getCallForwardMode() const {
	return getStringKeyValue(CALL_FORWARD_MODE_KEY);
}

string Config::getCallForwardPhoneNumber1() const {
	return getStringKeyValue(CALL_FORWARD_PHONENUMBER1_KEY);
}

string Config::getCallForwardPhoneNumber2() const {
	return getStringKeyValue(CALL_FORWARD_PHONENUMBER2_KEY);
}

string Config::getCallForwardPhoneNumber3() const {
	return getStringKeyValue(CALL_FORWARD_PHONENUMBER3_KEY);
}

bool Config::getVoiceMailActive() const {
	return getBooleanKeyValue(VOICE_MAIL_ACTIVE_KEY);
}

bool Config::getGeneralAutoStart() const {
	return getBooleanKeyValue(GENERAL_AUTOSTART_KEY);
}
//VOXOX - CJC - 2009.06.01 
bool Config::getWizardAutoStart() const {
	return getBooleanKeyValue(WIZARD_AUTOSTART_KEY);
}

bool Config::getWizardAssistandIsMale() const {
	return getBooleanKeyValue(WIZARD_ASSISTANT_IS_MALE);
}

//VOXOX - CJC - 2009.06.01 
int Config::getWizardStepsComplete() const {
	return getIntegerKeyValue(WIZARD_STEP_COMPLETE_KEY);
}


bool Config::getGeneralClickStartFreeCall() const {
	return getBooleanKeyValue(GENERAL_CLICK_START_FREECALL_KEY);
}

bool Config::getGeneralClickStartChat() const {
	return getBooleanKeyValue(GENERAL_CLICK_START_CHAT_KEY);
}

bool Config::getGeneralClickCallCellPhone() const {
	return getBooleanKeyValue(GENERAL_CLICK_CALL_CELLPHONE_KEY);
}

int Config::getGeneralAwayTimer() const {
	return getIntegerKeyValue(GENERAL_AWAY_TIMER_KEY);
}

bool Config::getShowGroups() const {
	return getBooleanKeyValue(GENERAL_SHOW_GROUPS_KEY);
}

bool Config::getShowOfflineContacts() const {
	return getBooleanKeyValue(GENERAL_SHOW_OFFLINE_CONTACTS_KEY);
}
//VOXOX CHANGE Rolando 03-20-09
bool Config::getShowProfileBar() const {
	return getBooleanKeyValue(SHOW_PROFILE_BAR_KEY);
}

//VOXOX CHANGE Rolando 03-25-09
bool Config::getShowCallBar() const {
	return getBooleanKeyValue(SHOW_CALL_BAR_KEY);
}

//VOXOX CHANGE Rolando 03-24-09
string Config::getCurrentFlag() const {
	return getStringKeyValue(CURRENT_FLAG_KEY);
}

//VOXOX CHANGE Rolando 03-24-09
string Config::getCurrentCountryDefault() const {
	return getStringKeyValue(CURRENT_COUNTRY_DEFAULT_KEY);
}

//VOXOX CHANGE by Rolando - 2009.06.29 
std::string Config::getLastStatusMessageAccountKey() const
{
	return getStringKeyValue(LAST_STATUS_MESSAGE_ACCOUNT_KEY);
}

//VOXOX CHANGE by Rolando - 2009.06.29 
std::string Config::getLastStatusMessageDisplayed() const
{
	return getStringKeyValue(LAST_STATUS_MESSAGE_DISPLAYED);
}

//VOXOX - JRT - 2009.05.03 
int Config::getContactSort() const 
{
	return getIntegerKeyValue(GENERAL_CONTACT_SORT_KEY);
}

int Config::getContactGrouping() const 
{
	return getIntegerKeyValue(GENERAL_CONTACT_GROUPING_KEY);
}

int Config::getGroupSort() const 
{
	return getIntegerKeyValue(GENERAL_GROUP_SORT_KEY);
}
//End VoxOx


std::string Config::getHyperlinkProtocol() const {
	return getStringKeyValue(GENERAL_HYPERLINKPROTOCOL_KEY);
}

std::string Config::getToolBarMode() const {
	return getStringKeyValue(GENERAL_TOOLBARMODE_KEY);
}

string Config::getChatTheme() const {
	return getStringKeyValue(APPEARANCE_CHATTHEME_KEY);
}

string Config::getChatThemeVariant() const {
	return getStringKeyValue(APPEARANCE_CHATTHEME_VARIANT_KEY);
}

string Config::getEmoticonPack() const {
	return getStringKeyValue(EMOTICON_CHAT_KEY);
}

string Config::getAskIMVoxOxComAddress() const {
	return getStringKeyValue(ASKIMVOXOXCOM_ADDRESS_KEY);
}

string Config::getSelectedDictionary() const {
	return getResourcesDir()+"dics/"+getStringKeyValue(SELECTED_DICTIONARY_KEY);
}
string Config::getLanguage() const {
	return getStringKeyValue(LANGUAGE_KEY);
}

bool Config::getNotificationShowToasterOnIncomingCall() const {
	return getBooleanKeyValue(NOTIFICATION_SHOW_TOASTER_ON_INCOMING_CALL_KEY);
}

//VOXOX - CJC - 2009.05.25 
bool Config::getNotificationPlaySoundOnIncomingCall() const {
	return getBooleanKeyValue(NOTIFICATION_PLAY_SOUND_ON_INCOMING_CALL_KEY);
}

bool Config::getNotificationShowToasterOnIncomingChat() const {
	return getBooleanKeyValue(NOTIFICATION_SHOW_TOASTER_ON_INCOMING_CHAT_KEY);
}
//VOXOX - CJC - 2009.05.25 
bool Config::getNotificationPlaySoundOnIncomingChat() const {
	return getBooleanKeyValue(NOTIFICATION_PLAY_SOUND_ON_INCOMING_CHAT_KEY);
}
//VOXOX - CJC - 2009.05.25 
bool Config::getNotificationPlaySoundOnCallClosed() const {
	return getBooleanKeyValue(NOTIFICATION_PLAY_SOUND_ON_CALL_CLOSED);
}

bool Config::getNotificationShowToasterOnContactOnline() const {
	return getBooleanKeyValue(NOTIFICATION_SHOW_TOASTER_ON_CONTACT_ONLINE_KEY);
}

bool Config::getNotificationDoNotDisturbNoWindow() const {
	return getBooleanKeyValue(NOTIFICATION_DONOTDISTURB_NO_WINDOW_KEY);
}

bool Config::getNotificationDoNotDisturbNoAudio() const {
	return getBooleanKeyValue(NOTIFICATION_DONOTDISTURB_NO_AUDIO_KEY);
}

bool Config::getNotificationAwayNoWindow() const {
	return getBooleanKeyValue(NOTIFICATION_AWAY_NO_WINDOW_KEY);
}

bool Config::getNotificationAwayNoAudio() const {
	return getBooleanKeyValue(NOTIFICATION_AWAY_NO_AUDIO_KEY);
}

bool Config::getPrivacyAllowCallFromAnyone() const {
	return getBooleanKeyValue(PRIVACY_ALLOW_CALL_FROM_ANYONE_KEY);
}

bool Config::getPrivacyAllowCallOnlyFromContactList() const {
	return getBooleanKeyValue(PRIVACY_ALLOW_CALL_ONLY_FROM_CONTACT_LIST_KEY);
}

bool Config::getPrivacyAllowChatsFromAnyone() const {
	return getBooleanKeyValue(PRIVACY_ALLOW_CHAT_FROM_ANYONE_KEY);
}

bool Config::getPrivacyAllowChatOnlyFromContactList() const {
	return getBooleanKeyValue(PRIVACY_ALLOW_CHAT_ONLY_FROM_CONTACT_LIST_KEY);
}

bool Config::getPrivacySignAsInvisible() const {
	return getBooleanKeyValue(PRIVACY_SIGN_AS_INVISIBLE_KEY);
}

bool Config::getVideoEnable() const {
	if (getVideoWebcamDevice().empty()) {
		return false;
	} else {
		return getBooleanKeyValue(VIDEO_ENABLE_KEY);
	}
}

string Config::getVideoWebcamDevice() const {
	WebcamDriver * webcam = WebcamDriver::getInstance();

	string deviceName = getStringKeyValue(VIDEO_WEBCAM_DEVICE_KEY);

	//FIXME see fixme DirectXWebcamDriver.h
	//string defaultDevice = webcam->getDefaultDevice();
	string defaultDevice = deviceName;

	if (defaultDevice == WEBCAM_NULL) {
		defaultDevice = String::null;
	}
	StringList deviceList = webcam->getDeviceList();

	if (deviceName.empty()) {
		return defaultDevice;
	} else if (!deviceList.contains(deviceName)) {
		return defaultDevice;
	} else {
		return deviceName;
	}
}

string Config::getVideoQuality() const {
	return getStringKeyValue(VIDEO_QUALITY_KEY);
}

bool Config::getXVideoEnable() const {
	return getBooleanKeyValue(VIDEO_ENABLE_XVIDEO_KEY);
}

bool Config::getVideoFlipEnable() const {
	return getBooleanKeyValue(VIDEO_ENABLE_FLIP_KEY);
}

string Config::getWenboxEnable() const {
	return getStringKeyValue(WENBOX_ENABLE_KEY);
}

StringList Config::getWenboxAudioOutputDeviceId() const {
	return getStringListKeyValue(WENBOX_AUDIO_OUTPUT_DEVICEID_KEY);
}

StringList Config::getWenboxAudioInputDeviceId() const {
	return getStringListKeyValue(WENBOX_AUDIO_INPUT_DEVICEID_KEY);
}

StringList Config::getWenboxAudioRingerDeviceId() const {
	return getStringListKeyValue(WENBOX_AUDIO_RINGER_DEVICEID_KEY);
}

bool Config::getIEActiveXEnable() const {
	return getBooleanKeyValue(IEACTIVEX_ENABLE_KEY);
}

StringList Config::getCmdServerAuthorizedDomains() const {
	return getStringListKeyValue(CMDSERVER_AUTHORIZEDDOMAINS_KEY);
}

string Config::getFileTransferDownloadFolder() const {
	return getStringKeyValue(FILETRANSFER_DOWNLOAD_FOLDER_KEY);
}

string Config::getLinuxPreferedBrowser() const {
	return getStringKeyValue(LINUX_PREFERED_BROWSER_KEY);
}

string Config::getLastUploadedFileFolder() const {
	return getStringKeyValue(FILETRANSFER_LASTUPLOADEDFILE_FOLDER_KEY);
}

bool Config::getCallEncryptionMode() const {
	return getBooleanKeyValue(CALL_ENCRYPTION_MODE_KEY);
}
string Config::getAccountCreationUrl() const {
	return getStringKeyValue(URL_ACCOUNTCREATION_KEY);
}

string Config::getForumUrl() const {
	return getStringKeyValue(URL_FORUM_KEY);
}

string Config::getCalloutUrl() const {
	return getStringKeyValue(URL_CALLOUT_KEY);
}

string Config::getSmsUrl() const {
	return getStringKeyValue(URL_SMS_KEY);
}

string Config::getVoiceMailUrl() const {
	return getStringKeyValue(URL_VOICEMAIL_KEY);
}

string Config::getExternalDirectoryUrl() const {
	return getStringKeyValue(URL_EXTERNALDIRECTORY_KEY);
}

string Config::getInternalDirectoryUrl() const {
	return getStringKeyValue(URL_INTERNALDIRECTORY_KEY);
}

string Config::getFaqUrl() const {
	return getStringKeyValue(URL_FAQ_KEY);
}

string Config::getAccountUrl() const {
	return getStringKeyValue(URL_ACCOUNT_KEY);
}

string Config::getBuyWengosUrl() const {
	return getStringKeyValue(URL_BUYWENGOS_KEY);
}

string Config::getDownloadUrl() const {
	return getStringKeyValue(URL_DOWNLOAD_KEY);
}

//VOXOX CHANGE FILE TRANSFER URL
string Config::getSendFileUrl() const {
	return getStringKeyValue(URL_SENDFILES_KEY);
}

string Config::getSendFaxUrl() const {
	return getStringKeyValue(URL_SENDFAX_KEY);
}



//VOXOX CHANGE CONTACT PROFILE
string Config::getContactProfileUrl() const {
	return getStringKeyValue(URL_CONTACTPROFILE_KEY);
}



string Config::getPhoneNumberUrl() const {
	return getStringKeyValue(URL_PHONENUMBER_KEY);
}

//VOXOX CHANGE CJC SUPPORT PHONE SETTINGS INSIDE FRAME
string Config::getConfigPhoneSettingsUrl() const {
	return getStringKeyValue(URL_CONFIGPHONESETTINGS_KEY);
}

//VOXOX CHANGE CJC SUPPORT PHONE SETTINGS INSIDE FRAME
string Config::getConfigTextSettingsUrl() const {
	return getStringKeyValue(URL_CONFIGTEXTSETTINGS_KEY);
}

string Config::getConfigNotificationSettingsUrl() const {//VOXOX - CJC - 2009.06.28 
	return getStringKeyValue(URL_CONFIGNOTIFICATIONSETTINGS_KEY);
}


string Config::getWikiUrl() const {
	return getStringKeyValue(URL_WIKI_KEY);
}

//VOXOX - SEMR - 2009.05.22 HELP MENU
string Config::getBugReportUrl() const {
	return getStringKeyValue(URL_BUGREPORT_KEY);
}

//VOXOX - SEMR - 2009.05.22 HELP MENU
string Config::getSuggestionsUrl() const {
	return getStringKeyValue(URL_SUGGESTIONS_KEY);
}

//VOXOX - SEMR - 2009.05.22 HELP MENU
string Config::getBillingUrl() const {
	return getStringKeyValue(URL_BILLING_KEY);
}

//VOXOX CHANGES by Rolando 04-02-09
string Config::getRatesUrl() const {
	return getStringKeyValue(URL_RATES_KEY);
}

string Config::getDeveloperPlatformUrl() const {
	return getStringKeyValue(URL_DEVELOPER_PLATFORM);
}

string Config::getLostPasswordUrl() const {
	return getStringKeyValue(URL_LOSTPASSWORD_KEY);
}

string Config::getLaunchpadUrl() const {
	return getStringKeyValue(URL_LAUNCHPAD_KEY);
}

string Config::getHttpHomeUrl() const {
	return getStringKeyValue(URL_HOME_HTTP_KEY);
}

string Config::getHttpsHomeUrl() const {
	return getStringKeyValue(URL_HOME_HTTPS_KEY);
}

string Config::getCompanyWebSiteUrl() const {
	return getStringKeyValue(URL_COMPANYWEBSITE_KEY);
}

string Config::getMyAccountUrl() const {
	return getStringKeyValue(URL_MYACCOUNT_KEY);
}

string Config::getChangePasswordUrl() const {//VOXOX - CJC - 2009.07.03 
	return getStringKeyValue(URL_CHANGEPASSWORD_KEY);
}

string Config::getContactManagerUrl() const {
	return getStringKeyValue(URL_CONTACT_MANAGER_KEY);
}





string Config::getJabberVoxoxServer() const {
	return getStringKeyValue(VOXOX_JABBER_SERVER_KEY);
}
bool Config::getSmsFeatureEnabled() const {
	return getBooleanKeyValue(FEATURE_SMS_KEY);
}

bool Config::getAudioConferenceFeatureEnabled() const {
	return getBooleanKeyValue(FEATURE_AUDIOCONFERENCE_KEY);
}

std::string Config::getConfigDir() {
	if (_configDir.empty()) {
		LOG_FATAL("getConfigDir() called before setConfigDir()");
	}
	return _configDir;
}

void Config::setConfigDir(const std::string& configDir) {
	_configDir = configDir;
}

bool Config::getSipUseTypingState() const
{
	return getBooleanKeyValue(SIP_USE_TYPING_STATE);
}

//-----------------------------------------------------------------------------
	
//VOXOX -ASV- 2009.12.04
//VOXOX - JRT - 2009.12.10 - Should be at user level, not in config.xml
//bool Config::getSoftUpdateShowRecommended() const
//{
//	return getBooleanKeyValue(SOFTUPDATE_SHOWRECOMMENDED);
//}

//VOXOX - JRT - 2009.11.05 - Debugging options which allow QA to alter behavior without having to rebuild the app.
bool Config::isDebugEnabled() const 
{
	return getBooleanKeyValue(DEBUG_ENABLED);
}

bool Config::getDebugSoftUpdateUpdate() const 
{
	return getBooleanKeyValue(DEBUG_SOFTUPDATE_UPDATE);
}

string Config::getDebugSoftUpdateType() const 
{
	return getStringKeyValue(DEBUG_SOFTUPDATE_TYPE);
}

string Config::getDebugSoftUpdateDownloadUrl() const 
{
	return getStringKeyValue(DEBUG_SOFTUPDATE_DOWNLOAD_URL);
}

string Config::getDebugSoftUpdateDescription() const 
{
	return getStringKeyValue(DEBUG_SOFTUPDATE_DESCRIPTION);
}

string Config::getDebugSoftUpdateVersion() const 
{
	return getStringKeyValue(DEBUG_SOFTUPDATE_VERSION);
}

string Config::getDebugSoftUpdateCmdLineParms() const 
{
	return getStringKeyValue(DEBUG_SOFTUPDATE_CMDLINE_PARMS);
}

bool Config::getDebugSoftUpdateDownloadNow() const 
{
	return getBooleanKeyValue(DEBUG_SOFTUPDATE_DOWNLOAD_NOW);
}

//-----------------------------------------------------------------------------
