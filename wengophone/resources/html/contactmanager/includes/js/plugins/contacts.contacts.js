(function() {
	contacts.loadContactData = function() {
		try {
			contacts.contactData = eval(qtEventSource.getContactList());
		} catch(e) {
			alert(e);
		}
	};
	
	contacts.loadContacts = function(gid,gname,sortby) {	
		//$(".list_contacts").empty();	
		var name;
		if(gid==0) {
			$(contacts.contactData).each(function(i) { // now enter all contacts
				contact = this;
				if(contact.fname.length > 0 || contact.lname.length > 0) { // is conctact has a name
					name = contact.fname + ' ' + contact.lname;
				} else if(contact.company.length > 0) { // if contat has no name, then use company name if it's available
					name = contact.company;
				} else { // if contact has no name or company entry use JID
					var id = contact.jid.split('@');
					name = id[0];
				}
				
				$(".list_contacts").append($("<li>").addClass("mergable").attr('id',contact.contactid).click(function() {
					contacts.loadContact($(this));
				}).append(
					$("<div>").addClass("contact_name").text(name)   //JRT
				));
			});
		} else {
			$(contacts.contactData).each(function(i) { // now enter all contacts
				if(this.groupid == gid) {
					contact = this;
					if(contact.fname.length > 0 || contact.lname.length > 0) { // is conctact has a name
						name = contact.fname + ' ' + contact.lname;
					} else if(contact.company.length > 0) { // if contat has no name, then use company name if it's available
						name = contact.company;
					} else { // if contact has no name or company entry use JID
						var id = contact.jid.split('@');
						name = id[0];
					}
					$(".list_contacts").append($("<li>").addClass("mergable").attr('id',contact.contactid).click(function() {
						contacts.loadContact($(this));
					}).append(
						$("<div>").addClass("contact_name").text(name)   //JRT
					));
				}
			});
		}
		
		return true;
	};
	
	contacts.loadContact = function(contact) {	
		//var contact_id = contact.attr('id');
		var contact_id = (jumpcid) ? jumpcid : contact.attr('id');
			jumpcid = null; // clear it so the same same contact isn't loaded no matter what you click
		//$('#output').val('').val(qtEventSource.getData(contact_id));
		//alert(qtEventSource.getData(contact_id));
	   	var cdetails  = eval('('+qtEventSource.getData(contact_id)+')');
	   	var c_id 	  = cdetails.pinfo.id;
	   	var c_cid 	  = cdetails.pinfo.contactid;
		var c_fname   = cdetails.pinfo.fname;
		var c_lname   = cdetails.pinfo.lname;
		var c_company = cdetails.pinfo.company;
		//var c_title   = cdetails.pinfo.title;
		//var group_id = cdetails.groups.id;
		var group_name;
		
		$(cdetails.groups).each(function() {
			//alert(this.groupName);
			group_name = this.groupName;
		});
		
		//alert(group_name);
		
		clickedid = cdetails.pinfo.contactid;

	    $('#bttn_edit_contact').show();
		$('#bttn_save_contact').hide();
		$('#bttn_save_contact_settings').hide();
		$('#bttn_edit_contact_settings').show();
		$('#Editable-Contact-Settings').hide();
		$('#Editable-Contact-Details').show();
		$('#bttn_save_new').hide();
	
		$(".list_contacts li").removeClass("selected");
		$(contact).addClass("selected");
		
		$('.editarea').hide();
		$('#Edit-Contacts').show();
		$("#Settings-My-Account").show();
		$("#Window-Contact-Info").show();
		
		$('.frm_copied').remove(); // clear any copies
		$('#frm_copy').show(); // hide original form
	
		var cloneForm = $('#frm_copy');
		var newForm_id = "frm_contact_"+c_id;
		
		cloneForm.clone(true).attr('id',newForm_id).addClass('frm_copied').insertBefore($('#frm_copy'));
		
		$('#frm_copy').hide(); // hide original form
		
		$('#'+newForm_id+' #cid').val(c_id);
		$('#'+newForm_id+' #cid2').val(c_cid);
		
		if(c_fname.length > 0) { $('#'+newForm_id+' #fname').val(c_fname).removeClass('example'); }
		if(c_lname.length > 0) { $('#'+newForm_id+' #lname').val(c_lname).removeClass('example'); }
		if(c_company.length > 0) { $('#'+newForm_id+' #company').val(c_company).removeClass('example'); }
		//if(c_title.length > 0) { $('#'+newForm_id+' #title').val(c_title); }
		$('#'+newForm_id+' #groupname').val(group_name);
		
		$(cdetails.phones).each(function() { // build phones
			var newPhone = $('#'+newForm_id+' .set_phones:first').clone(true).insertAfter($('#'+newForm_id+' .set_phones:last'));
			newPhone.find('.lbl_phones').val(this.label);
			newPhone.find('.lbl_phones').attr('id','phoneid_'+this.id);
			newPhone.find(':text').val(this.phone);
		});
		
		$(cdetails.emails).each(function() { // build emails
			var newEmail = $('#'+newForm_id+' .set_emails:first').clone(true).insertAfter($('#'+newForm_id+' .set_emails:last'));
			newEmail.find('.lbl_emails').val(this.label);
			newEmail.find('.lbl_emails').attr('id','emailid_'+this.id);
			newEmail.find(':text').val(this.email);
		});
		
		$(cdetails.networks).each(function() { // build services im
			var newNetwork = $('#'+newForm_id+' .set_services_im:first').clone(true).insertAfter($('#'+newForm_id+' .set_services_im:last'));
			newNetwork.find('.lbl_services_im').val(this.service);
			newNetwork.find('.lbl_services_im').attr('id','networkid_'+this.id);
			newNetwork.find(':text').val(this.label);
		});
		
		//$(cdetails.services_social).each(function() { // build services social
		//	var i = $('.frm_copied .set_services_social').size()-1;
		//	var newLabelName = $('.frm_copied .set_services_social:first').find('select').attr('name');
		//	var newInputName = $('.frm_copied .set_services_social:first').find('input').attr('name');
		//	$('#'+newForm_id+' .set_services_social:first').clone(true).insertAfter($('#'+newForm_id+' .set_services_social:last')).find('input:text').attr('name', newLabelName+parseInt(i+1)).val(this.service).parent().find('option:contains("'+this.label+'")').attr('selected', 'selected').parent('select').attr('name', newLabelName+parseInt(i+1));
		//});
		
		$(cdetails.addresses).each(function() { // build address
			var newAddress = $('#'+newForm_id+' .set_addresses:first').clone(true).insertAfter($('#'+newForm_id+' .set_addresses:last'));
			newAddress.find('.lbl_addresses').val(this.label);
			newAddress.find('.lbl_addresses').attr('id','addressid_'+this.id);
			newAddress.find('.address_street1').val(this.street1);
			newAddress.find('.address_city').val(this.city);
			newAddress.find('.address_state').val(this.state);
			newAddress.find('.address_zip').val(this.postalcode);
			newAddress.find('.address_country').val(this.country);
		});
		
		if(cdetails.phones.length > 0) $('#'+newForm_id+' .set_phones:first').remove();
		if(cdetails.emails.length > 0) $('#'+newForm_id+' .set_emails:first').remove();
		if(cdetails.networks.length > 0) $('#'+newForm_id+' .set_services_im:first').remove();
		//if(cdetails.emails.length > 0) $('#'+newForm_id+' .set_services_social:first').remove();
		if(cdetails.addresses.length > 0) $('#'+newForm_id+' .set_addresses:first').remove();
		$('#'+newForm_id+' .set_phones:last .bttn_add').attr({style: "display:inline-block;"});
		$('#'+newForm_id+' .set_emails:last .bttn_add').attr({style: "display:inline-block;"});
		//$('#'+newForm_id+' .set_services_im:last .bttn_add').attr({style: "display:inline-block;"});
		//$('#'+newForm_id+' .set_services_social:last .bttn_add').attr({style: "display:inline-block;"});
		$('#'+newForm_id+' .set_addresses:last .bttn_add').attr({style: "display:inline-block;"});
		contacts.makeNonEditable();
	}
	
	contacts.makeNonEditable = function() {
	
		var iscompany = $('.frm_copied').find('#set_bio_iscompany').find('#iscompany').is(':checked');
	
		$('.frm_copied fieldset').each(function() {
			var fieldset_type = $(this).attr('class');
			var cell_label, cell_value;
			var label = ($(this).find('input').attr('value').length > 0) ? $(this).children().find('select').attr('value') : 'None';
			var value = ($(this).find('input').attr('value').length > 0) ? $(this).find('input').attr('value') : '';
			
			switch(fieldset_type) {
				case 'set_groups':
					cell_label = '<span class=\"label\"><\/span>';
					cell_value = '<span class=\"value\">'+value+'<\/span>';
					break;
				case 'set_phones':
					cell_label = '<span class=\"label\">'+label+'<\/span>';
					cell_value = '<span class=\"value\">'+value+'<\/span>';
					break;
				case 'set_emails':
					cell_label = '<span class=\"label\">'+label+'<\/span>';
					cell_value = '<span class=\"value\">'+value+'<\/span>';
					break;
				case 'set_services_im':
					cell_label = '<span class=\"label\">'+label+'<\/span>';
					cell_value = '<span class=\"value\">'+value+'<\/span>';
					break;
				case 'set_services_social':
					cell_label = '<span class=\"label\">'+label+'<\/span>';
					cell_value = '<span class=\"value\">'+value+'<\/span>';
					break;
				case 'set_bio name':
					cell_label = '<span class=\"label\">Name<\/span>';
					cell_value = '<span class=\"value\">'+value+'<\/span>';
					break;
				case 'set_bio company':
					cell_label = '<span class=\"label\">Company<\/span>';
					cell_value = '<span class=\"value\">'+value+'<\/span>';
					break;
				case 'set_bio iscompany':
					cell_label = '<span class=\"label\">'+label+'<\/span>';
					cell_value = '<span class=\"value\">'+value+'<\/span>';
					break;
				case 'set_addresses':
					if($(this).find('.address_street1').attr('value').length > 0 && $(this).find('.address_street1').attr('value') != "Street") {
						cell_label  = '<div class=\"label\">'+label+'<\/div>';
						cell_value  = '<div class=\"value\"><div>';
						cell_value += $(this).find('.address_street1').attr('value')+'<br>';
						cell_value += $(this).find('.address_city').attr('value');
						cell_value += '&nbsp;'+$(this).find('.address_state').attr('value');
						cell_value += '&nbsp;'+$(this).find('.address_zip').attr('value')+'<br>';
						cell_value += $(this).find('.address_country').attr('value');
						cell_value += '</div><\/div>';
					} else {
						cell_label = '<span class=\"label\">None<\/span>';
						cell_value = '<span class=\"value\"><\/span>';	
					}
					break;
			}

			if(fieldset_type != "set_bio") {
				$(this).attr({ style: "display:none;" }).after("<div class=\"noneditable\">"+cell_label+cell_value+"<\/div>");
			} else {
				$(this).attr({ style: "display:none;" });
			}
		});
		
		if($('#fname').attr('value').length == 0 || iscompany) { // if only company
			$('.set_bio:first').after("<div class=\"noneditable\"><span class=\"company\">"+$('#company').attr('value')+"</span> <!--span class=\"jobtitle\">"+$('#title').attr('value')+"</span--><\/div>");
		} else { // if only contact
			$('.set_bio:first').after("<div class=\"noneditable\"><span class=\"firstname\">"+$('#fname').attr('value')+"</span> <span class=\"lastname\">"+$('#lname').attr('value')+"</span><\/div>");
		}
	};
	
	contacts.saveContact = function() {
		var savedcontact = {};
		var p_data  = [];
		var e_data  = [];
		var a_data  = [];
		var fname   = ($(".frm_copied #fname").val() != "First Name") ? $(".frm_copied #fname").val() : '';
		var lname   = ($(".frm_copied #lname").val() != "Last Name") ? $(".frm_copied #lname").val() : '';
		var company = ($(".frm_copied #company").val() != "Company") ? $(".frm_copied #company").val() : '';
		var title   = ($(".frm_copied #title").val() != "Job Title") ? $(".frm_copied #title").val() : '';
		
		$('.frm_copied .set_phones').each(function() { // Get all phone numbers
			//var select_id = $(this).find('.lbl_phones').attr('id').split('_');
			//var p_id = (select_id[1]) ? select_id[1] : 'new';
			var p_id = 0;
			var p_label = $(this).find('.lbl_phones').val();
			var p_number = $(this).find(':text').val();
			p_data.push({ 
				id: p_id,
				label: p_label,
				phone: p_number
			});
		});
		
		$('.frm_copied .set_emails').each(function() { // Get all email addresses
			//var select_id = $(this).find('.lbl_emails').attr('id').split('_');
			//var e_id = (select_id[1]) ? select_id[1] : 'new';
			var e_id = 0;
			var e_label = $(this).find('.lbl_emails').val();
			var e_email = $(this).find(':text').val();
			e_data.push({ 
				id: e_id,
				label: e_label,
				email: e_email
			});
		});
		
		$('.frm_copied .set_addresses').each(function() { // Get all addresses
			//var select_id = $(this).find('.lbl_addresses').attr('id').split('_');
			//var a_id = (select_id[1]) ? select_id[1] : 'new';
			var a_id = 0;
			var a_label = $(this).find('.lbl_addresses').val();
			var a_street1 = $(this).find('.address_street1').val();
			var a_city = $(this).find('.address_city').val();
			var a_state = $(this).find('.address_state').val();
			var a_zip = $(this).find('.address_zip').val();
			var a_country = $(this).find('.address_country').val();
			a_data.push({ 
				id: a_id,
				label: a_label,
				street1: a_street1,
				street2: '',
				street3: '',
				city: a_city,
				state: a_state,
				zip: a_zip,
				country: a_country
			});
		});
		
		savedcontact.pinfo = {"id":$(".frm_copied #cid").val(), "contactid":$(".frm_copied #cid2").val(), "fname": fname,"lname": lname,"company": company, "title": title};
		//savedcontact.groups = [{"id":XXX,"groupid":XXX,"groupName":XXX,"type":XXX}];
		savedcontact.phones = p_data;
		savedcontact.emails = e_data;
		savedcontact.addresses = a_data;
		savedcontact.websites = [];
		//alert($.toJSON(savedcontact));
		qtEventSource.setData($.toJSON(savedcontact));
	}
	
	contacts.IsNumeric = function (strString) {
		var strValidChars = "0123456789";
		var strChar;
		var blnResult = true;
	
		if (strString.length == 0) return false;
	
		//  test strString consists of valid characters listed above
		for (i = 0; i < strString.length && blnResult == true; i++) {
			strChar = strString.charAt(i);
			if (strValidChars.indexOf(strChar) == -1) {
				blnResult = false;
			}
		}
		return blnResult;
	};
})();