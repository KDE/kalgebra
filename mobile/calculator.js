var ui;

function addOperation()
{
	with(ui) {
		console.addOperation(display.text)
		display.selectAll()
	}
}

function configure(cfg, analitza)
{
	ui = cfg.newVerticalLayout();
	ui.addWidget(cfg.newQLineEdit("display"));
	ui.addWidget(cfg.newConsole("console"));
	
	ui.display.returnPressed.connect(addOperation);
	return ui;
}

