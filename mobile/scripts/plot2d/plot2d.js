configure.prototype.edit = function()
{
	this.dialog.show()
}

function configure(cfg)
{
	this.ui = cfg.newVerticalLayout();
	this.dialog = cfg.newFunctionsDialog("dialog");
	
	this.ui.addWidget(cfg.newGraph2D("graph"));
	this.ui.addWidget(cfg.newQPushButton("edit"));
	this.ui.edit.text = "Edit"
	
	this.ui.edit.clicked.connect(this, this.edit);
}

