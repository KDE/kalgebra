KAlgebraExtension.prototype.execute = function()
{
	var a=Analitza;
	with(this.ui) {
		list.clear();
		
		var tmp = a.unusedVariableName();
		print(a.execute(tmp+":="+display.text));
		for (var i=from.value; i<=to.value; i++) {
			print(":::: "+tmp + ", " + i);
			var args = new Array();
			args[0]=i;
			
			list.addItem(i+": "+a.executeFunc(tmp, args));
		}
		
		a.removeVariable(tmp);
	}
}

function KAlgebraExtension(cfg)
{
	this.ui = cfg.newVerticalLayout();
	this.ui.addWidget(cfg.newQLineEdit("display"));
	this.ui.addWidget(cfg.newQDoubleSpinBox("from"));
	this.ui.addWidget(cfg.newQDoubleSpinBox("to"));
	this.ui.addWidget(cfg.newListWidget("list"));
	this.ui.addWidget(cfg.newQPushButton("go"));
	this.ui.go.text = "Go!"
	
	this.ui.go.clicked.connect(this, this.execute);
}

