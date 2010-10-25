var ui;
var analitza;

function addOperation()
{
	with(ui) {
		console.addItem(analitza.execute(display.text))
		display.selectAll()
	}
}

function configure(cfg, a)
{
	analitza=a;
	
	ui = cfg.newVerticalLayout();
	ui.addWidget(cfg.newQLineEdit("display"));
	ui.addWidget(cfg.newListWidget("console"));
	
	ui.display.returnPressed.connect(addOperation);
	return ui;
}

