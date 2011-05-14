/*
 * jQuery.form.track.changes 1.0
 *          - A jQuery plugin to track changes within forms
 *
 * Copyright (c) 2008 Sunny Saxena (code.zhandwa.com)
 * Dual licensed under the MIT (MIT-LICENSE.txt)
 * and GPL (GPL-LICENSE.txt) licenses.
 *
 */
jQuery.fn.trackChanges = function(options) {

    // the form name
    var formName = $(this)[0].name;

    // list of options:
    // 1) events: which events to track on
    // 2) changeListName: this is the name of the list which will hold the changed form element names.
    //              This way when the form submits, we will have the list of all the changed elements.
    //              If the options dont carry a list name, the list name will default to {formName}TrackList
    // 3) changeListClass: class associated with the change list.
    //              Just in case you want to display it and then style it as well.
    // 4) changeListVisible: this will show the change list if set to true. helpful for debugging, etc,
	// 5) excludeClass: elements which have this class will be excluded from all monitoring
    options = jQuery.extend({
        events: "blur",
        changeListName: formName + "TrackList",
        changeListClass: "changeListClass",
        changeListVisible: false,
        excludeClass: null
    }, options);

    // container which holds the initial values
    var oldValues = {
        set: function(key, val) {
            this[key] = val;
        },
        get: function(key) {
            return this[key];
        }
    };

    // this is the change list which is appended at the very bottom of the form
    // the name of this form is created using the form name, if there is no name provided in the options
    var cName = options.changeListName;
    var displayProp = "display: none;";
    if (options.changeListVisible) {
        displayProp = "display: block;";
    }
    var changesList = '<select name="'+ cName +'" id="'+ cName +'" multiple="multiple" class="' +
        options.changeListClass + '" style="' + displayProp + '"></select>';
    this.append(changesList);

    // removes an element from the list
    function removeFromChangedValuesList(obj) {
        var cvs = $(obj.form).find("#" + cName)[0];
        if (!cvs) {
            return;
        }

        len = cvs.length;
        for (i = 0; i < len; i++) {
            if (obj.name == cvs.options[i].value) {
                cvs.options[i] = null;
                return;
            }
        }
    }

    // adds an element to the list
    function addToChangedValuesList(obj) {
        var cvs = $(obj.form).find("#" + cName)[0];
        if (!cvs) {
            return;
        }

        len = cvs.length;
        if (len == 0) {
            cvs.options[len] = new Option(obj.name, obj.name, true);
            cvs.options[len].selected = true;
        } else {
            for (i = 0; i < len; i++) {
                if (obj.name == cvs.options[i].value) {
                    return;
                }
            }
            cvs.options[len] = new Option(obj.name, obj.name, true);
            cvs.options[len].selected = true;
        }
    }

    // checks if the value has changed and accordingly
    // adds or removes the element name from the list
    function addToChangesList(obj) {
        if ($(obj).is("input[@type='checkbox']")) {
            if (obj.checked) {
                if (oldValues.get(obj.name)) {
                    removeFromChangedValuesList(obj);
                } else {
                    addToChangedValuesList(obj);
                }
            } else {
                if (oldValues.get(obj.name)) {
                    addToChangedValuesList(obj);
                } else {
                    removeFromChangedValuesList(obj);
                }
            }
            return;
        }

        if (oldValues.get(obj.name) != obj.value) {
            if(!$(obj).is("input[@type='radio']")) {
                addToChangedValuesList(obj);
            } else {
                if (obj.checked) {
                    addToChangedValuesList(obj);
                }
            }
        } else {
            removeFromChangedValuesList(obj);
        }
    }

    // here we read the original values and populate the oldValues object
    this.each(function() {
        var elements = $(this)[0].elements;
        for (i = 0; i < elements.length; i++) {
            var obj = elements[i];

            if ($(obj).hasClass(options.excludeClass)) {
                continue;    
            } else {
                if ($(obj).is("input[@type='radio']")) {
                    if (obj.checked) {
                        oldValues.set(obj.name, obj.value);
                    }
                } else if ($(obj).is("input[@type='checkbox']")) {
                    if (obj.checked) {
                        oldValues.set(obj.name, true);
                    } else {
                        oldValues.set(obj.name, false);
                    }
                } else if ($(obj).is("select")) {
                    if (obj.name == cName) {
                        return;
                    }
                    if (obj.value && obj.value == "") {
                        oldValues.set(obj.name, obj.options[obj.selectedIndex].text);
                    } else {
                        oldValues.set(obj.name, obj.value);
                    }
                } else if ($(obj).is("textarea")) {
                    oldValues.set(obj.name, $(obj).html());
                } else {
                    oldValues.set(obj.name, obj.value);
                }

                // bind the events to the element
                $(obj).bind(options.events, function(obj) {
                    addToChangesList(this);
                });
            }
        }
    });

    // we return the old values object, such that if the user has access to the initial values
    // the change list is available in the form as an element
    // the new values are obviously with the user in the form
    return oldValues;
};

