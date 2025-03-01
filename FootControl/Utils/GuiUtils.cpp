﻿#include "GuiUtils.h"
#include "Logger.h"
#include "../GUI/Sections.h"
#include "../GUI/Translations.h"
#include "../Themes/Themes.h"
void HelpMarker(const char* description) {
	ImGui::TextDisabled("(?)");

	if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(description);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void AddUnderLine(ImColor color) {
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	min.y = max.y;

	ImGui::GetWindowDrawList()->AddLine(min, max, color, 1.0f);
}

void TextURL(const char* name_, const char* URL_, bool SameLineBefore_, bool SameLineAfter_) {
	if (SameLineBefore_)
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.17f, 0.63f, 0.45f, 1.00f });
	ImGui::Text(name_);
	ImGui::PopStyleColor();

	if (ImGui::IsItemHovered()) {
		if (ImGui::IsMouseClicked(0))
			ShellExecuteA(0, 0, URL_, 0, 0, SW_SHOW);
		AddUnderLine(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
		ImGui::SetTooltip("Open in browser\n%s", URL_);
	}
	else
		AddUnderLine(ImGui::GetStyle().Colors[ImGuiCol_Button]);

	if (SameLineAfter_)
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
}

struct GroupPanelHeaderBounds
{
	ImRect left;
	ImRect right;
	bool collapsed;
};


static ImVector<GroupPanelHeaderBounds> _groupPanelStack;

static bool GroupPanelIsOpen(ImGuiID id)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiStorage* storage = window->DC.StateStorage;

	return storage->GetInt(id, 1) != 0;
}

static void GroupPanelSetOpen(ImGuiID id, bool open)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiStorage* storage = window->DC.StateStorage;

	storage->SetInt(id, open ? 1 : 0);
}


bool BeginGroupPanel(const char* label, bool node, const ImVec2& size)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	const ImGuiID id = window->GetID(label);
	ImGui::PushID(id);

	auto groupPanelPos = window->DC.CursorPos;
	auto itemSpacing = ImGui::GetStyle().ItemSpacing;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

	ImGui::BeginGroup(); // Outer group

	ImVec2 effectiveSize = size;
	if (size.x < 0.0f)
		effectiveSize.x = ImGui::GetContentRegionAvail().x;
	else
		effectiveSize.x = size.x;

	ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f)); // Adjusting group x size

	auto frameHeight = ImGui::GetFrameHeight();
	ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f)); ImGui::SameLine(0.0f, 0.0f); // Inner group spacing
	ImGui::BeginGroup(); // Inner group

	ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f)); ImGui::SameLine(0.0f, 0.0f); // Name text spacing
	ImGui::TextUnformatted(label);

	ImRect leftRect = { ImGui::GetItemRectMin(), ImGui::GetItemRectMax() };
	ImVec2 rightMax = ImVec2(groupPanelPos.x + effectiveSize.x - frameHeight, leftRect.Max.y);
	ImRect rightRect = { { rightMax.x, leftRect.Min.x }, rightMax };
	ImGui::SameLine(0.0f, 0.0f);

	ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));

	if (node)
	{
		leftRect.Min.x = groupPanelPos.x;

		const ImVec2 text_size = ImGui::CalcTextSize(label);
		bool isOpen = GroupPanelIsOpen(id);

		bool hovered;
		bool toggled = ImGui::ButtonBehavior(leftRect, id, &hovered, nullptr, ImGuiButtonFlags_PressedOnClick);
		if (toggled)
		{
			isOpen = !isOpen;
			GroupPanelSetOpen(id, isOpen);
		}

		const ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
		ImGui::RenderArrow(window->DrawList, { groupPanelPos.x, groupPanelPos.y + text_size.y * 0.15f }, text_col,
			isOpen ? ImGuiDir_Down : ImGuiDir_Right, 0.7f);

		if (!isOpen)
		{
			ImGui::PopStyleVar(2);
			ImGui::EndGroup();
			ImGui::EndGroup();
			ImGui::PopID();

			_groupPanelStack.push_back({ leftRect, rightRect, true });
			return false;
		}
	}

	ImGui::PopStyleVar(2);

	ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->Size.x -= frameHeight;

	auto itemWidth = ImGui::CalcItemWidth();
	ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

	_groupPanelStack.push_back({ leftRect, rightRect, false });
	return true;
}
const char* languages[] = { "English", "简体中文" };

void ConfigComboLanguage(ConfigField<int>& f_Language) {
	int currentLanguage = f_Language.getValue();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[fontindex_menu]);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 9.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 8.0f));

	if (ImGui::Combo(_("LANGUAGE_SELECT"), &currentLanguage, languages, IM_ARRAYSIZE(languages))) {
		f_Language.setValue(currentLanguage);
		config::setValue(f_Language, currentLanguage);

		ModuleOrder = {
	_("About"),
	_("Player"),
	_("World"),

	_("Visuals"),
			_("ESP"),
	_("Settings"),
	"Debug",
		};
	}

	ImGui::PopFont();
	ImGui::PopStyleVar(2);
}

void EndGroupPanel() {
	IM_ASSERT(_groupPanelStack.Size > 0); // Mismatched BeginGroupPanel()/EndGroupPanel() calls

	auto& info = _groupPanelStack.back();

	_groupPanelStack.pop_back();

	if (info.collapsed)
		return;

	ImGui::PopItemWidth();

	auto itemSpacing = ImGui::GetStyle().ItemSpacing;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

	ImGui::EndGroup(); // Inner group

	auto frameHeight = ImGui::GetFrameHeight();

	ImGui::SameLine(0.0f, 0.0f);
	ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
	ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

	ImGui::EndGroup(); // Outer group

	// Outer group rect
	auto itemMin = ImGui::GetItemRectMin();
	auto itemMax = ImGui::GetItemRectMax();

	ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
	ImRect frameRect = ImRect(itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f));

	auto& leftRect = info.left;

	leftRect.Min.x -= itemSpacing.x;
	leftRect.Max.x += itemSpacing.x;

	bool hasRightPart = info.right.Min.x != info.right.Max.x;
	auto& rightRect = info.right;

	if (hasRightPart) {
		rightRect.Min.x -= itemSpacing.x;
		rightRect.Max.x += itemSpacing.x;
	}

	// Drawing rectangle
	for (int i = 0; i < (hasRightPart ? 5 : 3); ++i) {
		switch (i) {
		case 0: // left half-plane
			ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(leftRect.Min.x, FLT_MAX), true);
			break;
		case 1: // right half-plane
			ImGui::PushClipRect(ImVec2(leftRect.Max.x, -FLT_MAX), ImVec2(hasRightPart ? rightRect.Min.x : FLT_MAX, FLT_MAX), true);
			break;
		case 2: // bottom
			ImGui::PushClipRect(ImVec2(leftRect.Min.x, leftRect.Max.y), ImVec2(leftRect.Max.x, FLT_MAX), true);
			break;
		case 3: // bottom select
			ImGui::PushClipRect(ImVec2(rightRect.Min.x, rightRect.Max.y), ImVec2(rightRect.Max.x, FLT_MAX), true);
			break;
		case 4: // right hand-plane
			ImGui::PushClipRect(ImVec2(rightRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true);
			break;
		}

		ImGui::GetWindowDrawList()->AddRect(frameRect.Min, frameRect.Max,
			ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)), halfFrame.x);
		ImGui::PopClipRect();
	}

	ImGui::PopStyleVar(2);

	// Restore content region
	ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->Size.x += frameHeight;

	// Add vertical spacing
	ImGui::Dummy(ImVec2(0.0f, 0.0f));
	ImGui::PopID();
}
