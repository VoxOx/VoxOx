		closeBtn = new Image();
		closeBtn.src = "images/header/header_button_close.png";
		closePBtn = new Image();
		closePBtn.src = "images/header/header_button_close_pressed.png";
		openBtn = new Image();
		openBtn.src = "images/header/header_button_open.png";
		openPBtn = new Image();
		openPBtn.src = "images/header/header_button_open_pressed.png";
		
	    isDebug = true;        
       
	    //Do this on load
		function initEvents() {
			if(document.getElementById("heading") == null) {
				document.getElementById("bodyNode").style.marginTop = "68px";
			}
			
            if(isDebug == false) {
	           document.getElementById("debug").style.display = "none";
	       }
	       
		   alignChat(true);
		}

	    //Debugging function
	    function trace(msg) {
			var node = document.createElement("div");
			var debugCon = document.getElementById("debug");
			node.innerHTML = msg;
			debugCon.appendChild(node);
		}
		
		
		//Appending new content to the message view
		function appendMessage(html) {
			shouldScroll = nearBottom();
		
			//Remove any existing insertion point
			insert = document.getElementById("insert");
			if(insert) insert.parentNode.removeChild(insert);

			//Append the new message to the bottom of our chat block
			chat = document.getElementById("Chat");
			range = document.createRange();
			range.selectNode(chat);
			documentFragment = range.createContextualFragment(html);
			chat.appendChild(documentFragment);
			
			alignChat(shouldScroll);
		}
		function appendNextMessage(html){
			shouldScroll = nearBottom();

			//Locate the insertion point
			insert = document.getElementById("insert");
		
			//make new node
			range = document.createRange();
			range.selectNode(insert.parentNode);
			newNode = range.createContextualFragment(html);

			//swap
			insert.parentNode.replaceChild(newNode,insert);
			
			alignChat(shouldScroll);
		}
		
		//Auto-scroll to bottom.  Use nearBottom to determine if a scrollToBottom is desired.
		function nearBottom() {
			return ( document.body.scrollTop >= ( document.body.offsetHeight - ( window.innerHeight * 1.2 ) ) );
		}
		function scrollToBottom() {
			document.body.scrollTop = document.body.offsetHeight;
		}
		
		//Dynamically exchange the active stylesheet
		function setStylesheet( id, url ) {
			var code = "<style id=\"" + id + "\" type=\"text/css\" media=\"screen,print\">";
			if( url.length ) code += "@import url( \"" + url + "\" );";
			code += "</style>";
			var range = document.createRange();
			var head = document.getElementsByTagName( "head" ).item(0);
			range.selectNode( head );
			documentFragment = range.createContextualFragment( code );
			head.removeChild( document.getElementById( id ) );
			head.appendChild( documentFragment );
		}
		
		//Swap an image with its alt-tag text on click
		document.onclick = imageCheck;
		function imageCheck() {		
			node = event.target;
			if(node.tagName == 'IMG' && node.alt && node.className!="avatar") {
				a = document.createElement('a');
				a.setAttribute('onclick', 'imageSwap(this)');
				a.setAttribute('src', node.src);
				a.className = node.className;
				text = document.createTextNode(node.alt);
				a.appendChild(text);
				node.parentNode.replaceChild(a, node);
			}
		}
		function imageSwap(node) {
			shouldScroll = nearBottom();

			//Swap the image/text
			img = document.createElement('img');
			img.setAttribute('src', node.src);
			img.className = node.className;
			img.setAttribute('alt', node.firstChild.nodeValue);
			node.parentNode.replaceChild(img, node);
			
			alignChat(shouldScroll);
		}
		
		//Align our chat to the bottom of the window.  If true is passed, view will also be scrolled down
		function alignChat(shouldScroll) {

			var windowHeight = window.innerHeight;
			
			if(windowHeight > 0) {
				var contentElement = document.getElementById('Chat');
				var contentHeight = contentElement.offsetHeight;
				if (windowHeight - contentHeight > 0) {
					contentElement.style.position = 'relative';
					contentElement.style.top = '0px';
				} else {
					contentElement.style.position = 'static';
				}
			}
			
			if(shouldScroll) scrollToBottom();
		}
		
		function toggleParent(parentId) {
			myParent = document.getElementById(parentId);
			toggleBtn = document.images["togglebutton"];
			if (myParent.style.display=="none") {
				myParent.style.display="block";
				toggleBtn.src = closeBtn.src;
				document.getElementById("bodyNode").style.marginTop = "68px";
			} else {
				myParent.style.display="none";
				toggleBtn.src = openBtn.src;
				document.getElementById("bodyNode").style.marginTop = "5px";
			}
			alignChat(shouldScroll);
			return true;
		}

		function buttonPressed(parentId) {
			myParent = document.getElementById(parentId);
			me = event.target;
			if (myParent.style.display=="none") {
				me.src = openPBtn.src;
			} else {
				me.src = closePBtn.src;
			}
			return true;
		}

		function windowDidResize() {
			alignChat(true/*nearBottom()*/); //nearBottom buggy with inactive tabs
		}
		
		window.onresize = windowDidResize;