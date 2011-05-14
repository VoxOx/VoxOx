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

#ifndef VOX_ENUMIMPROTOCOL_H
#define VOX_ENUMIMPROTOCOL_H

#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/EnumPresenceState.h>
#include <map>	
#include <util/String.h>

/**
 * Instant Messaging protocols.
 *
 * This class duplicates EnumIMProtocol because
 * protocols at network level are not always the one that we want to show
 * to the user.
 *
 * For example at network level there is no GoogleTalk protocol but just Jabber protocol;
 * at graphical level there is a GoogleTalk protocol for the user because it's easier.
 *
 * @see EnumIMProtocol
 * @author Philippe Bernery
 * @author Tanguy Krotoff
 */

typedef std::map<EnumPresenceState::PresenceState, EnumPresenceState::PresenceState> PresenceStateMap;

//class QtEnumIMProtocol : NonCopyable {
class QtEnumIMProtocol	//VOXOX - JRT - 2009.04.20  TODO: derive from EnumIMProtocol list once that is implemented.
{
public:
	enum IMProtocol 
	{
		IMProtocolUnknown	 =  0,
		IMProtocolAll		 =  1,

		//IM/Social
		IMProtocolMSN		 =  2,
		IMProtocolMYSPACE	 =  3,	//Social/IM
		IMProtocolFacebook	 =  4,	//Social
		IMProtocolTwitter	 =  5,	//Social
		IMProtocolSkype		 =  6,
		IMProtocolYahoo		 =  7,
		IMProtocolAIM		 =  8,
		IMProtocolICQ		 =  9,
		IMProtocolJabber	 = 10,
		IMProtocolGoogleTalk = 11,
		IMProtocolVoxOx		 = 12,

		//SIP
		IMProtocolWengo		 = 13,
		IMProtocolSIP		 = 14,
		IMProtocolSIPSIMPLE	 = 15,
	};

	enum ServerProtocolId
	{
		ServerProtocolIdNone		= -1,
		ServerProtocolIdUnknown		=  0,

		//NOTE: DO NOT RENUMBER OR REORDER THESE UNLESS THE SERVER SIDE TABLE CHANGES.
		ServerProtocolIdMSN			=  1,
		ServerProtocolIdAIM			=  2,
		ServerProtocolIdICQ			=  3,
		ServerProtocolIdJabber		=  4,
		ServerProtocolIdYahoo		=  5,
		ServerProtocolIdGoogleTalk	=  6,
		ServerProtocolIdGeneral		=  7,
		ServerProtocolIdFacebook	=  8,
		ServerProtocolIdTwitter		=  9,
		ServerProtocolIdVoxOx		= 10,
		ServerProtocolIdSkype		= 11,
		ServerProtocolIdMySpace		= 12,
		//End of server defined values
	};


	QtEnumIMProtocol();

	QtEnumIMProtocol::IMProtocol		getQtProtocol()					{ return _qtProtocol;			}
	EnumIMProtocol::IMProtocol			getModelProtocol()				{ return _modelProtocol;		}
	QtEnumIMProtocol::ServerProtocolId	getServerProtocolId()			{ return _serverProtocolId;		}
	String								getName()						{ return _name;					}

	String								getChatProtocol()				{ return _chatProtocol;			}
	int									getOrder()						{ return _order;				}
	bool								useInIMAccountMenu()			{ return _useInIMAccountMenu;	}
	bool								isIMProtocol()					{ return _isIMProtocol;			}
	bool								supportsPresence()				{ return _supportsPresence;		}	//VOXOX - JRT - 2009.08.28 
	bool								supportsPresenceNote()			{ return _supportsPresenceNote;	}	//VOXOX - JRT - 2009.09.10 
	bool								isIconPresenceBased();		//VOXOX CHANGE by Rolando - 2009.07.27 

	bool								canDoMultiChat()				{ return _canDoMultiChat;		}
	String								getDefaultImageFile()			{ return _defaultImageFile;		}

