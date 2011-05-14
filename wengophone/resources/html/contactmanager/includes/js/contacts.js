var theme			= 'default';
var themePath		= 'themes';
var gatewayURL	   = 'gateway.php';
var pluginPath	   = 'includes/js/plugins/contacts.';

(function() {

	var contacts = window.contacts = function(){};
	var loadedplugins = [];
	contacts.alerted = [];
	
	// load settings plugin
	contacts.loadplugin = function(plugin) {
		// load plugins not already loaded
		if(jQuery.inArray(plugin, loadedplugins) == -1) {
			$.getScript(pluginPath+plugin+".js", function(){
				loadedplugins.push(plugin);
			});
		}
	};
	
	contacts.init = function(start) {
        try {
     		contacts.loadContactData(); //JRT - do we need to load contacts before groups?
   		    contacts.loadGroups();
        } catch(e) {
            alert(e);
        }
        //End JRT
        
		switch(start) {
			case 'groups': // start off in groups
				$('#Edit-Groups').show();
				break;
			case 'contact': // start off in a specific contact
				contacts.loadContact();
				break;
			default: // if no start point, start in groups
				$('#Edit-Groups').show();
				break;
		}

		$('#bttn_save_new').hide();
		$('#bttn_save_group').hide();
		
	};
	
})();
 