#include "Profiler.hpp"
#include "SceneMain/DeferredContainer.hpp"

Profiler* Profiler::instance = nullptr;

Profiler::Profiler() : tex(nullptr), show_test(false), show_other(false), floatSlider(0.0f) {
	VBE_ASSERT(instance == nullptr, "Created two debug drawers");
	instance = this;

	tex = Texture2D::createFromFile("data/debugFont.png");
	tex->setFilter(GL_NEAREST, GL_NEAREST);

	setUpdatePriority(100);
	setDrawPriority(100);

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
	io.RenderDrawListsFn = &Profiler::renderHandle;
	io.SetClipboardTextFn = &Profiler::setClipHandle;
	io.GetClipboardTextFn = &Profiler::getClipHandle;
}

Profiler::~Profiler() {
	delete model.mesh;
	delete tex;
	ImGui::Shutdown();
	instance = nullptr;
}

void Profiler::renderHandle(ImDrawList** const cmd_lists, int cmd_lists_count) {
	instance->render(cmd_lists, cmd_lists_count);
}

const char* Profiler::getClipHandle() {
	return instance->getClip();
}

void Profiler::setClipHandle(const char* text, const char* text_end) {
	instance->setClipHandle(text, text_end);
}

void Profiler::render(ImDrawList** const cmd_lists, int cmd_lists_count) const {
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

const char* Profiler::getClip() const {
	return SDL_GetClipboardText();
}

void Profiler::setClip(const char* text, const char* text_end) const {
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

void Profiler::update(float deltaTime) {
	//INPUT
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime == 0.0f ? 0.00001f : deltaTime;
	io.MouseWheel = Environment::getMouse()->getMouseWheelMovement().y != 0 ? (Environment::getMouse()->getMouseWheelMovement().y > 0 ? 1 : -1) : 0;
	io.KeyCtrl = Environment::getKeyboard()->isKeyHeld(Keyboard::LControl);
	io.KeyShift = Environment::getKeyboard()->isKeyHeld(Keyboard::LShift);
	io.MouseDown[0] = Environment::getMouse()->isButtonHeld(Mouse::Left);
	io.MousePos = vec2f(Environment::getMouse()->getMousePos());
	ImGui::NewFrame();
	//UI
	ImGui::Text("Hello, world!");
	ImGui::SliderFloat("float", &floatSlider, 0.0f, 1.0f);
	show_test ^= ImGui::Button("Test Window");
	show_other ^= ImGui::Button("Another Window");
	ImGui::Text("60.0f FPS");
	if (show_test) {
		ImGui::SetNewWindowDefaultPos(ImVec2(50, 20));        // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
		ImGui::ShowTestWindow(&show_test);
	}
	if (show_other) {
		ImGui::Begin("Another Window", &show_other, ImVec2(200,100));
		ImGui::Text("Hello");
		ImGui::End();
	}
}

void Profiler::draw() const {
	ImGui::Render();
}