	String								getDefaultProtocolImage()		{ return _defaultProtocolImage;		}//VOXOX - CJC - 2009.07.29 We should not be using the protocol name to get the image path


	QtEnumIMProtocol::IMProtocol		getKey()						{ return getQtProtocol();		}	//For use with VoxMapTemplate.

	String getIconPath();
	String getBigIconPath();
	String getImageName();
	String getNetworkIconPath( EnumPresenceState::PresenceState presenceState );
		
	void setQtProtocol		( QtEnumIMProtocol::IMProtocol val )		{ _qtProtocol		  = val;	}
	void setModelProtocol	( EnumIMProtocol::IMProtocol   val )		{ _modelProtocol	  = val;	}
	void setServerProtocolId( QtEnumIMProtocol::ServerProtocolId  val )	{ _serverProtocolId	  = val;	}
	void setName			( String					   val )		{ _name				  = val;	}

	void setChatProtocol	  ( const char*			    val )			{ _chatProtocol		  = val;	}
	void setOrder			  ( int						val )			{ _order			  = val;	}
	void setUseInIMAccountMenu( bool					val )			{ _useInIMAccountMenu = val;	}
	void setIsIMProtocol      ( bool				    val )			{ _isIMProtocol		  = val;	}
	void setSupportsPresence  ( bool				    val )			{ _supportsPresence   = val;	}
	void setSupportsPresenceNote( bool					val )			{ _supportsPresenceNote = val;	}
	void setCanDoMultiChat	  ( bool					val )			{ _canDoMultiChat	  = val;	}
	void setDefaultImageFile  ( const char*				val )			{ _defaultImageFile   = val;	}
	void setDefaultProtocolImage( const char*			val )			{ _defaultProtocolImage   = val;	}//VOXOX - CJC - 2009.07.29  We should not be using the protocol name to get the image

	
	void setName			  ( const char*				val )			{ _name = val;	}
	
	void addPresenceStateMapEntry( EnumPresenceState::PresenceState psKey, EnumPresenceState::PresenceState psValue );
	EnumPresenceState::PresenceState getMappedPresenceState( EnumPresenceState::PresenceState ps );

protected:
	void initVars();

private:
	QtEnumIMProtocol::IMProtocol		_qtProtocol;		//Presentation
	EnumIMProtocol::IMProtocol			_modelProtocol;		//Model
	QtEnumIMProtocol::ServerProtocolId	_serverProtocolId;	//Server

	String							_name;
	String							_chatProtocol;
	int								_order;
	bool							_useInIMAccountMenu;
	bool							_isIMProtocol;
	bool							_supportsPresence;
	bool							_supportsPresenceNote;

	bool							_canDoMultiChat;
	String							_defaultImageFile;
	String							_defaultProtocolImage;//VOXOX - CJC - 2009.07.29 We should not be using the name of the protocol to get the image lets create one

	PresenceStateMap				_psMap;
};

//=============================================================================

typedef std::map<QtEnumIMProtocol::IMProtocol, QtEnumIMProtocol> ProtocolMap;

class QtEnumIMProtocolMap : public ProtocolMap
{
public:
	static QtEnumIMProtocolMap & getInstance() 
	{
		if (!_instance) 
		{
			_instance = new QtEnumIMProtocolMap;
		}
		return *_instance;
	}

	static void deleteInstance() 
	{
		if ( _instance )
		{
			delete _instance;
			_instance = NULL;
		}
	}

	static std::string	getQtProtocolName   ( QtEnumIMProtocol::IMProtocol protocol );
	static bool			hasIM			    ( QtEnumIMProtocol::IMProtocol protocol );
	static bool			supportsPresence    ( EnumIMProtocol::IMProtocol   protocol );	//VOXOX - JRT - 2009.08.28 
	static bool			supportsPresence    ( QtEnumIMProtocol::IMProtocol protocol );	//VOXOX - JRT - 2009.08.28 
	static bool			supportsPresenceNote( QtEnumIMProtocol::IMProtocol protocol );	//VOXOX - JRT - 2009.09.10 
	
