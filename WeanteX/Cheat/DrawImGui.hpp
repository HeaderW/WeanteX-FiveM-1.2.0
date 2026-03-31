typedef struct
{
	DWORD r;
	DWORD g;
	DWORD b;
	DWORD a;
} RGBA;

ImColor ConvertToImColor(float rgb[3]) {
	int r = static_cast<int>(rgb[0] * 255.0f);
	int g = static_cast<int>(rgb[1] * 255.0f);
	int b = static_cast<int>(rgb[2] * 255.0f);
	return ImColor(r, g, b);
}

ImColor DarkenColor(const ImColor& color, float factor) {
	float r = color.Value.x;
	float g = color.Value.y;
	float b = color.Value.z;
	r *= factor;
	g *= factor;
	b *= factor;
	r = (r < 0) ? 0 : (r > 1) ? 1 : r;
	g = (g < 0) ? 0 : (g > 1) ? 1 : g;
	b = (b < 0) ? 0 : (b > 1) ? 1 : b;
	return ImColor(r, g, b, color.Value.w);
}

void DrawLine(ImDrawList* drawList, int x1, int y1, int x2, int y2, RGBA* color, int thickness) {
	drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(ImVec4(color->r / 255.0f, color->g / 255.0f, color->b / 255.0f, color->a / 255.0f)), thickness);
}

void DrawLine(ImDrawList* drawList, const ImVec2& x, const ImVec2& y, ImU32 color, float width) {
	drawList->AddLine(x, y, color, width);
}

void DrawLineOutline(ImVec2 a, ImVec2 b, ImColor color, float width) {
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	ImVec4 mainColor = color.Value;
	ImVec4 outlineColorDark = ImVec4(0, 0, 0, 2.8f);
	ImVec4 outlineColorLight = ImVec4(0.1f, 0.1f, 0.1f, 0.4f);
	const int numOutlines = 3;
	for (int i = numOutlines; i > 0; i--) {
		float currentWidth = width + (i * 0.8f);
		float t = i / (float)numOutlines;
		ImVec4 currentColor;
		currentColor.x = outlineColorDark.x + (outlineColorLight.x - outlineColorDark.x) * t;
		currentColor.y = outlineColorDark.y + (outlineColorLight.y - outlineColorDark.y) * t;
		currentColor.z = outlineColorDark.z + (outlineColorLight.z - outlineColorDark.z) * t;
		currentColor.w = outlineColorDark.w + (outlineColorLight.w - outlineColorDark.w) * t;

		drawList->AddLine(a, b, ImGui::ColorConvertFloat4ToU32(currentColor), currentWidth);
	}
	drawList->AddLine(a, b, color, width);
	ImVec4 glowColor = mainColor;
	glowColor.w = 20.3f;
	drawList->AddLine(a, b, ImGui::ColorConvertFloat4ToU32(glowColor), width * 0.8f);
}

void DrawRect(ImDrawList* drawList, int x, int y, int w, int h, RGBA* color, int thickness) {
	drawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color->r / 255.0f, color->g / 255.0f, color->b / 255.0f, color->a / 255.0f)), 0, 0, thickness);
}

void DrawFilledRect(ImDrawList* drawList, int x, int y, int w, int h, RGBA* color) {
	drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color->r / 255.0f, color->g / 255.0f, color->b / 255.0f, color->a / 255.0f)), 0, 0);
}

void DrawCircleFilled(ImDrawList* drawList, int x, int y, int radius, RGBA* color) {
	drawList->AddCircleFilled(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->r / 255.0f, color->g / 255.0f, color->b / 255.0f, color->a / 255.0f)));
}

void DrawCircle(ImDrawList* drawList, int x, int y, int radius, RGBA* color, int segments) {
	drawList->AddCircle(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->r / 255.0f, color->g / 255.0f, color->b / 255.0f, color->a / 255.0f)), segments);
}

void DrawOutlinedText(ImDrawList* drawList, ImFont* pFont, const std::string& text, const ImVec2& pos, float size, ImU32 color, bool center) {
	std::stringstream stream(text);
	std::string line;
	ImU32 outlineColor = ImGui::GetColorU32(ImVec4(0, 0, 0, 1));
	float y = 0.0f;
	int index = 0;
	while (std::getline(stream, line)) {
		ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, line.c_str());
		ImVec2 textPos;
		if (center) {
			textPos = ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * index);
		}
		else {
			textPos = ImVec2(pos.x, pos.y + textSize.y * index);
		}


		drawList->AddText(pFont, size, textPos + ImVec2(-1, 0), outlineColor, line.c_str());
		drawList->AddText(pFont, size, textPos + ImVec2(0, 1), outlineColor, line.c_str());
		drawList->AddText(pFont, size, textPos + ImVec2(0, -1), outlineColor, line.c_str());
		drawList->AddText(pFont, size, textPos + ImVec2(1, 0), outlineColor, line.c_str());


		drawList->AddText(pFont, size, textPos, color, line.c_str());
		y = pos.y + textSize.y * (index + 1);
		index++;
	}
}

void DrawArrowTriangleOutlined(ImDrawList* drawList, ImVec2 start, ImVec2 end, ImColor color, float thickness) {
	ImVec2 direction = ImVec2(end.x - start.x, end.y - start.y);
	float length = sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length == 0) return;

	direction.x /= length;
	direction.y /= length;

	float arrowSize = 8.0f;
	ImVec2 perpendicular = ImVec2(-direction.y, direction.x);

	ImVec2 arrowTip = end;
	ImVec2 arrowLeft = ImVec2(end.x - direction.x * arrowSize + perpendicular.x * arrowSize * 0.5f,
		end.y - direction.y * arrowSize + perpendicular.y * arrowSize * 0.5f);
	ImVec2 arrowRight = ImVec2(end.x - direction.x * arrowSize - perpendicular.x * arrowSize * 0.5f,
		end.y - direction.y * arrowSize - perpendicular.y * arrowSize * 0.5f);


	drawList->AddTriangle(arrowTip, arrowLeft, arrowRight, ImColor(0, 0, 0, 255), thickness + 1.0f);

	drawList->AddTriangleFilled(arrowTip, arrowLeft, arrowRight, color);
}