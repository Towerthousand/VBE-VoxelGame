#include "Profiler.hpp"
#include "SceneMain/DeferredContainer.hpp"

int Profiler::intVars[Profiler::INT_VAR_COUNT];
float Profiler::timeVars[Profiler::TIME_VAR_COUNT];
vec3f Profiler::vec3fVars[Profiler::VEC3F_VAR_COUNT];
Profiler* Profiler::instance = nullptr;

Profiler::Profiler() :
	timeAvgOffset(-1),
	updateTimeStart(0.0f), updateTimeEnd(0.0f),
	drawTimeStart(0.0f), drawTimeEnd(0.0f),
	swapTimeStart(0.0f), swapTimeEnd(0.0f),
	frameCount(0), timePassed(0.0f),
	FPS(0), showProfiler(false), sampleRate(1.0f),
	tex(nullptr) {
	//setup singleton
	VBE_ASSERT(instance == nullptr, "Created two debug drawers");
	instance = this;

	//reset debug vars
	memset(&intVars, 0, sizeof(intVars));
	memset(&timeVars, 0, sizeof(timeVars));
	memset(&vec3fVars, 0, sizeof(vec3fVars));

	//font tenxture
	tex = Texture2D::createFromFile("data/debugFont.png");
	tex->setFilter(GL_NEAREST, GL_NEAREST);

	setUpdatePriority(100);
	setDrawPriority(100);

	//setup UI model
	model.program = Programs.get("debugDraw");
	std::vector<Vertex::Element> elems = {
		Vertex::Element(Vertex::Attribute::Position, Vertex::Element::Float, 2),
		Vertex::Element(Vertex::Attribute::TexCoord, Vertex::Element::Float, 2),
		Vertex::Element(Vertex::Attribute::Color, Vertex::Element::UnsignedByte, 4, Vertex::Element::ConvertToFloatNormalized)
	};

	Vertex::Format format(elems);
	model.mesh = Mesh::loadEmpty(format, Mesh::STREAM, false);

	//setup ui
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = Environment::getScreen()->getWidth();
	io.DisplaySize.y = Environment::getScreen()->getHeight();
	io.IniFilename = "imgui.ini";
	io.RenderDrawListsFn = &Profiler::renderHandle;
	io.SetClipboardTextFn = &Profiler::setClipHandle;
	io.GetClipboardTextFn = &Profiler::getClipHandle;
	io.IniSavingRate = -1.0f; //disable ini

	//add watcher
	Watcher* w = new Watcher();
	w->addTo(this);
}

Profiler::~Profiler() {
	delete model.mesh;
	delete tex;
	ImGui::Shutdown();
	instance = nullptr;
}

