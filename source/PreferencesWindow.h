/*
 * Copyright 2024 Nexus6 <nexus6@disroot.org>
 * All Rights Reserved. Distributed under the terms of the MIT License.
 */

#pragma once

#include "PreferencesWindow.h"
#include "Preferences.h"

#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <ColorControl.h>
#include <ListView.h>
#include <RadioButton.h>
#include <Spinner.h>
#include <StringView.h>
#include <TabView.h>
#include <TextControl.h>
#include <Window.h>

const int kTimeout = 1000000;

const uint32 kMsgAlwaysOnTop = 'alot';
const uint32 kMsgAutoHide = 'auhi';
const uint32 kMsgDrawOuterFrame = 'drof';
const uint32 kMsgHideEffectDelay = 'stch';
const uint32 kMsgBackgroundColor = 'bgco';
const uint32 kMsgTabColor = 'taco';
const uint32 kMsgChangeTabName = 'tana';
const uint32 kMsgModifyName = 'mona';
const uint32 kMsgAddTab = 'adta';
const uint32 kMsgRemoveTab = 'reta';
const uint32 kMsgSelectTab = 'seta';
const uint32 kMsgOuterFrameColor = 'ofco';


class PreferencesWindow : public BWindow
{
public:
						PreferencesWindow(BRect frame, BView* mainView);
						~PreferencesWindow();

	virtual void		MessageReceived(BMessage* message);
	virtual void		WindowActivated(bool active);
	virtual bool		QuitRequested();

private:
	bool				fLoadSettings;
	void				_LoadSettings();
	Preferences*		fPreferences;

	void				_InitControls();

	BColorControl*		fBackgroundColorControl;
	BColorControl*		fOuterFrameColorControl;
	BCheckBox*			fAlwaysOnTopControl;
	BCheckBox*			fAutoHideControl;
	BCheckBox*			fDrawOuterFrameControl;
	BSpinner*			fHideEffectDelayControl;

	BListView*			fTabListView;
	BButton*			fAddTabButton;
	BButton*			fRemoveTabButton;
	BColorControl*		fTabColorControl;
	BTextControl*		fTabNameControl;

	BView*				fGeneralPrefsView;
	BView*				fTabPrefsView;
	BTabView*			fTabView;

	BMessageRunner*		fMessageRunner;
};
