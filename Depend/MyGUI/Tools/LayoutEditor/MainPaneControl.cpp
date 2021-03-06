#include "MainPaneControl.h"

namespace tools
{
	MainPaneControl::MainPaneControl() :
		wraps::BaseLayout("MainPane.layout"),
		mMainMenuControl(nullptr),
		mToolsControl(nullptr),
		mWorkspaceControl(nullptr)
	{
		assignBase(mMainMenuControl, "MainMenuControl");
		assignBase(mToolsControl, "ToolsControl");
		assignBase(mWorkspaceControl, "WorkspaceControl");
	}

	MainPaneControl::~MainPaneControl()
	{
	}

	void MainPaneControl::setVisible(bool _value)
	{
		mMainWidget->setVisible(_value);
	}

} // namespace tools