bool Profiler::shown() {
	return (instance != nullptr && instance->showProfiler);
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
	char* c = SDL_GetClipboardText();
	clip = std::string(c);
	SDL_free(c);
	return clip.c_str();
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
	if(Environment::getKeyboard()->isKeyPressed(Keyboard::F1)) showProfiler = !showProfiler;
	updateTimeEnd = Environment::getClock();
	timeVars[DrawTime] = drawTimeEnd-drawTimeStart;
	timeVars[UpdateTime] = updateTimeEnd-updateTimeStart;
	timeVars[FrameTime] = updateTimeEnd-drawTimeStart;
	timeVars[SwapTime] = swapTimeEnd-swapTimeStart;
	timeVars[ShadowWorldTime] = timeVars[ShadowChunkBFSTime]+timeVars[ShadowChunkRebuildTime]+timeVars[ShadowChunkDrawTime];
	timeVars[PlayerWorldTime] = timeVars[PlayerChunkBFSTime]+timeVars[PlayerChunkRebuildTime]+timeVars[PlayerChunkDrawTime];
	//INPUT
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime == 0.0f ? 0.00001f : deltaTime;
	io.MouseWheel = Environment::getMouse()->getMouseWheelMovement().y != 0 ? (Environment::getMouse()->getMouseWheelMovement().y > 0 ? 1 : -1) : 0;
	io.KeyCtrl = Environment::getKeyboard()->isKeyHeld(Keyboard::LControl);
	io.KeyShift = Environment::getKeyboard()->isKeyHeld(Keyboard::LShift);
	io.MouseDown[0] = Environment::getMouse()->isButtonHeld(Mouse::Left);
	io.MousePos = vec2f(Environment::getMouse()->getMousePos());
	ImGui::NewFrame();
	//UPDATE INTERNAL DEBUG VARS
	for(int i = 0; i < TIME_VAR_COUNT; ++i)	timeAccumVars[i] += timeVars[i];
	timePassed += deltaTime;
	if(timePassed >= sampleRate) {
		timeAvgOffset = (timeAvgOffset + 1) % 100;
		timePassed -= sampleRate;
		FPS = float(frameCount)/sampleRate;
		for(int i = 0; i < TIME_VAR_COUNT; ++i)	timeAvgVars[i][timeAvgOffset] = (timeAccumVars[i]/float(frameCount))*1000.0f;
		memset(&timeAccumVars, 0, sizeof(timeAccumVars));
		frameCount = 0;
	}
	frameCount++;
	//UI
	if(showProfiler) {
		std::string tag;
		ImGui::SetNewWindowDefaultPos(ImVec2(50, 50));
		ImGui::Begin("VoxelGame Profiler", nullptr, ImVec2(450,700), -1.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);
		ImGui::Text("With V-Sync enabled, frame time will\nnot go below 16ms");
		ImGui::Text("FPS: %i", FPS);
		ImGui::Separator();
		if(ImGui::CollapsingHeader("General frame times", nullptr, true, true)) {
			tag = std::string("Frame Time (curr: ") + Utils::toString(timeAvgVars[FrameTime][timeAvgOffset], 4, 2, true) + " ms)";
			ImGui::PlotLines("50ms\n\n\n\n0 ms", timeAvgVars[FrameTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 50.0f, vec2f(350,60));
			if (ImGui::IsHovered()) ImGui::SetTooltip("Overall frame time for the update-draw loop");
			tag = std::string("Update Time (curr: ") + Utils::toString(timeAvgVars[UpdateTime][timeAvgOffset], 4, 2, true) + " ms)";
			ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[UpdateTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
			if (ImGui::IsHovered()) {
				ImGui::SetTooltip("");
				ImGui::BeginTooltip();
				ImGui::Text("Update Time Breakdown");
				tag = std::string("World Update Time (curr: ") + Utils::toString(timeAvgVars[WorldUpdateTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[WorldUpdateTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				ImGui::EndTooltip();
			}
			tag = std::string("Draw Time (curr: ") + Utils::toString(timeAvgVars[DrawTime][timeAvgOffset], 4, 2, true) + " ms)";
			ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[DrawTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
			if (ImGui::IsHovered()) {
				ImGui::SetTooltip("");
				ImGui::BeginTooltip();
				ImGui::Text("Draw Time Breakdown");
				tag = std::string("Deferred Pass Time (curr: ") + Utils::toString(timeAvgVars[DeferredPassTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[DeferredPassTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				tag = std::string("Shadow Build Pass Time (curr: ") + Utils::toString(timeAvgVars[ShadowBuildPassTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[ShadowBuildPassTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				tag = std::string("Light Pass Time (curr: ") + Utils::toString(timeAvgVars[LightPassTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[LightPassTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				tag = std::string("Ambient+Visibility Pass Time (curr: ") + Utils::toString(timeAvgVars[AmbinentShadowPassTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[AmbinentShadowPassTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				tag = std::string("Forward Pass Time (curr: ") + Utils::toString(timeAvgVars[ForwardPassTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[ForwardPassTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				tag = std::string("Blur Pass Time (curr: ") + Utils::toString(timeAvgVars[BlurPassTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[BlurPassTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				tag = std::string("UI Pass Time (curr: ") + Utils::toString(timeAvgVars[UIDrawTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[UIDrawTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				ImGui::EndTooltip();
			}
			tag = std::string("Swap Time (curr: ") + Utils::toString(timeAvgVars[SwapTime][timeAvgOffset], 4, 2, true) + " ms)";
			ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[SwapTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
			if (ImGui::IsHovered()) ImGui::SetTooltip("Swap time is accountable for left-over time in V-Sync'd loop\nand for CPU-GPU sync (finishing all the draw commands during the frame).\nIf it rises during low FPS, the application is GPU Bottlenecked.");
		}
		if(ImGui::CollapsingHeader("World Draw Time", nullptr, true, true)) {
			tag = std::string("Player Cam World Draw Time (curr: ") + Utils::toString(timeAvgVars[PlayerWorldTime][timeAvgOffset], 4, 2, true) + " ms)";
			ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[PlayerWorldTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
			if (ImGui::IsHovered()) {
				ImGui::SetTooltip("");
				ImGui::BeginTooltip();
				ImGui::Text("Player Cam World Draw Time Breakdown");
				tag = std::string("Player Rebuild Time (curr: ") + Utils::toString(timeAvgVars[PlayerChunkRebuildTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[PlayerChunkRebuildTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				tag = std::string("Player BFS Time (curr: ") + Utils::toString(timeAvgVars[PlayerChunkBFSTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[PlayerChunkBFSTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				tag = std::string("Player Draw Time (curr: ") + Utils::toString(timeAvgVars[PlayerChunkDrawTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[PlayerChunkDrawTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				ImGui::EndTooltip();
			}
			tag = std::string("Shadow Cam World Draw Time (curr: ") + Utils::toString(timeAvgVars[ShadowWorldTime][timeAvgOffset], 4, 2, true) + " ms)";
			ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[ShadowWorldTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
			if (ImGui::IsHovered()) {
				ImGui::SetTooltip("");
				ImGui::BeginTooltip();
				ImGui::Text("Shadow Cam World Draw Time Breakdown");
				tag = std::string("Shadow Rebuild Time (curr: ") + Utils::toString(timeAvgVars[ShadowChunkRebuildTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[ShadowChunkRebuildTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				tag = std::string("Shadow BFS Time (curr: ") + Utils::toString(timeAvgVars[ShadowChunkBFSTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[ShadowChunkBFSTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				tag = std::string("Shadow Draw Time (curr: ") + Utils::toString(timeAvgVars[ShadowChunkDrawTime][timeAvgOffset], 4, 2, true) + " ms)";
				ImGui::PlotLines("25ms\n\n\n\n0 ms", timeAvgVars[ShadowChunkDrawTime], 100, timeAvgOffset, tag.c_str(), 0.00f, 25.0f, vec2f(350,60));
				ImGui::EndTooltip();
			}
		}
		ImGui::Separator();
		ImGui::Text("Player Position: [%f, %f, %f]", vec3fVars[PlayerPos].x, vec3fVars[PlayerPos].y, vec3fVars[PlayerPos].z);
		ImGui::Text("Player Chunk Coord: [%i, %i, %i]", int(std::floor(vec3fVars[PlayerPos].x)) >> CHUNKSIZE_POW2, int(std::floor(vec3fVars[PlayerPos].y)) >> CHUNKSIZE_POW2, int(std::floor(vec3fVars[PlayerPos].z)) >> CHUNKSIZE_POW2);
		ImGui::Text("Columns added last frame: %i", intVars[ColumnsAdded]);
		ImGui::Text("Player Chunks: %i", intVars[PlayerChunksDrawn]);
		ImGui::Text("Shadow Chunks: %i", intVars[SunChunksDrawn]);
		ImGui::End();

		ImGui::SetNewWindowDefaultPos(ImVec2(1500, 50));
		ImGui::Begin("Controls", nullptr, ImVec2(310, 230), -1.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGui::Text("Controls for this demo:");
		ImGui::Separator();
		ImGui::Columns(2, nullptr, true);
		ImGui::Text("Move"); ImGui::NextColumn();
		ImGui::Text("W,A,S,D"); ImGui::NextColumn();
		ImGui::Text("Look Around"); ImGui::NextColumn();
		ImGui::Text("Mouse"); ImGui::NextColumn();
		ImGui::Text("Jump"); ImGui::NextColumn();
		ImGui::Text("Space"); ImGui::NextColumn();
		ImGui::Text("Place a light"); ImGui::NextColumn();
		ImGui::Text("L"); ImGui::NextColumn();
		ImGui::Text("Toggle interface"); ImGui::NextColumn();
		ImGui::Text("F1"); ImGui::NextColumn();
		ImGui::Text("Toggle sun camera"); ImGui::NextColumn();
		ImGui::Text("Q"); ImGui::NextColumn();
		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::Text("While showing the interface,\nthe mouse will be visible.\nHover the profiler graphs for\nmore info");
		ImGui::End();
	}
	if(Environment::getKeyboard()->isKeyPressed(Keyboard::F1)) Environment::getMouse()->setRelativeMouseMode(!showProfiler);
	//ImGui::ShowTestWindow();
	//RESET VARS FOR NEXT FRAME
	memset(&timeVars, 0, sizeof(timeVars));
	memset(&intVars, 0, sizeof(intVars));
	drawTimeStart = Environment::getClock();
}

void Profiler::draw() const {
	float uiDrawTime = Environment::getClock();
	ImGui::Render();
	drawTimeEnd = Environment::getClock();
	swapTimeStart = drawTimeEnd;
	timeVars[UIDrawTime] = drawTimeEnd-uiDrawTime;
}


Profiler::Watcher::Watcher() {
	setUpdatePriority(-100);
	setDrawPriority(-100);
}

Profiler::Watcher::~Watcher() {
}

void Profiler::Watcher::update(float deltaTime) {
	(void) deltaTime;
	instance->swapTimeEnd = Environment::getClock();
	instance->updateTimeStart = instance->swapTimeEnd;
}

void Profiler::Watcher::draw() const {
}
