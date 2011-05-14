(function() {	
	contacts.buildCountrySelect = function(select) {
		var addOption = function (index, item) {
			$(select).append($("<option>").attr("value",$(item).find("countrycode").text()).text(" "+$(item).find("countryname").text()+" "+$(item).find("countrycode").text()));
		}
		$(countryxml).find('flag').each(function(i,item) { addOption(i,item) });
	};
	
	contacts.labelCountryCodes = function() {
		$.get('includes/flags.xml',function(data) { // populate both country code dropdown
			countryxml = data;
			contacts.buildCountrySelect($("#opt_countrycodes"));
		},'xml');
	};
})();