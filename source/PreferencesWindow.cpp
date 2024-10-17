/*
 * Copyright 2024 Nexus6 <nexus6@disroot.org>
 * All Rights Reserved. Distributed under the terms of the MIT License.
 */


#include "PreferencesWindow.h"

#include <Catalog.h>
#include <CheckBox.h>
#include <GroupLayout.h>
#include <Locale.h>
#include <LayoutBuilder.h>
#include <MessageRunner.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PreferencesWindow"


PreferencesWindow::PreferencesWindow(BRect frame, BView* mainView)
	:
	BWindow(frame, B_TRANSLATE("Dockbert Preferences"), B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL,
		B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fLoadSettings(true),
	fPreferences(new Preferences(mainView)),
	fMessageRunner(nullptr)
{
	_InitControls();
	CenterOnScreen();
}


PreferencesWindow::~PreferencesWindow()
{
	delete fPreferences;
}

void
PreferencesWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMsgAlwaysOnTop: {
			fPreferences->SetProperty<bool>("AlwaysOnTop", fAlwaysOnTopControl->Value());
			break;
		}
		case kMsgAutoHide: {
			fPreferences->SetProperty<bool>("AutoHide", fAutoHideControl->Value());
			fHideEffectDelayControl->SetEnabled(fAutoHideControl->Value());
			break;
		}
		case kMsgDrawOuterFrame: {
			fPreferences->SetProperty<bool>("DrawOuterFrame", fDrawOuterFrameControl->Value());
			fOuterFrameColorControl->SetEnabled(fDrawOuterFrameControl->Value());
			break;
		}
		case kMsgHideEffectDelay: {
			fPreferences->SetProperty<int32>("HideEffectDelay", fHideEffectDelayControl->Value());
			break;
		}
		case kMsgBackgroundColor: {
			fPreferences->SetProperty<rgb_color>("BackColor", fBackgroundColorControl->ValueAsColor());
			break;
		}
		case kMsgOuterFrameColor: {
			fPreferences->SetProperty<rgb_color>("OuterFrameColor", fOuterFrameColorControl->ValueAsColor());
			break;
		}
		case kMsgAddTab: {
			int32 count = fPreferences->CountProperties("tabs");
			fPreferences->CreateProperty("tab", count);
			break;
		}
		case kMsgRemoveTab: {
			int32 count = fPreferences->CountProperties("tabs");
			fPreferences->CreateProperty("tab", count);
			break;
		}
		case kMsgSelectTab: {
			auto index = fTabListView->CurrentSelection();
			BString defString;
			auto name = fPreferences->GetTabProperty(index, "Name", defString);
			fTabNameControl->SetText(name.String());
			fTabNameControl->SetEnabled(true);
			rgb_color color;
			color = fPreferences->GetTabProperty(index, "FrameColor", color);
			fTabColorControl->SetValue(color);
			fTabColorControl->SetEnabled(true);
			break;
		}
		case kMsgTabColor: {
			auto index = fTabListView->CurrentSelection();
			rgb_color color = fTabColorControl->ValueAsColor();
			fPreferences->SetTabProperty(index, "FrameColor", color);
			break;
		}
		case kMsgChangeTabName: {
			if (fMessageRunner != nullptr)
				fMessageRunner->SetInterval(0);

			auto index = fTabListView->CurrentSelection();
			auto name = BString(fTabNameControl->Text());
			fPreferences->SetTabProperty(index, "Name", name);
			auto item = dynamic_cast<BStringItem*>(fTabListView->ItemAt(index));
			if (item != nullptr) {
				item->SetText(name);
				fTabListView->InvalidateItem(index);
			}
			message->PrintToStream();
			break;
		}
		case kMsgModifyName: {
			if (fMessageRunner == nullptr || fMessageRunner->SetInterval(kTimeout) != B_OK) {
				delete fMessageRunner;
				BMessage msg(kMsgChangeTabName);
				fMessageRunner = new BMessageRunner(BMessenger(this), &msg, kTimeout, 1);
				message->PrintToStream();
			}
			break;
		}
		default: {
			BWindow::MessageReceived(message);
			break;
		}
	}
}

void
PreferencesWindow::WindowActivated(bool active)
{
	if (fLoadSettings) {
		_LoadSettings();
		fLoadSettings = false;
	}

	if (!active && IsHidden())
		PostMessage(B_QUIT_REQUESTED);

	BWindow::WindowActivated(active);
}

bool
PreferencesWindow::QuitRequested()
{
	if (!IsHidden())
		Hide();
	return false;
}

