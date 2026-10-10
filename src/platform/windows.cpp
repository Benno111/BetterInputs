#include <Geode/utils/Keyboard.hpp>
#include <Geode/cocos/robtop/glfw/glfw3.h>

#include "BetterTextInputNode.hpp"

#include "utils.hpp"

using namespace geode::prelude;

namespace
{
	ListenerHandle g_keyboardListener;
	ListenerHandle g_mouseListener;

	bool isPressed(KeyboardInputData const& data)
	{
		return data.action == KeyboardInputData::Action::Press ||
			data.action == KeyboardInputData::Action::Repeat;
	}

	bool hasModifier(KeyboardInputData const& data, uint8_t modifier)
	{
		return (data.modifiers & modifier) != 0;
	}

	ListenerResult handleKeyboardInput(KeyboardInputData& data)
	{
		if (!g_selectedInput)
			return ListenerResult::Propagate;

		const bool isDown = isPressed(data);
		const bool isPress = data.action == KeyboardInputData::Action::Press;
		const bool isCtrl = hasModifier(data, KeyboardModifier::Control);
		const bool isShift = hasModifier(data, KeyboardModifier::Shift);

		if (!isDown)
			return ListenerResult::Stop;

		// on click, can be held
		if (!isCtrl && !isShift) {
			switch (data.key)
			{
				case KEY_Escape:
					if (!BI::geode::get<bool>("alternate-deselect"))
					{
						g_selectedInput->deselectInput();
						return ListenerResult::Stop;
					}
					break;

				case KEY_Backspace:
				case KEY_Delete:
					g_selectedInput->onDelete(false, data.key == KEY_Delete);
					return ListenerResult::Stop;

				default:
					break;
			}
		}

		switch (data.key)
		{
			case KEY_Up:
				g_selectedInput->onUpArrowKey(isShift);
				return ListenerResult::Stop;

			case KEY_Down:
				g_selectedInput->onDownArrowKey(isShift);
				return ListenerResult::Stop;

			case KEY_Right:
				g_selectedInput->onRightArrowKey(isCtrl, isShift);
				return ListenerResult::Stop;

			case KEY_Left:
				g_selectedInput->onLeftArrowKey(isCtrl, isShift);
				return ListenerResult::Stop;

			default:
				break;
		}

		// this is what onGLFWKeyCallback actually does to check for control lol
		if (isPress && isCtrl && !isShift) {
			switch (data.key)
			{
				case KEY_A:
					g_selectedInput->highlightFromToPos(0, -1);
					return ListenerResult::Stop;

				case KEY_Insert:
				case KEY_C:
					g_selectedInput->onCopy();
					return ListenerResult::Stop;

				case KEY_V:
					g_selectedInput->onPaste();
					return ListenerResult::Stop;

				case KEY_X:
					g_selectedInput->onCut();
					return ListenerResult::Stop;

				case KEY_Backspace:
				case KEY_Delete:
					g_selectedInput->onDelete(true, data.key == KEY_Delete);
					return ListenerResult::Stop;

				default:
					break;
			}
		}

		if (isPress)
		{
			if (!isCtrl)
			{
				switch (data.key)
				{
					case KEY_Home:
						g_selectedInput->onHomeKey(isShift);
						return ListenerResult::Stop;

					case KEY_End:
						g_selectedInput->onEndKey(isShift);
						return ListenerResult::Stop;

					default:
						break;
				}
			}

			if (isShift && !isCtrl) {
				switch (data.key)
				{
					case KEY_Insert:
						g_selectedInput->onPaste();
						return ListenerResult::Stop;

					default:
						break;
				}
			}
		}

		return isPress ? ListenerResult::Propagate : ListenerResult::Stop;
	}

	ListenerResult handleMouseInput(MouseInputData& data)
	{
		if (!g_selectedInput || data.button != MouseInputData::Button::Left)
			return ListenerResult::Propagate;

		if (data.action == MouseInputData::Action::Press)
		{
			CCSize winSize = CCDirector::sharedDirector()->getWinSize();
			CCPoint mousePos = BI::cocos::getMousePosition();

			// OpenGL's mouse origin is the bottom left
			// CCTouch's mouse origin is top left (because of course it is)
			CCTouch touch{};
			touch.setTouchInfo(0, mousePos.x, winSize.height - mousePos.y);

			g_selectedInput->useUpdateBlinkPos(true);

			// 🥰
			g_selectedInput->ccTouchBegan(&touch, nullptr);
		}
		else
			g_selectedInput->useUpdateBlinkPos(false);

		return ListenerResult::Propagate;
	}
}

$on_mod(Loaded)
{
	g_keyboardListener = KeyboardInputEvent().listen(handleKeyboardInput, Priority::VeryEarly);
	g_mouseListener = MouseInputEvent().listen(handleMouseInput, Priority::VeryEarly);
}
