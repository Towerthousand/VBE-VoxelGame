#include "DebugDrawer.hpp"
#include "SceneMain/DeferredContainer.hpp"

DebugDrawer* DebugDrawer::instance = nullptr;

DebugDrawer::DebugDrawer() : renderer(nullptr), tex(nullptr) {
	VBE_ASSERT(instance == nullptr, "Created two debug drawers");
	instance = this;

	tex = Texture2D::createFromFile("data/debugFont.png");
	tex->setFilter(GL_NEAREST, GL_NEAREST);

	setUpdatePriority(100);
	setDrawPriority(100);

	renderer = (DeferredContainer*)getGame()->getObjectByName("deferred");
	model.program = Programs.get("debugDraw");
	std::vector<Vertex::Element> elems = {
		Vertex::Element(Vertex::Attribute::Position, Vertex::Element::Float, 2),
		Vertex::Element(Vertex::Attribute::TexCoord, Vertex::Element::Float, 2),
		Vertex::Element(Vertex::Attribute::Color, Vertex::Element::UnsignedByte, 4, Vertex::Element::ConvertToFloatNormalized)
	};

	Vertex::Format format(elems);
	model.mesh = Mesh::loadEmpty(format, Mesh::STREAM, false);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = Environment::getScreen()->getWidth();
	io.DisplaySize.y = Environment::getScreen()->getHeight();
	io.IniFilename = "imgui.ini";
	io.RenderDrawListsFn = &DebugDrawer::renderHandle;
	io.SetClipboardTextFn = &DebugDrawer::setClipHandle;
	io.GetClipboardTextFn = &DebugDrawer::getClipHandle;
}

DebugDrawer::~DebugDrawer() {
	delete model.mesh;
	ImGui::Shutdown();
	instance = nullptr;
}

void DebugDrawer::renderHandle(ImDrawList** const cmd_lists, int cmd_lists_count) {
	instance->render(cmd_lists, cmd_lists_count);
}

const char* DebugDrawer::getClipHandle() {
	return instance->getClip();
}

void DebugDrawer::setClipHandle(const char* text, const char* text_end) {
	instance->setClipHandle(text, text_end);
}

void DebugDrawer::render(ImDrawList** const cmd_lists, int cmd_lists_count) const {
	//if(renderer->getMode() != DeferredContainer::Forward) return;
	if (cmd_lists_count == 0)
		return;

	GL_ASSERT(glDisable(GL_CULL_FACE));
	GL_ASSERT(glDepthFunc(GL_ALWAYS));
	GL_ASSERT(glEnable(GL_SCISSOR_TEST));

	// Setup orthographic projection matrix
	const float width = ImGui::GetIO().DisplaySize.x;
	const float height = ImGui::GetIO().DisplaySize.y;
	mat4f perspective = glm::ortho(0.0f, width, height, 0.0f, -1.0f, +1.0f);
	model.program->uniform("MVP")->set(perspective);

	// Set texture for font
	model.program->uniform("fontTex")->set(tex);

	// Render command lists
	for (int n = 0; n < cmd_lists_count; n++) {
		const ImDrawList* cmd_list = cmd_lists[n];
		model.mesh->setVertexData((const unsigned char*)cmd_list->vtx_buffer.begin(), cmd_list->vtx_buffer.size());

		int vtx_offset = 0;
		const ImDrawCmd* pcmd_end = cmd_list->commands.end();
		for (const ImDrawCmd* pcmd = cmd_list->commands.begin(); pcmd != pcmd_end; pcmd++) {
			GL_ASSERT(glScissor((int)pcmd->clip_rect.x, (int)(height - pcmd->clip_rect.w), (int)(pcmd->clip_rect.z - pcmd->clip_rect.x), (int)(pcmd->clip_rect.w - pcmd->clip_rect.y)));
			model.draw(vtx_offset, pcmd->vtx_count);
			vtx_offset += pcmd->vtx_count;
		}
	}
	GL_ASSERT(glDisable(GL_SCISSOR_TEST));
	GL_ASSERT(glDepthFunc(GL_LEQUAL));
	GL_ASSERT(glEnable(GL_CULL_FACE));
}

const char* DebugDrawer::getClip() const {
	return SDL_GetClipboardText();
}

void DebugDrawer::setClip(const char* text, const char* text_end) const {
	if (!text_end)
		text_end = text + strlen(text);

	if (*text_end == 0) {
		// Already got a zero-terminator at 'text_end', we don't need to add one
		SDL_SetClipboardText(text);
	}
	else {
		// Add a zero-terminator because sdl function doesn't take a size
		char* buf = (char*)malloc(text_end - text + 1);
		memcpy(buf, text, text_end-text);
		buf[text_end-text] = '\0';
		SDL_SetClipboardText(buf);
		free(buf);
	}
}

void DebugDrawer::update(float deltaTime) {
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime == 0.0f ? 0.00001f : deltaTime;
	io.MouseWheel = Environment::getMouse()->getMouseWheelMovement().y != 0 ? (Environment::getMouse()->getMouseWheelMovement().y > 0 ? 1 : -1) : 0;
	io.KeyCtrl = Environment::getKeyboard()->isKeyHeld(Keyboard::LControl);
	io.KeyShift = Environment::getKeyboard()->isKeyHeld(Keyboard::LShift);
	io.MouseDown[0] = Environment::getMouse()->isButtonPressed(Mouse::Left);
	ImGui::NewFrame();
}

void DebugDrawer::draw() const {
	ImGui::SetNewWindowDefaultPos(ImVec2(100, 20));        // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
	ImGui::ShowTestWindow();
	ImGui::Render();
}
