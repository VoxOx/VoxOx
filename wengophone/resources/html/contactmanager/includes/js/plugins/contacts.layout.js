$(function() {

	var contactsLayout, contactsList, groupDetails;

	contactsLayout = $('#Contacts').layout({
		north: {
			initClosed:		true,
			spacing_open:	0,
			spacing_closed: 0
		},
		south: {
			initClosed:     true,
			spacing_open:	0,
			spacing_closed: 0,
			size:28
		},
		west: {
			size: 			326,
			minSize:		326,
			spacing_open:	0,
		},
		east: {
			initClosed:		true,
			spacing_open:	0,
			spacing_closed: 0
		},
		center: {
			
		},
		// enable showOverflow on west-pane so popups will overlap north pane
		west__showOverflowOnHover: true,
		west__resizable: false
	
	//,	west__fxSettings_open: { easing: "easeOutBounce", duration: 750 }
	});
	
	contactsList = $('#List').layout({
			center__paneSelector:	".inner-center" 
		,	north__paneSelector:	".inner-north" 
		,	west__paneSelector:		".inner-west" 
		,	east__paneSelector:		".inner-east" 
		,	north__initClosed:		  		true
		, 	north__size:			31
		,	west__size:				146
		,	west__minSize:			146
		//,	east__size:				75 
		,	spacing_open:			0  // ALL panes
		,	spacing_closed:			0  // ALL panes
		,	west__spacing_closed:	0
		,	east__spacing_closed:	0
		,	east__initClosed:		true
	});

});