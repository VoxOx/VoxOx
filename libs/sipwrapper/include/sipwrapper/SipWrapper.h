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

#ifndef OWSIPWRAPPER_H
#define OWSIPWRAPPER_H

#include <sipwrapper/EnumPhoneCallState.h>
#include <sipwrapper/EnumPhoneLineState.h>
#include <sipwrapper/EnumTone.h>
#include <sipwrapper/EnumNatType.h>
#include <sipwrapper/EnumVideoQuality.h>
#include <sipwrapper/CodecList.h>

#include <util/Interface.h>
#include <util/Event.h>

#include <string>

#include <pixertool/pixertool.h>

class AudioDevice;
class WebcamVideoFrame;

/**
 * Wrapper for SIP stacks.
 *
 * A SIP stack is a piece of code that implements the SIP protocol
 * http://www.cs.columbia.edu/sip/
 *
 * Using this interface, different SIP stacks can be implemented
 * like phApi or sipX.
 * Even other voip protocols can be implemented like IAX http://www.asterisk.org/
 *
 * SipWrapper works like a plugin interface. One can imagine to convert SipWrapper implementations
 * (SIP stacks) into .dll (or .so) in order to load them dynamically at runtime.
 * SIP stacks can also be all integrated inside the final application binary.
 * This first solution is ideally the prefered one specially if each SIP stack
 * has a lot of dependencies.
 * The last solution helps when it comes to distribute easily the final application.
 *
 * In order to integrate a new SIP implementation for WengoPhone,
 * create a subdirectory by the name of the SIP implementation.
 * Subclass the SipWrapper interface. SIP events/callbacks are handled via the Event class.
 *
 * Create a factory implementing the interface SipWrapperFactory.
 * Inside main.cpp just instanciates the right factory for your plugin.
 * Check the current SIP wrapper implementations: phApi and sipX.
 *
 * SipWrapper handles telephony and video only.
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 * @author Julien Bossart
 */
class SipWrapper : Interface {
public:

	/**
	 * Phone call state changed.
	 *
	 * @param sender this class
	 * @param callId phone call identifier that changed its state
	 * @param state new call state
	 * @param from usually a SIP address (when state = PhoneCallStateIncoming) or empty string
	 */
	Event<void (SipWrapper & sender, int callId,
		EnumPhoneCallState::PhoneCallState state, const std::string & from)> phoneCallStateChangedEvent;

	/**
	 * Phone line state changed.
	 *
	 * @param sender this class
	 * @param lineId phone line identifier that changed its state
	 * @param state new line state
	 */
	Event<void (SipWrapper & sender, int lineId,
		EnumPhoneLineState::PhoneLineState state)> phoneLineStateChangedEvent;

	/**
	 * A video frame has been received from the network.
	 *
	 * @param sender this class
	 * @param callId phone call that received the video frame
	 * @param remoteVideoFrame remote (network) webcam video frame
	 * @param localVideoFrame local webcam video frame
	 */
	Event<void (SipWrapper & sender, int callId,
		piximage* remoteVideoFrame, piximage* localVideoFrame)> videoFrameReceivedEvent;



	/**
	 * Incoming SUBSCRIBE request
	 *
	 * @param sender this class
	 * @param sid    Subsciption ID
	 * @param from   subscription originator
	 */
	Event<void (SipWrapper & sender, int sid, const std::string& from, const std::string& evtType)> incomingSubscribeEvent;


	virtual ~SipWrapper() {
	}

	/**
	 * @name Initialization Methods
	 * @{
	 */

	/**
	 * Initializes the SIPWrapper.
	 */
	virtual void init() = 0;

	/**
	 * Terminates the SIP connection.
	 */
	virtual void terminate() = 0;

	/** @} */

	/**
	 * @name Virtual Line Methods
	 * @{
	 */

	/** An error occured while creating a virtual line. */
	static const int VirtualLineIdError = -1;

	/**
	 * Creates and adds a virtual phone line.
	 *
	 * TODO phAddAuthInfo() should be separated from phAddVline()
	 * This method has to be reworked
	 *
	 * @param displayName display name inside the SIP URI (e.g tanguy inside "tanguy <sip:tanguy_k@wengo.fr>")
	 * @param username most of the time same as identity
	 * @param identity first part of the SIP URI (e.g tanguy_k inside "sip:tanguy_k@wengo.fr")
	 * @param password password corresponding to the SIP URI
	 * @param realm realm/domain
	 * @param proxyServer SIP proxy server
	 * @param registerServer SIP register server
	 * @return the corresponding id of the virtual line just created or VirtualLineIdError if failed to create
	 *         the virtual line
	 */
	virtual int addVirtualLine(const std::string & displayName,
		const std::string & username,
		const std::string & identity,
		const std::string & password,
		const std::string & realm,
		const std::string & proxyServer,
		const std::string & registerServer) = 0;

