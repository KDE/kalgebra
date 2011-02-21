KAlgebraExtension.prototype.addOperation = function()
{
	with(this.ui) {
		console.addItem(Analitza.execute(display.text))
		display.selectAll()
		console.scrollDown()
	}
}

function KAlgebraExtension(cfg)
{
	this.ui = cfg.newVerticalLayout();
	this.ui.addWidget(cfg.newListWidget("console"));
	this.ui.addWidget(cfg.newQLineEdit("display"));
	this.ui.display.setFocus()
	
	this.ui.display.returnPressed.connect(this, this.addOperation);
}
