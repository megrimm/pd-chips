/** FlattrLoader */

var FlattrLoader = {

	instance: false,
	queryString: false,
	validParams: ['mode', 'https', 'uid', 'category', 'button', 'language', 'html5-key-prefix'],
	validButtonParams: ['uid', 'owner', 'category', 'button', 'language', 'hidden', 'tags', 'title', 'url', 'description','revsharekey'],
	options: {},

	createIframe: function(data) {

		var compact = false;
		if (data.button == 'compact') {
			var compact = true;
		}

		var iframe = document.createElement('iframe');
		iframe.setAttribute('src', ( this.getParam('https') == 1 ? 'https' : 'http' ) +'://'+ this.getParam('domain', 'api.flattr.com') + '/button/view/?'+ this.encodeData(data));
		iframe.setAttribute('class', 'FlattrButton');
		iframe.setAttribute('width', (compact == true ? 110 : 55) );
		iframe.setAttribute('height', (compact == true ? 20 : 62) );
		iframe.setAttribute('frameBorder', 0);
		iframe.setAttribute('scrolling', 'no');
		iframe.setAttribute('border', 0);
		iframe.setAttribute('marginHeight', 0);
		iframe.setAttribute('marginWidth', 0);
		iframe.setAttribute('allowTransparency', 'true');

		return iframe;
	},

	encodeData: function(data) {

		var result = '';

		for (prop in data) {

			var value = data[prop];

			if (prop == 'description') {
				value = this.stripTags(value, '<br>');
				if (value.length > 1000) {
					value = value.substring(0, 1000);
				}
			}

			value = value.replace(/^\s+|\s+$/g, '').replace(/\s{2,}|\t+/g, ' ');
			result += prop + '=' + encodeURIComponent(value) + '&';
		}

		return result;
	},

	getParam: function (key, defaultValue)
	{		
		if ( key in this.options ) {
			return this.options[key];
		}

	    return defaultValue;
	},

	init: function() {

		try
		{
			var scripts = document.getElementsByTagName("script");

			for (var i = (scripts.length - 1); i >= 0; i--) {

				var instance = scripts[ i ];

				if (!instance.hasAttribute('src')) {
					continue;
				}

				var src = instance.getAttribute('src');	
				var re = new RegExp('^(http(?:s?))://(api\.(?:.*\.?)flattr\.(?:com|dev))', 'i');

				var result = src.match(re);
				if (result) {

					this.options['domain'] = result[2].toString();
					this.options['https']  = (result[1].toString() == 'https' ? 1 : 0);
	
					var pos = src.indexOf('?');
					if (pos) {
						var qs = src.substring(++pos);

					    var params = qs.split("&");
					    for (var i = 0; i < params.length; i++) {
					        var pair = params[i].split("=");

					        if (this.validParam(pair[0], this.validParams)) {
					        	this.options[pair[0]] = pair[1];
					        }
					    }
					}

					this.instance = instance;
					break;
				}

			}
		}
		catch(e)
		{
			// ge fel
		}

		switch(this.getParam('mode', 'manual')) {

			case 'direct': this.render(); break;

			case 'auto':
			case 'automatic':
					var that = this;

					this.domReady(function() {
						that.setup();
					});
				break;

			case 'manual':
			default:
		}

	},

	loadButton: function(elm) {

		var data = {};
		var dataValue = null;

		for (prop in this.options) {
			if (this.validParam(prop, this.validButtonParams)) {
				data[prop] = this.options[prop];
			}
		}

		if (elm.getAttribute('href')) {
			data.url = elm.getAttribute('href');
		}

		if (elm.getAttribute('title')) {
			data.title = elm.getAttribute('title');
		}

		if (elm.getAttribute('lang')) {
			data.language = elm.getAttribute('lang');
		}

		if (elm.innerHTML) {
			data.description = elm.innerHTML;
		}

		if ( ((dataValue = elm.getAttribute('rev')) !== null && (dataValue.substring(0, 6) == 'flattr')) || 
				((dataValue = elm.getAttribute('rel')) !== null && (dataValue.substring(0, 6) == 'flattr')) ) {

			dataValue = dataValue.substring(7).split(';');
			for (var i = 0; i < dataValue.length; i++) {
				var pair = dataValue[i].split(":"),
					pairKey = pair.shift();

				if (this.validParam(pairKey, this.validButtonParams)) {
					data[pairKey] = pair.join(':');
				}
			}
		}
		else
		{
			for (field in this.validButtonParams) {
	
				if ( (dataValue = elm.getAttribute(this.getParam('html5-key-prefix', 'data-flattr') + '-' + this.validButtonParams[field])) !== null ) {
					data[this.validButtonParams[field]] = dataValue;
				}
			}
		}

		this.replaceWith(elm, this.createIframe(data));
	},

	render: function(options, target, position) {

		var data = {};
		for (prop in this.options) {
			if (this.validParam(prop, this.validButtonParams)) {
				data[prop] = this.options[prop];
			}
		}

		try
		{
			if (options) {
				for (prop in options) {
					if (this.validParam(prop, this.validButtonParams)) {
						data[prop] = options[prop];
					}
				}
			} else {

				if (window['flattr_uid']) { data.uid = flattr_uid; }
				if (window['flattr_url']) { data.url = flattr_url; }
				if (window['flattr_btn']) { data.button = flattr_btn; }
				if (window['flattr_hide']) { data.hidden = (flattr_hide == true ? 1 : 0); }
				if (window['flattr_cat']) { data.category = flattr_cat; }
				if (window['flattr_tag']) { data.tags = flattr_tag; }
				if (window['flattr_lng']) { data.language = flattr_lng; }
				if (window['flattr_tle']) { data.title = flattr_tle; }
				if (window['flattr_dsc']) { data.description = flattr_dsc; }

			}
			
			var frame = this.createIframe(data);
	
			if (target) {
				
				if (typeof(target) == 'string') {
					target = document.getElementById(target);
					if (!target) {
						// ge fel
					}
				}

				switch(position) {
					case 'before':
							target.parentNode.insertBefore(frame, target);
						break;
					case 'replace':
							this.replaceWith(target, frame);
						break;
					case 'append':
					default:
							target.appendChild(frame);
						break;
				}
				
			} else {
				
				if (this.getParam('mode', 'manual') == 'direct') {

					this.replaceWith(this.instance, this.createIframe(data));

				} else {
					// ge fel
				}
				
			}

		}
		catch(e)
		{
			// ge fel
		}

	},

	replaceWith: function (old, content)
	{
		if (typeof content == 'string') {

			if ('outerHTML' in document.documentElement) {
				old.outerHTML = content;
			}
			else {
				var range = document.createRange();
				range.selectNode(old);
				content = range.createContextualFragment(content);

				old.parentNode.replaceChild(content, old);
			}
		}

		var parent = old.parentNode;
		parent.replaceChild(content, old);
	},

	setup: function() {
		var tmp, i, btns;
		if (document.querySelectorAll) {
			try {
				btns = document.querySelectorAll('a.FlattrButton');
			} catch (e) {}
		}
		if (btns == undefined) {
			btns = [];
			tmp = document.getElementsByTagName('a');
			for(i = (tmp.length - 1); i >= 0 ; i--) {
				if (/FlattrButton/.test(tmp[i].className)) {
					btns[btns.length] = tmp[i];
				}
			}
		}
		for(i = (btns.length - 1); i >= 0 ; i--) {
			this.loadButton(btns[i]);
		}
	},

	stripTags: function(str, allowed_tags) {
	
	    var key = '', allowed = false;
	    var matches = [];
	    var allowed_array = [];
	    var allowed_tag = '';
	    var i = 0;
	    var k = '';
	    var html = '';
	
	    var replacer = function (search, replace, str) {
	        return str.split(search).join(replace);
	    };
	
	    // Build allowed tags associative array
	    if (allowed_tags) {
	        allowed_array = allowed_tags.match(/([a-zA-Z0-9]+)/gi);
	    }
	
	    str += '';
	
	    // Match tags
	    matches = str.match(/(<\/?[\S][^>]*>)/gi);
	
	    // Go through all HTML tags
	    for (key in matches) {
	        if (isNaN(key)) {
	            // IE7 Hack
	            continue;
	        }
	
	        // Save HTML tag
	        html = matches[key].toString();
	
	        // Is tag not in allowed list? Remove from str!
	        allowed = false;
	
	        // Go through all allowed tags
	        for (k in allowed_array) {
	            // Init
	            allowed_tag = allowed_array[k];
	            i = -1;
	
	            if (i != 0) { i = html.toLowerCase().indexOf('<'+allowed_tag+'>');}
	            if (i != 0) { i = html.toLowerCase().indexOf('<'+allowed_tag+' ');}
	            if (i != 0) { i = html.toLowerCase().indexOf('</'+allowed_tag)   ;}
	
	            // Determine
	            if (i == 0) {
	                allowed = true;
	                break;
	            }
	        }
	
	        if (!allowed) {
	            str = replacer(html, "", str); // Custom replace. No regexing
	        }
	    }
	
	    return str;
	},
	
	validParam: function(key, arr) {
		
		for (var i = 0; i < arr.length; i++) {
			if ( arr[i] == key ) {
				return true;
			}
		}
		
		return false;
	}
	
};

// From https://github.com/ded/domready
!function(a,b){function m(a){l=1;while(a=c.shift())a()}var c=[],d,e,f=!1,g=b.documentElement,h=g.doScroll,i="DOMContentLoaded",j="addEventListener",k="onreadystatechange",l=/^loade|c/.test(b.readyState);b[j]&&b[j](i,e=function(){b.removeEventListener(i,e,f),m()},f),h&&b.attachEvent(k,d=function(){/^c/.test(b.readyState)&&(b.detachEvent(k,d),m())}),a.domReady=h?function(b){self!=top?l?b():c.push(b):function(){try{g.doScroll("left")}catch(c){return setTimeout(function(){a.domReady(b)},50)}b()}()}:function(a){l?a():c.push(a)}}(FlattrLoader,document);

FlattrLoader.init();