	/**
	 * Registers a given virtual phone line.
	 *
	 * @param	lineId id of the virtual phone line to register
	 * @return	0 if making a register succeeds, otherwise 1
	 */
	virtual int registerVirtualLine(int lineId) = 0;

	/**
	 * Removes a given virtual phone line.
	 *
	 * @param	lineId id of the virtual phone line to remove
	 * @param	force if true, forces the removal without waiting for the unregister
				if false, removal is not forced
	 */
	virtual void removeVirtualLine(int lineId, bool force = false) = 0;

	/** @} */

	/**
	 * @name Call Methods
	 * @{
	 */

	/** An error occured while creating a phone call. */
	static const int CallIdError = -1;

	/**
	 * Dials a phone number.
	 *
	 * @param lineId line to use to dial the phone number
	 * @param sipAddress SIP address to call (e.g phone number to dial)
	 * @param enableVideo enable/disable video usage
	 * @return the phone call id (callId)
	 */
	virtual int makeCall(int lineId, const std::string & sipAddress, bool enableVideo) = 0;

	/**
	 * Notifies the remote side (the caller) that this phone is ringing.
	 *
	 * This corresponds to the SIP code "180 Ringing".
	 *
	 * @param callId id of the phone call to make ringing
	 */
	virtual void sendRingingNotification(int callId, bool enableVideo) = 0;

	/**
	 * Accepts a given phone call.
	 *
	 * @param callId id of the phone call to accept
	 * @param enableVideo enable/disable video usage
	 */
	virtual void acceptCall(int callId, bool enableVideo) = 0;

	/**
	 * Rejects a given phone call.
	 *
	 * @param callId id of the phone call to reject
	 */
	virtual void rejectCall(int callId) = 0;

	/**
	 * Closes a given phone call.
	 *
	 * @param callId id of the phone call to close
	 */
	virtual void closeCall(int callId) = 0;

	/**
	 * Holds a given phone call.
	 *
	 * @param callId id of the phone call to hold
	 */
	virtual void holdCall(int callId) = 0;

	/**
	 * Resumes a given phone call.
	 *
	 * @param callId id of the phone call to resume
	 */
	virtual void resumeCall(int callId) = 0;

	/**
	 * Blind transfer the specified call to another party.
	 *
	 * @param callId id of the phone call to transfer
	 * @param sipAddress transfer target
	 */
	virtual void blindTransfer(int callId, const std::string & sipAddress) = 0;

	/**
	 * Sends a DTMF to a given phone call.
	 *
	 * @param callId phone call id to send a DTMF
	 * @param dtmf DTMF tone to send
	 */
	virtual void playDtmf(int callId, char dtmf) = 0;

	/**
	 * Sends and plays a sound file to a given phone call.
	 *
	 * @param callId phone call id to play the sound file
	 * @param soundFile sound file to play
	 */
	virtual void playSoundFile(int callId, const std::string & soundFile) = 0;

	/**
	 * Sets the audio codec list sorted by preference
	 *
	 * @param audioCodecList the audio codec list
	 */
	virtual void setAudioCodecList(const StringList & audioCodecList) = 0;

	/**
	 * Gets the audio codec in use by a given phone call.
	 *
	 * @param callId phone call id
	 * @return audio codec in use
	 */
	virtual CodecList::AudioCodec getAudioCodecUsed(int callId) = 0;

	/**
	 * Gets the video codec in use by a given phone call.
	 *
	 * @param callId phone call id
	 * @return video codec in use
	 */
	virtual CodecList::VideoCodec getVideoCodecUsed(int callId) = 0;

	/**
	 * Sets the video device.
	 *
	 * @param deviceName the name of the video device
	 */
	virtual void setVideoDevice(const std::string & deviceName) = 0;

	/**
	 * Sets calls encrytion.
	 *
	 * @param enable if True encryption is activated
	 */
	virtual void setCallsEncryption(bool enable) = 0;

	/**
	 * @param callId phone call id
	 * @return  True if encrytion is activated for the given call
	 */
	virtual bool isCallEncrypted(int callId) = 0;

	/**
	 * @return  True if is Initialized
	 */

	virtual bool isInitialized(void) = 0;

	/** @} */

	/**
	 * @name Configuration Methods
	 * @{
	 */

	/**
	 * Sets proxy parameters.
	 */
	virtual void setProxy(const std::string & address, unsigned port,
		const std::string & login, const std::string & password) = 0;

