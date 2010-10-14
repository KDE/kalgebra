var ui;
var a;

function execute()
{
	with(ui) {
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

function configure(cfg, analitza)
{
	a=analitza;
	ui = cfg.newVerticalLayout();
	ui.addWidget(cfg.newQLineEdit("display"));
	ui.addWidget(cfg.newQDoubleSpinBox("from")); ui.addWidget(cfg.newQDoubleSpinBox("to"));
	ui.addWidget(cfg.newListWidget("list"));
	ui.addWidget(cfg.newQPushButton("go"));
	ui.go.text = "Go!"
	
	ui.go.clicked.connect(execute);
	return ui;
}

