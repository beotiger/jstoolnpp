var FileManager = { 
	ajaxBg : null, 
	multilanTitles : null, 
	inputChecks : null, 
	selectedItems : null, 
	sortName : null, 
	sortOrder : null, 
	isSearch : null, 
	delayID : 0, 
	miniMainViewHeight : 120, 
	isIE : null, 
	
	/*
	 * ��ֹ�¼�����
	 */
	stopBubble : function(e) { 
		var e = e ? e : window.event; 
		if(window.event) { // IE
			e.cancelBubble = true; 
		} else { // FF
			// e.preventDefault();
			e.stopPropagation(); 
		}
	}, 
	
	/*
	 * ʲôҲ����
	 */
	doNothing : function() { 
		void(0); 
	}, 
	
	/*
	 * ��ϸ��Ϣģʽʱ��������Ƶ���Ŀ�ϵĲ���
	 */
	detailViewItemOver : function(item) { 
		var detailViewItem = $(item); 
		detailViewItem.addClass("selected"); 
	}, 
	
	/*
	 * ��ϸ��Ϣģʽʱ��������Ƴ���Ŀ�Ĳ���
	 */
	detailViewItemOut : function(item) { 
		var detailViewItem = $(item); 
		var checkBox = $(detailViewItem.children().get(0)).children().get(0); // �Ƚϱ��İ취
		if(checkBox.checked != true) { 
			detailViewItem.removeClass("selected"); 
		}
	}, 
	
	/*
	 * ��ͼ��ģʽʱ��������Ƶ���Ŀ�ϵĲ���
	 */
	largeiconViewItemOver : function(item) { 
		var largeiconViewItem = $(item); 
		largeiconViewItem.addClass("selected"); 
	}, 
	
	/*
	 * ��ͼ��ģʽʱ��������Ƴ���Ŀ�Ĳ���
	 */
	largeiconViewItemOut : function(item) { 
		var largeiconViewItem = $(item); 
		var checkBox = $(largeiconViewItem.children().get(0)).children().get(0); // �Ƚϱ��İ취
		if(checkBox.checked != true) { 
			largeiconViewItem.removeClass("selected"); 
		}
	}, 
	
	/*
	 * ��ͼ��ģʽʱ�����������Ŀ�Ĳ���
	 */
	largeiconViewItemClicked : function(item) { 
		var largeiconViewItem = $(item); 
		var checkBox = $(largeiconViewItem.children().get(0)).children().get(0); // �Ƚϱ��İ취
		if(checkBox.checked) { 
			$(checkBox).removeAttr("checked"); 
		} else { 
			$(checkBox).attr("checked", "checked"); 
			largeiconViewItem.addClass("selected"); 
		}
		FileManager.viewItemCheck(); 
	}, 
	
	/*
	 * ��Ŀѡ��ı�
	 */
	viewItemCheck : function() { 
		FileManager.setButton("toolbarCut", "images/toolbar-cut-disable.gif", 
			FileManager.doNothing, "disable", ""); 
		FileManager.setButton("toolbarCopy", "images/toolbar-copy-disable.gif", 
			FileManager.doNothing, "disable", ""); 
		FileManager.setButton("toolbarRename", 
			"images/toolbar-rename-disable.gif", FileManager.doNothing, 
			"disable", ""); 
		FileManager.setButton("toolbarDelete", 
			"images/toolbar-delete-disable.gif", FileManager.doNothing, 
			"disable", ""); 
		
		var count = this.inputChecks.length; 
		var checkedItemsCount = 0; 
		this.selectedItems = new Array(); 
		
		for(var i = 0; i < count; i++) { 
			var checkBox = this.inputChecks.get(i); 
			var item = $(checkBox.parentNode.parentNode); // CheckBox ��Ӧ����Ŀ
			if(checkBox.checked) { 
				checkedItemsCount++; 
				this.selectedItems.push(checkBox.name); 
				item.addClass("selected"); 
			} else { 
				item.removeClass("selected"); 
			}
		}
		
		if(checkedItemsCount > 0) { 
			FileManager.setButton("toolbarCut", "images/toolbar-cut.gif", 
				FileManager.clickCut, "", "disable"); 
			FileManager.setButton("toolbarCopy", "images/toolbar-copy.gif", 
				FileManager.clickCopy, "", "disable"); 
			FileManager.setButton("toolbarDelete", "images/toolbar-delete.gif", 
				FileManager.clickDelete, "", "disable"); 
			if(checkedItemsCount == 1) { 
				FileManager.setButton("toolbarRename", 
					"images/toolbar-rename.gif", FileManager.clickRename, 
					"", "disable"); 
			}
		}
	}, 
	
	/*
	 * ���ð�ť
	 */
	setButton : function(className, src, clickFunc, addClass, removeClass) { 
		var buttons = $("div#toolbar .toolbarButton"); 
		
		for(var i = 0; i < buttons.length; i++) { 
			var button = $(buttons.get(i)); 
			if(button.hasClass(className)) { 
				button.get(0).onclick = clickFunc; 
				if(addClass != "")
					button.addClass(addClass); 
				if(removeClass != "")
					button.removeClass(removeClass); 
				var img = button.children("img"); 
				img.attr("src", src); 
			}
		}
	}, 
	
	/*
	 * ������������Ĳ���
	 */
	clickRename : function() { 
		FileManager.displayAjaxBg(true); 
		
		FileManager.setOldname(); 
		
		FileManager.displayAjaxInput("func/post.func.php", "rename", "rename", 
			true); 
	}, 
	
	/*
	 * ������½�Ŀ¼�Ĳ���
	 */
	clickNewFolder : function() { 
		// alert("newfolder");
		FileManager.displayAjaxBg(true); 
		
		FileManager.displayAjaxInput("func/post.func.php", "newfolder", 
			"new folder", true); 
	}, 
	
	/*
	 * ����˼��еĲ���
	 */
	clickCut : function() { 
		// alert("cut");
		FileManager.sendAjaxOper("cut"); 
	}, 
	
	/*
	 * ����˸��ƵĲ���
	 */
	clickCopy : function() { 
		FileManager.sendAjaxOper("copy"); 
	}, 
	
	/*
	 * �����ճ���Ĳ���
	 */
	clickPaste : function() { 
		var subdir = $("input#subdir").attr("value"); 
		var returnURL = $("input#return").val(); 
		
		FileManager.displayAjaxBg(false); 
		
		var ajaxWait = $("div#ajaxWait"); 
		
		ajaxWait.css("left", FileManager.getLeftMargin() + "px"); 
		ajaxWait.fadeIn(); 
		
		$.post("func/post.func.php", { 
				"oper" : "paste", 
				"subdir" : subdir, 
				"return" : returnURL
			}, function(data) { 
				// alert(data);
				window.location.reload(); 
			}); 
		/*
		 * $.get("func/paste.ajax.php?subdir=" + subdir + "&return=" +
		 * returnURL, function(data) { // alert(data); window.location.reload();
		 * });
		 */
	}, 
	
	/*
	 * �����ɾ���Ĳ���
	 */
	clickDelete : function() { 
		FileManager.displayAjaxBg(true); 
		
		var ajaxDelete = $("div#ajaxDelete"); 
		
		ajaxDelete.css("left", FileManager.getLeftMargin() + "px"); 
		ajaxDelete.fadeIn(); 
		
	}, 
	
	/*
	 * ȷ��ɾ����Ĳ���
	 */
	doDelete : function() { 
		// ׼������
		var ajaxDelete = $("div#ajaxDelete"); 
		ajaxDelete.css("display", "none"); 
		
		this.ajaxBg.get(0).onclick = FileManager.doNothing; 
		
		var ajaxWait = $("div#ajaxWait"); 
		
		ajaxWait.css("left", FileManager.getLeftMargin() + "px"); 
		ajaxWait.fadeIn(); 
		
		var itemsStr = this.selectedItems.join("|"); 
		
		// var subdir = $("input#subdir").val();
		
		$.post("func/post.func.php", { 
				"oper" : "delete", 
				"items" : itemsStr
			}, function(data) { 
				// alert(data);
				if(data == "ok") { 
					window.location.reload(); 
				}
			}); 
	}, 
	
	/*
	 * ������ϴ��Ĳ���
	 */
	clickUpload : function() { 
		FileManager.displayAjaxBg(true); 
		FileManager.displayAjaxInput("func/post.func.php", "upload", "upload", 
			false); 
	}, 
	
	/*
	 * �����ȫѡ�Ĳ���
	 */
	selectAll : function() { 
		var count = FileManager.inputChecks.length; 
		
		for(var i = 0; i < count; i++) { 
			var checkBox = $(FileManager.inputChecks.get(i)); 
			checkBox.attr("checked", "checked"); 
		}
		
		FileManager.viewItemCheck(); 
	}, 
	
	/*
	 * �����ȡ��ѡ��Ĳ���
	 */
	deselect : function() { 
		var count = FileManager.inputChecks.length; 
		
		for(var i = 0; i < count; i++) { 
			var checkBox = $(FileManager.inputChecks.get(i)); 
			checkBox.removeAttr("checked"); 
		}
		
		FileManager.viewItemCheck(); 
	}, 
	
	/*
	 * ������ʾ����ļ�ͷ
	 */
	setSortArrow : function(name, order) { 
		this.sortName = name; 
		this.sortOrder = order; 
	}, 
	
	/*
	 * ����Ϊ����ģʽ
	 */
	setSearchMode : function(isSearch) { 
		this.isSearch = isSearch; 
	}, 
	
	/*
	 * �����Ϣ
	 */
	getMessage : function() { 
		$.get("func/getmessage.ajax.php", function(data) { 
				if(data != "") { 
					var phpfmMessage = $("#phpfmMessage"); 
					if(phpfmMessage.length == 1) { 
						var msg; 
						var stat; 
						
						data = data.split("|PHPFM|"); 
						msg = data[0]; 
						stat = data[1]; 
						
						phpfmMessage.html(msg); 
						if(stat == 2) { 
							// ������Ϣ
							phpfmMessage.addClass("wrong"); 
						} else { 
							phpfmMessage.removeClass("wrong"); 
						}
						
						phpfmMessage.fadeIn(); 
					}
					
					clearTimeout(this.delayID); 
					this.delayID = setTimeout("FileManager.closeMessage()", 10000); 
				}
			}); 
		
	}, 
	
	/*
	 * �ر���Ϣ
	 */
	closeMessage : function() { 
		$("#phpfmMessage").fadeOut(); 
	}, 
	
	/*
	 * �����߾࣬ʹ�����벿�־���
	 */
	getLeftMargin : function() { 
		var viewWidth = document.documentElement.clientWidth; 
		var leftMargin = (viewWidth - 420) / 2; // ����
		return leftMargin; 
	}, 
	
	/*
	 * ���� ajax
	 */
	sendAjaxOper : function(oper) { 
		
		var itemsStr = this.selectedItems.join("|"); 
		
		// var subdir = $("input#subdir").val();
		
		$.post("func/post.func.php", { 
				"oper" : oper, 
				"items" : itemsStr
			}, function(data) { 
				if(data == "ok" && FileManager.isSearch == false) { 
					FileManager.setButton("toolbarPaste", 
						"images/toolbar-paste.gif", FileManager.clickPaste, "", 
						"disable"); 
				} else { 
					FileManager.setButton("toolbarPaste", 
						"images/toolbar-paste-disable.gif", 
						FileManager.doNothing, "disable", ""); 
				}
			}); 
		
		setTimeout("FileManager.getMessage()", 500); 
	}, 
	
	/*
	 * �����޸�����ʱ��ԭ����
	 */
	setOldname : function() { 
		$("div#oldnameLine").css("display", "block"); 
		var oldPathInput = $("input#renamePath"); 
		var oldnameInput = $("input#oldname"); 
		var newnameInput = $("input#newname"); 
		var path = this.selectedItems[0]; 
		oldPathInput.attr("value", path); 
		var oldname = path.substring(path.lastIndexOf("/") + 1, path.length); 
		oldnameInput.attr("value", oldname); // ��ʾԭ�ļ���
		newnameInput.attr("value", oldname); 
	}, 
	
	/*
	 * ����޸�����������е�ԭ����
	 */
	cleanOldname : function() { 
		var oldnameInput = $("input#oldname"); 
		var newnameInput = $("input#newname"); 
		oldnameInput.attr("value", ""); // ��ʾԭ�ļ���
		newnameInput.attr("value", ""); 
	}, 
	
	/*
	 * ��ʼ�� ajax
	 */
	initFuncPre : function() { 
		this.ajaxBg = $("div#ajaxBg"); 
		
		var rawTitles = $("div#ajaxInput > .ajaxHeader > span"); 
		rawTitles = $(rawTitles[0]); 
		rawTitles = rawTitles.html(); 
		rawTitles = rawTitles.split("|"); 
		
		this.multilanTitles = new Array(); 
		var count = rawTitles.length; 
		var rawTitle, 
		key, 
		value; 
		for(var i = 0; i < count; ++i) { 
			rawTitle = rawTitles[i]; 
			rawTitle = rawTitle.split(":"); 
			key = rawTitle[0]; 
			value = rawTitle[1]; 
			this.multilanTitles[key] = value; 
		}
		
		var ajaxFuncClose = $("div.ajaxHeader > .ajaxFuncClose"); 
		var count = ajaxFuncClose.length; 
		for(var i = 0; i < count; i++) { 
			ajaxFuncClose.get(i).onclick = FileManager.closeAjax; 
		}
	}, 
	
	/*
	 * ��ʾ Ajax ����İ�͸������
	 */
	displayAjaxBg : function(canClose) { 
		if(canClose) { 
			this.ajaxBg.get(0).onclick = FileManager.closeAjax; 
		} else { 
			this.ajaxBg.get(0).onclick = FileManager.doNothing; 
		}
		this.ajaxBg.css("height", document.documentElement.scrollHeight + "px"); 
		this.ajaxBg.css("display", "block"); 
	}, 
	
	/*
	 * ��ʾ ajax ���벿��
	 */
	displayAjaxInput : function(action, oper, title, isInput) { 
		var ajaxInput = $("div#ajaxInput"); 
		var operInput = $("input#oper"); 
		operInput.val(oper); 
		var form = ajaxInput.children("form"); 
		form.attr("action", action); 
		var titleSpan = ajaxInput.children("div.ajaxHeader").children("span"); 
		titleSpan.html(this.multilanTitles[title]); 
		if(isInput) { 
			// form.removeAttr("enctype");
			$("div#divInput").removeClass("ajaxHidden"); 
			$("div#divUpload").addClass("ajaxHidden"); 
			
		} else { 
			// form.attr("enctype", "multipart/form-data");
			$("div#divInput").addClass("ajaxHidden"); 
			$("div#divUpload").removeClass("ajaxHidden"); 
		}
		ajaxInput.css("left", FileManager.getLeftMargin() + "px"); 
		ajaxInput.fadeIn(); 
		if(isInput) { 
			$("input#newname").focus(); 
			$("input#newname").get(0).select(); 
		}
	}, 
	
	/*
	 * ��ʾ AudioPlayer
	 */
	displayAjaxAudioPlayer : function() { 
		var ajaxAudioPlayer = $("div#ajaxAudioPlayer"); 
		
		ajaxAudioPlayer.css("left", FileManager.getLeftMargin() + "px"); 
		ajaxAudioPlayer.fadeIn(); 
	}, 
	
	/*
	 * �ر� ajax ����
	 */
	closeAjax : function() { 
		var ajaxInput = $("div#ajaxInput"); 
		if(ajaxInput.is(":visible"))
			ajaxInput.fadeOut(); 
		
		var ajaxDelete = $("div#ajaxDelete"); 
		if(ajaxDelete.is(":visible"))
			ajaxDelete.fadeOut(); 
		
		$("div#oldnameLine").css("display", "none"); 
		FileManager.cleanOldname(); 
		FileManager.ajaxBg.css("display", "none"); 
		
		var ajaxAudioPlayer = $("div#ajaxAudioPlayer"); 
		if(ajaxAudioPlayer.is(":visible")) { 
			AudioPlayer.close("pAudioPlayer"); // IE 9 has a bug on this call
			ajaxAudioPlayer.fadeOut(); 
		}
	}, 
	
	changeMainViewListHeight : function() { 
		// ����Ӧ mainViewList �߶�
		var mainViewList = $("div#mainViewList"); 
		var mainViewListOffset = mainViewList.offset(); 
		var footerHeight = $("div#footer").height(); 
		var windowHeight = $(window).height(); 
		var mainViewListHeight; 
		
		if(this.isIE && $.browser.version < 8) { 
			return; 
		} else { 
			mainViewListHeight = windowHeight - mainViewListOffset.top
			 - footerHeight - 30; 
			mainViewListHeight = mainViewListHeight > FileManager.miniMainViewHeight ? mainViewListHeight
			 : FileManager.miniMainViewHeight; 
			mainViewList.css("height", mainViewListHeight + "px"); 
			mainViewList.css("overflow", "auto"); 
		}
	}, 
	
	toolbarButtonMouseIn : function() { 
		if(!$(this).hasClass("disable")) { 
			$(this).css("border-bottom", "1px solid #B8D1ED"); 
		}
	}, 
	
	toolbarButtonMouseOut : function() { 
		$(this).css("border-bottom", "1px solid white"); 
	}, 
	
	/*
	 * ׼��������
	 */
	initToolbar : function() { 
		var buttons = $("div#toolbar .toolbarButton"); 
		
		if(buttons.filter(".toolbarBack").hasClass("disable")) { // ����
			buttons.filter(".toolbarBack").find("img").attr("src", 
				"images/toolbar-back-disable.gif"); 
		}
		if(buttons.filter(".toolbarForward").hasClass("disable")) { // ǰ��
			buttons.filter(".toolbarForward").find("img").attr("src", 
				"images/toolbar-forward-disable.gif"); 
		}
		
		buttons.filter(".toolbarRefresh").click(function() { 
				window.location.reload(); // ˢ��
			}); 
		
		// buttons.filter(".toolbarSelectAll").click(FileManager.selectAll); //
		// ȫѡ
		// buttons.filter(".toolbarDeselect").click(FileManager.deselect); //
		// ȡ��ѡ��
		buttons.filter(".toolbarPaste").hasClass("disable") ? null : buttons
		.filter(".toolbarPaste").click(FileManager.clickPaste); // ճ��
		
		if(FileManager.isSearch) { 
			// ����ģʽ
			buttons.filter(".toolbarUp").addClass("disable"); // ����
			buttons.filter(".toolbarNewFolder").addClass("disable"); // �½�Ŀ¼
			buttons.filter(".toolbarUpload").addClass("disable"); // �ϴ�
		} else { 
			// ���ģʽ
			buttons.filter(".toolbarNewFolder").click(
				FileManager.clickNewFolder); // �½�Ŀ¼
			buttons.filter(".toolbarUpload").click(FileManager.clickUpload); // �ϴ�
		}
		
		buttons.hover(FileManager.toolbarButtonMouseIn, 
			FileManager.toolbarButtonMouseOut); // ��ť hover ʱ��Ч��
		
		$("#toolbar form#searchForm input[type='submit']").hover(
			FileManager.toolbarButtonMouseIn, 
			FileManager.toolbarButtonMouseOut); // ��ť hover ʱ��Ч��
		
		$("#mainView .header span #checkSelectAll").click(function() { 
				if(this.checked)
					FileManager.selectAll(); 
				else
					FileManager.deselect(); 
			}); 
	}, 
	
	/*
	 * ׼������ͼ
	 */
	initMainView : function() { 
		// changeMainViewHeight();
		FileManager.changeMainViewListHeight(); 
		$(window).resize(FileManager.changeMainViewListHeight); 
		
		var detailViewItems = $("ul#detailView"); 
		var largeiconViewItems = $("div#largeiconView"); 
		if(detailViewItems.length > 0) { 
			// ����ϸ��ͼ
			var items = detailViewItems.children("li"); 
			var count = items.length; 
			for(var i = 0; i < count; i++) { 
				var item = $(items.get(i)); 
				if(!item.hasClass("empty")) { 
					var jsObj = item.get(0); 
					jsObj.onmouseover = function() { 
						FileManager.detailViewItemOver(this); 
					}; 
					jsObj.onmouseout = function() { 
						FileManager.detailViewItemOut(this); 
					}; 
				}
			}
		} else if(largeiconViewItems.length > 0) { 
			// �Ǵ�ͼ����ͼ
			var items = largeiconViewItems.children("div.largeIconItem"); 
			var count = items.length; 
			for(var i = 0; i < count; i++) { 
				var item = $(items.get(i)); 
				if(!item.hasClass("empty")) { 
					var jsObj = item.get(0); 
					jsObj.onmouseover = function() { 
						FileManager.largeiconViewItemOver(this); 
					}; 
					jsObj.onmouseout = function() { 
						FileManager.largeiconViewItemOut(this); 
					}; 
					jsObj.onclick = function() { 
						FileManager.largeiconViewItemClicked(this); 
					}; 
					var as = jsObj.getElementsByTagName("a"); 
					for(var j = 0; j < as.length; j++) { 
						var a = as[j]; 
						a.onclick = function(e) { 
							FileManager.stopBubble(e); 
						}; 
					}
					
				}
			}
		}
		
		this.inputChecks = $("input.inputCheck"); 
		var count = this.inputChecks.length; 
		for(var i = 0; i < count; i++) { 
			var check = this.inputChecks.get(i); 
			check.onclick = function(e) { 
				FileManager.viewItemCheck(); 
				FileManager.stopBubble(e); 
			}; 
		}
		FileManager.viewItemCheck(); 
	}, 
	
	/*
	 * ׼�� AudioPlayer
	 */
	initAudioPlayer : function() { 
		AudioPlayer.setup("images/player.swf", { 
				width : 290, 
				initialvolume : 100
			}); 
		
		$("a.audioPlayer").click(function() { 
				var audioObj = $(this); 
				var audioLink = audioObj.attr("href"); 
				var divFlash = $("div.flashAudioPlayer"); 
				
				// Player
				AudioPlayer.embed("pAudioPlayer", { 
						soundFile : audioLink
					}); 
				var pLink = divFlash.find("p#link"); 
				pLink.html("<a href=\"" + audioLink + "\">"
					 + audioObj.attr("title") + "</a>"); 
				
				FileManager.displayAjaxBg(true); 
				FileManager.displayAjaxAudioPlayer(); 
				
				return false; 
			}); 
	}, 
	
	initMediaPreview : function() { 
		// lightbox
		$('a.lightboxImg').lightBox({ 
				overlayOpacity : 0.5, 
				autoAdapt : true
			}); 
		
		// AudioPlayer
		FileManager.initAudioPlayer(); 
	}
	
}

/*
 * ��ʼ��
 */
FileManager.init = function() { 
	FileManager.isIE = $.browser.msie ? true : false; 
	// alert($.browser.version);
	var str = "#mainView > .header > span." + FileManager.sortName + " > a"; 
	var item = $(str); 
	item.addClass("sort" + FileManager.sortOrder); 
	
	// FileManager.initFullPath();
	jqMenu.init(); 
	
	FileManager.initToolbar(); 
	
	FileManager.initMainView(); 
	
	FileManager.initFuncPre(); 
	
	FileManager.getMessage(); 
	
	FileManager.initMediaPreview(); 
}; 

// $(window).load(init); // ����׼������
$(FileManager.init);