	/**
	 * Sets HTTP tunnel parameters.
	 */
	virtual void setTunnel(const std::string & address, unsigned port, bool ssl) = 0;

	/**
	 * Sets the NAT type.
	 *
	 * @see EnumNatType
	 */
	virtual void setNatType(EnumNatType::NatType natType) = 0;

	/**
	 * Sets the NAT type.
	 *
	 * @see EnumVideoQuality
	 */
	virtual void setVideoQuality(EnumVideoQuality::VideoQuality videoQuality) = 0;

	/**
	 * Sets the SIP parameters.
	 */
	virtual void setSIP(const std::string & server, unsigned serverPort, unsigned localPort) = 0;

	/** @} */


	/**
	 * @name Conference Methods
	 * @{
	 */

	/** An error occured while creating a phone call. */
	static const int ConfIdError = -1;

	/**
	 * Creates a new conference given a phone call.
	 *
	 * Conferences are an association of calls
	 * where the audio media is mixed.
	 *
	 * @param callid phone call that will be transformed into a conference call
	 * @return conference id or ConfIdError if an error occured
	 */
	virtual int createConference() = 0;

	/**
	 * Joins (adds) an existing call into a conference.
	 *
	 * @param confId conference id
	 * @param callId phone call id
	 */
	virtual void joinConference(int confId, int callId) = 0;

	/**
	 * Splits (removes) an existing call from a conference.
	 *
	 * @param confId conference id
	 * @param callId existing call to remove from the conference
	 */
	virtual void splitConference(int confId, int callId) = 0;

	/** @} */


	/**
	 * @name Audio Methods
	 * @{
	 */

	/**
	 * Sets the call input device (in-call microphone).
	 *
	 * @param device input device
	 * @return true if no error, false otherwise
	 */
	virtual bool setCallInputAudioDevice(const AudioDevice & device) = 0;

	/**
	 * Sets the call ringer/alerting device.
	 *
	 * @param device ringer device
	 * @return true if no error, false otherwise
	 */
	virtual bool setRingerOutputAudioDevice(const AudioDevice & device) = 0;

	/**
	 * Sets the call output device (in-call speaker).
	 *
	 * @param device output device
	 * @return true if no error, false otherwise
	 */
	virtual bool setCallOutputAudioDevice(const AudioDevice & device) = 0;

	/**
	 * Enables or disables Acoustic Echo Cancellation (AEC).
	 *
	 * @param enable true if AEC enable, false if AEC should be disabled
	 */
	virtual void enableAEC(bool enable) = 0;

	/**
	 * Enables or disables half duplex mode.
	 *
	 * Two modes exist:
	 * - one where microphone signal has priority over speaker signal
	 * - one where speaker signal has priority over microphone signal
	 *
	 * @param enable true if half duplex enable, false if half duplex should be disabled
	 */
	virtual void enableHalfDuplex(bool enable) = 0;

	/**
	 * Enables or disables presence and IM.
	 *
	 * @param enable true to enable presence and IM, false if they should be disabled
	 */
	virtual void enablePIM(bool enable) = 0;

	/**
	 * Plays the designed file (sipX).
	 *
	 * Mix a sound file into the outgoing network audio stream (phApi).
	 *
	 * The file may be a raw 16 bit signed PCM at 8000 samples/sec, mono, little endian (sipX).
	 * RAW audio files containing 16Bit signed PCM sampled at 16KHZ are supported (phApi).
	 *
	 * @param soundFilename sound file to play
	 */
	//virtual playSoundFile(const std::string & soundFilename) = 0;

	/**
	 * Retrieves current codec preference setting.
	 *
	 * @return current codec list supported by the SIP stack
	 */
	//virtual List<AudioCodec> getCodecList() const = 0;

	/**
	 * Sets current codec preference setting.
	 *
	 * @param codecList new codec list
	 */
	//virtual void setCodecList(const List<AudioCodec> & codecList) = 0;

	/** @} */

	/**
	 * @name Video Methods
	 * @{
	 */

	/**
	 * Set video image flip.
	 * This parameter is dynamic so it can be set during a call.
	 *
	 * @param flip if true flip the image
	 */
	virtual void flipVideoImage(bool flip) = 0;

	/** @} */

	/**
	 * Sets the plugins path.
	 *
	 * @param path the absolute path to the plugins (e.g: "/home/robert/plugins")
	 */
	virtual void setPluginPath(const std::string & path) = 0;


	/**
	   
	 */
	virtual void setUaName(const std::string& name) { }
	virtual void setSipOptions(const std::string& optname, const std::string& optval) { }


	virtual void acceptSubscription(int sid) { }
	virtual void rejectSubscription(int sid) { }

	

};

#endif	//OWSIPWRAPPER_H
