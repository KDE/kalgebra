var ui;

function configure(cfg, analitza)
{
	ui = cfg.newTreeView("view");
	ui.rootIsDecorated = false;
	ui.setModel(cfg.variablesModel());
	return ui;
}