void
PreferencesWindow::_InitControls()
{
	// General settings
	fBackgroundColorControl = new BColorControl(B_ORIGIN, B_CELLS_32x8, 8,
		"BackgroundColorControl", new BMessage(kMsgBackgroundColor));
	fOuterFrameColorControl = new BColorControl(B_ORIGIN, B_CELLS_32x8, 8,
		"OuterFrameColorControl", new BMessage(kMsgOuterFrameColor));
	fAlwaysOnTopControl = new BCheckBox(B_TRANSLATE("Always on top"), new BMessage(kMsgAlwaysOnTop));
	fAutoHideControl = new BCheckBox(B_TRANSLATE("Auto hide"), new BMessage(kMsgAutoHide));
	fDrawOuterFrameControl = new BCheckBox(B_TRANSLATE("Draw outer frame"), new BMessage(kMsgDrawOuterFrame));
	fHideEffectDelayControl = new BSpinner("HideEffectDelay", B_TRANSLATE("Hide effect delay"), new BMessage(kMsgHideEffectDelay));

	BBox *backgroundBox = new BBox("BackgroundBox");
	backgroundBox->AddChild(BLayoutBuilder::Group<>()
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(new BStringView("", ""))
			.Add(fBackgroundColorControl)
			.End()
		.View());
	backgroundBox->SetLabel(B_TRANSLATE("Background"));

	BBox *frameBox = new BBox("FrameBox");
	frameBox->AddChild(BLayoutBuilder::Group<>()
		.AddGroup(B_VERTICAL, B_USE_WINDOW_SPACING)
			.Add(fDrawOuterFrameControl)
			.Add(fOuterFrameColorControl)
			.End()
		.View());
	frameBox->SetLabel(B_TRANSLATE("Frame"));

	fGeneralPrefsView = BLayoutBuilder::Group<>()
		.AddGroup(B_VERTICAL, 5)
			.SetInsets(B_USE_WINDOW_SPACING)
			.Add(fAlwaysOnTopControl)
			.Add(fAutoHideControl)
			.Add(fHideEffectDelayControl)
			.AddStrut(10)
			.Add(backgroundBox)
			.AddStrut(10)
			.Add(frameBox)
		.End()
		.View();

	// Tabs settings
	fTabListView = new BListView("TabListView");
	fTabListView->SetSelectionMessage(new BMessage(kMsgSelectTab));
	fAddTabButton = new BButton(B_TRANSLATE("Add"), new BMessage(kMsgAddTab));
	fRemoveTabButton = new BButton(B_TRANSLATE("Remove"), new BMessage(kMsgAddTab));
	fRemoveTabButton->SetEnabled(false);

	BStringView *tabBackgroundColorLabel = new BStringView("tabBackgroundColorLabel",
		B_TRANSLATE("Background color"));
	fTabColorControl = new BColorControl(B_ORIGIN, B_CELLS_32x8, 8,
		"TabColorControl", new BMessage(kMsgTabColor));
	fTabColorControl->SetEnabled(false);
	fTabNameControl = new BTextControl(B_TRANSLATE("Name"), "", new BMessage(kMsgModifyName));
	fTabNameControl->SetEnabled(false);
	fTabNameControl->SetModificationMessage(new BMessage(kMsgModifyName));

	fTabPrefsView = BLayoutBuilder::Group<>()
		.AddGroup(B_VERTICAL, 5)
			.SetInsets(B_USE_WINDOW_SPACING)
			.AddGroup(B_VERTICAL)
				.Add(fTabListView)
				.End()
			.AddGroup(B_HORIZONTAL)
				.SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_VERTICAL_UNSET))
				.Add(fAddTabButton)
				.Add(fRemoveTabButton)
				.End()
			.AddStrut(5)
			.Add(fTabNameControl)
			.Add(tabBackgroundColorLabel)
			.Add(fTabColorControl)
			.AddGlue()
			.End()
		.View();

	fTabView = new BTabView("prefs");
	fTabView->AddTab(fGeneralPrefsView);
	fTabView->TabAt(0)->SetLabel(B_TRANSLATE("General settings"));
	fTabView->AddTab(fTabPrefsView);
	fTabView->TabAt(1)->SetLabel(B_TRANSLATE("Tabs settings"));

	BLayoutBuilder::Group<>(this)
		.Add(fTabView)
		.End();
}

void
PreferencesWindow::_LoadSettings()
{
	// general
	fAlwaysOnTopControl->SetValue(fPreferences->GetProperty("AlwaysOnTop", false));
	fAutoHideControl->SetValue(fPreferences->GetProperty("AutoHide", false));
	fHideEffectDelayControl->SetEnabled(fAutoHideControl->Value());
	fDrawOuterFrameControl->SetValue(fPreferences->GetProperty("DrawOuterFrame", false));
	fHideEffectDelayControl->SetValue(fPreferences->GetProperty<int32>("HideEffectDelay", 600000));
	fBackgroundColorControl->SetValue(fPreferences->GetProperty<rgb_color>("BackColor", (rgb_color){229,235,231,255}));
	fOuterFrameColorControl->SetValue(fPreferences->GetProperty<rgb_color>("OuterFrameColor", (rgb_color){96,96,96,255}));
	fOuterFrameColorControl->SetEnabled(fDrawOuterFrameControl->Value());

	// tabs
	int32 countTabs = fPreferences->CountProperties("tabs");
	for (int i = 0; i < countTabs; i++) {
		BString name;
		name = fPreferences->GetTabProperty(i, "Name", name);
		auto index = fPreferences->GetTabProperty(i, "Index", -1);

		if (!name.IsEmpty() && index != -1) {
			BStringItem *item = new BStringItem(name);
			fTabListView->AddItem(item, index);
		}
	}

}

