(function() {
    // load all groups
    contacts.loadGroups = function() {
        var results;
        try {
        	//alert(qtEventSource.getContactGroups());
            results = eval(qtEventSource.getContactGroups());
            var totalCount = 0;
            var addRow = function (group) {
                var className = (group.id == "0") ? "topentry" : "mergeable";
                $(".list_groups").prepend($("<li>").click(function() {
                    $(".list_groups li").removeClass("selected");
                    $(this).addClass("selected");
                    $(".list_contacts").empty();
                    $('.editarea').hide();
           			$('#Edit-Groups').show();
                    contacts.loadGroupContacts(group);
                }).addClass(className).append(
                    $("<div>").addClass("group_name").text(group.groupName)).append(
                    $("<div>").addClass("group_totalcontacts").text(group.count)
                ));
            }
            $(results).each(function(i){
                addRow(this);   //JRT - removed Curtis's temp line
                totalCount += parseInt(this.count);
            });
            addRow({"id":"0","groupName":"All Contacts","count":totalCount});
            $('.grp_name').empty().append("All Contacts");
            $('.grp_count').empty().append(totalCount+" Contact");
        } catch(e) {
            alert(e);
        }
    };
    
    // group clicked function
    contacts.loadGroupContacts = function(group) {
        $('.grp_name').empty().append(group.groupName);

        if(group.count == 1)
            $('.grp_count').empty().append(group.count+" Contact");
        else if(group.count > 1)
            $('.grp_count').empty().append(group.count+" Contacts");
        else
            $('.grp_count').empty().append("No Contacts");

        //JRT - we should call this when == 0 so contact list is cleared.
        if(group.count > 0) { // if group has 1 or more contacts. 
            $('.editarea').hide();
            $('#Edit-Groups').show();
            
            contacts.loadContacts(group.id,group.groupName,'contactid');    //JRT - was 'lname'
        }
    };
})();