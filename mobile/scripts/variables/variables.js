function KAlgebraExtension(cfg)
{
	this.ui = cfg.newTreeView("view");
	this.ui.rootIsDecorated = false;
	this.ui.setModel(cfg.variablesModel());
}

