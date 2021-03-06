#ifndef __WIDGET_TYPES_H__
#define __WIDGET_TYPES_H__

#include "SkinInfo.h"
#include "WidgetStyle.h"
#include "PossibleValue.h"

namespace tools
{
	class WidgetTypes :
		public MyGUI::Singleton<WidgetTypes>
	{
	public:
		void initialise();
		void shutdown();

		WidgetStyle* findWidgetStyle(const std::string& _type);
		std::vector<std::string> findPossibleValues(const std::string& _name);

		void clearAllSkins();

		void addWidgetSkinType(const std::string& _type, const std::string& _skin, const std::string& _group, const std::string& _button_name);

		const SkinGroups& getSkinGroups()
		{
			return mSkinGroups;
		}

	private:
		void loadWidgets(MyGUI::xml::ElementPtr _node, const std::string& _file, MyGUI::Version _version);
		void loadValues(MyGUI::xml::ElementPtr _node, const std::string& _file, MyGUI::Version _version);

		WidgetStyle* getWidgetType(const std::string& _name);

		PossibleValue* getPossibleValue(const std::string& _name);

	private:
		VectorWidgetType mWidgetTypes;
		SkinGroups mSkinGroups;
		VectorPossibleValue mPossibleValues;
	};

} // namespace tools

#endif // __WIDGET_TYPES_H__
