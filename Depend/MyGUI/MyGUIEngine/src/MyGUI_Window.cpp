/*!
	@file
	@author		Albert Semenov
	@date		11/2007
*/
/*
	This file is part of MyGUI.

	MyGUI is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	MyGUI is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with MyGUI.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "MyGUI_Precompiled.h"
#include "MyGUI_Window.h"
#include "MyGUI_Macros.h"
#include "MyGUI_Gui.h"
#include "MyGUI_ControllerManager.h"
#include "MyGUI_InputManager.h"
#include "MyGUI_WidgetManager.h"
#include "MyGUI_ResourceSkin.h"

namespace MyGUI
{

	const float WINDOW_ALPHA_ACTIVE = ALPHA_MAX;
	const float WINDOW_ALPHA_FOCUS = 0.7f;
	const float WINDOW_ALPHA_DEACTIVE = 0.3f;
	const float WINDOW_SPEED_COEF = 3.0f;

	const int WINDOW_SNAP_DISTANSE = 10;

	Window::Window() :
		mWidgetCaption(nullptr),
		mMouseRootFocus(false),
		mKeyRootFocus(false),
		mIsAutoAlpha(false),
		mSnap(false),
		mAnimateSmooth(false),
		mClient(nullptr)
	{
	}

	void Window::initialiseOverride()
	{
		Base::initialiseOverride();

		// FIXME нам нужен фокус клавы
		setNeedKeyFocus(true);

		// дефолтные размеры
		mMinmax.set(0, 0, 3000, 3000);

		bool main_move = false;
		if (isUserString("MainMove"))
		{
			setUserString("Scale", "1 1 0 0");
			main_move = true;
		}

		assignWidget(mClient, "Client");
		if (mClient != nullptr)
		{
			if (main_move)
			{
				mClient->setUserString("Scale", "1 1 0 0");
				mClient->eventMouseButtonPressed += newDelegate(this, &Window::notifyMousePressed);
				mClient->eventMouseDrag += newDelegate(this, &Window::notifyMouseDrag);
			}
			setWidgetClient(mClient);
		}

		assignWidget(mWidgetCaption, "Caption");
		if (mWidgetCaption != nullptr)
		{
			mWidgetCaption->eventMouseButtonPressed += newDelegate(this, &Window::notifyMousePressed);
			mWidgetCaption->eventMouseDrag += newDelegate(this, &Window::notifyMouseDrag);
		}

		VectorWidgetPtr buttons = getSkinWidgetsByName("Button");
		for (VectorWidgetPtr::iterator iter = buttons.begin(); iter != buttons.end(); ++iter)
		{
			(*iter)->eventMouseButtonClick += newDelegate(this, &Window::notifyPressedButtonEvent);
		}

		VectorWidgetPtr actions = getSkinWidgetsByName("Action");
		for (VectorWidgetPtr::iterator iter = actions.begin(); iter != actions.end(); ++iter)
		{
			(*iter)->eventMouseButtonPressed += newDelegate(this, &Window::notifyMousePressed);
			(*iter)->eventMouseDrag += newDelegate(this, &Window::notifyMouseDrag);
		}
	}

	void Window::shutdownOverride()
	{
		mClient = nullptr;
		mWidgetCaption = nullptr;

		Base::shutdownOverride();
	}

	void Window::onMouseChangeRootFocus(bool _focus)
	{
		mMouseRootFocus = _focus;
		updateAlpha();

		Base::onMouseChangeRootFocus(_focus);
	}

	void Window::onKeyChangeRootFocus(bool _focus)
	{
		mKeyRootFocus = _focus;
		updateAlpha();

		Base::onKeyChangeRootFocus(_focus);
	}

	void Window::onMouseDrag(int _left, int _top)
	{
		// на тот случай, если двигать окно, можно за любое место виджета
		notifyMouseDrag(this, _left, _top);

		Base::onMouseDrag(_left, _top);
	}

	void Window::onMouseButtonPressed(int _left, int _top, MouseButton _id)
	{
		notifyMousePressed(this, _left, _top, _id);

		Base::onMouseButtonPressed(_left, _top, _id);
	}

	void Window::notifyMousePressed(MyGUI::Widget* _sender, int _left, int _top, MouseButton _id)
	{
		if (MouseButton::Left == _id)
		{
			mPreActionCoord = mCoord;
			mCurrentActionScale = IntCoord::parse(_sender->getUserString("Scale"));
		}
	}

	void Window::notifyPressedButtonEvent(MyGUI::Widget* _sender)
	{
		eventWindowButtonPressed(this, _sender->getUserString("Event"));
	}

	void Window::notifyMouseDrag(MyGUI::Widget* _sender, int _left, int _top)
	{
		const IntPoint& point = InputManager::getInstance().getLastLeftPressed();

		IntCoord coord = mCurrentActionScale;
		coord.left *= (_left - point.left);
		coord.top *= (_top - point.top);
		coord.width *= (_left - point.left);
		coord.height *= (_top - point.top);

		if (coord.left == 0 && coord.top == 0)
			setSize((mPreActionCoord + coord).size());
		else if (coord.width == 0 && coord.height == 0)
			setPosition((mPreActionCoord + coord).point());
		else
			setCoord(mPreActionCoord + coord);

		// посылаем событие о изменении позиции и размере
		eventWindowChangeCoord(this);
	}

	void Window::updateAlpha()
	{
		if (!mIsAutoAlpha) return;

		float alpha;
		if (mKeyRootFocus) alpha = WINDOW_ALPHA_ACTIVE;
		else if (mMouseRootFocus) alpha = WINDOW_ALPHA_FOCUS;
		else alpha = WINDOW_ALPHA_DEACTIVE;

		ControllerFadeAlpha* controller = createControllerFadeAlpha(alpha, WINDOW_SPEED_COEF, true);
		ControllerManager::getInstance().addItem(this, controller);
	}

	void Window::setAutoAlpha(bool _auto)
	{
		mIsAutoAlpha = _auto;
		if (!_auto) setAlpha(ALPHA_MAX);
		else
		{
			if (mKeyRootFocus) setAlpha(WINDOW_ALPHA_ACTIVE);
			else if (mMouseRootFocus) setAlpha(WINDOW_ALPHA_FOCUS);
			else setAlpha(WINDOW_ALPHA_DEACTIVE);
		}
	}

	void Window::setPosition(const IntPoint& _point)
	{
		IntPoint point = _point;
		// прилепляем к краям
		if (mSnap)
		{
			IntCoord coord(point, mCoord.size());
			getSnappedCoord(coord);
			point = coord.point();
		}

		Base::setPosition(point);
	}

	void Window::setSize(const IntSize& _size)
	{
		IntSize size = _size;
		// прилепляем к краям

		if (size.width < mMinmax.left) size.width = mMinmax.left;
		else if (size.width > mMinmax.right) size.width = mMinmax.right;
		if (size.height < mMinmax.top) size.height = mMinmax.top;
		else if (size.height > mMinmax.bottom) size.height = mMinmax.bottom;
		if ((size.width == mCoord.width) && (size.height == mCoord.height) ) return;

		if (mSnap)
		{
			IntCoord coord(mCoord.point(), size);
			getSnappedCoord(coord);
			size = coord.size();
		}

		Base::setSize(size);
	}

	void Window::setCoord(const IntCoord& _coord)
	{
		IntPoint pos = _coord.point();
		IntSize size = _coord.size();

		if (size.width < mMinmax.left)
		{
			int offset = mMinmax.left - size.width;
			size.width = mMinmax.left;
			if ((pos.left - mCoord.left) > offset) pos.left -= offset;
			else pos.left = mCoord.left;
		}
		else if (size.width > mMinmax.right)
		{
			int offset = mMinmax.right - size.width;
			size.width = mMinmax.right;
			if ((pos.left - mCoord.left) < offset) pos.left -= offset;
			else pos.left = mCoord.left;
		}
		if (size.height < mMinmax.top)
		{
			int offset = mMinmax.top - size.height;
			size.height = mMinmax.top;
			if ((pos.top - mCoord.top) > offset) pos.top -= offset;
			else pos.top = mCoord.top;
		}
		else if (size.height > mMinmax.bottom)
		{
			int offset = mMinmax.bottom - size.height;
			size.height = mMinmax.bottom;
			if ((pos.top - mCoord.top) < offset) pos.top -= offset;
			else pos.top = mCoord.top;
		}

		// прилепляем к краям
		if (mSnap)
		{
			IntCoord coord(pos, size);
			getSnappedCoord(coord);
			size = coord.size();
		}

		IntCoord coord(pos, size);
		if (coord == mCoord) return;

		Base::setCoord(coord);
	}

	void Window::setCaption(const UString& _caption)
	{
		if (mWidgetCaption != nullptr) mWidgetCaption->setCaption(_caption);
		else Base::setCaption(_caption);
	}

	const UString& Window::getCaption()
	{
		if (mWidgetCaption != nullptr) return mWidgetCaption->getCaption();
		return Base::getCaption();
	}

	void Window::destroySmooth()
	{
		ControllerFadeAlpha* controller = createControllerFadeAlpha(ALPHA_MIN, WINDOW_SPEED_COEF, false);
		controller->eventPostAction += newDelegate(action::actionWidgetDestroy);
		ControllerManager::getInstance().addItem(this, controller);
	}

	void Window::animateStop(Widget* _widget)
	{
		if (mAnimateSmooth)
		{
			ControllerManager::getInstance().removeItem(this);
			mAnimateSmooth = false;
		}
	}

	void Window::setVisible(bool _visible)
	{

		if (mAnimateSmooth)
		{
			ControllerManager::getInstance().removeItem(this);
			setAlpha(getAlphaVisible());
			setEnabledSilent(true);
			mAnimateSmooth = false;
		}

		Base::setVisible(_visible);
	}

	float Window::getAlphaVisible()
	{
		return (mIsAutoAlpha && !mKeyRootFocus) ? WINDOW_ALPHA_DEACTIVE : ALPHA_MAX;
	}

	void Window::getSnappedCoord(IntCoord& _coord)
	{
		if (abs(_coord.left) <= WINDOW_SNAP_DISTANSE) _coord.left = 0;
		if (abs(_coord.top) <= WINDOW_SNAP_DISTANSE) _coord.top = 0;

		const IntSize view_size = getParentSize();

		if ( abs(_coord.left + _coord.width - view_size.width) < WINDOW_SNAP_DISTANSE) _coord.left = view_size.width - _coord.width;
		if ( abs(_coord.top + _coord.height - view_size.height) < WINDOW_SNAP_DISTANSE) _coord.top = view_size.height - _coord.height;
	}

	void Window::setVisibleSmooth(bool _visible)
	{
		mAnimateSmooth = true;
		ControllerManager::getInstance().removeItem(this);

		if (_visible)
		{
			setEnabledSilent(true);
			if (!getVisible())
			{
				setAlpha(ALPHA_MIN);
				Base::setVisible(true);
			}
			ControllerFadeAlpha* controller = createControllerFadeAlpha(getAlphaVisible(), WINDOW_SPEED_COEF, true);
			controller->eventPostAction += newDelegate(this, &Window::animateStop);
			ControllerManager::getInstance().addItem(this, controller);
		}
		else
		{
			setEnabledSilent(false);
			ControllerFadeAlpha* controller = createControllerFadeAlpha(ALPHA_MIN, WINDOW_SPEED_COEF, false);
			controller->eventPostAction += newDelegate(action::actionWidgetHide);
			ControllerManager::getInstance().addItem(this, controller);
		}
	}

	ControllerFadeAlpha* Window::createControllerFadeAlpha(float _alpha, float _coef, bool _enable)
	{
		ControllerItem* item = ControllerManager::getInstance().createItem(ControllerFadeAlpha::getClassTypeName());
		ControllerFadeAlpha* controller = item->castType<ControllerFadeAlpha>();

		controller->setAlpha(_alpha);
		controller->setCoef(_coef);
		controller->setEnabled(_enable);

		return controller;
	}

	void Window::setMinSize(const IntSize& _value)
	{
		mMinmax.left = _value.width;
		mMinmax.top = _value.height;
	}

	IntSize Window::getMinSize()
	{
		return IntSize(mMinmax.left, mMinmax.top);
	}

	void Window::setMaxSize(const IntSize& _value)
	{
		mMinmax.right = _value.width;
		mMinmax.bottom = _value.height;
	}

	IntSize Window::getMaxSize()
	{
		return IntSize(mMinmax.right, mMinmax.bottom);
	}

	void Window::setPropertyOverride(const std::string& _key, const std::string& _value)
	{
		if (_key == "AutoAlpha") setAutoAlpha(utility::parseValue<bool>(_value));
		else if (_key == "Snap") setSnap(utility::parseValue<bool>(_value));
		else if (_key == "MinSize") setMinSize(utility::parseValue<IntSize>(_value));
		else if (_key == "MaxSize") setMaxSize(utility::parseValue<IntSize>(_value));
		else
		{
			Base::setPropertyOverride(_key, _value);
			return;
		}
		eventChangeProperty(this, _key, _value);
	}

} // namespace MyGUI
