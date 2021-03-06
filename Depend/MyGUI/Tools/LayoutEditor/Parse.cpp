/*!
	@file
	@author		Georgiy Evmenov
	@date		10/2008
*/

#include "precompiled.h"
#include "Parse.h"

namespace tools
{
	namespace utility
	{
		bool checkParseFileName(MyGUI::Edit* _edit)
		{
			static const MyGUI::UString colour = MyGUI::LanguageManager::getInstance().getTag("ColourError");
			const MyGUI::UString& text = _edit->getOnlyText();
			size_t index = _edit->getTextCursor();

			bool success = false;

			if (text.find_first_of("*?") == std::string::npos)
			{
				success = MyGUI::DataManager::getInstance().isDataExist(text);
			}
			else
			{
				success = false;
			}

			if (success)
				_edit->setCaption(text);
			else
				_edit->setCaption(colour + text);

			_edit->setTextCursor(index);

			return success;
		}

	} // namespace utility
} // namespace tools
