var ui;
var dialog;
var a;

function edit()
{
	dialog.show()
}

function configure(cfg, analitza)
{
	a=analitza;
	ui = cfg.newVerticalLayout();
	dialog = cfg.newFunctionsDialog("dialog");
	
	ui.addWidget(cfg.newGraph2D("graph"));
	ui.addWidget(cfg.newQPushButton("edit"));
	ui.edit.text = "Edit"
	
	ui.edit.clicked.connect(edit);
	return ui;
}

