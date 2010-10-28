KAlgebraExtension.prototype.addOperation = function()
{
	with(this.ui) {
		console.addItem(Analitza.execute(display.text))
		display.selectAll()
	}
}

function KAlgebraExtension(cfg)
{
	this.ui = cfg.newVerticalLayout();
	this.ui.addWidget(cfg.newQLineEdit("display"));
	this.ui.addWidget(cfg.newListWidget("console"));
	
	this.ui.display.returnPressed.connect(this, this.addOperation);
}
