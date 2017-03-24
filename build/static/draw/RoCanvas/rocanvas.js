/* RoCanvas.js version 1.4.0
* Converts any canvas object into a RoCanvas HTML 5 Drawing board
* Adds tools, shapes, color and size selection, etc
* Full documentation at http://re.trotoys.com/article/rocanvas/ */

// rocanvas instances
var RoCanvasInstances = {};

var RoCanvas= function () {	
	// internal vars
	this.clickX = [];
	this.clickY = [];
	this.startX = 0;
	this.startY = 0;
	this.clearRect = [0,0,0,0];
	this.clearCircle = [0,0,0];
	this.clickDrag = [];
	this.paint = false;
	this.context = {};	
	
	// changeable defaults
	this.shape = "round";	
	this.color = "#000";	
	this.tool = "path";
	this.drawTool = "path";
	this.lineWidth = 5;
	
	// toolbar
	this.toolbar = {
		colors: ["#FFF","#000","#FF0000","#00FF00","#0000FF","#FFFF00","#00FFFF"],
		custom_color: true,
		sizes: [2, 5, 10, 25],
		tools: ["path","rectangle","filledrectangle","circle","filledcircle"],
		clearButton: {"text": "Clear Canvas"},
		saveButton: null
	};
	
	var self = this;
	
	// the "constructor" that actually takes a div and converts it into RoCanvas
	// @param id string, the DOM ID of the div
	// @param vars - optionally pass custom vars, toolbar etc  
	this.RO = function(id, vars) {		
		self.id = id;
		
		// add to instances
		RoCanvasInstances[id] = self;
		
		// this file location folder
		self.fileLocation();		
		
		// if settings or tools are passed overwrite them
		vars = vars||{};		
		
		if(vars['toolbar'])
		{
			for(var key in vars['toolbar'])
			{
				self.toolbar[key]=vars['toolbar'][key];
			}
		}		
		
		// if vars[settings] is passed allow changing some defaults 	
		if(vars['settings'])
		{
			// allow only shape, color, tool, lineWidth
			for(var key in vars['settings'])
			{
				if(!(key=='shape' || key=='color' || key=='tool' || key=='lineWidth')) continue;
				
				self[key]=vars['settings'][key];			
			}
		}	
		
		// prepare canvas		
		self.canvas = document.getElementById(id);			
		document.getElementById(id).style.cursor='crosshair';	
		
		// get canvas parent and append div for the tools
		var parent=self.canvas.parentNode;
		var toolBarDOM=document.createElement("div");		

		// add colors
		toolBarHTML="";
		if(self.toolbar.colors)
		{
			toolBarHTML='<div style="clear:both;">&nbsp;</div>';
			toolBarHTML+='<div style="float:left;">Colors:</div>';
			for(c in self.toolbar['colors'])
			{
				toolBarHTML+="<a href=\"#\" class=\"roCanvasColorPicker\" onclick=\"RoCanvasInstances['"+self.id+"'].setColor('"
					+self.toolbar['colors'][c]+"');return false;\" style=\"background:"+self.toolbar['colors'][c]+";\">&nbsp;</a> ";
			}
		}	
		
		// custom color choice?
		if(self.toolbar.custom_color) {
			toolBarHTML += "&nbsp; Custom:&nbsp;<a href=\"#\" class=\"roCanvasColorPicker\" style=\"background:white;\" onclick=\"RoCanvasInstances['"+self.id+"'].setColor(this.style.background);return false;\" id='customColorChoice"+ self.id +"'>&nbsp;</a> #<input type='text' size='6' maxlength='6' onkeyup=\"RoCanvasInstances['"+self.id+"'].customColor(this.value);\">";
		}	
			
		// add sizes
		if(self.toolbar.sizes)
		{
			toolBarHTML+='<div style="clear:both;">&nbsp;</div>';
			toolBarHTML+='<div style="float:left;">Sizes:</div>';
			for(s in self.toolbar['sizes'])
			{
				toolBarHTML+="<a href=\"#\" class=\"roCanvasColorPicker\" onclick=\"RoCanvasInstances['"+self.id+"'].setSize("+self.toolbar['sizes'][s]
					+");return false;\" style=\"width:"+self.toolbar['sizes'][s]+"px;height:"
					+self.toolbar['sizes'][s]+"px;margin-left:15px;\">&nbsp;</a>";	
			}
		}		
		
		// add tools
		if(self.toolbar.tools)
		{
			toolBarHTML+='<div style="clear:both;">&nbsp;</div>';
			toolBarHTML+='<div style="float:left;">Tools:</div>';
			for (tool in self.toolbar['tools'])
			{
				toolBarHTML+="<a href='#' onclick=\"RoCanvasInstances['"+self.id+"'].setTool('"+self.toolbar['tools'][tool]+"');return false;\"><img src=\""+self.filepath+"/img/tool-"+self.toolbar['tools'][tool]+".png\" width='25' height='25'></a> ";
			}
		}
		
		// add buttons
		if(self.toolbar.clearButton || self.toolbar.saveButton)
		{
			toolBarHTML+='<div style="clear:both;">&nbsp;</div>';
			toolBarHTML+="<p>";
			
			if(self.toolbar.clearButton)
			{
				toolBarHTML+='<input type="button" value="'+self.toolbar.clearButton.text+'"' + " onclick=\"RoCanvasInstances['"+self.id+"'].clearCanvas();\">";
			}			
			
			if(self.toolbar.saveButton)
			{
				var saveButtonCallback="";
				if(self.toolbar.saveButton.callback) saveButtonCallback=' onclick="'+ self.toolbar.saveButton.callback + '(this);"';
				toolBarHTML+='<input type="button" id="RoCanvasSave_'+ this.id +'" value="'+self.toolbar.saveButton.text+'"'+saveButtonCallback+'>';
			}			
			toolBarHTML+="</p>";
		}
		
		toolBarDOM.innerHTML=toolBarHTML;
		parent.appendChild(toolBarDOM);
		
		// Check the element is in the DOM and the browser supports canvas
		if(self.canvas.getContext) 
		{
			 // Initaliase a 2-dimensional drawing context
			 self.context = self.canvas.getContext('2d');			 
			 self.context.strokeStyle = self.color;
			 self.context.lineJoin = self.shape;
			 self.context.lineWidth = self.lineWidth;	
		}
		
		/* declare touch actions */
		
		var touchX,touchY;
		
		self.canvas.addEventListener('touchstart', function(e){			
			getTouchPos(e);
			
			self.startX=touchX;
			self.startY=touchY;
				
			self.paint = true;	
		  
			if(self.drawTool=='path')
			{
				self.addClick(touchX, touchY);
				self.redraw();
			}
			
			event.preventDefault();
		}, false);
		
		self.canvas.addEventListener('touchmove', function(e){
			getTouchPos(e);
			
			self.startX=touchX;
			self.startY=touchY;
			
			if(self.paint)
		    {		    	
				 // clear any rectangles that should be cleared
			    self.context.clearRect(self.clearRect[0],self.clearRect[1],
					 self.clearRect[2],self.clearRect[3]);			    
			    // clear any circles that have to be cleared
			    // set color to white but remember old color
			    self.context.strokeStyle=self.context.fillStyle='#ffffff';		    
			    self.context.beginPath();
			    self.context.arc(self.clearCircle[0],self.clearCircle[1],self.clearCircle[2],0,Math.PI*2);
			    self.context.closePath();
			    self.context.stroke();
			    self.context.fill();   
			    self.setColor(self.color);
					
				// draw different shapes				
				switch(self.drawTool)
				{
					case 'rectangle':		
					case 'filledrectangle':		
						w = e.pageX - touchX - self.startX;
						h = e.pageY - touchY - self.startY;
												
						// insert postions for clearing			
						self.clearRect=[self.startX, self.startY, w, h];
						
						if(self.drawTool=='rectangle')
						{
							self.context.strokeRect(self.startX, self.startY, w, h);			
						}
						else
						{				
							self.context.fillRect(self.startX, self.startY, w, h);			
						}
					break;
			        case 'circle':
			        case 'filledcircle':
			            w = Math.abs(e.pageX - touchX - self.startX);
			            h = Math.abs(e.pageY - touchY - self.startY);
			               
			            // r is the bigger of h and w
			            r = h>w?h:w;
			            
			            // remember to clear it								            
			            self.clearCircle=[self.startX, self.startY, r];
			            
			            self.context.beginPath();
			            // draw from the center
			            self.context.arc(self.startX,self.startY,r,0,Math.PI*2);
			            self.context.closePath();
			            
			            if(self.drawTool=='circle') 
			            {
			            	// fill with white, then stroke
			            	var oldColor=self.color;			            	
			            	self.setColor("#FFFFFF");
			            	self.context.fill();
			            	
			            	self.setColor(oldColor);
			            	self.context.stroke();
			            }            
			            else self.context.fill();
			        break;
					default:
						self.addClick(e.pageX - document.getElementById(id).offsetLeft, e.pageY - document.getElementById(id).offsetTop, true);
					break;
				}
		    
				self.redraw();
			
				event.preventDefault();
			}
		},false);
		
		function getTouchPos(e) {
			if (!e)
				var e = event;

			if (e.touches) {
				if (e.touches.length == 1) { // Only deal with one finger
					var touch = e.touches[0]; // Get the information for finger #1
					touchX=touch.pageX-touch.target.offsetLeft;
					touchY=touch.pageY-touch.target.offsetTop;
				}
			}
		}
		
		/* declare mouse actions */
		
		// on mouse down
		self.canvas.addEventListener('mousedown', function(e){			
		  var mouseX = e.pageX - this.offsetLeft;
		  self.startX=mouseX;
		  var mouseY = e.pageY - this.offsetTop;		  
		  self.startY=mouseY;
				
		  self.paint = true;	
		  
		  if(self.drawTool=='path')
		  {
				self.addClick(mouseX, mouseY);
				self.redraw();
		  }
		}, false);
		
		// on dragging
		self.canvas.addEventListener('mousemove', function(e)
		{
		    if(self.paint)
		    {		    	
				 // clear any rectangles that should be cleared
			    self.context.clearRect(self.clearRect[0],self.clearRect[1],
					 self.clearRect[2],self.clearRect[3]);			    
			    // clear any circles that have to be cleared
			    // set color to white but remember old color
			    self.context.strokeStyle=self.context.fillStyle='#ffffff';		    
			    self.context.beginPath();
			    self.context.arc(self.clearCircle[0],self.clearCircle[1],self.clearCircle[2],0,Math.PI*2);
			    self.context.closePath();
			    self.context.stroke();
			    self.context.fill();   
			    self.setColor(self.color);
					
				// draw different shapes				
				switch(self.drawTool)
				{
					case 'rectangle':		
					case 'filledrectangle':		
						w = e.pageX - this.offsetLeft - self.startX;
						h = e.pageY - this.offsetTop - self.startY;
												
						// insert postions for clearing			
						self.clearRect=[self.startX, self.startY, w, h];
						
						if(self.drawTool=='rectangle')
						{
							self.context.strokeRect(self.startX, self.startY, w, h);			
						}
						else
						{				
							self.context.fillRect(self.startX, self.startY, w, h);			
						}
					break;
			        case 'circle':
			        case 'filledcircle':
			            w = Math.abs(e.pageX - this.offsetLeft - self.startX);
			            h = Math.abs(e.pageY - this.offsetTop - self.startY);
			               
			            // r is the bigger of h and w
			            r = h>w?h:w;
			            
			            // remember to clear it								            
			            self.clearCircle=[self.startX, self.startY, r];
			            
			            self.context.beginPath();
			            // draw from the center
			            self.context.arc(self.startX,self.startY,r,0,Math.PI*2);
			            self.context.closePath();
			            
			            if(self.drawTool=='circle') 
			            {
			            	// fill with white, then stroke
			            	var oldColor=self.color;			            	
			            	self.setColor("#FFFFFF");
			            	self.context.fill();
			            	
			            	self.setColor(oldColor);
			            	self.context.stroke();
			            }            
			            else self.context.fill();
			        break;
					default:
						self.addClick(e.pageX - document.getElementById(id).offsetLeft, e.pageY - document.getElementById(id).offsetTop, true);
					break;
			}
		    
		    self.redraw();
		  }
		}, false);
		
		// when mouse is released
		self.canvas.addEventListener('mouseup', function(e){
		  self.paint = false;
		  
		  self.clickX = new Array();
		  self.clickY = new Array();
		  self.clickDrag = new Array();
		  self.clearRect=[0,0,0,0];
		  self.clearCircle=[0,0,0]; 	 	
		}, false);
		
		this.canvas.addEventListener('mouseleave', function(e){
		  self.paint = false;
		}, false);
	};
	
	this.addClick = function(x, y, dragging)
	{
	  self.clickX.push(x);
	  self.clickY.push(y);
	  self.clickDrag.push(dragging);
	};
	
	this.redraw = function()
	{		
	  for(var i=0; i < self.clickX.length; i++)
	  {		
	    self.context.beginPath();
	    if(self.clickDrag[i] && i){	    	
	      self.context.moveTo(self.clickX[i-1], self.clickY[i-1]);
	     }else{	     	
	       self.context.moveTo(self.clickX[i]-1, self.clickY[i]);
	     }
	     
	     self.context.lineTo(self.clickX[i], self.clickY[i]);
	     self.context.closePath();	     
	     self.context.stroke();
	  }
	};
	
	// blank the entire canvas
	this.clearCanvas = function()
	{
		oldLineWidth=self.context.lineWidth;	
		self.context.clearRect(0,0,self.canvas.width,self.canvas.height);
	   self.canvas.width = self.canvas.width;
	    
	   self.clickX = new Array();
	   self.clickY = new Array();
	   RoCanvas.clickDrag = new Array();
	   self.setSize(oldLineWidth);
	   self.context.lineJoin = self.shape;
	   self.setColor(self.color);
	};
	
	// sets the size of the drawing line in pixels
	this.setSize = function(px)
	{
	    self.context.lineWidth=px;
	};

	// sets the tool to draw
	this.setTool = function(tool)
	{
		self.drawTool=tool;	
	};
	
	this.setColor = function setColor(col)
	{		
	   self.context.strokeStyle = col;
		self.context.fillStyle = col;
		self.color=col;
	};
	
	// finds the location of this file
	// required to render proper include path for images	
	this.fileLocation = function()
	{
		var scripts = document.getElementsByTagName('script');
		for(i=0; i<scripts.length;i++)
		{
			if(scripts[i].src && scripts[i].src.indexOf("rocanvas.js")>0)
			{
				path=scripts[i].src;
			}
		}		
		path=path.replace(/rocanvas\.js.*$/, '');
		
		self.filepath=path;
	}; 
	
	// update custom color when value is typed in the box
	this.customColor = function(val) {		
		document.getElementById('customColorChoice' + this.id).style.background = "#" + val;
		this.setColor('#'+val);
	}
	
	// serialize the drawing board data
	this.serialize = function() {
		var strImageData = this.canvas.toDataURL();
		return strImageData;  
	}
}