	static EnumPresenceState::PresenceState getMappedPresenceState( QtEnumIMProtocol::IMProtocol protocol, EnumPresenceState::PresenceState ps );

	/**
	 * Converts a protocol into a string.
	 *
	 * @return the string
	 */
	String toString(QtEnumIMProtocol::IMProtocol protocol);	
	String toString(EnumIMProtocol::IMProtocol protocol);

	/**
	 * Converts a string into a protocol.
	 *
	 * @return the protocol
	 */
	QtEnumIMProtocol::IMProtocol toIMProtocol(const String& protocol);

	/**
	 * Converts an EnumIMProtocol into a QtEnumIMProtocol.
	 *
	 * @return the protocol
	 */
	QtEnumIMProtocol::IMProtocol toQtIMProtocol(EnumIMProtocol::IMProtocol tgtModelProtocol);
	QtEnumIMProtocol::IMProtocol toQtIMProtocol(EnumIMProtocol::IMProtocol tgtModelProtocol, bool isVoxOx, bool isGtalk );

	/**
	 * Converts a QtEnumIMProtocol to an EnumIMProtocol.
	 *
	 * @return the protocol
	 */
	EnumIMProtocol::IMProtocol			toModelIMProtocol   ( QtEnumIMProtocol::IMProtocol		  protocol   );
	EnumIMProtocol::IMProtocol			toModelIMProtocol   ( QtEnumIMProtocol::ServerProtocolId  protocolId );
	QtEnumIMProtocol::ServerProtocolId  toServerProtocolId  ( QtEnumIMProtocol::IMProtocol		  qtProtocol );
	QtEnumIMProtocol::IMProtocol		fromServerProtocolId( QtEnumIMProtocol::ServerProtocolId  protocolId );

	String getIconPath();

	QtEnumIMProtocol*	findByModelProtocol	  ( EnumIMProtocol::IMProtocol modelProtocol );
	QtEnumIMProtocol*	findByQtProtocol	  ( QtEnumIMProtocol::IMProtocol qtProtocol  );
	QtEnumIMProtocol*	findByServerProtocolId( QtEnumIMProtocol::ServerProtocolId  protocolId   );

	//Some convenience methods'
	bool	canDoMultiChat     ( EnumIMProtocol::IMProtocol protocol );
	String	getDefaultImageFile( QtEnumIMProtocol::IMProtocol protocol );
	String	getDefaultProtocolImage( QtEnumIMProtocol::IMProtocol protocol );//VOXOX - CJC - 2009.07.29  We should not be using the protocol name to get the image

protected:
	QtEnumIMProtocolMap();	//should be private for a singleton.

	void create();
	void add( QtEnumIMProtocol::IMProtocol protocol, EnumIMProtocol::IMProtocol modelProtocol, const char* name,
			  int nOrder, bool useInIMAccountMenu, const char* chatProtocol, bool bIMProtocol, bool bSupportsPresence, bool bSupportsPresenceNote,
			  QtEnumIMProtocol::ServerProtocolId protocolId, bool canDoMultiChat, const char* defaultImageFile,const char* defaultProtocolImage );

	//VOXOX - JRT - 2009.09.15 
	void addPsMaps();
	void addShortPsMap   ( QtEnumIMProtocol::IMProtocol tgtProtocol );	
	void addMySpacePsMap ( QtEnumIMProtocol::IMProtocol tgtProtocol );	
	void addJabberPsMap  ( QtEnumIMProtocol::IMProtocol tgtProtocol );	
	void addFacebookPsMap( QtEnumIMProtocol::IMProtocol tgtProtocol );	

	QtEnumIMProtocol* Lookup( const QtEnumIMProtocol::IMProtocol tgtProtocol );

private:
	static QtEnumIMProtocolMap* _instance;
};

#endif	//VOX_ENUMIMPROTOCOL_H
